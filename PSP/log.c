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
#include <pspkernel.h>
#include "log.h"
//#include <direct.h>
#include <stdio.h>
#ifndef STDARG_H
#include <stdarg.h>
#endif

#include <string.h>
static FILE *logfile=0;

static char szLogfile[256]=""; // static so if dir change the log is still ok...
char *GetAppPath();
char LogPath[512];
void SetLogPath(char *path){
	strcpy(&szLogfile[0],path);
	strcpy(&szLogfile[0] + strlen(&szLogfile[0]),"/PSXP_OUT.TXT");
	strcpy(LogPath,path);
}

#define DLOGIT 1
#define MLOGIT 0
#define HWLOGIT 1

char outstr[256];
extern int Log;
int hwlog = 1; 
static int readbefore=0;
#define printf pspDebugScreenPrintf



void DLog (const char *fmt, ...)
{
#if (DLOGIT == 1)
/*	if(1){
		char buf [1024];
		char buf2[5];
		strcpy(buf2,"\n");
		va_list ap;
		int len;
		int log_fd;

		if(!readbefore){
			readbefore =1;
			log_fd = sceIoOpen(szLogfile, PSP_O_CREAT | PSP_O_WRONLY, 0777);
		}
		log_fd = sceIoOpen(szLogfile, PSP_O_CREAT | PSP_O_APPEND | PSP_O_WRONLY, 0644);

		if (log_fd >= 0) {
			va_start(ap, fmt);
			len = vsnprintf(buf, sizeof(buf), fmt, ap);
			va_end(ap);
			sceIoWrite(log_fd, buf, len);
			sceIoWrite(log_fd, buf2, strlen(buf2));
			sceIoClose(log_fd);
		}
	}*/
#endif
}



/*
void DLog(char const *fmt,...)
	{
#if(DLOGIT==1)
	

	if(Log){

		//chdir(LogPath);
		if (! readbefore)
			{
					//sprintf(szLogfile,"game.log");
				logfile=fopen(szLogfile,"at");
			if (logfile){
				fprintf(logfile,"******************\n : Log System initted\n");
			fclose(logfile);
				}
			readbefore=1;
			}
		logfile=fopen(szLogfile,"at");
		//logfile=fopen(szLogfile,"w");
		if(logfile){
			fprintf(logfile,"%9d : ",1);
			va_list arglist;
			va_start(arglist,fmt);
			vfprintf(logfile,fmt,arglist);
			fprintf(logfile,"\n");
			va_end(arglist);
			fclose(logfile);
		}
	} 
#endif 
}
*/
/*
void DLog(char const *fmt,...)
{
	char tmp[1024];
#if(DLOGIT==1)
	va_list arglist;
	va_start(arglist,fmt);
	vsprintf(tmp,fmt,arglist);
	va_end(arglist);
	strcat(tmp,"\n");
	printf("%s",tmp);
#endif 
}
*/
void MLog(char const *fmt,...)
	{
#if(MLOGIT==1)
	

	if(Log){

		//chdir(LogPath);
		if (! readbefore)
			{
					//sprintf(szLogfile,"game.log");
					logfile=fopen(szLogfile,"wt");
			if (logfile){
				fprintf(logfile,"******************\n : Log System initted\n");
				fclose(logfile);
				}
			readbefore=1;
			}
		logfile=fopen(szLogfile,"at");
		if(logfile){
			fprintf(logfile,"%9d : ",1);
			va_list arglist;
			va_start(arglist,fmt);
			vfprintf(logfile,fmt,arglist);
			fprintf(logfile,"\n");
			va_end(arglist);
			fclose(logfile);
		}
	}
#endif
}

void HWLog(char const *fmt,...)
	{
#if (HWLOGIT == 1)
	if(Log){
		char buf [1024];
		char buf2[5];
		strcpy(buf2,"\n");
		va_list ap;
		int len;
		int log_fd;

		if(!readbefore){
			readbefore =1;
			log_fd = sceIoOpen(szLogfile, PSP_O_CREAT | PSP_O_WRONLY, 0777);
		}
		log_fd = sceIoOpen(szLogfile, PSP_O_CREAT | PSP_O_APPEND | PSP_O_WRONLY, 0644);

		if (log_fd >= 0) {
			va_start(ap, fmt);
			len = vsnprintf(buf, sizeof(buf), fmt, ap);
			va_end(ap);
			sceIoWrite(log_fd, buf, len);
			sceIoWrite(log_fd, buf2, strlen(buf2));
			sceIoClose(log_fd);
		}
	}
#endif
}

