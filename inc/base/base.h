/********************************************************
 * Description : base
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2024
 ********************************************************/

#ifndef BASE_H
#define BASE_H


#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>

#ifdef _MSC_VER
    #define GOOFER_CDECL           __cdecl
    #define GOOFER_STDCALL         __stdcall
    #ifdef EXPORT_GOOFER_DLL
        #define GOOFER_API         __declspec(dllexport)
    #else
        #ifdef USE_GOOFER_DLL
            #define GOOFER_API     __declspec(dllimport)
        #else
            #define GOOFER_API
        #endif // USE_GOOFER_DLL
    #endif // EXPORT_GOOFER_DLL
#else
    #define GOOFER_CDECL
    #define GOOFER_STDCALL
    #define GOOFER_API
#endif // _MSC_VER

#ifdef __cplusplus
    #define GOOFER_C_API(return_type)            extern "C" GOOFER_API return_type GOOFER_CDECL
    #define GOOFER_EXTERN_TYPE(variable_type)    extern "C" GOOFER_API variable_type
#else
    #define GOOFER_C_API(return_type)            extern     GOOFER_API return_type GOOFER_CDECL
    #define GOOFER_EXTERN_TYPE(variable_type)    extern     GOOFER_API variable_type
#endif // __cplusplus

#define GOOFER_CXX_API(return_type)              extern     GOOFER_API return_type GOOFER_CDECL

#ifndef __FILENAME__
    #ifdef _MSC_VER
        #define __FILENAME__                     (strrchr("\\" __FILE__, '\\') + 1)
    #else
        #define __FILENAME__                     (strrchr("/" __FILE__, '/') + 1)
    #endif // _MSC_VER
#endif // __FILENAME__

#define RUN_LOG_ERR(fmt, ...) run_log(1, __FILENAME__, __FUNCTION__, __LINE__, "[ERR] " fmt, ##__VA_ARGS__)
#define RUN_LOG_WAR(fmt, ...) run_log(2, __FILENAME__, __FUNCTION__, __LINE__, "[WAR] " fmt, ##__VA_ARGS__)
#define RUN_LOG_DBG(fmt, ...) run_log(3, __FILENAME__, __FUNCTION__, __LINE__, "[DBG] " fmt, ##__VA_ARGS__)
#define RUN_LOG_TRK(fmt, ...) run_log(4, __FILENAME__, __FUNCTION__, __LINE__, "[TRK] " fmt, ##__VA_ARGS__)

GOOFER_CXX_API(void) set_log_max_level(int level);
GOOFER_CXX_API(void) set_log_simplify(bool simplify);
GOOFER_CXX_API(void) run_log(int level, const char * file, const char * func, int line, const char * format, ...);

GOOFER_CXX_API(void) sleep_ms(uint32_t ms);


#endif // BASE_H
