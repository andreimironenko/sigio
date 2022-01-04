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
 

#include <sigio.h>


#define BUF_SIZE 212992

/* Maximum size of messages exchanged
between client to server */
#define SV_SOCK_PATH "/tmp/ud_ucase"

using namespace one;

void errExit(const char *format, ...)
{
  va_list argList;
  va_start(argList, format);
  printf(format, argList);    
  va_end(argList);
  exit(EXIT_FAILURE);
}

void loadavg_handler(struct inotify_event* i)
{

    printf("    wd =%2d; ", i->wd);
    EAGAIN;
       if (i->cookie > 0)
           printf("cookie =%4d; ", i->cookie);

       printf("mask = ");
       if (i->mask & IN_ACCESS)        printf("IN_ACCESS ");
       if (i->mask & IN_ATTRIB)        printf("IN_ATTRIB ");
       if (i->mask & IN_CLOSE_NOWRITE) printf("IN_CLOSE_NOWRITE ");
       if (i->mask & IN_CLOSE_WRITE)   printf("IN_CLOSE_WRITE ");
       if (i->mask & IN_CREATE)        printf("IN_CREATE ");
       if (i->mask & IN_DELETE)        printf("IN_DELETE ");
       if (i->mask & IN_DELETE_SELF)   printf("IN_DELETE_SELF ");
       if (i->mask & IN_IGNORED)      printf("IN_IGNORED ");
       if (i->mask & IN_ISDIR)      printf("IN_ISDIR ");
       if (i->mask & IN_MODIFY)        printf("IN_MODIFY ");
       if (i->mask & IN_MOVE_SELF)  printf("IN_MOVE_SELF ");
       if (i->mask & IN_MOVED_FROM)    printf("IN_MOVED_FROM ");
       if (i->mask & IN_MOVED_TO)    printf("IN_MOVED_TO ");
       if (i->mask & IN_OPEN)        printf("IN_OPEN ");
       if (i->mask & IN_Q_OVERFLOW)    printf("IN_Q_OVERFLOW ");
       if (i->mask & IN_UNMOUNT)      printf("IN_UNMOUNT ");
       printf("\n");

       if (i->len > 0)
           printf("        name = %s\n", i->name);
}

int main()
{
    auto& sio = sigio::get();
    std::string loadavg = "/proc/interrupts";
    sio.activate(loadavg, IN_ALL_EVENTS, loadavg_handler);
    sio.is_activated(loadavg) ? printf("%s is activated \n", loadavg.c_str()): printf("%s is not activated \n", loadavg.c_str());
    while(true)
    {
        sleep(5);
        printf("looping ...\n");
    }

    return 0;
}
