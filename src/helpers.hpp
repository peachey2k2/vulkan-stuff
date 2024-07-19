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

#ifndef size_t
typedef std::size_t size_t;
#endif

typedef glm::mat2 mat2;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

//colors
const vec3 COLOR_RED = vec3(1.0, 0.0, 0.0);
const vec3 COLOR_GREEN = vec3(0.0, 1.0, 0.0);
const vec3 COLOR_BLUE = vec3(0.0, 0.0, 1.0);
const vec3 COLOR_YELLOW = vec3(1.0, 1.0, 0.0);
const vec3 COLOR_MAGENTA = vec3(1.0, 0.0, 1.0);
const vec3 COLOR_CYAN = vec3(0.0, 1.0, 1.0);
const vec3 COLOR_WHITE = vec3(1.0, 1.0, 1.0);
const vec3 COLOR_BLACK = vec3(0.0, 0.0, 0.0);
