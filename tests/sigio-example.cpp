#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include <chrono>
#include <ratio>
#include <memory>
#include <functional>

#include <sys/un.h>
#include <sys/socket.h> 
#include <ctype.h> 
#include <stdarg.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> 
#include <fcntl.h>
#include <signal.h>
 

#include "sigio.h"


#define BUF_SIZE 212992

/* Maximum size of messages exchanged
between client to server */
#define SV_SOCK_PATH "/tmp/ud_ucase"
  

using namespace one;

static pid_t child = -1;
static bool done = false;
static size_t write_bytes = 0;
static ssize_t read_bytes = 0;
static char* cursor = NULL;
static const size_t data_size = 16*1024;
static char* data = NULL;
static ssize_t sent_bytes = 0;
static struct sockaddr_un svaddr, claddr;
static size_t count = 0;
static bool is_write_in_progress = false;
static uint32_t sleep_time_usec = 100;
static const size_t count_max = 100;


/* Set nonzero on receipt of SIGIO */         
static void sigint_handler(int sig) 
{
  printf("Signal SIGKILL has received!\n");
  done = true; 
} 

void errExit(const char *format, ...)
{
  va_list argList;
  va_start(argList, format);
  printf(format, argList);    
  va_end(argList);
  exit(EXIT_FAILURE);
}

void io_callback(siginfo_t* si)
{
  printf("+++ io_callback +++ \n");
  char buf[BUF_SIZE];

  printf("signal = %d\n", si->si_signo);
  printf("fd = %d\n", si->si_fd);
  count ++;
  printf("count = %ld\n", count);
  switch(si->si_code)
  {
    case POLL_IN:
      printf("POLL_IN\n");
      
      int remaining_bytes;
      //ioctl(si->si_fd, FIONREAD, &remaining_bytes);

      for(ioctl(si->si_fd, FIONREAD, &remaining_bytes); remaining_bytes;)
      {
        printf("remaining_bytes = %d\n", remaining_bytes);
        read_bytes = read(si->si_fd, buf, BUF_SIZE);

        if (read_bytes == -1)
        {
          if (errno == EAGAIN || errno == EWOULDBLOCK)
          {
            printf("EAGAIN, break \n");
            break;
          }
          else 
          {
            errExit("sendto unexpected errno=%d, errmsg = %s\n", errno, strerror(errno));
          }
        }
        else if (read_bytes >= 0) 
        {
          printf("%ld bytes has been read\n", read_bytes);
          remaining_bytes -= read_bytes;
        }
        else 
        {
          errExit("read unexpected returned value %ld", read_bytes);
        }
      }
      break;

    case POLL_OUT:
      printf("POLL_OUT\n");

      if (write_bytes > 0 && cursor != NULL) 
      {
        while(true) 
        { 
          sent_bytes = sendto(si->si_fd, cursor, write_bytes, 0, (struct sockaddr *) &svaddr,
              sizeof(struct sockaddr_un));

          if (sent_bytes == -1)
          {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
              printf("%ld bytes has been sent\n", sent_bytes);
              cursor += sent_bytes;
              write_bytes -= sent_bytes;
            }
            else 
            {
              errExit("sendto unexpected errno=%d, errmsg = %s\n", errno, strerror(errno));

            }
          }
          else if (sent_bytes == write_bytes)
          {
            printf("all bytes have been sent, no need to continue\n");
            cursor = NULL;
            write_bytes = 0;
          }
          else 
          {
            errExit("sendto unexpected value %d\n", sent_bytes);
          }
        }
      }
      break;

    case POLL_ERR:
      printf("POLL_ERR\n");
      break;

    case POLL_PRI:
      printf("POLL_PRI\n");
      break;

    case POLL_HUP:
      printf("POLL_HUP\n");
      break;
  };

  printf("--- io_callback --- \n");
}

void start_unix_socket_server(bool is_echo_on = true, uint32_t sleep_usec = 10000);
void start_unix_socket_server(bool is_echo_on, uint32_t sleep_usec)
{
  struct sockaddr_un svaddr, claddr;
  int sfd, j;                                       
  ssize_t numBytes;
  socklen_t len;                                   
  char buf[data_size];
  sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (sfd == -1)
    errExit("socket");

  /* Create server socket */
  /* Construct well-known address and bind server socket to it */                     
  if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT)
    errExit("remove-%s", SV_SOCK_PATH);
  memset(&svaddr, 0, sizeof(struct sockaddr_un));
  svaddr.sun_family = AF_UNIX;
  strncpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) - 1);
  if (bind(sfd, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_un)) == -1)
    errExit("bind");
  /* Receive messages, convert to uppercase, and return to client */

  len = sizeof(struct sockaddr_un);
  while (!done) 
  {
    usleep(sleep_usec);
    if (is_echo_on)
    { 
      numBytes = recvfrom(sfd, buf, data_size, 0,
          (struct sockaddr *) &claddr, &len);
      if (numBytes == -1)
        errExit("recvfrom");
      printf("Server received %ld bytes from %s\n", (long) numBytes,
          claddr.sun_path);
    }
    else
    {
      numBytes = data_size;
    }

    if (sendto(sfd, buf, numBytes, 0, (struct sockaddr *) &claddr, len) != numBytes)
    {
      errExit("server: sendto has returned -1 \n");
      exit(-1);
    }

    printf("%ld bytes have sent back\n", numBytes);
  }

  close(sfd);
}

