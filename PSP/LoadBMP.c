// **********
// CRaster::LoadBMPFile (FileName);
//   - loads a BMP file into a CRaster object
//   * supports non-RLE-compressed files of 1, 2, 4, 8 & 24 bits-per-pixel
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <time.h>
//#include "psxcommon.h"
#include "loadbmp.h"

unsigned short RGB(unsigned char r, unsigned char g, unsigned char b) { 
	return ((((b>>3) & 0x1F)<<10)+(((g>>3) & 0x1F)<<5)+(((r>>3) & 0x1F)<<0)+0x8000);
}
#define GetA(col)      ((col)>>24)
#define GetR(col)      (((col)>>16) & 0xFF)
#define GetG(col)      (((col)>>8) & 0xFF)
#define GetB(col)      ((col) & 0xFF)

//doesn't have to be fast.
int ConvertTo16(PSP_BITMAP *bmp){
	HWLog("ConvertTo16 1 bpp = %d",bmp->BPP);
	if(bmp->BPP == 0) return 0;
	HWLog("ConvertTo16 2");
	if(bmp->BPP == 16) return 1; //16bpp already
	HWLog("ConvertTo16 3 ");
	unsigned short *newsurf;
	unsigned short pix;
	unsigned long lpix;
	int x,y;
	RGBQUAD *col;
	newsurf = malloc_psp(bmp->Width*bmp->Height*2); //alloc new surf
	for(y=0;y<bmp->Height;y++ ){
		for(x=0;x<bmp->Width;x++ ){
			if(bmp->BPP == 8){
				//get from the palette
				col = &bmp->Palette[bmp->surface[y*bmp->Width + x]];
				pix = RGB565(GetR(col->rgbRed),GetG(col->rgbGreen),GetB(col->rgbBlue));
			}
			if(bmp->BPP == 24){
				// convert the pix
				lpix = bmp->surface[y*bmp->Width + x];
				pix = RGB565(GetR(lpix),GetG(lpix),GetB(lpix));
				newsurf[y*bmp->Width + x ] = pix;
			}
		}
	}
	free_psp(bmp->surface);
	bmp->surface = (char *)newsurf;
	bmp->BPP =16;//
	return 1; //conversion complete
}

void InitBMP(PSP_BITMAP *bmp){
	HWLog("Initing bmp");
	bmp->BPP =0;
	bmp->Height =0;
	bmp->Width =0;
	bmp->Palette =0;
	bmp->surface =0;
	bmp->BytesPerRow =0;
}

