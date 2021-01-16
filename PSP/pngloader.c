/***************************pngloader.h*************************/

/*************************pngloader.c***************************/

#include "pngloader.h"
#include <stdio.h>

unsigned short RGB565(unsigned char r, unsigned char g, unsigned char b) { //smhzc
	//return ((((b>>3) & 0x1F)<<10)+(((g>>3) & 0x1F)<<5)+(((r>>3) & 0x1F)<<0)+0x8000);
	return ((((b>>3) & 0x1F)<<10)+(((g>>3) & 0x1F)<<5)+(((r>>3) & 0x1F)));
}

void BPP24_to_16(struct pngImage *png)
{
	unsigned short *newdat = malloc(png->width*2);
	unsigned short *tdat = newdat;
	int y,x;
	unsigned char r,g,b;
	for (y=0;y<png->height;y++){
		for(x=0;x<png->width;x++){
			r = png->data[png->rowbytes * y + (x*3)];
			g = png->data[png->rowbytes * y + (x*3)+1];
			b = png->data[png->rowbytes * y + (x*3)+2];
			tdat = RGB565(r,g,b);
			tdat++;
		}
		//memcpy(&png->data[png->rowbytes * y],newdat,png->width*2);
		tdat = newdat;
	}
	free(newdat);
}

int loadPNGImage(char *fileName, struct pngImage *png)
 {  
  FILE* pngFile = fopen(fileName, "r");
  char *signature[8];
  
  if(! pngFile)
   {
  	//DLog("could not load \'%s\'. aborting\n", fileName);
  	return 0xf000;
   }
  fread(signature, 8, 1, pngFile);
  int is_png = !png_sig_cmp((png_bytep)signature, 0, 8);
  if (!is_png)
   {
  	//DLog("file \'%s\' is not in png format\n", fileName);
  	return 0xf010;
   }
  png -> png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)0, 0, 0);
  if(! png -> png_ptr)
   {
  	//DLog("could not create png pointer\n");
    return 0xf011;
   }
  
  png -> info_ptr = png_create_info_struct(png -> png_ptr);
  if (!png -> info_ptr)
   {
    png_destroy_read_struct(&png -> png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    return 0xf012;
   }
  
  png -> end_info = png_create_info_struct(png -> png_ptr);

  //png_set_filler(png ->png_ptr, 0xff, PNG_FILLER_BEFORE);

  if (!png -> end_info) 
   {
    png_destroy_read_struct(&png -> png_ptr, &png -> info_ptr, (png_infopp)NULL);
	  return 0xf013;
   }
  png_init_io(png -> png_ptr, pngFile);
  png_set_sig_bytes(png -> png_ptr, 8);
  png_read_info(png -> png_ptr, png -> info_ptr);
   
  png_get_IHDR(png -> png_ptr, png -> info_ptr, &png -> width, &png -> height, &png -> bit_depth, &png -> color_type, &png -> interlace_type, &png -> compression_type, &png -> filter_type);
   
  if(png -> info_ptr -> channels == 1){
    png -> bpp = 8;
	//DLog("8bpp");
  }else if(png -> info_ptr -> channels == 2){
    png -> bpp = 16;
	//DLog("16bpp");
  }else if(png -> info_ptr -> channels == 3){
    png -> bpp = 24;
	//DLog("24bpp");
  }else if(png -> info_ptr -> channels == 4){
    png -> bpp = 32;
	//DLog("32bpp");
  }else
  	//DLog("libpng -> error in bit depth of file \'%s\'\n", fileName);

  

  png -> rowbytes = png_get_rowbytes(png -> png_ptr, png -> info_ptr);
  png -> data = (unsigned char*)malloc(png -> rowbytes * png -> height * sizeof(char));
  png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * png -> height);
  
  int row;
  /*
  for (row = (int)png->height-1; row >= 0 ; row--)
    row_pointers[((int)png->height-1)-row] = &png ->data[png -> rowbytes * row];
  */
  for (row = 0; row < png->height; row++)
    row_pointers[row] = &png ->data[png -> rowbytes * row];

  png_read_image(png -> png_ptr, row_pointers);
  free(row_pointers);
  png_read_end(png -> png_ptr, png -> info_ptr);
  
  fclose(pngFile);
  return 0;
 }
 