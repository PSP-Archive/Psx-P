/*  Pcsx - Pc Psx Emulator
 *  Copyright (C) 1999-2003  Pcsx Team
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "PsxCommon.h"

#ifdef _MSC_VER_
#pragma warning(disable:4244)
#endif

#define CheckErr(func) \
    err = SysLibError(); \
    if (err != NULL) { SysMessage(_("Error loading %s: %s"), func, err); return -1; }

#if defined (__MACOSX__)
#define LoadSym(dest, src, name, checkerr) \
    dest = (src) SysLoadSym(drv, "_" name); \
    if(!checkerr) { SysLibError(); /*clean error*/ } \
    if (checkerr == 1) CheckErr(name); \
    if (checkerr == 2) { err = SysLibError(); if (err != NULL) errval = 1; }
#else
#define LoadSym(dest, src, name, checkerr) \
    dest = (src) SysLoadSym(drv, name); if (checkerr == 1) CheckErr(name); \
    if (checkerr == 2) { err = SysLibError(); if (err != NULL) errval = 1; }
#endif

static const char *err;
static int errval;

void *hGPUDriver;

void ConfigurePlugins();
 // these are needed for the gl version
void CALLBACK GPU__displayText(char *pText) {
	SysPrintf("%s\n", pText);
}

extern int StatesC;
long CALLBACK GPU__freeze(unsigned long ulGetFreezeData, GPUFreeze_t *pF) {
	pF->ulFreezeVersion = 1;
	if (ulGetFreezeData == 0) {
		int val;

		val = GPU_readStatus();
		val = 0x04000000 | ((val >> 29) & 0x3);
		GPU_writeStatus(0x04000003);
		GPU_writeStatus(0x01000000);
		GPU_writeData(0xa0000000);
		GPU_writeData(0x00000000);
		GPU_writeData(0x02000400);
		GPU_writeDataMem((unsigned long*)pF->psxVRam, 0x100000/4);
		GPU_writeStatus(val);

		val = pF->ulStatus;
		GPU_writeStatus(0x00000000);
		GPU_writeData(0x01000000);
		GPU_writeStatus(0x01000000);
		GPU_writeStatus(0x03000000 | ((val>>24)&0x1));
		GPU_writeStatus(0x04000000 | ((val>>29)&0x3));
		GPU_writeStatus(0x08000000 | ((val>>17)&0x3f) | ((val>>10)&0x40));
		GPU_writeData(0xe1000000 | (val&0x7ff));
		GPU_writeData(0xe6000000 | ((val>>11)&3));

/*		GPU_writeData(0xe3000000 | pF->ulControl[0] & 0xffffff);
		GPU_writeData(0xe4000000 | pF->ulControl[1] & 0xffffff);
		GPU_writeData(0xe5000000 | pF->ulControl[2] & 0xffffff);*/
		return 1;
	}
	if (ulGetFreezeData == 1) {
		int val;

		val = GPU_readStatus();
		val = 0x04000000 | ((val >> 29) & 0x3);
		GPU_writeStatus(0x04000003);
		GPU_writeStatus(0x01000000);
		GPU_writeData(0xc0000000);
		GPU_writeData(0x00000000);
		GPU_writeData(0x02000400);
		GPU_readDataMem((unsigned long*)pF->psxVRam, 0x100000/4);
		GPU_writeStatus(val);

		pF->ulStatus = GPU_readStatus();

/*		GPU_writeStatus(0x10000003);
		pF->ulControl[0] = GPU_readData();
		GPU_writeStatus(0x10000004);
		pF->ulControl[1] = GPU_readData();
		GPU_writeStatus(0x10000005);
		pF->ulControl[2] = GPU_readData();*/
		return 1;
	}
	if(ulGetFreezeData==2) {
		long lSlotNum=*((long *)pF);
		char Text[32];

		sprintf (Text, "*PCSX*: Selected State %ld", lSlotNum+1);
		GPU_displayText(Text);
		return 1;
	}
	return 0;
}

long CALLBACK GPU__configure(void) { return 0; }
long CALLBACK GPU__test(void) { return 0; }
void CALLBACK GPU__about(void) {}
void CALLBACK GPU__makeSnapshot(void) {}
void CALLBACK GPU__keypressed(int key) {}
long CALLBACK GPU__getScreenPic(unsigned char *pMem) { return -1; }
long CALLBACK GPU__showScreenPic(unsigned char *pMem) { return -1; }
void CALLBACK GPU__clearDynarec(void (CALLBACK *callback)(void)) { }

