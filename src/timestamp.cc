#include <ctime>
#include <sys/time.h>
#include <unistd.h>

#include "timestamp.hh"
#include "util.hh"

/* nanoseconds per millisecond */
static const uint64_t MILLION = 1000000;

/* nanoseconds per second */
static const uint64_t BILLION = 1000 * MILLION;

/* helper functions */
static timespec current_time( void )
{
#if _POSIX_TIMERS > 0
  timespec ret;
  SystemCall( "clock_gettime", clock_gettime( CLOCK_REALTIME, &ret ) );
  return ret;
#else
  timeval t;
  SystemCall( "gettimeofday", gettimeofday( &t, NULL ) );
  return timestamp_conv( t );
#endif
}

static uint64_t timestamp_ms_raw( const timespec & ts )
{
  const uint64_t nanos = ts.tv_sec * BILLION + ts.tv_nsec;
  return nanos / MILLION;
}

/* Convert a timeval to a timespec */
timespec const timestamp_conv ( const timeval & tv )
{
  timespec ts;
  ts.tv_sec = tv.tv_sec;
  ts.tv_nsec = tv.tv_usec * 1000;
  return ts;
}

/* Current time in milliseconds since the start of the program */
uint64_t timestamp_ms( void )
{
  return timestamp_ms( current_time() );
}

uint64_t timestamp_ms( const timespec & ts )
{
  const static uint64_t EPOCH = timestamp_ms_raw( current_time() );
  return timestamp_ms_raw( ts ) - EPOCH;
}
