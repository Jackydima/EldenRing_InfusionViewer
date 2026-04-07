#pragma once
#include <iostream>


namespace logger
{
#ifndef DEBUG
    template <typename... Args>
    inline void print(const char*, Args...)
    {
    }

    template <typename... Args>
    inline void println(const char*, Args...)
    {
    }
#else
    template <typename... Args>
    inline void print(const char* format, Args... args)
    {
        std::fprintf(stdout, format, args...);
    }
    template <typename... Args>
    inline void println(const char* format, Args... args)
    {
        std::fprintf(stdout, format, args...);
        std::fprintf(stdout, "\n");
    }
#endif
}