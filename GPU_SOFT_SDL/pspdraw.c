/***************************************************************************
                          pspdraw.c  -  description
                             -------------------
    begin                : Sun Oct 28 2001
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
#include <pspgu.h>
#include <pspgum.h>
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
int            iDebugMode=0;
char *               pCaptionText;
PSXPoint_t     ptCursorPoint[8];
unsigned short usCursorActive=0;
int           Xpitch,depth=32;
char *        Xpixels;
char *        pCaptionText;

SDL_Surface *display,*XFimage,*XPimage=NULL;

SDL_Surface *Ximage=NULL,*XCimage=NULL;

Uint32 sdl_mask=SDL_SWSURFACE|SDL_DOUBLEBUF|SDL_ASYNCBLIT|SDL_RLEACCEL|SDL_HWACCEL;
SDL_Rect rectdst,rectsrc;

void DestroyDisplay(void)
{
if(display){
if(XCimage) SDL_FreeSurface(XCimage);  
if(Ximage) SDL_FreeSurface(Ximage);

if(XFimage) SDL_FreeSurface(XFimage); 

SDL_FreeSurface(display);
}       
SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void CreateDisplay(void)
{

if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0)
   {
	  fprintf (stderr,"(x) Failed to Init SDL!!!\n");
	  return;
   }
display = SDL_SetVideoMode(480,272,depth,!iWindowMode*SDL_FULLSCREEN|sdl_mask); 					   
					   
Ximage = SDL_CreateRGBSurface(sdl_mask,480,272,depth,0x00ff0000,0x0000ff00,0x000000ff,0);
XCimage= SDL_CreateRGBSurface(sdl_mask,480,272,depth,0x00ff0000,0x0000ff00,0x000000ff,0);

XFimage= SDL_CreateRGBSurface(sdl_mask,170,15,32,0x00ff0000,0x0000ff00,0x000000ff,0);

XCimage->pixels = (char *)malloc(1024*1024*4); 
memset(XCimage->pixels,0,1024*1024*4);
Xpixels=(char *)Ximage->pixels;

}

unsigned char * pBackBuffer=0;

void blit_init(void){
	//cache stuff
}


void BlitScreen32(unsigned char * surf,long x,long y)
{
 unsigned char * pD;
 unsigned int startxy;
 unsigned long lu;unsigned short s;
 unsigned short row,column;
 unsigned short dx=PreviousPSXDisplay.Range.x1;
 unsigned short dy=PreviousPSXDisplay.DisplayMode.y;
 long lPitch=(dx+PreviousPSXDisplay.Range.x0)<<2;

 if(PreviousPSXDisplay.Range.y0)                       // centering needed?
  {
   surf+=PreviousPSXDisplay.Range.y0*lPitch;
   dy-=PreviousPSXDisplay.Range.y0;
  }

 surf+=PreviousPSXDisplay.Range.x0<<2;

 if(PSXDisplay.RGB24)
  {
   for(column=0;column<dy;column++)
    {
     startxy=((1024)*(column+y))+x;
     pD=(unsigned char *)&psxVuw[startxy];

     for(row=0;row<dx;row++)
      {
      // lu=*((unsigned long *)pD);
      // align the ptr Yoshihiro :=p
	memcpy(&lu,(unsigned long *)pD,sizeof(pD));//=*((unsigned long *)pD);
      
        *((unsigned long *)((surf)+(column*lPitch)+(row<<2)))=
          0xff000000|(RED(lu)<<16)|(GREEN(lu)<<8)|(BLUE(lu));
       pD+=3;
       
     }
    }
  }
 else
  {
   for(column=0;column<dy;column++)
    {
     startxy=((1024)*(column+y))+x;
     for(row=0;row<dx;row++)
      {
       s=psxVuw[startxy++];
       *((unsigned long *)((surf)+(column*lPitch)+(row<<2)))=
        ((((s<<19)&0xf80000)|((s<<6)&0xf800)|((s>>7)&0xf8))&0xffffff)|0xff000000;
      }
    }
  }
}


void XStretchBlt32(unsigned char * pBB,int sdx,int sdy,int ddx,int ddy)
{ 
 unsigned long * pSrc=(unsigned long *)pBackBuffer; 
 unsigned long * pSrcR=NULL; 
 unsigned long * pDst=(unsigned long *)pBB;
 unsigned long * pDstR=NULL;
 int x,y,cyo=-1,cy;
 int xpos, xinc;
 int ypos, yinc;
#ifdef USE_DGA2
 int DGA2fix;
 int dga2Fix;
 if (!iWindowMode)
  {
   DGA2fix = (char*)pBB == Xpixels;
   dga2Fix = dgaDev->mode.imageWidth - dgaDev->mode.viewportWidth;
   dga2Fix/=2;
  } else DGA2fix = dga2Fix = 0;
#endif

 xinc = (sdx << 16) / ddx;

 ypos=0;
 yinc = (sdy << 16) / ddy;

 for(y=0;y<ddy;y++,ypos+=yinc)
  {
   cy=(ypos>>16);

   if(cy==cyo)
    {
#ifndef USE_DGA2
     pDstR=pDst-ddx;
#else
     pDstR=pDst-(ddx+dga2Fix);
#endif
     for(x=0;x<ddx;x++) *pDst++=*pDstR++; 
    }
   else 
    { 
     cyo=cy;
     pSrcR=pSrc+(cy*sdx);
     xpos = 0L;
     for(x=ddx;x>0;--x) 
      {
       pSrcR+= xpos>>16;
       xpos -= xpos&0xffff0000;
       *pDst++=*pSrcR; 
       xpos += xinc; 
      }
    } 
#ifdef USE_DGA2
   if (DGA2fix) pDst+= dga2Fix;
#endif
  } 
}


void DoBufferSwap(void){

  BlitScreen32(pBackBuffer,
            PSXDisplay.DisplayPosition.x,
            PSXDisplay.DisplayPosition.y);
 

 XStretchBlt32((unsigned char *)Xpixels,
               PreviousPSXDisplay.Range.x1+PreviousPSXDisplay.Range.x0,
               PreviousPSXDisplay.DisplayMode.y,
               iResX,iResY);


}

void          DoClearScreenBuffer(void){


}

void          DoClearFrontBuffer(void){

	DLog("After DoClearFrontBuffer");
}

unsigned long ulInitDisplay(void){
//	sceKernelDcacheWritebackAll();
      pBackBuffer=(unsigned char *)malloc(640*512*sizeof(unsigned long));
      memset(pBackBuffer,0,640*512*sizeof(unsigned long));
	CreateDisplay();
}






void CloseDisplay(void){
 if(pBackBuffer)  free(pBackBuffer);
 pBackBuffer=0;
DestroyDisplay();
}

void CreatePic(unsigned char * pMem){
unsigned char * p=(unsigned char *)malloc(128*96*4);
 unsigned char * ps; int x,y;

 ps=p;

 if(iDesktopCol==16)
  {
   unsigned short s;
   for(y=0;y<96;y++)
    {
     for(x=0;x<128;x++)
      {
       s=(*(pMem+0))>>3;
       s|=((*(pMem+1))&0xfc)<<3;
       s|=((*(pMem+2))&0xf8)<<8;
       pMem+=3;
       *((unsigned short *)(ps+y*256+x*2))=s;
      }
    }
  }
 else
 if(iDesktopCol==15)
  {
   unsigned short s;
   for(y=0;y<96;y++)
    {
     for(x=0;x<128;x++)
      {
       s=(*(pMem+0))>>3;
       s|=((*(pMem+1))&0xfc)<<2;
       s|=((*(pMem+2))&0xf8)<<7;
       pMem+=3;
       *((unsigned short *)(ps+y*256+x*2))=s;
      }
    }
  }
 else
 if(iDesktopCol==32)
  {
   unsigned long l;
   for(y=0;y<96;y++)
    {
     for(x=0;x<128;x++)
      {
       l=  *(pMem+0);
       l|=(*(pMem+1))<<8;
       l|=(*(pMem+2))<<16;
       pMem+=3;
       *((unsigned long *)(ps+y*512+x*4))=l;
      }
    }
  }


  XPimage = SDL_CreateRGBSurfaceFrom((void *)p,128,96,
			depth,depth*16,
			0x00ff0000,0x0000ff00,0x000000ff,
			0);
}

void DestroyPic(void){
   SDL_FillRect(display,NULL,0);
   SDL_FreeSurface(XPimage);
   XPimage=0; 
}

void DisplayPic(void){
 rectdst.x=iResX-128;
 rectdst.y=0;
 rectdst.w=128;
 rectdst.h=96;
 SDL_BlitSurface(XPimage,NULL,display,&rectdst);
}

void ShowGpuPic(void){

}

void ShowTextGpuPic(void){

}