void FreeBMP(PSP_BITMAP *bmp){
	if(bmp->surface){
		free_psp(bmp->surface);
	}
	if(bmp->Palette){
		free_psp(bmp->Palette);
	}
	InitBMP(bmp);
}
int LoadBMP (char * szFile,PSP_BITMAP *bmp)
{
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	BITMAPINFO *pbmi=0;
	int BytesPerRow,n;
	// Open file.
	HWLog("LoadBMP 1");
	FILE *fp = fopen(szFile,"rb");
	HWLog("LoadBMP 2");
	if(!fp)return 0;
	// Load bitmap fileheader & infoheader
//	fread((char*)&bmfh,sizeof (BITMAPFILEHEADER),1,fp);
	fread((char*)&bmfh,14,1,fp);
	fread((char*)&bmih,sizeof (BITMAPINFOHEADER),1,fp);
	HWLog("LoadBMP 3");
	// Check filetype signature
//	if (bmfh.bfType!='MB'){ 
//		return 2;		// File is not BMP
//	}

	// Assign some short variables:
	bmp->BPP=bmih.biBitCount;
	bmp->Width=bmih.biWidth;
	bmp->Height= (bmih.biHeight>0) ? bmih.biHeight : -bmih.biHeight; // absoulte value
	HWLog("LoadBMP 4 ,%d, %d", bmp->Width ,bmp->Height );
	BytesPerRow = bmp->Width * bmp->BPP / 8;
	BytesPerRow += (4-BytesPerRow%4) % 4;	// int alignment
	HWLog("LoadBMP 6 %d",BytesPerRow);
	// If BPP aren't 24, load Palette:

	if (bmp->BPP==24) {
		HWLog("LoadBMP 7");
		pbmi = (BITMAPINFO*)malloc_psp(sizeof(BITMAPINFO));
	}else{
		HWLog("LoadBMP 8");
		pbmi = (BITMAPINFO*)malloc_psp(sizeof(BITMAPINFOHEADER)+(1<<bmp->BPP)*sizeof(RGBQUAD));
		bmp->Palette=(RGBQUAD*)((char*)pbmi+sizeof(BITMAPINFOHEADER));
	}

	pbmi->bmiHeader=bmih;

	// Load Raster

//	bmpfile.seekg (bmfh.bfOffBits,ios::beg);
	HWLog("LoadBMP 9");

	//fseek(fp,bmfh.bfOffBits,0);//seek from the beginning
	fseek(fp,0x36,SEEK_SET);//seek from the beginning
	bmp->surface=malloc_psp(BytesPerRow*bmp->Height);

	HWLog("allocating image size = %d",BytesPerRow*bmp->Height);
	bmp->BytesPerRow = BytesPerRow;
	// (if height is positive the bmp is bottom-up, read it reversed)
		if (bmih.biHeight>0){
			DLog("***Reading bottom up Height %d",bmp->Height );
			for (n=bmp->Height-1;n>=0;n--){
			//	HWLog("reading row %d",n);
				fread(bmp->surface+BytesPerRow*n,BytesPerRow,1,fp);
			}
			//DLog("read %n rows",n);
			//bmpfile.read (Raster+BytesPerRow*n,BytesPerRow);
		}else{
			//bmpfile.read (Raster,BytesPerRow*Height);
			HWLog("reading surface1 ");
			fread(bmp->surface, BytesPerRow*bmp->Height,1,fp);
		}

	// so, we always have a up-bottom raster (that is negative height for windows):
	//pbmi->bmiHeader.biHeight=-Height;

	//bmpfile.close();
	fclose(fp);

	return 1;
}




int too_big(w, h) { // function for checking if a image is 'weirdly too big'
// just to prevent overflows (imagine the file you're
// loading is not even a BMP)
	if((w*h)>(1024*1024)) 
		return 1; // max size arbitrarily set to
	else 
		return 0;
}


// pixelsize is the size, in bytes, of each pixel (4 in case of RGBA8888)
// wb and hb are pointers to variables that will be set to the width and
// height of the image
// pixeldatalines is an array of pointers which point to the start addresses
// of every horizontal line in the statically allocated image buffer
//
// returns non-null value on success

void *loadBitmapFile_noMalloc(char *filename, int pixelsize, int *wb, int *hb, char **pixeldatalines) {
	FILE *fp;
	int w = 0, h = 0;
	int i, j, temp;
	fp = fopen(filename,"rb");
	if(!fp) {
		return NULL;
	}
	fseek(fp,0x12,SEEK_SET);
	fread(&w,4,1,fp);
	fseek(fp,0x16,SEEK_SET);
	fread(&h,4,1,fp);

	if(h<0) h = -h;
	if(w<0) w = -w;
	if(too_big(w, h)) return NULL;

	*hb = h;
	*wb = w;

	fseek(fp,0x36,SEEK_SET);

	for(i=0; i<h; ++i) {
		fread(pixeldatalines[i], pixelsize*w,1,fp);
	}

	for(i=0; i<h; ++i) { // reverse red and blue components
		for(j=0; j<w; ++j) {
			temp = pixeldatalines[i][j*pixelsize];
			pixeldatalines[i][j*pixelsize] = pixeldatalines[i][j*pixelsize+2];
			pixeldatalines[i][j*pixelsize+2] = temp;
		}
	}
	fclose(fp);
	return (void *)0xDEADBEEF; // ignore
}


