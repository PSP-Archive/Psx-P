/***************************************************************************
                          Gudraw.c  -  description
                             -------------------
    copyright            : (C) 2006 by Yoshihiro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/

//*************************************************************************//

#include "stdafx.h"

#define _IN_DRAW

#include "externals.h"
#include "gpu.h"
#include "draw.h"
#include "prim.h"
#include "menu.h"
#include "interp.h"
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <SDL.h>
#include <pspgum.h>
#include <pspgu.h>
#include <psppower.h>
#include <psprtc.h>

//DEFINE

#define GUPSX
//#define SDLPSX

////////////////////////////////////////////////////////////////////////////////////
// misc globals
////////////////////////////////////////////////////////////////////////////////////

int            iResX;
int            iResY;
long           lLowerpart;
BOOL           bIsFirstFrame = TRUE;
BOOL           bCheckMask=FALSE;
unsigned short sSetMask=0;
unsigned long  lSetMask=0;
int            iDesktopCol=16;
int            iShowFPS=0;
int            iWinSize; 
int            iUseScanLines=0;
int            iUseNoStretchBlt=0;
int            iFastFwd=0;
int            iDebugMode=0;
int            iFVDisplay=0;
PSXPoint_t     ptCursorPoint[8];
unsigned short usCursorActive=0;

unsigned int   LUT16to32[65536];
unsigned int   RGBtoYUV[65536];
char *               pCaptionText;
char *Xpixels = 0;




#define	FRAMESIZE32			(BUF_WIDTH * SCR_HEIGHT * sizeof(u32))

#define SLICE_SIZE			64 // change this to experiment with different page-cache sizes
#define TEXTURE_FLAGS		(GU_TEXTURE_16BIT | GU_COLOR_5551 | GU_VERTEX_16BIT | GU_TRANSFORM_2D)



#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272
#define PSP_LINE_SIZE 512
#define VRAM_ADDR	(0x04000000)

#define SCREEN_WIDTH	480
#define SCREEN_HEIGHT	272

#define	PIXELSIZE	1				//in short
#define	LINESIZE	512				//in short
#define	FRAMESIZE	0x88000			//in byte



///////SDL Stretching and INIT 


// SDL Yoshihiro 
Uint32 sdl_mask=SDL_HWSURFACE|SDL_ASYNCBLIT|SDL_RLEACCEL|SDL_NOFRAME;//SDL_SWSURFACE|SDL_DOUBLEBUF;
SDL_Surface *display,*XFimage,*XPimage,*XCimage,*Ximage=NULL;
SDL_Surface *Ximage16,*Ximage24;
SDL_Rect rectdst,rectsrc;
SDL_Surface *buf;
unsigned char * pBackBuffer=0;



struct Vertex
{
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
};
//static unsigned int __attribute__((aligned(16))) list[262144];

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)


static unsigned int __attribute__((aligned(16))) list[1024*512*2];
static int gecbid = -1;
static short *ScreenVertex = (short *)0x441FC100;
static unsigned int *GEcmd = (unsigned int *)0x441FC000;
static u16* psp_gu_vram_base = (u16*) (0x44000000);

u16*
psp_gu_get_vram_addr(void)
{
  return psp_gu_vram_base;
}

static void
Ge_Finish_Callback(int id, void *arg)
{
}


int
psp_gu_init(void)
{
	//sceDisplaySetMode(0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);

	//sceDisplayWaitVblankStart();
	//sceDisplaySetFrameBuf((void*)psp_gu_vram_base, PSP_LINE_SIZE, PSP_DISPLAY_PIXEL_FORMAT_565, PSP_DISPLAY_SETBUF_NEXTFRAME);

	sceGuInit();
	
	sceGuStart(GU_DIRECT, list);
	sceGuDrawBuffer(GU_PSM_8888 , (void*)0, PSP_LINE_SIZE);
	sceGuDispBuffer(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, (void*)(0), PSP_LINE_SIZE);
	sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);

	sceGuDepthBuffer((void*) 0x88000, PSP_LINE_SIZE);
	sceGuOffset(2048 - (PSP_SCREEN_WIDTH / 2), 2048 - (PSP_SCREEN_HEIGHT / 2));
	sceGuViewport(2048, 2048, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceGuDepthRange(0xc350, 0x2710);

	sceGuScissor(0, 0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);

	sceGuEnable(GU_SCISSOR_TEST);
	sceGuTexMode(GU_PSM_8888 , 0, 0, 0);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);
	
	sceGuAlphaFunc(GU_GREATER, 0, 0xff);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuAmbientColor(0xffffffff);
	sceGuDisable(GU_BLEND);
	//sceGuEnable(GU_BLEND);
	sceGuFinish();
	sceGuSync(0, 0);

