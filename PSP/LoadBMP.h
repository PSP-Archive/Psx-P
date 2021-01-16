


typedef struct tagRGBQUAD {
  unsigned char    rgbBlue; 
  unsigned char    rgbGreen; 
  unsigned char    rgbRed; 
  unsigned char    rgbReserved; 
} RGBQUAD; 


typedef struct tagBITMAPFILEHEADER { 
  unsigned short    bfType; 
  unsigned long   bfSize; 
  unsigned short    bfReserved1; 
  unsigned short    bfReserved2; 
  unsigned long   bfOffBits; 
} BITMAPFILEHEADER, *PBITMAPFILEHEADER; 

typedef struct tagBITMAPINFOHEADER{
  unsigned long  biSize; 
  long   biWidth; 
  long   biHeight; 
  unsigned short   biPlanes; 
  unsigned short   biBitCount; 
  unsigned long  biCompression; 
  unsigned long  biSizeImage; 
  long   biXPelsPerMeter; 
  long   biYPelsPerMeter; 
  unsigned long  biClrUsed; 
  unsigned long  biClrImportant; 
} BITMAPINFOHEADER, *PBITMAPINFOHEADER; 

typedef struct tagBITMAPINFO { 
  BITMAPINFOHEADER bmiHeader; 
  RGBQUAD          bmiColors[1]; 
} BITMAPINFO, *PBITMAPINFO; 

typedef struct PSPBITMAP{
	char *surface; // the bitmap
	int Width,Height; // size
	int BPP; //bits per pixel
	RGBQUAD *Palette;//palette if any
	int BytesPerRow;
}PSP_BITMAP;


void InitBMP(PSP_BITMAP *bmp);
void FreeBMP(PSP_BITMAP *bmp);
int LoadBMP (char * szFile,PSP_BITMAP *bmp);
int ConvertTo16(PSP_BITMAP *bmp);

