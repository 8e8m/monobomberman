/* util.h & time.h */

#ifndef UTIL_H_
#define UTIL_H_

/* these things that are highly portable among programs and thus written in a modular style */

#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#define always_inline static inline __attribute__((always_inline))

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define CLAMP(a,b,c) (a)<(b)?(b):(a)>(c)?(c):(a)

always_inline void Root(char * filename) {
  char path[PATH_MAX], * terminator;
  if (!realpath(filename, path)) { return; }
  if ((terminator = strrchr(path, '/'))) {
    *terminator = '\0';
    if(chdir(path)) { abort(); }
  }
}

always_inline float Angle(float a_x, float a_y, float b_x, float b_y) {
  return atan2f(b_y - a_y, b_x - a_x);
}

always_inline float Distance(float a_x, float a_y, float b_x, float b_y) {
  return sqrtf(powf(b_x - a_x, 2) + powf(b_y - a_y, 2));
}

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#endif

/* time.h */
#ifndef TIME_H_
#define TIME_H_
#include <time.h>

#define TIMESPEC_TO_F64(ts) ((double)(ts).tv_sec + ((double)(ts).tv_nsec / TIMESPEC_HZ))
enum { TIMESPEC_HZ = 1000000000 };

typedef struct timespec timespec_t;

static const timespec_t one_second = {1, 0}, zero_seconds = {0, 0};

always_inline timespec_t timespec_add(timespec_t a, timespec_t b) {
    a.tv_sec += b.tv_sec;
    a.tv_nsec += b.tv_nsec;
    if (a.tv_nsec >= 1000000000) {
        a.tv_sec++;
        a.tv_nsec -= 1000000000;
    }
    return a;
}

always_inline timespec_t timespec_sub(timespec_t a, timespec_t b) {
    a.tv_sec -= b.tv_sec;
    a.tv_nsec -= b.tv_nsec;
    if (a.tv_nsec < 0) {
        a.tv_sec--;
        a.tv_nsec += 1000000000;
    }
    return a;
}

always_inline int timespec_cmp(timespec_t a, timespec_t b) {
  return a.tv_sec > b.tv_sec ?
    (1) :
    a.tv_sec < b.tv_sec ?
    (-1) :
    (
      a.tv_nsec > b.tv_nsec ?
      (1) :
      a.tv_nsec < b.tv_nsec ?
      (-1) : 0
    );
}

always_inline timespec_t timespec_max(timespec_t a, timespec_t b) {
  return a.tv_sec > b.tv_sec ?
    a :
    a.tv_sec < b.tv_sec ?
    b :
    (
      a.tv_nsec > b.tv_nsec ?
      a : b
    );
}

always_inline timespec_t timespec_min(timespec_t a, timespec_t b) {
  return a.tv_sec < b.tv_sec ?
    a :
    a.tv_sec > b.tv_sec ?
    b :
    (
      a.tv_nsec < b.tv_nsec ?
      a : b
    );
}
#endif /* TIME_H_ */