//	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);

	PspGeCallbackData gecb;
	gecb.signal_func = NULL;
	gecb.signal_arg = NULL;
	gecb.finish_func = Ge_Finish_Callback;
	gecb.finish_arg = NULL;
	gecbid = sceGeSetCallback(&gecb);

  return 1;
}

void InitDisplay()
{
      unsigned long lu;
	static int qid = -1;
	ScreenVertex[0] = 0;
	ScreenVertex[1] = 0;
	ScreenVertex[2] = 0;
	ScreenVertex[3] = 0;
	ScreenVertex[4] = 0;
	ScreenVertex[5] = PreviousPSXDisplay.DisplayMode.x;
	ScreenVertex[6] = PreviousPSXDisplay.DisplayMode.y;
	ScreenVertex[7] = 480;
	ScreenVertex[8] = 272;
	ScreenVertex[9] = 0;
	// Set Draw Buffer
	GEcmd[ 0] = 0x9C000000UL | ((u32)(unsigned char *)psp_gu_get_vram_addr() & 0x00FFFFFF);
	GEcmd[ 1] = 0x9D000000UL | (((u32)(unsigned char *)psp_gu_get_vram_addr() & 0xFF000000) >> 8) | 512;
	// Set Tex Buffer

  if(PSXDisplay.RGB24)
  {
	GEcmd[ 2] = 0xA0000000UL | ((u32)(unsigned char *)psxVuw & 0x00FFFFFF);
	GEcmd[ 3] = 0xA8000000UL | (((u32)(unsigned char *)psxVuw & 0xFF000000) >> 8) | 1024;
	}else{
	GEcmd[ 2] = 0xA0000000UL | ((u32)(unsigned char *)psxVuw & 0x00FFFFFF);
	GEcmd[ 3] = 0xA8000000UL | (((u32)(unsigned char *)psxVuw & 0xFF000000) >> 8) | 1024;
  }

	// Tex size
	GEcmd[ 4] = 0xB8000000UL | (14 << 8) | 14;
	// Tex Flush
	GEcmd[ 5] = 0xCB000000UL;
	// Set Vertex
	GEcmd[ 6] = 0x12000000UL | (1 << 23) | (0 << 11) | (0 << 9) | (2 << 7) | (0 << 5) | (0 << 2) | 2;
	GEcmd[ 7] = 0x10000000UL;
	GEcmd[ 8] = 0x02000000UL;
	GEcmd[ 9] = 0x10000000UL | (((u32)(void *)ScreenVertex & 0xFF000000) >> 8);
	GEcmd[10] = 0x01000000UL | ((u32)(void *)ScreenVertex & 0x00FFFFFF);
	// Draw Vertex
	GEcmd[11] = 0x04000000UL | (6 << 16) | 3;
	// List End
	GEcmd[12] = 0x0F000000UL;
	GEcmd[13] = 0x0C000000UL;
	GEcmd[14] = 0;
	GEcmd[15] = 0;

	sceKernelDcacheWritebackAll();
	qid = sceGeListEnQueue(&GEcmd[0], &GEcmd[14], gecbid, NULL);
	sceGeListSync(qid, 0);
}

