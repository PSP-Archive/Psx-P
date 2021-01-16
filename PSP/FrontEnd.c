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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "psxcommon.h"
#include "Sio.h"
#include "log.h"
#include "png.h"
#include  "pngloader.h"
#include <pspdebug.h>
#include <math.h>
#include <string.h>
#include <pspctrl.h>
#include <pspgu.h>
#include <psprtc.h>
//#include "pg.h"



extern char app_path[128];
extern long g_runbios;

typedef struct configparm{
	char name[32];
	long *var;
}configP;

configP configparms[30];
int totalconfig=0;
void InitConfigParms(){
	int c=0;
	strcpy(configparms[c].name,"XA Audio");
	configparms[c].var = &Config.Xa;
	c++;
	strcpy(configparms[c].name,"SIO");
	configparms[c].var = &Config.Sio;
	c++;
	strcpy(configparms[c].name,"MDec");
	configparms[c].var = &Config.Mdec;
	c++;
	strcpy(configparms[c].name,"Auto detect PAL or NTSC");
	configparms[c].var = &Config.PsxAuto;
	c++;
	strcpy(configparms[c].name,"Off = NTSC , On = PAL");
	configparms[c].var = &Config.PsxType;
	c++;
	strcpy(configparms[c].name,"Cdda");
	configparms[c].var = &Config.Cdda;
	c++;
	strcpy(configparms[c].name,"Use Internal HLE Bios");
	configparms[c].var = &Config.HLE;
	c++;
	strcpy(configparms[c].name,"Use Interpreted Cpu core");
	configparms[c].var = &Config.Cpu;
	c++;
	strcpy(configparms[c].name,"PsxOut");
	configparms[c].var = &Config.PsxOut;
	c++;
	strcpy(configparms[c].name,"SpuIrq");
	configparms[c].var = &Config.SpuIrq;
	c++;
	strcpy(configparms[c].name,"Kingdom Hearts - Parasite Eve 2 Fix");
	configparms[c].var = &Config.RCntFix;
	c++;
	strcpy(configparms[c].name,"Use Net");
	configparms[c].var = &Config.UseNet;
	c++;
	strcpy(configparms[c].name,"VSyncWA");
	configparms[c].var = &Config.VSyncWA;
	c++;
	strcpy(configparms[c].name,"Run CD through psx Bios [HLE = 0 only]");
	configparms[c].var = &g_runbios;
//	c++;
	//
	totalconfig =c;
}

int selposconfig=0;
void DisplayConfigParms(){
	int c;
	// 1 = green	// 0 = red
	// selected, green = white	// selected, red = grey
	for (c=0;c<totalconfig;c++){
		if(selposconfig == c && *configparms[c].var==1){
			pspDebugScreenSetTextColor(0xff00ff00); // green
		}else if (selposconfig == c && *configparms[c].var==0){
			pspDebugScreenSetTextColor(0xff0000ff); // red
		}else if (*configparms[c].var==0){
			pspDebugScreenSetTextColor(0x99999999); // grey
		}else if (*configparms[c].var==1){
			pspDebugScreenSetTextColor(0xffffffff); // white
		}
		pspDebugScreenPrintf("%s\n",configparms[c].name);
		
	}

}
extern void SaveConfig();

void DoConfig(){
	
	int done =0;
	SceCtrlData pad,oldPad;

	InitConfigParms();

	pspDebugScreenSetXY(0,0);

	int cnt;
	long tm;
	for (cnt=0;cnt<100;cnt++)//pspDebugScreenPrintf("\n");
	while(!done){
		//cnt =0;
		sceDisplayWaitVblankStart();
		pspDebugScreenSetTextColor(0xffffffff);
		pspDebugScreenSetXY(0, 3);

		DisplayConfigParms();
		if(sceCtrlPeekBufferPositive(&pad, 1))
		{
			if (pad.Buttons != oldPad.Buttons)
			{
				if(pad.Buttons & PSP_CTRL_SQUARE){

					if(*configparms[selposconfig].var){
						*configparms[selposconfig].var =0;
					}else{
						*configparms[selposconfig].var =1;
					}

				}
				if(pad.Buttons & PSP_CTRL_TRIANGLE){
					//delay
					done =1;
				}
				if(pad.Buttons & PSP_CTRL_UP){
					selposconfig--;
					if(selposconfig < 0)selposconfig=0;
				}
				if(pad.Buttons & PSP_CTRL_DOWN){
					selposconfig++;
					if(selposconfig >= totalconfig -1)selposconfig=totalconfig-1;
				}

			}
			oldPad = pad;
		}

	}
	//save the config
	SaveConfig();

}

typedef struct fname{
	char name[256];
}f_name;

typedef struct flist{
	f_name fname[256];
	int cnt;
}f_list;

f_list filelist;

void ClearFileList(){
	filelist.cnt =0;
}

void GetFileList(const char *root)
{
	int dfd;
	dfd = sceIoDopen(root);
	if(dfd > 0){
		SceIoDirent dir;
		while(sceIoDread(dfd, &dir) > 0)
		{
			if(dir.d_stat.st_attr & FIO_SO_IFDIR)
			{
				//directories
			}
			else
			{				
				strcpy(filelist.fname[filelist.cnt].name,dir.d_name);
				filelist.cnt++;				
			}
		}
		sceIoDclose(dfd);
	}
}

