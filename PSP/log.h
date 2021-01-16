/*----------------------------------------------------------
*                                      
*    __  __                      
*   | | / /__  __ _____    ___    __   _____  ___
*   |  / / \ \/ //     \  /   \ /`__'\|  __| /   \
*   |  \ \  \  // /| |\ \|  O  |  ___/| |  `|  O  | 
*   |_| \_\  |_|_/ |_| \_\\_/|_|\___\ |_|    \_/|_|
*                  
*
*   Kymaera (c) 2004 
*	Universal Arcade Cabinet Front End
*
*	Steve Hernandez: steveh@crosslink.net 
*   Windows, Linux and Dos version
*   See the Readme.txt and License.txt for more information          
*
*
* -----------------------------------------------------------
*/

#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif// __cplusplus
//void Log(char const *fmt, ...);
void DLog(char const *fmt, ...);
//#define DLog pspDebugScreenPrintf
void ILog(char const *fmt, ...);
void SetLogPath(char *path);

#ifdef __cplusplus
};
#endif// __cplusplus
/*
#ifndef NDEBUG
#define DLog Log
#else
static inline void DLog(char const * , ...) {};
#endif
*/

//#define DLog(x) Log(x)

#endif

