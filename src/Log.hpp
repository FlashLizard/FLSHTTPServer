#ifndef FLS_LOG
#define FLS_LOG
#include <cstdio>
#include <cstring>

namespace Log
{
    const extern int Debug;
    const extern int Warn;
    const extern int Info;
    const extern int Error;
    const extern int None;

    extern int globalLevel;

    template<typename... Args>
    void print(int level,const char *prefix, const char *msg,const Args&... args)
    {
        if(level<globalLevel)
            return;
        printf("[ %s ]", prefix);
        printf(msg, args...);
        printf("\n");
    }

    template<typename... Args>
    void debug(const char *msg, const Args... args)
    {
#ifdef DEBUG
        print(Debug,"DEBUG", msg, args...);
#endif
    }
    template<typename... Args>
    void info(const char *msg, const Args... args)
    {
        print(Info,"INFO", msg, args...);
    }
    template<typename... Args>
    void error(const char *msg, const Args... args)
    {
        print(Error,"ERROR", msg, args...);
    }
    template<typename... Args>
    void warn(const char *msg, const Args... args)
    {
        print(Warn,"WARN", msg, args...);
    }
}

#endif