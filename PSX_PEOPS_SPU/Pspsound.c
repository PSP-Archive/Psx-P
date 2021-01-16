/***************************************************************************
                            psp.c  -  description
                             -------------------
    begin                : Sat Mar 01 2003
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
// 2006/10/20 - Create by Yoshihiro
//
//*************************************************************************//

#include "stdafx.h"

#include "externals.h"
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspaudiolib.h>
#include "sound.h"
#include <pspaudio.h>
#include <SDL.h>
#include <SDL_mixer.h>
#ifdef __PSP__

int soundBufferIndex;
u8 *sdl_snd_buffer = NULL;
int sdlSoundLen = 0;
int soundBufferLen = 0;
int soundBufferTotalLen = 0;
typedef int (*pg_threadfunc_t)(int args, void *argp);
////////////////////////////////////////////////////////////////////////
// small linux time helper... only used for watchdog
////////////////////////////////////////////////////////////////////////

unsigned long XtimeGetTime()
{

}

////////////////////////////////////////////////////////////////////////
// oss globals
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// SETUP SOUND
////////////////////////////////////////////////////////////////////////

static volatile int sound_active;
static int sound_handle;
static SceUID sound_thread;
static int sound_volume;
static int sound_enabled;
static s16 sound_buffer[2][SOUND_BUFFER_SIZE];

static struct sound_t sound_info;


/******************************************************************************
	グローバル変数
******************************************************************************/

struct sound_t *sound = &sound_info;


/******************************************************************************
	ローカル関数
******************************************************************************/

void 
audioCallback(void *userdata,u8 *buf,int length)
{
  if (sdl_snd_buffer) {
    if (length > soundBufferLen) {
      length = soundBufferLen;
    }

 //    memcpy(buf, pSpuBuffer, length);

  }

}


void SetupSound(void)
{
SDL_AudioSpec audio;

  
    audio.freq = 44100;
    int soundBufferLen = 1024*2;


  audio.format   = AUDIO_S16;
  audio.channels = 2;
  audio.samples  = soundBufferLen / 4;
  audio.callback = audioCallback;
  audio.userdata = NULL;
  if(SDL_OpenAudio(&audio, NULL)) {
    fprintf(stderr,"Failed to open audio: %s\n", SDL_GetError());
  }

  if (sdl_snd_buffer == NULL) {
    sdl_snd_buffer = (u8 *)malloc(soundBufferLen);
  }

  
  soundBufferTotalLen = soundBufferLen*10;
  sdlSoundLen = 0;

  sceKernelDelayThread(800000); // Give sound some time to init
  return 1;

}

////////////////////////////////////////////////////////////////////////
// REMOVE SOUND
////////////////////////////////////////////////////////////////////////

void RemoveSound(void)
{

}

////////////////////////////////////////////////////////////////////////
// GET BYTES BUFFERED
////////////////////////////////////////////////////////////////////////

unsigned long SoundGetBytesBuffered(void)
{
 unsigned long l;
/*
 if(handle == NULL)                                 // failed to open?
  return SOUNDSIZE;
 l = 0;//snd_pcm_avail_update(handle);
 if(l<0) return 0;
 if(l<buffer_size/2)                                 // can we write in at least the half of fragments?
      l=SOUNDSIZE;                                   // -> no? wait
 else l=0;                                           // -> else go on
*/
 return l;
}

////////////////////////////////////////////////////////////////////////
// FEED SOUND DATA
////////////////////////////////////////////////////////////////////////

void SoundFeedStreamData(unsigned char* pSound,long lBytes)
{

//sound_buffer[0] = pSound;
//memcpy(sdl_snd_buffer,(u8*)pSpuBuffer,sizeof(pSpuBuffer));
}

void sound_thread_set_priority(int priority)
{
	//sceKernelChangeThreadPriority(sound_thread, priority);
}

#endif
