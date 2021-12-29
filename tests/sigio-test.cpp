// Created by amironenko on 19/11/2020.
//
#include "sigio-test.h"

using namespace std;
using namespace chrono;
using namespace one;

int SigioTest::sfd;
struct sockaddr_un SigioTest::svaddr;
struct sockaddr_un SigioTest::claddr;
one::sigio& SigioTest::sio(sigio::get());
unsigned char SigioTest::buf[BUF_SIZE];

void SigioTest::io_callback(siginfo_t* si)
{
  printf("+++ io_callback +++");
  ssize_t numRead = 0;

  printf("signal = %d\n", si->si_signo);
  printf("fd = %d\n", si->si_fd);
  switch(si->si_code)
  {
    case POLL_IN:
      printf("POLL_IN\n");
      while((numRead = read(si->si_fd, buf, BUF_SIZE)) > 0)
      {
        for (size_t i = 0; i < (size_t)numRead; i++)
        {
          printf("%02x ", buf[i]);
        }
        printf("\n");
      }
      break;
   
    case POLL_OUT:
      printf("POLL_OUT\n");
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

  printf("--- io_callback ---");
}

void SigioTest::errExit(const char *format, ...)
{
  va_list argList;
  va_start(argList, format);
  printf(format, argList);    
  va_end(argList);
  exit(EXIT_FAILURE);
}

void SigioTest::start_unix_socket_server()
{
  struct sockaddr_un svaddr, claddr;
  int sfda, j;
  ssize_t numBytes;
  socklen_t len;
  char buf[BUF_SIZE];
  sfda = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (sfda == -1)
    errExit("socket");

  /* Create server socket */
  /* Construct well-known address and bind server socket to it */
  if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT)
    errExit("remove-%s", SV_SOCK_PATH);
  memset(&svaddr, 0, sizeof(struct sockaddr_un));
  svaddr.sun_family = AF_UNIX;
  strncpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) - 1);
  if (bind(sfda, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_un)) == -1)
    errExit("bind");
  /* Receive messages, convert to uppercase, and return to client */
  for (;;) {
    len = sizeof(struct sockaddr_un);
    numBytes = recvfrom(sfda, buf, BUF_SIZE, 0,
        (struct sockaddr *) &claddr, &len);
    if (numBytes == -1)
      errExit("recvfrom");
    printf("Server received %ld bytes from %s\n", (long) numBytes,
        claddr.sun_path);
    for (j = 0; j < numBytes; j++)
      buf[j] = toupper((unsigned char) buf[j]);
    if (sendto(sfda, buf, numBytes, 0, (struct sockaddr *) &claddr, len) !=
        numBytes)
      errExit("sendto");
  }
}

SigioTest::SigioTest()
{
  int child;

  // initialization
  if ((child = fork()) == -1)
  { 
    perror("fork");
  }
  else if (child) 
  {
    printf("starting parent process\n");
  } 
  else {
    // This is the child, unix socket server,
    // once it receives datagram, it changes all characters to capital
    // and sends a new string back to the client
    printf("starting child process with the unix socket server\n");
    start_unix_socket_server();
  }
}

void SigioTest::SetUp()
{
  // initialization or some code to run before each test
  /* Create client socket; bind to unique pathname (based on PID) */
  sfd = socket(AF_UNIX, SOCK_DGRAM, 0); 
  EXPECT_FALSE(sfd == -1);

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

  sio.activate(sfd, &SigioTest::io_callback);
}

void SigioTest::TearDown()
{
  // code to run after each test;
  // can be used instead of a destructor,
  // but exceptions can be handled in this function only
}

SigioTest::~SigioTest()
{
  // resources cleanup, no exceptions allowed
  sio.deactivate(sfd);
  close(sfd);
}

TEST_F(SigioTest, SendReceive)
{
  EXPECT_TRUE(true);
  std::chrono::seconds period_sec = 5s;
  std::chrono::nanoseconds period_nsec = 0ns;
  const char* hello = "hello world!";

  for(int i = 0; i < 2; i++)
  {
    if (sendto(sfd, hello, sizeof(hello), 0, (struct sockaddr *) &svaddr,
          sizeof(struct sockaddr_un)) != sizeof(hello))
    {
      errExit("sendto");
    }
    sleep(1);
  }
}

TEST_F(SigioTest, StopStart)
{
  EXPECT_TRUE(true);
  std::chrono::seconds period_sec = 5s;
  std::chrono::nanoseconds period_nsec = 0ns;
}

TEST_F(SigioTest, Reset)
{
  EXPECT_TRUE(true);
  std::chrono::seconds period_sec = 5s;
  std::chrono::nanoseconds period_nsec = 0ns;
}