//#define LoadGpuSym1(dest, name) \
//	LoadSym(GPU_##dest, GPU##dest, name, 1);
#define LoadGpuSym1(dest, name) \
	LoadSym(GPU_##dest, CBGPU##dest, name, 1);

#define LoadGpuSym0(dest, name) \
	LoadSym(GPU_##dest, CBGPU##dest, name, 0); \
	if (GPU_##dest == NULL) GPU_##dest = (CBGPU##dest) GPU__##dest;
//#define LoadGpuSym0(dest, name) \
//	LoadSym(GPU_##dest, GPU##dest, name, 0); \
//	if (GPU_##dest == NULL) GPU_##dest = (GPU##dest) GPU__##dest;

#define LoadGpuSymN(dest, name) \
	LoadSym(GPU_##dest, CBGPU##dest, name, 0);
//#define LoadGpuSymN(dest, name) \
//	LoadSym(GPU_##dest, GPU##dest, name, 0);

#define PSPLoadGpuSym(dest) \
	GPU_##dest = (CBGPU##dest) GPU##dest;

#define PSPLoadGpuSym1(dest) \
	GPU_##dest = (CBGPU##dest) GPU__##dest;

extern long CALLBACK GPUinit(void);
extern long CALLBACK GPUopen(void);
extern long CALLBACK GPUshutdown(void);
extern long CALLBACK GPUclose(void);
extern void CALLBACK GPUwriteStatus(unsigned long);
extern void CALLBACK GPUwriteData(unsigned long);
extern void CALLBACK GPUwriteDataMem(unsigned long *, int);
extern unsigned long CALLBACK GPUreadStatus(void);
extern unsigned long CALLBACK GPUreadData(void);
extern void CALLBACK GPUreadDataMem(unsigned long *, int);
extern long CALLBACK GPUdmaChain(unsigned long *,unsigned long);
extern void CALLBACK GPUupdateLace(void);
extern long CALLBACK GPUconfigure(void);
extern long CALLBACK GPUtest(void);
extern void CALLBACK GPUabout(void);
extern void CALLBACK GPUmakeSnapshot(void);

int LoadGPUplugin(char *GPUdll) {
	void *drv;

	hGPUDriver = SysLoadLibrary(GPUdll);
	if (hGPUDriver == NULL) { 
		GPU_configure = NULL;
		SysMessage (_("Could Not Load GPU Plugin %s"), GPUdll); return -1; 
	}
	drv = hGPUDriver;
	SysPrintf("address of init %d",GPU_init);
	SysPrintf("address of init2 %d",GPUinit);
//	LoadGpuSym1(init, "GPUinit");
		//GPU_##dest = (GPU##dest) GPU__##dest
	GPU_init = (CBGPUinit) GPUinit;
	SysPrintf("address of init2 %d",GPU_init);

	PSPLoadGpuSym(open);
	//GPU_open = (CBGPUopen) GPUopen;
	PSPLoadGpuSym(shutdown);
	
	PSPLoadGpuSym(close);
	PSPLoadGpuSym(readData);
	PSPLoadGpuSym(readDataMem);
	PSPLoadGpuSym(readStatus);
	PSPLoadGpuSym(writeData);
	PSPLoadGpuSym(writeDataMem);
	PSPLoadGpuSym(writeStatus);
	PSPLoadGpuSym(dmaChain);
	PSPLoadGpuSym(updateLace);	

	PSPLoadGpuSym1(displayText);	
	PSPLoadGpuSym1(freeze);
	PSPLoadGpuSym1(getScreenPic);
	PSPLoadGpuSym1(showScreenPic);
	PSPLoadGpuSym1(clearDynarec);
	PSPLoadGpuSym1(configure);
	PSPLoadGpuSym1(test);
	PSPLoadGpuSym1(about);
	PSPLoadGpuSym1(makeSnapshot);
	PSPLoadGpuSym1(keypressed);

/*long CALLBACK GPU__configure(void) { return 0; }
long CALLBACK GPU__test(void) { return 0; }
void CALLBACK GPU__about(void) {}
void CALLBACK GPU__makeSnapshot(void) {}
void CALLBACK GPU__keypressed(int key) {}
long CALLBACK GPU__getScreenPic(unsigned char *pMem) { return -1; }
long CALLBACK GPU__showScreenPic(unsigned char *pMem) { return -1; }
void CALLBACK GPU__clearDynarec(void (CALLBACK *callback)(void)) { }*/
	return 0;
}

void *hCDRDriver;

long CALLBACK CDR__play(unsigned char *sector) { return 0; }
long CALLBACK CDR__stop(void) { return 0; }
/*
long CALLBACK CDR__getStatus(struct CdrStat *stat) {
	HWLog("CDR__getStatus 1");
    if (cdOpenCase){
		HWLog("CDR__getStatus 2");
		stat->Status = 0x10;
	}else{
		HWLog("CDR__getStatus 3");
		stat->Status = 0;
	}
	HWLog("CDR__getStatus 4");
    return 0;
}
*/
long CALLBACK CDR__getStatus(struct CdrStat *stat) {
	HWLog("CDR__getStatus 1");
    if (cdOpenCase){
		HWLog("CDR__getStatus 2");
		stat->Status = 0x10;
	}else{
		HWLog("CDR__getStatus 3");
		stat->Status = 0;
	}
	HWLog("CDR__getStatus 4");
    return 0;
}

char* CALLBACK CDR__getDriveLetter(void) { return NULL; }
unsigned char* CALLBACK CDR__getBufferSub(void) { return NULL; }
long CALLBACK CDR__configure(void) { return 0; }
long CALLBACK CDR__test(void) { return 0; }
void CALLBACK CDR__about(void) {}

#define LoadCdrSym1(dest, name) \
	CDR_##dest = (CDR##dest) CDR__##dest; // smhzc fake bind for psp's sake 

//#define LoadCdrSym1(dest, name) \   smhzc
//	LoadSym(CDR_##dest, CDR##dest, name, 1);

#define LoadCdrSym0(dest, name) \
	LoadSym(CDR_##dest, CDR##dest, name, 0); \
	if (CDR_##dest == NULL) CDR_##dest = (CDR##dest) CDR__##dest;

#define LoadCdrSymN(dest, name) \
	LoadSym(CDR_##dest, CDR##dest, name, 0);

int LoadCDRplugin(char *CDRdll) {
	void *drv;

	SysPrintf("Loading CDRdriver \n");
	hCDRDriver = SysLoadLibrary(CDRdll);
	if (hCDRDriver == NULL) {
		SysPrintf("Could Not load CDR plugin \n");
		CDR_configure = NULL;
		SysMessage (_("Could Not load CDR plugin %s"), CDRdll);  return -1;
	}
	drv = hCDRDriver;
	LoadCdrSym1(init, "CDRinit");
	LoadCdrSym1(shutdown, "CDRshutdown");
	LoadCdrSym1(open, "CDRopen");
	LoadCdrSym1(close, "CDRclose");
	LoadCdrSym1(getTN, "CDRgetTN");
	LoadCdrSym1(getTD, "CDRgetTD");
	LoadCdrSym1(readTrack, "CDRreadTrack");
	LoadCdrSym1(getBuffer, "CDRgetBuffer");
	/*
	LoadCdrSym0(play, "CDRplay");
	LoadCdrSym0(stop, "CDRstop");
	LoadCdrSym0(getStatus, "CDRgetStatus");	
	LoadCdrSym0(getDriveLetter, "CDRgetDriveLetter");
	LoadCdrSym0(getBufferSub, "CDRgetBufferSub");
	LoadCdrSym0(configure, "CDRconfigure");
	LoadCdrSym0(test, "CDRtest");
	LoadCdrSym0(about, "CDRabout");
*/
	LoadCdrSym1(play, "CDRplay");
	LoadCdrSym1(stop, "CDRstop");
	LoadCdrSym1(getStatus, "CDRgetStatus");	
	LoadCdrSym1(getDriveLetter, "CDRgetDriveLetter");
	LoadCdrSym1(getBufferSub, "CDRgetBufferSub");
	LoadCdrSym1(configure, "CDRconfigure");
	LoadCdrSym1(test, "CDRtest");
	LoadCdrSym1(about, "CDRabout");

	DLog("after loading cdr plugins");
	return 0;
}

void *hSPUDriver;

long CALLBACK SPU__configure(void) { return 0; }
void CALLBACK SPU__about(void) {}
long CALLBACK SPU__test(void) { return 0; }

unsigned short regArea[10000];
unsigned short spuCtrl,spuStat,spuIrq;
unsigned long spuAddr;

void CALLBACK SPU__writeRegister(unsigned long add,unsigned short value) { // Old Interface
	unsigned long r=add&0xfff;
	regArea[(r-0xc00)/2] = value;

	if(r>=0x0c00 && r<0x0d80) {
		unsigned char ch=(r>>4)-0xc0;
		switch(r&0x0f) {//switch voices
			case 0:  //left volume
        		SPU_setVolumeL(ch,value);
				return;
			case 2: //right volume
				SPU_setVolumeR(ch,value);
				return;
			case 4:  //frequency
				SPU_setPitch(ch,value);
				return;
			case 6://start address
	            		SPU_setAddr(ch,value);
				return;
     //------------------------------------------------// level
//     			case 8:
//       			s_chan[ch].ADSRX.AttackModeExp  = (val&0x8000)?TRUE:FALSE;
//       			s_chan[ch].ADSRX.AttackRate     = (float)((val>>8) & 0x007f)*1000.0f/240.0f;
//       			s_chan[ch].ADSRX.DecayRate      = (float)((val>>4) & 0x000f)*1000.0f/240.0f;
//      			s_chan[ch].ADSRX.SustainLevel   = (float)((val)    & 0x000f);

//       			return;
//     			case 10:
//       			s_chan[ch].ADSRX.SustainModeExp = (val&0x8000)?TRUE:FALSE;
//       			s_chan[ch].ADSRX.ReleaseModeExp = (val&0x0020)?TRUE:FALSE;
//       			s_chan[ch].ADSRX.SustainRate    = ((float)((val>>6) & 0x007f))*R_SUSTAIN;
//       			s_chan[ch].ADSRX.ReleaseRate    = ((float)((val)    & 0x001f))*R_RELEASE;
//       			if(val & 0x4000) s_chan[ch].ADSRX.SustainModeDec=-1.0f;
//       			else             s_chan[ch].ADSRX.SustainModeDec=1.0f;
//       			return;
//     			case 12:
//       			return;
//     			case 14:                                    
//       			s_chan[ch].pRepeat=spuMemC+((unsigned long) val<<3);
//       			return;
		}
    		return;
	}

	switch(r) {
		case H_SPUaddr://SPU-memory address
    			spuAddr = (unsigned long) value<<3;
		//	spuAddr=value * 8;
    			return;
		case H_SPUdata://DATA to SPU
//      		spuMem[spuAddr/2] = value;
//         		spuAddr+=2;
//        		if(spuAddr>0x7ffff) spuAddr=0;
			SPU_putOne(spuAddr,value);
    			spuAddr+=2;
    			return;
		case H_SPUctrl://SPU control 1
    			spuCtrl=value;
    			return;
		case H_SPUstat://SPU status
                        spuStat=value & 0xf800;
    			return;
		case H_SPUirqAddr://SPU irq
    			spuIrq = value;
    			return;
		case H_SPUon1://start sound play channels 0-16
		        SPU_startChannels1(value);
    			return;
		case H_SPUon2://start sound play channels 16-24
    			SPU_startChannels2(value);
    			return;
		case H_SPUoff1://stop sound play channels 0-16
    			SPU_stopChannels1(value);
    			return;
		case H_SPUoff2://stop sound play channels 16-24
    			SPU_stopChannels2(value);
    			return;		
	}
}

unsigned short CALLBACK SPU__readRegister(unsigned long add) {
	switch(add&0xfff) {// Old Interface
		case H_SPUctrl://spu control
    			return spuCtrl;
		case H_SPUstat://spu status
    			return spuStat;
		case H_SPUaddr://SPU-memory address
                         return (unsigned short)(spuAddr>>3);
		case H_SPUdata://DATA to SPU
    			spuAddr+=2;
//        		if(spuAddr>0x7ffff) spuAddr=0;
//        		return spuMem[spuAddr/2];
			return SPU_getOne(spuAddr);
		case H_SPUirqAddr://spu irq
    			return spuIrq;
		//case H_SPUIsOn1:
    			//return IsSoundOn(0,16);
		//case H_SPUIsOn2:
    			//return IsSoundOn(16,24);
	}
	return regArea[((add&0xfff)-0xc00)/2];
}

void CALLBACK SPU__writeDMA(unsigned short val) {
	SPU_putOne(spuAddr, val);
	spuAddr += 2;
	if (spuAddr > 0x7ffff) spuAddr = 0;
}

unsigned short CALLBACK SPU__readDMA(void) {
	unsigned short tmp = SPU_getOne(spuAddr);
	spuAddr += 2;
	if (spuAddr > 0x7ffff) spuAddr = 0;
	return tmp;
}

void CALLBACK SPU__writeDMAMem(unsigned short *pMem, int iSize) {
	while (iSize > 0) {
		SPU_writeDMA(*pMem);
		iSize--;
		pMem++;
	}		
}

void CALLBACK SPU__readDMAMem(unsigned short *pMem, int iSize) {
	while (iSize > 0) {
		*pMem = SPU_readDMA();
		iSize--;
		pMem++;
	}		
}

void CALLBACK SPU__playADPCMchannel(xa_decode_t *xap) {}

long CALLBACK SPU__freeze(unsigned long ulFreezeMode, SPUFreeze_t *pF) {
	if (ulFreezeMode == 2) {
		memset(pF, 0, 16);
		strcpy((char *)pF->PluginName, "Pcsx");
		pF->PluginVersion = 1;
		pF->Size = 0x200 + 0x80000 + 16 + sizeof(xa_decode_t);

		return 1;
	}
	if (ulFreezeMode == 1) {
		unsigned long addr;
		unsigned short val;

		val = SPU_readRegister(0x1f801da6);
		SPU_writeRegister(0x1f801da6, 0);
		SPU_readDMAMem((unsigned short *)pF->SPURam, 0x80000/2);
		SPU_writeRegister(0x1f801da6, val);

		for (addr = 0x1f801c00; addr < 0x1f801e00; addr+=2) {
			if (addr == 0x1f801da8) { pF->SPUPorts[addr - 0x1f801c00] = 0; continue; }
			pF->SPUPorts[addr - 0x1f801c00] = SPU_readRegister(addr);
		}

		return 1;
	}
	if (ulFreezeMode == 0) {
		unsigned long addr;
		unsigned short val;
		unsigned short *regs = (unsigned short *)pF->SPUPorts;

		val = SPU_readRegister(0x1f801da6);
		SPU_writeRegister(0x1f801da6, 0);
		SPU_writeDMAMem((unsigned short *)pF->SPURam, 0x80000/2);
		SPU_writeRegister(0x1f801da6, val);

		for (addr = 0x1f801c00; addr < 0x1f801e00; addr+=2) {
			if (addr == 0x1f801da8) { regs++; continue; }
			SPU_writeRegister(addr, *(regs++));
		}

		return 1;
	}

	return 0;
}

void CALLBACK SPU__registerCallback(void (CALLBACK *callback)(void)) {}

#define LoadSpuSymFAKE(dest, name) \
	SPU_##dest = (SPU##dest) SPU__##dest;

#define LoadSpuSymFAKE1(dest, name) \
	SPU_##dest = (SPU##dest) SPU_d_##dest;

#define LoadSpuSym1(dest, name) \
	LoadSym(SPU_##dest, SPU##dest, name, 1);

#define LoadSpuSym2(dest, name) \
	LoadSym(SPU_##dest, SPU##dest, name, 2);

#define LoadSpuSym0(dest, name) \
	LoadSym(SPU_##dest, SPU##dest, name, 0); \
	if (SPU_##dest == NULL) SPU_##dest = (SPU##dest) SPU__##dest;

#define LoadSpuSymE(dest, name) \
	LoadSym(SPU_##dest, SPU##dest, name, errval); \
	if (SPU_##dest == NULL) SPU_##dest = (SPU##dest) SPU__##dest;

#define LoadSpuSymN(dest, name) \
	LoadSym(SPU_##dest, SPU##dest, name, 0); \

void SPU_d_async(unsigned long v);


int LoadSPUplugin(char *SPUdll) {
	void *drv;

	hSPUDriver = SysLoadLibrary(SPUdll);
	if (hSPUDriver == NULL) {
		SPU_configure = NULL;
		SysMessage (_("Could not open SPU plugin %s"), SPUdll); return -1;
	}
	drv = hSPUDriver;
	//LoadSpuSymFAKE
	LoadSpuSymFAKE1(init, "SPUinit");
	LoadSpuSymFAKE1(shutdown, "SPUshutdown");
	LoadSpuSymFAKE1(open, "SPUopen");
	LoadSpuSymFAKE1(close, "SPUclose");
	LoadSpuSymFAKE(configure, "SPUconfigure");
	LoadSpuSymFAKE(about, "SPUabout");
	LoadSpuSymFAKE(test, "SPUtest");
	errval = 0;
	LoadSpuSymFAKE1(startChannels1, "SPUstartChannels1");
	LoadSpuSymFAKE1(startChannels2, "SPUstartChannels2");
	LoadSpuSymFAKE1(stopChannels1, "SPUstopChannels1");
	LoadSpuSymFAKE1(stopChannels2, "SPUstopChannels2");
	LoadSpuSymFAKE1(putOne, "SPUputOne");
	LoadSpuSymFAKE1(getOne, "SPUgetOne");
	LoadSpuSymFAKE1(setAddr, "SPUsetAddr");
	LoadSpuSymFAKE1(setPitch, "SPUsetPitch");
	LoadSpuSymFAKE1(setVolumeL, "SPUsetVolumeL");
	LoadSpuSymFAKE1(setVolumeR, "SPUsetVolumeR");
	LoadSpuSymFAKE(writeRegister, "SPUwriteRegister");
	LoadSpuSymFAKE(readRegister, "SPUreadRegister");		
	LoadSpuSymFAKE(writeDMA, "SPUwriteDMA");
	LoadSpuSymFAKE(readDMA, "SPUreadDMA");
	LoadSpuSymFAKE(writeDMAMem, "SPUwriteDMAMem");
	LoadSpuSymFAKE(readDMAMem, "SPUreadDMAMem");
	LoadSpuSymFAKE(playADPCMchannel, "SPUplayADPCMchannel");
	LoadSpuSymFAKE(freeze, "SPUfreeze");
	LoadSpuSymFAKE(registerCallback, "SPUregisterCallback");
	LoadSpuSymFAKE1(async, "SPUasync");
/*
	LoadSpuSym1(init, "SPUinit");
	LoadSpuSym1(shutdown, "SPUshutdown");
	LoadSpuSym1(open, "SPUopen");
	LoadSpuSym1(close, "SPUclose");
	LoadSpuSym0(configure, "SPUconfigure");
	LoadSpuSym0(about, "SPUabout");
	LoadSpuSym0(test, "SPUtest");
	errval = 0;
	LoadSpuSym2(startChannels1, "SPUstartChannels1");
	LoadSpuSym2(startChannels2, "SPUstartChannels2");
	LoadSpuSym2(stopChannels1, "SPUstopChannels1");
	LoadSpuSym2(stopChannels2, "SPUstopChannels2");
	LoadSpuSym2(putOne, "SPUputOne");
	LoadSpuSym2(getOne, "SPUgetOne");
	LoadSpuSym2(setAddr, "SPUsetAddr");
	LoadSpuSym2(setPitch, "SPUsetPitch");
	LoadSpuSym2(setVolumeL, "SPUsetVolumeL");
	LoadSpuSym2(setVolumeR, "SPUsetVolumeR");
	LoadSpuSymE(writeRegister, "SPUwriteRegister");
	LoadSpuSymE(readRegister, "SPUreadRegister");		
	LoadSpuSymE(writeDMA, "SPUwriteDMA");
	LoadSpuSymE(readDMA, "SPUreadDMA");
	LoadSpuSym0(writeDMAMem, "SPUwriteDMAMem");
	LoadSpuSym0(readDMAMem, "SPUreadDMAMem");
	LoadSpuSym0(playADPCMchannel, "SPUplayADPCMchannel");
	LoadSpuSym0(freeze, "SPUfreeze");
	LoadSpuSym0(registerCallback, "SPUregisterCallback");
	LoadSpuSymN(async, "SPUasync");
*/
	return 0;
}


void *hPAD1Driver;
void *hPAD2Driver;

static unsigned char buf[256];
unsigned char stdpar[10] = { 0x00, 0x41, 0x5a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
unsigned char mousepar[8] = { 0x00, 0x12, 0x5a, 0xff, 0xff, 0xff, 0xff };
unsigned char analogpar[9] = { 0x00, 0xff, 0x5a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static int bufcount, bufc;

PadDataS padd1, padd2;

unsigned char _PADstartPoll(PadDataS *pad) {
	bufc = 0;

//	pad->controllerType = PSE_PAD_TYPE_STANDARD;
	switch (pad->controllerType) {
		case PSE_PAD_TYPE_MOUSE:
			mousepar[3] = pad->buttonStatus & 0xff;
			mousepar[4] = pad->buttonStatus >> 8;
			mousepar[5] = pad->moveX;
			mousepar[6] = pad->moveY;

			memcpy(buf, mousepar, 7);
			bufcount = 6;
			break;
		case PSE_PAD_TYPE_ANALOGPAD: // scph1150
			analogpar[1] = 0x73;
			analogpar[3] = pad->buttonStatus & 0xff;
			analogpar[4] = pad->buttonStatus >> 8;
			analogpar[5] = pad->rightJoyX;
			analogpar[6] = pad->rightJoyY;
			analogpar[7] = pad->leftJoyX;
			analogpar[8] = pad->leftJoyY;

			memcpy(buf, analogpar, 9);
			bufcount = 8;
			break;
		case PSE_PAD_TYPE_ANALOGJOY: // scph1110
			analogpar[1] = 0x53;
			analogpar[3] = pad->buttonStatus & 0xff;
			analogpar[4] = pad->buttonStatus >> 8;
			analogpar[5] = pad->rightJoyX;
			analogpar[6] = pad->rightJoyY;
			analogpar[7] = pad->leftJoyX;
			analogpar[8] = pad->leftJoyY;

			memcpy(buf, analogpar, 9);
			bufcount = 8;
			break;
		case PSE_PAD_TYPE_STANDARD:
		default:
			stdpar[3] = pad->buttonStatus & 0xff;
			stdpar[4] = pad->buttonStatus >> 8;
//			DLog("dat1 %d",stdpar[3]);
//			DLog("dat2 %d",stdpar[4]);
			memcpy(buf, stdpar, 5);
			bufcount = 4;
	}

	return buf[bufc++];
}
unsigned char _PADpoll(unsigned char value) {
	if (bufc > bufcount) return 0;
	return buf[bufc++];
}


unsigned char CALLBACK PAD1__startPoll(int pad) {
	PadDataS padd;

	PAD1_readPort1(&padd);

	return _PADstartPoll(&padd);
}

unsigned char CALLBACK PAD1__poll(unsigned char value) {
	return _PADpoll(value);
}

long CALLBACK PAD1__configure(void) { return 0; }
void CALLBACK PAD1__about(void) {}
long CALLBACK PAD1__test(void) { return 0; }
long CALLBACK PAD1__query(void) { return 3; }
long CALLBACK PAD1__keypressed() { return 0; }
void CALLBACK PAD1__setSensitive(int f){return;}
void CALLBACK PAD2__setSensitive(int f){return;}

extern long PAD__init(long flags);
extern long PAD__shutdown(void) ;
extern long PAD__open(void);
extern long PAD__close(void);
extern long PAD__readPort1(PadDataS* pad);
extern long PAD__readPort2(PadDataS* a);

#define LoadPad1Sym1(dest, name) \
	LoadSym(PAD1_##dest, PAD##dest, name, 1);

#define LoadPad1SymN(dest, name) \
	LoadSym(PAD1_##dest, PAD##dest, name, 0);

//#define LoadPad1Sym0(dest, name) \
//	LoadSym(PAD1_##dest, PAD##dest, name, 0); \
//	if (PAD1_##dest == NULL) PAD1_##dest = (PAD##dest) PAD1__##dest;
#define LoadPad1Sym0(dest, name) \
	PAD1_##dest = (PAD##dest) PAD1__##dest;

#define LoadPad1Sym(dest, name) \
	PAD1_##dest = (PAD##dest) PAD__##dest;

int LoadPAD1plugin(char *PAD1dll) {
	void *drv;
	SysPrintf("In LoadPAD1plugin");
	hPAD1Driver = SysLoadLibrary(PAD1dll);
	if (hPAD1Driver == NULL) {
		PAD1_configure = NULL;
		SysMessage (_("Could Not load PAD1 plugin %s"), PAD1dll); return -1;
	}
	SysPrintf("In LoadPAD1plugin 1");
	drv = hPAD1Driver;
	SysPrintf("In LoadPAD1plugin 2");
	LoadPad1Sym(init, "PADinit");
	LoadPad1Sym(shutdown, "PADshutdown");
	LoadPad1Sym(open, "PADopen");
	LoadPad1Sym(close, "PADclose");	
	LoadPad1Sym(readPort1, "PADreadPort1");

	LoadPad1Sym0(query, "PADquery");
	LoadPad1Sym0(configure, "PADconfigure");
	LoadPad1Sym0(test, "PADtest");
	LoadPad1Sym0(about, "PADabout");
	LoadPad1Sym0(keypressed, "PADkeypressed");


	LoadPad1Sym0(startPoll, "PADstartPoll");
	LoadPad1Sym0(poll, "PADpoll");
	LoadPad1Sym0(setSensitive, "PADsetSensitive");
	SysPrintf("In LoadPAD1plugin end");
	return 0;
}

unsigned char CALLBACK PAD2__startPoll(int pad) {
	PadDataS padd;

	PAD2_readPort2(&padd);
	
	return _PADstartPoll(&padd);
}

unsigned char CALLBACK PAD2__poll(unsigned char value) {
	return _PADpoll(value);
}

long CALLBACK PAD2__configure(void) { return 0; }
void CALLBACK PAD2__about(void) {}
long CALLBACK PAD2__test(void) { return 0; }
long CALLBACK PAD2__query(void) { return 3; }
long CALLBACK PAD2__keypressed() { return 0; }

#define LoadPad2Sym1(dest, name) \
	LoadSym(PAD2_##dest, PAD##dest, name, 1);

//#define LoadPad2Sym0(dest, name) \
//	LoadSym(PAD2_##dest, PAD##dest, name, 0); \
//	if (PAD2_##dest == NULL) PAD2_##dest = (PAD##dest) PAD2__##dest;

#define LoadPad2SymN(dest, name) \
	LoadSym(PAD2_##dest, PAD##dest, name, 0);

#define LoadPad2Sym(dest, name) \
	PAD2_##dest = (PAD##dest) PAD__##dest;

#define LoadPad2Sym0(dest, name) \
	PAD2_##dest = (PAD##dest) PAD1__##dest;

int LoadPAD2plugin(char *PAD2dll) {
	void *drv;

	hPAD2Driver = SysLoadLibrary(PAD2dll);
	if (hPAD2Driver == NULL) {
		PAD2_configure = NULL;
		SysMessage (_("Could Not load PAD plugin %s"), PAD2dll); return -1;
	}
	drv = hPAD2Driver;
	LoadPad2Sym(init, "PADinit");
	LoadPad2Sym(shutdown, "PADshutdown");
	LoadPad2Sym(open, "PADopen");
	LoadPad2Sym(close, "PADclose");

	LoadPad2Sym0(query, "PADquery");	
	LoadPad2Sym0(configure, "PADconfigure");
	LoadPad2Sym0(test, "PADtest");
	LoadPad2Sym0(about, "PADabout");
	LoadPad2Sym0(keypressed, "PADkeypressed");
	LoadPad2Sym0(startPoll, "PADstartPoll");
	LoadPad2Sym0(poll, "PADpoll");

	LoadPad2Sym0(setSensitive, "PADsetSensitive");
//	LoadPad2Sym0(readPort2, "PADreadPort2");
	LoadPad2Sym(readPort2, "PADreadPort2");
	SysPrintf("after pad 2 bindings");
	return 0;
}

void *hNETDriver;

void CALLBACK NET__setInfo(netInfo *info) {}
void CALLBACK NET__keypressed(int key) {}
long CALLBACK NET__configure(void) { return 0; }
long CALLBACK NET__test(void) { return 0; }
void CALLBACK NET__about(void) {}

#define LoadNetSym1(dest, name) \
	LoadSym(NET_##dest, NET##dest, name, 1);

#define LoadNetSymN(dest, name) \
	LoadSym(NET_##dest, NET##dest, name, 0);

#define LoadNetSym0(dest, name) \
	LoadSym(NET_##dest, NET##dest, name, 0); \
	if (NET_##dest == NULL) NET_##dest = (NET##dest) NET__##dest;

int LoadNETplugin(char *NETdll) {
	void *drv;

	hNETDriver = SysLoadLibrary(NETdll);
	if (hNETDriver == NULL) {
		SysMessage (_("Could Not load NET plugin %s"), NETdll); return -1;
	}
	drv = hNETDriver;
	LoadNetSym1(init, "NETinit");
	LoadNetSym1(shutdown, "NETshutdown");
	LoadNetSym1(open, "NETopen");
	LoadNetSym1(close, "NETclose");
	LoadNetSymN(sendData, "NETsendData");
	LoadNetSymN(recvData, "NETrecvData");
	LoadNetSym1(sendPadData, "NETsendPadData");
	LoadNetSym1(recvPadData, "NETrecvPadData");
	LoadNetSym1(queryPlayer, "NETqueryPlayer");
	LoadNetSym1(pause, "NETpause");
	LoadNetSym1(resume, "NETresume");
	LoadNetSym0(setInfo, "NETsetInfo");
	LoadNetSym0(keypressed, "NETkeypressed");
	LoadNetSym0(configure, "NETconfigure");
	LoadNetSym0(test, "NETtest");
	LoadNetSym0(about, "NETabout");

	return 0;
}

void CALLBACK clearDynarec(void) {
	psxCpu->Reset();
}

int LoadPlugins() {
	int ret;
	char Plugin[256];
	//Loading 
	SysPrintf("Loading cdr plugin\n");
	sprintf(Plugin, "%s%s", Config.PluginsDir, Config.Cdr);
	if (LoadCDRplugin(Plugin) == -1) return -1;
	SysPrintf("Loading sound plugin\n");
	sprintf(Plugin, "%s%s", Config.PluginsDir, Config.Spu);
	if (LoadSPUplugin(Plugin) == -1) return -1;
	SysPrintf("Loading GPU plugin\n");
	sprintf(Plugin, "%s%s", Config.PluginsDir, Config.Gpu);
	if (LoadGPUplugin(Plugin) == -1) return -1;
	SysPrintf("Loading PAD1 plugin\n");
	sprintf(Plugin, "%s%s", Config.PluginsDir, Config.Pad1);
	if (LoadPAD1plugin(Plugin) == -1) return -1;
	SysPrintf("Loading PAD2 plugin\n");
	sprintf(Plugin, "%s%s", Config.PluginsDir, Config.Pad2);
	if (LoadPAD2plugin(Plugin) == -1) return -1;

/*
	if (!strcmp("Disabled", Config.Net)) Config.UseNet = 0;
	else {
		Config.UseNet = 1;
		sprintf(Plugin, "%s%s", Config.PluginsDir, Config.Net);
		if (LoadNETplugin(Plugin) == -1) return -1;
	}
*/
	SysPrintf("CDRInit");
	ret = CDR_init();
	if (ret < 0) { SysMessage (_("CDRinit error : %d"), ret); return -1; }
	SysPrintf("GPUInit");
	ret = GPU_init();
	if (ret < 0) { SysMessage (_("GPUinit error: %d"), ret); return -1; }
	SysPrintf("SPUInit");
	ret = SPU_init();
	if (ret < 0) { SysMessage (_("SPUinit error: %d"), ret); return -1; }
	SysPrintf("PAD1Init");
	ret = PAD1_init(1);
	if (ret < 0) { SysMessage (_("PAD1init error: %d"), ret); return -1; }
	SysPrintf("PAD2Init");
	ret = PAD2_init(2);
	if (ret < 0) { SysMessage (_("PAD2init error: %d"), ret); return -1; }
	if (Config.UseNet) {
		ret = NET_init();
		if (ret < 0) { SysMessage (_("NETinit error: %d"), ret); return -1; }
	}
	SysPrintf("after inits");
	return 0;
}
int NetOpened = 0;
void ReleasePlugins() {
	if (hCDRDriver  == NULL || hGPUDriver  == NULL || hSPUDriver == NULL ||
		hPAD1Driver == NULL || hPAD2Driver == NULL) return;

	if (Config.UseNet) {
		int ret = NET_close();
		if (ret < 0) Config.UseNet = 0;
		NetOpened = 0;
	}

	CDR_shutdown();
	GPU_shutdown();
	SPU_shutdown();
	PAD1_shutdown();
	PAD2_shutdown();
	if (Config.UseNet && hNETDriver != NULL) NET_shutdown(); 

	SysCloseLibrary(hCDRDriver); hCDRDriver = NULL;
	SysCloseLibrary(hGPUDriver); hGPUDriver = NULL;
	SysCloseLibrary(hSPUDriver); hSPUDriver = NULL;
	SysCloseLibrary(hPAD1Driver); hPAD1Driver = NULL;
	SysCloseLibrary(hPAD2Driver); hPAD2Driver = NULL;
	if (Config.UseNet && hNETDriver != NULL) {
		SysCloseLibrary(hNETDriver); hNETDriver = NULL;
	}
}
