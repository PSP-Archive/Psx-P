/*  Pcsx - Pc Psx Emulator
 *  Copyright (C) 1999-2002  Pcsx Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <pspgu.h>
#include <pspgum.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h> 
#include <errno.h>
#include <string.h>
#include "psxcommon.h"
#include "zlib.h"
#include <png.h>
#include "Sio.h"
#include "log.h"
#include <setjmp.h>

#define _IN_DRAW

#include "externals.h"
#include "gpu.h"
#include "draw.h"
#include "prim.h"
#include "menu.h"
#include "interp.h"
#include "psp/vfpu_ops.h"

PSP_MODULE_INFO("PSXP Alpha 2 based on PCSX Emulator", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER|0x00004000);


int UseGui = 1;
long LoadCdBios;
char app_path[128];
void SysPrintf(char *fmt, ...) ;

/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf

void dump_threadstatus(void);

int done = 0;


/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	done = 1;
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread,
				     0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

int *ar;
char **av;

int ROM_LOAD_TYPE =0;//psx file =0, bin =1, bin.gz =2
char romname[256];
//int g_biosmode =1; // 0 = hle // 1 = psx bios
int g_runbios =0;
int NeedReset = 0;
int Running =0;

void RunCD(){ // run the cd, no bios
	LoadCdBios = 0;
	SysPrintf("RunCD\n");
	newCD(romname); 
//	OpenPlugins();
	SysReset();
	NeedReset = 0;
	CheckCdrom();
	if (LoadCdrom() == -1) {
		DLog("failed to load cd rom - RunCD");
		ClosePlugins();

		exit(0);//fail
	}
	Running = 1;
	DLog("executing cpu");
	psxCpu->Execute();
}

void RunCDBIOS(){ // run the bios on the cd?
	SysPrintf("RunCDBIOS\n");
	LoadCdBios = 1;
	newCD(romname); 
//	OpenPlugins();
	CheckCdrom();
	SysReset();
	NeedReset = 0;
	Running = 1;
	psxCpu->Execute();
}

void RunEXE(){
	SysPrintf("RunEXE\n");
//	OpenPlugins();
	SysReset();
	NeedReset = 0;
	Load(romname);
	Running = 1;
	psxCpu->Execute();
}

//Yoshihiro stuff ^_^ 

void RunBios(){
	SysPrintf("RunBios\n");
	SysReset(); 
	Running = 1;
	NeedReset = 0;
	psxCpu->Execute();
}


void SaveConfig(){
	
}

void LoadConfig(){
	
		InitConfig();
}

void InitConfig(){
	memset(&Config, 0, sizeof(PcsxConfig));

	Config.PsxType = 1; // ntsc
	Config.Xa      = 0; //disable xa decoding (audio)
	Config.Sio     = 0; //disable sio interrupt ?
	Config.Mdec    = 0; //movie decode
	Config.QKeys   = 0;//Button_GetCheck(GetDlgItem(hW,IDC_QKEYS));
	Config.Cdda    = 0; //diable cdda playback
	Config.PsxAuto = 1;// autodetect pal/ntsc
	Config.Cpu	   = 1; //interpreter
	Config.SpuIrq  = 0;
	Config.RCntFix = 0;//Parasite Eve 2, Vandal Hearts 1/2 Fix
	Config.VSyncWA = 0; // interlaced /non ? something with the display timer
	Config.PsxOut = 1;
	//Config.Bias = 10;
	Config.UseNet = 0;

	strcpy(Config.Net, "Disabled"); 
	strcpy(Config.Net, _("Disabled"));
	strcpy(Config.BiosDir,app_path);
	strcat(Config.BiosDir,"/BIOS/");

	sprintf(Config.Mcd1, "%s/MC/Mcd001.mcr",app_path);
	sprintf(Config.Mcd2, "%s/MC/Mcd002.mcr",app_path);
}

void Psxp_States_Load(int num) {
	char Text[256];
	int ret;

	SysReset();
	NeedReset = 0;

	sprintf (Text, "%s/SaveStates/%s.sav",app_path, CdromId);
	ret = LoadState(Text);
	sprintf (Text, "*PSXP*: %s State %d", !ret ? "Loaded" : "Error Loading", StatesC+1);
	GPU_displayText(Text);

	Running = 1;
	psxCpu->Execute();
}

void Psxp_States_Save(int num) {
	char Text[256];
	int ret;

	if (NeedReset) {
		SysReset();
		NeedReset = 0;
	}
	GPU_updateLace();

	sprintf (Text, "%s/SaveStates/%s.sav",app_path, CdromId);
	GPU_freeze(2, (GPUFreeze_t *)&num);
	ret = SaveState(Text);
	sprintf (Text, "*PSXP*: %s State %d", !ret ? "Saved" : "Error Saving", StatesC+1);
	GPU_displayText(Text);

	Running = 1;
	psxCpu->Execute();
}





jmp_buf env;
int main(int argc, char* argv[])
{
	char *dat;
	char *file = NULL;
	int runcd = 0;
	int loadst = 0;
	int i;

	getcwd(app_path,256);

	ar = &argc;
	av = &argv;

	pspDebugScreenInit();

	SetLogPath(app_path);

	SetupCallbacks();
	
	LoadConfig();

	SysInit();

	sceDisplaySetMode(PSP_DISPLAY_PIXEL_FORMAT_565, 480,272);
	//sceKernelDcacheWritebackInvalidateAll();

//	pgGeInit();


	scePowerSetClockFrequency(333, 333, 166);

	
	OpenPlugins();

	SysPrintf("Loading Gui\n");

	setjmp (env);

	DoMainGui();

	if (Config.HLE){
		strcpy(Config.Bios, "HLE"); // 
	}else{
		strcpy(Config.Bios, "scph1001.bin");  	
	}



	SysPrintf("After Gui\n");

	SysPrintf("Loading rom %s",romname);
	SysPrintf("rom type %d",ROM_LOAD_TYPE);
//	ROM_LOAD_TYPE = 0; //kludge
	switch(ROM_LOAD_TYPE){
		case 0://psx file
			RunEXE();
			break;
		case 1: //bin file or bin.gz
				RunCD();			
			
			break;	
		case 2:
			RunBios();
			break;	
	      case 3:
			RunCDBIOS();
			break;
	}
	return 0;
}



#if 1
int SysInit() {
    SysPrintf("start SysInit()\r\n");

    SysPrintf("psxInit()\r\n");
	Config.Cpu = 1;// interpreter
	psxInit();

    SysPrintf("LoadPlugins()\r\n");
	LoadPlugins();
    SysPrintf("LoadMcds()\r\n");
	LoadMcds(Config.Mcd1, Config.Mcd2);

	SysPrintf("end SysInit()\r\n");
	return 0;
}

void SysReset() {
	DLog("System reset");
    SysPrintf("start SysReset()\r\n");
	psxReset();
	SysPrintf("end SysReset()\r\n");
}

void SysClose() {
	psxShutdown();
	ReleasePlugins();
}

void SysPrintf(char *fmt, ...) {
	va_list list;
	char msg[512];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);
	strcat(msg,"\n");
	DLog(msg);
}

void *SysLoadLibrary(char *lib) {
	return 0xbaadf00d;
}

void *SysLoadSym(void *lib, char *sym) {
//	return dlsym(lib, sym);
	return lib; //smhzc
}

const char *SysLibError() {
//	return dlerror();
}

void SysCloseLibrary(void *lib) {
//	dlclose(lib);
}

void SysUpdate() {
//	PADhandleKey(PAD1_keypressed());
//	PADhandleKey(PAD2_keypressed());
}

void SysRunGui() {
//	RunGui();
}

void SysMessage(char *fmt, ...) {
	
}

void exit(int code){

}

#endif

