
#include "plugins.h"
//#include "psp.h"
#include <setjmp.h>
#include <pspctrl.h>
long  PadFlags = 0;

long PAD__init(long flags) {
	PadFlags |= flags;


	return 0;
}

long PAD__shutdown(void) {
	return 0;
}

long PAD__open(void)
{

	SysPrintf("start PAD1_open()\r\n");
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(0);

	SysPrintf("end PAD1_open()\r\n");
	return 0;
}

extern void Psxp_States_Load(int num);
extern void Psxp_States_Save(int num);
extern void CALLBACK GPUmakeSnapshot(void); 


long PAD__close(void) {
	return 0;
}
extern jmp_buf env;
long PAD__readPort1(PadDataS* pad) {
//	SysPrintf("start PAD1_readPort()\r\n");

	SceCtrlData psxpad;	
      sceCtrlPeekBufferPositive(&psxpad, 1); 
	
	unsigned short pad_status = 0xffff;
	 
	if (psxpad.Buttons & PSP_CTRL_SELECT ){
		pad_status &= ~(1<<2); 
	}

	if (psxpad.Buttons & PSP_CTRL_START){
		pad_status &= ~(1<<3);
	}
	if (psxpad.Buttons & PSP_CTRL_TRIANGLE){
		pad_status &= ~(1<<12);
	}
	if (psxpad.Buttons & PSP_CTRL_SQUARE){
		pad_status &= ~(1<<15);
	}
	if (psxpad.Buttons & PSP_CTRL_CIRCLE){
		pad_status &= ~(1<<13);
	}

	if (psxpad.Buttons & PSP_CTRL_CROSS){
		pad_status &= ~(1<<14);
	}

	//L1
	if (psxpad.Buttons & PSP_CTRL_LTRIGGER){
		pad_status &= ~(1<<8);
	}
	//R1
	if (psxpad.Buttons & PSP_CTRL_RTRIGGER){
		pad_status &= ~(1<<9);
	}
	
	if (psxpad.Buttons & PSP_CTRL_UP)
		pad_status &= ~(1<<4);
	if (psxpad.Buttons & PSP_CTRL_DOWN)
		pad_status &= ~(1<<6);
	if (psxpad.Buttons & PSP_CTRL_LEFT)
		pad_status &= ~(1<<7);
	if (psxpad.Buttons & PSP_CTRL_RIGHT)
		pad_status &= ~(1<<5);

      // ShutDown the emu back to XMB  Yoshihiro ^_^ 
	if((psxpad.Buttons & PSP_CTRL_LTRIGGER) && (psxpad.Buttons & PSP_CTRL_RTRIGGER) &&(psxpad.Buttons & PSP_CTRL_CROSS)){
	 SysClose();
	 sceKernelExitGame();
	}

      if((psxpad.Buttons & PSP_CTRL_LTRIGGER) && (psxpad.Buttons & PSP_CTRL_RTRIGGER) &&(psxpad.Buttons & PSP_CTRL_START))
           Psxp_States_Save(0);

      // PSXP load states
      if((psxpad.Buttons & PSP_CTRL_LTRIGGER) && (psxpad.Buttons & PSP_CTRL_RTRIGGER) &&(psxpad.Buttons & PSP_CTRL_SELECT))
	Psxp_States_Load(0);

      if((psxpad.Buttons & PSP_CTRL_LTRIGGER) && (psxpad.Buttons & PSP_CTRL_RTRIGGER) &&(psxpad.Buttons & PSP_CTRL_TRIANGLE))
	GPUmakeSnapshot();

	pad->buttonStatus = pad_status;

	pad->controllerType = 4; 

	return 0;
}

long PAD__readPort2(PadDataS*a) {
	return -1;
}