void pgGeInit()
{
	static unsigned int GeInit[] = {
	0x01000000, 0x02000000,
	0x10000000, 0x12000000, 0x13000000, 0x15000000, 0x16000000, 0x17000000,
	0x18000000, 0x19000000, 0x1A000000, 0x1B000000, 0x1C000000, 0x1D000000,
	0x1E000000, 0x1F000000,
	0x20000000, 0x21000000, 0x22000000, 0x23000000, 0x24000000, 0x25000000,
	0x26000000, 0x27000000, 0x28000000, 0x2A000000, 0x2B000000, 0x2C000000,
	0x2D000000, 0x2E000000, 0x2F000000,
	0x30000000, 0x31000000, 0x32000000, 0x33000000, 0x36000000, 0x37000000,
	0x38000000, 0x3A000000, 0x3B000000, 0x3C000000, 0x3D000000, 0x3E000000,
	0x3F000000,
	0x40000000, 0x41000000, 0x42000000, 0x43000000, 0x44000000, 0x45000000,
	0x46000000, 0x47000000, 0x48000000, 0x49000000, 0x4A000000, 0x4B000000,
	0x4C000000, 0x4D000000,
	0x50000000, 0x51000000, 0x53000000, 0x54000000, 0x55000000, 0x56000000,
	0x57000000, 0x58000000, 0x5B000000, 0x5C000000, 0x5D000000, 0x5E000000,
	0x5F000000,
	0x60000000, 0x61000000, 0x62000000, 0x63000000, 0x64000000, 0x65000000,
	0x66000000, 0x67000000, 0x68000000, 0x69000000, 0x6A000000, 0x6B000000,
	0x6C000000, 0x6D000000, 0x6E000000, 0x6F000000,
	0x70000000, 0x71000000, 0x72000000, 0x73000000, 0x74000000, 0x75000000,
	0x76000000, 0x77000000, 0x78000000, 0x79000000, 0x7A000000, 0x7B000000,
	0x7C000000, 0x7D000000, 0x7E000000, 0x7F000000,
	0x80000000, 0x81000000, 0x82000000, 0x83000000, 0x84000000, 0x85000000,
	0x86000000, 0x87000000, 0x88000000, 0x89000000, 0x8A000000, 0x8B000000,
	0x8C000000, 0x8D000000, 0x8E000000, 0x8F000000,
	0x90000000, 0x91000000, 0x92000000, 0x93000000, 0x94000000, 0x95000000,
	0x96000000, 0x97000000, 0x98000000, 0x99000000, 0x9A000000, 0x9B000000,
	0x9C000000, 0x9D000000, 0x9E000000, 0x9F000000,
	0xA0000000, 0xA1000000, 0xA2000000, 0xA3000000, 0xA4000000, 0xA5000000,
	0xA6000000, 0xA7000000, 0xA8000000, 0xA9000000, 0xAA000000, 0xAB000000,
	0xAC000000, 0xAD000000, 0xAE000000, 0xAF000000,
	0xB0000000, 0xB1000000, 0xB2000000, 0xB3000000, 0xB4000000, 0xB5000000,
	0xB8000000, 0xB9000000, 0xBA000000, 0xBB000000, 0xBC000000, 0xBD000000,
	0xBE000000, 0xBF000000,
	0xC0000000, 0xC1000000, 0xC2000000, 0xC3000000, 0xC4000000, 0xC5000000,
	0xC6000000, 0xC7000000, 0xC8000000, 0xC9000000, 0xCA000000, 0xCB000000,
	0xCC000000, 0xCD000000, 0xCE000000, 0xCF000000,
	0xD0000000, 0xD2000000, 0xD3000000, 0xD4000000, 0xD5000000, 0xD6000000,
	0xD7000000, 0xD8000000, 0xD9000000, 0xDA000000, 0xDB000000, 0xDC000000,
	0xDD000000, 0xDE000000, 0xDF000000,
	0xE0000000, 0xE1000000, 0xE2000000, 0xE3000000, 0xE4000000, 0xE5000000,
	0xE6000000, 0xE7000000, 0xE8000000, 0xE9000000, 0xEB000000, 0xEC000000,
	0xEE000000,
	0xF0000000, 0xF1000000, 0xF2000000, 0xF3000000, 0xF4000000, 0xF5000000,
	0xF6000000,	0xF7000000, 0xF8000000, 0xF9000000,
	0x0F000000, 0x0C000000};

	int qid;
	sceKernelDcacheWritebackAll();
	qid = sceGeListEnQueue(&GeInit[0], 0, -1, 0);
	//sceGeListSync(qid, 0);
	
	static unsigned int GEcmd[64];
	// Draw Area
	GEcmd[ 0] = 0x15000000UL | (0 << 10) | 0;
	GEcmd[ 1] = 0x16000000UL | (271 << 10) | 639;
	// Tex Enable
	GEcmd[ 2] = 0x1E000000UL | 1;
	// Viewport
	GEcmd[ 3] = 0x42000000UL | (((int)((float)(480)) >> 8) & 0x00FFFFFF);
	GEcmd[ 4] = 0x43000000UL | (((int)((float)(-272)) >> 8) & 0x00FFFFFF);
	GEcmd[ 5] = 0x44000000UL | (((int)((float)(50000)) >> 8) & 0x00FFFFFF);
	GEcmd[ 6] = 0x45000000UL | (((int)((float)(2048)) >> 8) & 0x00FFFFFF);
	GEcmd[ 7] = 0x46000000UL | (((int)((float)(2048)) >> 8) & 0x00FFFFFF);
	GEcmd[ 8] = 0x47000000UL | (((int)((float)(60000)) >> 8) & 0x00FFFFFF);
	GEcmd[ 9] = 0x4C000000UL | (1024 << 4);
	GEcmd[10] = 0x4D000000UL | (1024 << 4);
	// Model Color
	GEcmd[11] = 0x54000000UL;
	GEcmd[12] = 0x55000000UL | 0xFFFFFFFF;
	GEcmd[13] = 0x56000000UL | 0xFFFFFFFF;
	GEcmd[14] = 0x57000000UL | 0xFFFFFFFF;
	GEcmd[15] = 0x58000000UL | 0xFF;
	// Depth Buffer
	GEcmd[16] = 0x9E000000UL | 0x110000;
	GEcmd[17] = 0x9F000000UL | 512;
	// Tex
	GEcmd[18] = 0xC2000000UL | (0 << 16) | (0 << 8) | 0;
	GEcmd[19] = 0xC3000000UL | 1;
	GEcmd[20] = 0xC6000000UL | (1 << 8) | 1;
	GEcmd[21] = 0xC7000000UL | (1 << 8) | 1;
	GEcmd[22] = 0xC9000000UL | (0 << 16) | (0 << 8) | 3;
	// Pixel Format
	GEcmd[23] = 0xD2000000UL | 3;
	// Scissor
	GEcmd[24] = 0xD4000000UL | (0 << 10) | 0;
	GEcmd[25] = 0xD5000000UL | (271 << 10) | 639;
	// Depth
	GEcmd[26] = 0xD6000000UL | 10000;
	GEcmd[27] = 0xD7000000UL | 50000;
	// List End
	GEcmd[28] = 0x0F000000UL;
	GEcmd[29] = 0x0C000000UL;
	GEcmd[30] = 0;
	sceKernelDcacheWritebackAll();
	qid = sceGeListEnQueue(&GEcmd[0], &GEcmd[30], -1, 0);
	//sceGeListSync(qid, 0);
	
	PspGeCallbackData  gecb;
	gecb.signal_func = NULL;
	gecb.signal_arg = NULL;
	gecb.finish_func = Ge_Finish_Callback;
	gecb.finish_arg = NULL;
	gecbid = sceGeSetCallback(&gecb);
	
//	InitDisplay();
}


    
void init_video()
{
}


