#pragma once

#include <iostream>
#include <stdexcept>
#include <cstdlib>

// look, i like c++ but at times it's a bit too verbose.
// so here's what we will be doing:
#define scast static_cast
#define dcast dynamic_cast
#define ccast const_cast
#define rcast reinterpret_cast

// mmmmm gotta save those keystrokes
#ifndef u8
typedef uint8_t u8;
#endif

#ifndef u16
typedef uint16_t u16;
#endif

#ifndef u32
typedef uint32_t u32;
#endif

#ifndef u64
typedef uint64_t u64;
#endif

#ifndef i8
typedef int8_t i8;
#endif

#ifndef i16
typedef int16_t i16;
#endif

#ifndef i32
typedef int32_t i32;
#endif

#ifndef i64
typedef int64_t i64;
#endif

#ifndef f32
typedef float f32;
#endif

#ifndef f64
typedef double f64;
#endif

