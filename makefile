TARGET = PSXP

###### PSX-P MakeFile for PSP only plugin embeded Yoshihiro ^_^
###### plugins pad , CD & PSX-SPU NULL are from PCSX Dreamcast port by PCSX TEAM 
###### And the PacManFan Gui wait the the new gui in the next psx-p release 
OBJS = \
PCSX_CORE/CdRom.o \
PCSX_CORE/Decode_XA.o \
PCSX_CORE/DisR3000A.o \
PCSX_CORE/Gte.o \
PCSX_CORE/Mdec.o \
PCSX_CORE/Misc.o \
PCSX_CORE/PsxBios.o \
PCSX_CORE/PsxCounters.o \
PCSX_CORE/PsxDma.o \
PCSX_CORE/PsxHLE.o \
PCSX_CORE/PsxHw.o \
PCSX_CORE/PsxMem.o \
PCSX_CORE/R3000A.o \
PCSX_CORE/Sio.o \
PCSX_CORE/Spu.o \
PCSX_CORE/PsxInterpreter.o \
PSX-CDROM/PlugCD.o \
PSX-PAD/PlugPAD.o \
GPU_SOFT_SDL/cfg.o \
GPU_SOFT_SDL/conf.o \
GPU_SOFT_SDL/draw_sdl.o \
GPU_SOFT_SDL/fps.o \
GPU_SOFT_SDL/gpu.o \
GPU_SOFT_SDL/menu.o \
GPU_SOFT_SDL/prim.o \
GPU_SOFT_SDL/soft.o \
GPU_SOFT_SDL/zn.o \
PSX_SPU_NULL/PlugSPU.o \
PSP/PSPMain.o \
PSP/plugins.o \
PSP/Plugin.o \
PSP/pngloader.o \
PSP/FrontEnd.o \
PSP/log.o \
PSP/vram.o 


#PSX_PEOPS_SPU/cfg.o \
#PSX_PEOPS_SPU/spu.o \
#PSX_PEOPS_SPU/dma.o \
#PSX_PEOPS_SPU/freeze.o \
#PSX_PEOPS_SPU/Pspsound.o \
#PSX_PEOPS_SPU/zn.o \
#PSX_PEOPS_SPU/psemu.o \
#PSX_PEOPS_SPU/registers.o \

PSP_EBOOT_SND0 = ./IconePSXP/SND0.AT3 
PSP_EBOOT_ICON = ./IconePSXP/ICON0.png
PSP_EBOOT_PIC1 = ./IconePSXP/pic1.png

PSPBIN = $(shell psp-config --psp-prefix)/bin

INCDIR = ./PSP ./PCSX_CORE ./PSX-CDROM ./MIPS_JIT_R4000 ./GPU_SOFT_SDL ./SDL_PSP_LIB_BUILD/include/SDL
CFLAGS = -Os  -fsingle-precision-constant -funswitch-loops  -fbranch-target-load-optimize2 -D__PSP__ -D_SDL 
#-D_SDL2  

#-D_OPENGL

## Not Yet But Soon : -DPSP_DYNAREC

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)


LIBDIR =
LDFLAGS = 
LIBS=  ./SDL_PSP_LIB_BUILD/lib/libSDL.a ./SDL_PSP_LIB_BUILD/lib/libSDL_mixer.a -lpsphprm -lpsppower -lz -lpng -lc -lm -lpsputility -lpspdebug -lpspge -lpspdisplay -lpspctrl -lpspsdk -lpspuser -lpspkernel -lpsprtc  -lpspaudiolib -lpspaudio -lpspgu


EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSXP Playstation Emulator For PSP (PCSX*)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