void DestroyDisplay(void)
{
}


void clear_screen(u16 color)
{

}

void DoClearScreenBuffer(void) {

}

void DoClearFrontBuffer(void)   
{

}

void
psp_gu_normal_blit(u16* src,int sync)
{
	u16* dest = psp_gu_vram_base;
	static int qid = -1;
	ScreenVertex[0] = 0;
	ScreenVertex[1] = 0;
	ScreenVertex[2] = 0;
	ScreenVertex[3] = 0;
	ScreenVertex[4] = 0;
	ScreenVertex[5] = PreviousPSXDisplay.DisplayMode.x;
	ScreenVertex[6] = PreviousPSXDisplay.DisplayMode.y;
	ScreenVertex[7] = 640;
	ScreenVertex[8] = 272;
	ScreenVertex[9] = 0;

	// Set Draw Buffer
	GEcmd[ 0] = 0x9C000000UL | ((u32)(unsigned char *)dest & 0x00FFFFFF);
	GEcmd[ 1] = 0x9D000000UL | (((u32)(unsigned char *)dest & 0xFF000000) >> 8) | 512;
	// Set Tex Buffer
	GEcmd[ 2] = 0xA0000000UL | ((u32)(unsigned char *)src & 0x00FFFFFF);
	GEcmd[ 3] = 0xA8000000UL | (((u32)(unsigned char *)src & 0xFF000000) >> 8) | 1024;
	// Tex size
	if(PreviousPSXDisplay.DisplayMode.x >= 480)
	{
        GEcmd[ 4] = 0xB8000000UL | (8 << 8)|14;
      }else	GEcmd[ 4] = 0xB8000000UL | (8 << 8)|14;
	// Tex Flush
	GEcmd[ 5] = 0xCB000000UL;
	// Set Vertex
	GEcmd[ 6] = 0x12000000UL | (1 << 23) | (0 << 11) | (0 << 9) | (2 << 7) | (0 << 5) | (0 << 2) | 2;
	GEcmd[ 7] = 0x10000000UL;
	GEcmd[ 8] = 0x02000000UL;
	GEcmd[ 9] = 0x10000000UL | (((u32)(void *)ScreenVertex & 0xFF000000) >> 8);
	GEcmd[10] = 0x01000000UL | ((u32)(void *)ScreenVertex & 0x00FFFFFF);
	// Draw Vertex
	GEcmd[11] = 0x04000000UL | (6 << 16) | 2;
	// List End
	GEcmd[12] = 0x0F000000UL;
	GEcmd[13] = 0x0C000000UL;
	GEcmd[14] = 0;
	GEcmd[15] = 0;

	sceKernelDcacheWritebackAll();
	qid = sceGeListEnQueue(&GEcmd[0], &GEcmd[14], gecbid, NULL);
	if (sync && qid >= 0) sceGeListSync(qid, 0);
}

void DoBufferSwap(void)
{
InitDisplay();
}

unsigned long ulInitDisplay(void){
pgGeInit();

}

void CloseDisplay(void)
{

}

void CreatePic(unsigned char * pMem)
{
}

void DestroyPic(void)
{
}

void DisplayPic(void)
{
}

void ShowGpuPic(void){

}

void ShowTextGpuPic(void)
{
}