void test_01(uint32_t sleep_time)
{
  socklen_t len;                                   
  len = sizeof(struct sockaddr_un);
  ssize_t numBytes = 0;

  // initialization
  if ((child = fork()) == -1)
  { 
    perror("fork");
  }
  else if (child) 
  {
    printf("starting parent process\n");
    int sfd;

    // initialization or some code to run before each test
    /* Create client socket; bind to unique pathname (based on PID) */                    
    sfd = socket(AF_UNIX, SOCK_DGRAM, 0); 

    memset(&claddr, 0, sizeof(struct sockaddr_un)); 
    claddr.sun_family = AF_UNIX; 
    snprintf(claddr.sun_path, sizeof(claddr.sun_path), 
        "/tmp/ud_ucase_cl.%ld", (long) getpid()); 
    if (bind(sfd, (struct sockaddr *) &claddr, sizeof(struct sockaddr_un)) == -1) 
      errExit("bind"); 

    /* Construct address of server */
    memset(&svaddr, 0, sizeof(struct sockaddr_un));
    svaddr.sun_family = AF_UNIX;
    strncpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) - 1);

    auto& sio = sigio::get();
    sio.activate(sfd, io_callback);

    write_bytes = data_size;
    cursor = data;

    while(!done && count < count_max)
    {
      usleep(sleep_time);
      //printf("write_bytes = %ld\n", write_bytes);
      //printf("is_write_in_progress = %d\n", is_write_in_progress);
      //if (write_bytes != 0 and !is_write_in_progress)
      write_bytes = data_size;
      cursor = data;
  
      if (true)
      {
        sent_bytes = sendto(sfd, cursor, write_bytes, 0, (struct sockaddr *) &svaddr,
            sizeof(struct sockaddr_un));
        printf("sent_bytes = %ld\n", sent_bytes);

        if (sent_bytes == -1)
        {
          if (errno == EAGAIN || errno == EWOULDBLOCK)
          {
            printf("EAGAIN\n");
            is_write_in_progress = true;
          }
          else 
          {
            is_write_in_progress = false;
            //kill the child process
            kill(child, SIGKILL);
            wait(NULL);
            close(sfd);

            errExit("sendto unexpected errno=%d, errmsg = %s\n", errno, strerror(errno));
          }
        }
        else if (sent_bytes > 0) 
        {
          printf("all bytes have been sent, no need to continue\n");
          cursor += sent_bytes;
          write_bytes -= sent_bytes;
          if(write_bytes == 0)
            is_write_in_progress = false;
        }
        else 
        {
          //kill the child process
          kill(child, SIGKILL);
          wait(NULL);
          close(sfd);

          errExit("sendto unexpected value %d", sent_bytes);
        }
      }

      if (false)
      { 
        numBytes = recvfrom(sfd, cursor, data_size, 0,
            (struct sockaddr *) &claddr, &len);
        if (numBytes == -1)
        {
          if (errno == EAGAIN || errno == EWOULDBLOCK)
          {
            printf("EAGAIN\n");
          }
          else 
          {
            is_write_in_progress = false;
            //kill the child process
            kill(child, SIGKILL);
            wait(NULL);
            close(sfd);

            errExit("sendto unexpected errno=%d, errmsg = %s\n", errno, strerror(errno));
            exit(-1);
          }
        }

        printf("client received %ld bytes from %s\n", (long) numBytes, claddr.sun_path);
      }

    }
    //kill the child process
    kill(child, SIGKILL);
    wait(NULL);
    close(sfd);
  } 
  else 
  {
    // This is the child, unix socket server,
    // once it receives datagram, it changes all characters to capital
    // and sends a new string back to the client
    printf("starting child process with the unix socket server\n");
    start_unix_socket_server(true, sleep_time);
  }
}


int main()
{
  if (signal(SIGINT, sigint_handler) == SIG_ERR)     
  {
    exit(EXIT_FAILURE);
  }

  data = (char*) malloc(data_size);
  if(!data)
  {
    errExit("data memory allocation has failed\n");
  }

  test_01(100); // 1ms

  return 0;
}
