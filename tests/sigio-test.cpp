// Created by amironenko on 19/11/2020.
//
#include <chrono>
#include <ratio>
#include <memory>
#include <functional>

#include <gtest/gtest.h>

#include "timer.h"

using namespace std;
using namespace chrono;
using namespace posixcpp;

class TimerTest: public ::testing::Test {
  protected:

  public:
    int _tick = 0;

    TimerTest()
    {
      // initialization;
    }

    void SetUp( ) override
    {
      // initialization or some code to run before each test
    }

    void TearDown( ) override
    {
      // code to run after each test;
      // can be used instead of a destructor,
      // but exceptions can be handled in this function only
      _tick = 0;
    }

    void increment_tick(void* tick)
    {
      EXPECT_EQ((long)tick, (long)&_tick);
      (*((int*)tick))++;
    }

    ~TimerTest( )  override {
      // resources cleanup, no exceptions allowed
    }
};

TEST_F(TimerTest, GetTimeOut)
{
  EXPECT_TRUE(true);
  std::chrono::seconds period_sec = 5s;
  std::chrono::nanoseconds period_nsec = 0ns;

  std::unique_ptr<timer> tm (
      new timer(
        period_sec,
        period_nsec,
        std::bind(&TimerTest::increment_tick, this, std::placeholders::_1), // callback
       (void*) &_tick )                                                     // pointer to data
      );
  tm->start();

  int max_ticks = 5;
  for(int i = 0; i < max_ticks; i++)
  {
    sleep(period_sec.count());
    EXPECT_EQ(_tick, i+1);
    cout << "tick: " << _tick << endl;
  }

  tm->stop();
  sleep(period_sec.count());

  EXPECT_EQ(_tick, max_ticks);
}

TEST_F(TimerTest, SuspendResume)
{
  EXPECT_TRUE(true);
  std::chrono::seconds period_sec = 5s;
  std::chrono::nanoseconds period_nsec = 0ns;

  std::unique_ptr<timer> tm (
      new timer(
        period_sec,
        period_nsec,
        std::bind(&TimerTest::increment_tick, this, std::placeholders::_1), // callback
        (void*) &_tick,                                                     // pointer to data
        true
        )
      );

  tm->start();

  sleep(1);

  tm->suspend();

  sleep(1);

  tm->resume();

  sleep(4);

  EXPECT_EQ(_tick, 1);
}

TEST_F(TimerTest, StopStart)
{
  EXPECT_TRUE(true);
  std::chrono::seconds period_sec = 5s;
  std::chrono::nanoseconds period_nsec = 0ns;

  // create a new timer object with timeout of 5s
  std::unique_ptr<timer> tm (
      new timer(
        period_sec,
        period_nsec,
        std::bind(&TimerTest::increment_tick, this, std::placeholders::_1), // callback
       (void*) &_tick )                                                     // pointer to data
      );
  // start the timer
  tm->start();

  // wait for 2s
  sleep(period_sec.count()/2);

  // stop the time, it should reset the timer as well
  tm->stop();

  // make sure that the timer is not running
  // let's wait for another 4 seconds for the time out
  // and check the _tick counter is not incremented
  sleep(period_sec.count()/2 + period_sec.count()%2);
  EXPECT_EQ(_tick, 0);

  // lets start timer again
  tm->start();

  // wait until time-out
  sleep(period_sec.count() + 1);
  // and make sure that _tick is incremented now
  EXPECT_EQ(_tick, 1);
}

TEST_F(TimerTest, Reset)
{
  EXPECT_TRUE(true);
  std::chrono::seconds period_sec = 5s;
  std::chrono::nanoseconds period_nsec = 0ns;

  // create a new timer object with timeout of 5s
  std::unique_ptr<timer> tm (
      new timer(
        period_sec,
        period_nsec,
        std::bind(&TimerTest::increment_tick, this, std::placeholders::_1), // callback
       (void*) &_tick )                                                     // pointer to data
      );
  // start the timer
  tm->start();

  // wait for 2s
  sleep(period_sec.count()/2);

  // reset timer, means stop() and start()
  tm->reset();

  // wait until time-out
  sleep(period_sec.count() + 1);
  // and make sure that _tick is incremented now
  EXPECT_EQ(_tick, 1);
}


