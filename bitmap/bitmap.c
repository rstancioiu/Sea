#include "bitmap.h"

void draw_letter(char c) 
{
	int character = c-'A';
	int x = 0, y= 0;
	int i,j;
	for(i=0;i<64;++i)
	{
		for(int j=0;j<55;++j)
		{
			int number = i*64+j;
			unsigned int r = letters[character][number].red;
			unsigned int g = letters[character][number].green;
			unsigned int b = letters[character][number].blue;
			put_pixel_RGB24(x+i,y+j,r,g,b);
		}
	}
}