#pragma once

#include <iostream>
#include <stdexcept>

#ifdef NDEBUG

#define ASSERT(condition, message) ((void)0)
#define ASSERT_FATAL(condition, message) ((void)0)

#else

// not planning on using this one
#define ASSERT(condition, message) if (!(condition)) throw std::runtime_error(message)

// call this for fatal errors
#define ASSERT_FATAL(condition, message) if (!(condition)) throw wmac::engine_fatal_exception(message)

#endif

namespace wmac {
    class engine_exception : public std::runtime_error {
    const char* const prefix = "\e[36m[ERROR] ";
    public:
        engine_exception(const std::string& p_message) : std::runtime_error(prefix + p_message) {}
    };

    class engine_fatal_exception : public engine_exception {
    const char* const prefix = "\e[36m[FATAL ERROR] ";
    public:
        engine_fatal_exception(const std::string& p_message) : engine_exception(p_message) {}
    };
}

