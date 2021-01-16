/***************************pngloader.h*************************/
#ifndef __png_loader_header_
#define __png_loader_header_

#include <png.h>

#ifdef __cplusplus
extern "C"
 {
#endif

  struct pngImage
   {
    unsigned long width, height; /* typedef long png_uint_32 ^_^;;*/
    int bit_depth, color_type, filter_type, compression_type, interlace_type;
    png_structp png_ptr;
    png_infop info_ptr, end_info;
    int rowbytes;
    png_colorp palette; int num_palette;
  
    unsigned int bpp;
    unsigned char *data;
   };
 
  typedef struct pngImage pngImage;

  int loadPNGImage(char *fileName, struct pngImage *png);
   
#ifdef __cplusplus
 }
#endif

#endif
