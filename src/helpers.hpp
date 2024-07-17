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
#define u8 uint8_t
#endif

#ifndef u16
#define u16 uint16_t
#endif

#ifndef u32
#define u32 uint32_t
#endif

#ifndef u64
#define u64 uint64_t
#endif

#ifndef i8
#define i8 int8_t
#endif

#ifndef i16
#define i16 int16_t
#endif

#ifndef i32
#define i32 int32_t
#endif

#ifndef i64
#define i64 int64_t
#endif

#ifndef f32
#define f32 float
#endif

#ifndef f64
#define f64 double
#endif