int selpos=0;
void DisplayFileList(){
	int c,x,y;
	x=28; y=32;
	for (c=0;c<filelist.cnt;c++){
		if(selpos == c){
			pspDebugScreenSetTextColor(0xffffffff);
		}else{
			pspDebugScreenSetTextColor(0x99999999);
		}
		
		//	mh_print(x, y,filelist.fname[c].name,0xffffffff);
			pspDebugScreenPrintf("%s\n",filelist.fname[c].name);
			y+=10;
			
	}
		//pgScreenFlipV();
}

int HasExtension(char *filename){
	if(filename[strlen(filename)-4] == '.'){
		return 1;
	}
	return 0;
}
void GetExtension(const char *srcfile,char *outext){
	if(HasExtension((char *)srcfile)){
		strcpy(outext,srcfile + strlen(srcfile) - 3);
	}else{
		strcpy(outext,"");
	}
}

extern char app_path[128];
extern char romname[256];
extern int ROM_LOAD_TYPE;//psx file =0, bin =1, bin.gz =2
void DoMainGui(){
//	DoSplash();
	int done =0;
	char ext[4];
	char tmp[256];
	char appsave[1024];
	strcpy(appsave,app_path);

	SceCtrlData pad,oldPad;
	
	DLog("DoMainGui1, apppath = %s",app_path);

	ClearFileList();

//	DLog("DoMainGui2, apppath = %s",app_path);

	pspDebugScreenSetXY(0,0);

//	DLog("DoMainGui3, apppath = %s",app_path);

	sprintf(tmp,"%s/ISO",app_path);

//	DLog("DoMainGui4, apppath = %s",app_path);

	GetFileList(tmp);

//

	strcpy(app_path,appsave);

	DLog("DoMainGui5, apppath = %s",app_path);
	int cnt;
	long tm;
	for(cnt=0;cnt<100;cnt++)pspDebugScreenPrintf("\n");
	while(1){
		sceDisplayWaitVblankStart();
		pspDebugScreenSetTextColor(0xffffffff);
		pspDebugScreenSetXY(1, 0);
		pspDebugScreenPrintf("\n");
		pspDebugScreenPrintf("\n");
		pspDebugScreenPrintf("  Welcome on PSXP Yoshihiro's Based on *PCSX* Core \n\n");
		pspDebugScreenPrintf("  press X to start your Game Without the Bios logo \n\n");
		pspDebugScreenPrintf("  press O to start your Game With the Bios logo ;=)\n\n");
		pspDebugScreenPrintf("  press START to boot the bios shell for edit your MC \n\n");
		pspDebugScreenPrintf("  press SQUARE now for exit :=X \n\n");
		//pspDebugScreenSetXY(30, 0);
		DisplayFileList();
		if(sceCtrlPeekBufferPositive(&pad, 1))
		{
			if (pad.Buttons != oldPad.Buttons)
			{
				if(pad.Buttons & PSP_CTRL_SQUARE){
					SysClose();
	                        sceKernelExitGame();
				}

				if(pad.Buttons & PSP_CTRL_START){
				// Run Bios Shell ^_^ Yoshihiro
				ROM_LOAD_TYPE = 2;
				break;
				}
				if(pad.Buttons & PSP_CTRL_CROSS){
					sprintf(romname,"%s/ISO/%s",app_path,filelist.fname[selpos].name);
					DLog("romname = %s",romname);
					DLog("DoMainGui2, apppath = %s",app_path);
					//check file type, psx or bin
					GetExtension(romname,ext);
					if(!stricmp(ext,"bin")||!stricmp(ext,"iso")||!stricmp(ext,"img")||!stricmp(ext,"mds")){				
						ROM_LOAD_TYPE = 1;//bin file
						break;
					}else if(!stricmp(ext,"psx")||!stricmp(ext,"psx")){ // psx file
						ROM_LOAD_TYPE =0;
						break;
					}
					
				}
					if(pad.Buttons & PSP_CTRL_CIRCLE){
					sprintf(romname,"%s/ISO/%s",app_path,filelist.fname[selpos].name);
					DLog("romname = %s",romname);
					DLog("DoMainGui2, apppath = %s",app_path);
					//check file type, psx or bin
					GetExtension(romname,ext);
					if(!stricmp(ext,"bin")||!stricmp(ext,"iso")||!stricmp(ext,"img")||!stricmp(ext,"mds")){
						if(!Config.HLE){				
						ROM_LOAD_TYPE = 3;//bin file
				            }else ROM_LOAD_TYPE =1; 

						break;
					}else if(!stricmp(ext,"psx")||!stricmp(ext,"psx")){ // psx file
						ROM_LOAD_TYPE =0;
						break;
					}
					
				}

				if(pad.Buttons & PSP_CTRL_UP){
					selpos--;
					if(selpos < 0)selpos=0;
				}
				if(pad.Buttons & PSP_CTRL_DOWN){
					selpos++;
					if(selpos >= filelist.cnt -1)selpos=filelist.cnt-1;
				}

			}
			oldPad = pad;
		}

	}
	SysPrintf("DoMainGui 6\n");

}

