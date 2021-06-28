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
 
#include <gtest/gtest.h>

#include "sigio.h"


#define BUF_SIZE 212992

/* Maximum size of messages exchanged
between client to server */
#define SV_SOCK_PATH "/tmp/ud_ucase"
  
class SigioTest: public ::testing::Test {
   
  protected:
  void errExit(const char *format, ...);
  void start_unix_socket_server();

  public:
  static int sfd;
  static struct sockaddr_un svaddr, claddr;
  static unsigned char buf[BUF_SIZE];

  static void io_callback(siginfo_t* si); 
    SigioTest();
    void SetUp()    override;
    void TearDown() override;
    ~SigioTest()    override;
};

