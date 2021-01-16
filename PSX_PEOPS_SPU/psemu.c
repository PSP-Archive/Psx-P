/***************************************************************************
                          psemu.c  -  description
                             -------------------
    begin                : Wed May 15 2002
    copyright            : (C) 2002 by Pete Bernert
    email                : BlackDove@addcom.de
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
// History of changes:
//
// 2002/05/15 - Pete
// - generic cleanup for the Peops release
//
//*************************************************************************//

#include "stdafx.h"

#define _IN_PSEMU

#include "externals.h"
#include "regs.h"
#include "dma.h"

////////////////////////////////////////////////////////////////////////
// OLD, SOMEWHAT (BUT NOT MUCH) SUPPORTED PSEMUPRO FUNCS
////////////////////////////////////////////////////////////////////////

unsigned short CALLBACK SPU_d_getOne(unsigned long val)
{
 if(spuAddr!=0xffffffff)
  {
   return SPU_d_readDMA();
  }
 if(val>=512*1024) val=512*1024-1;
 return spuMem[val>>1];
}

void CALLBACK SPU_d_putOne(unsigned long val,unsigned short data)
{
 if(spuAddr!=0xffffffff)
  {
   SPU_d_writeDMA(data);
   return;
  }
 if(val>=512*1024) val=512*1024-1;
 spuMem[val>>1] = data;
}

void CALLBACK SPU_d_playSample(unsigned char ch)
{
}

void CALLBACK SPU_d_setAddr(unsigned char ch, unsigned short waddr)
{
 s_chan[ch].pStart=spuMemC+((unsigned long) waddr<<3);
}

void CALLBACK SPU_d_setPitch(unsigned char ch, unsigned short pitch)
{
 SetPitch(ch,pitch);
}

void CALLBACK SPU_d_setVolumeL(unsigned char ch, short vol)
{
 SetVolumeR(ch,vol);
}

void CALLBACK SPU_d_setVolumeR(unsigned char ch, short vol)
{
 SetVolumeL(ch,vol);
}               

void CALLBACK SPU_d_startChannels1(unsigned short channels)
{
 SoundOn(0,16,channels);
}

void CALLBACK SPU_d_startChannels2(unsigned short channels)
{
 SoundOn(16,24,channels);
}

void CALLBACK SPU_d_stopChannels1(unsigned short channels)
{
 SoundOff(0,16,channels);
}

void CALLBACK SPU_d_stopChannels2(unsigned short channels)
{
 SoundOff(16,24,channels);
}

void CALLBACK SPU_d_playSector(unsigned long mode, unsigned char * p)
{
 if(!iUseXA) return;                                    // no XA? bye
}

