#include <cstdio>
#include <cstring>

namespace Log
{
    template<typename... Args>
    void print(const char *prefix, const char *msg,const Args&... args)
    {
        printf("[ %s ]", prefix);
        printf(msg, args...);
        printf("\n");
    }

    template<typename... Args>
    void debug(const char *msg, const Args... args)
    {
#ifdef DEBUG
        print("DEBUG", msg, args...);
#endif
    }
    template<typename... Args>
    void info(const char *msg, const Args... args)
    {
        print("INFO", msg, args...);
    }
    template<typename... Args>
    void error(const char *msg, const Args... args)
    {
        print("ERROR", msg, args...);
    }
    template<typename... Args>
    void warn(const char *msg, const Args... args)
    {
        print("WARN", msg, args...);
    }
}