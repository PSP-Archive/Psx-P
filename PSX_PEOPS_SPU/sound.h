/******************************************************************************

	sound.c

	PSP サウンドスレッド

******************************************************************************/

#ifndef PSP_SOUND_THREAD_H
#define PSP_SOUND_THREAD_H

#define SOUND_SAMPLES		(736*2)
#define SOUND_BUFFER_SIZE	(SOUND_SAMPLES*2)

struct sound_t
{
	int  stack;
	int  stereo;
	void (*callback)(int p);
};

#define MAXOUT		(+32767)
#define MINOUT		(-32768)

#define Limit(val, max, min)			\
{										\
	if (val > max) val = max;			\
	else if (val < min) val = min;		\
}

extern struct sound_t *sound;

void sound_thread_init(void);
void sound_thread_exit(void);
void sound_thread_enable(int enable);
void sound_thread_set_volume(void);
int sound_thread_start(void);
void sound_thread_stop(void);
void sound_thread_set_priority(int priority);

#endif /* PSP_SOUND_THREAD_H */
