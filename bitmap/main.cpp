#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "bitmap_image.hpp"

using namespace std;

int main()
{
  freopen("test.out","w",stdout);
  bitmap_image image("./test.bmp");
  if(!image)
  {
  	printf("Error -failed to open: input.bmp\n");
  	return 1;
  }

  unsigned char red,green,blue;
  const unsigned int height=image.height();
  const unsigned int width =image.width();

  for(size_t y=0;y<height;++y)
  {
  	for(size_t x=0;x<width;++x)
  	{
  		image.get_pixel(x,y,red,green,blue);
  		cout<<(int)red<<" "<<(int)green<<" "<<(int)blue<<endl;
  	}
  }
   return 0;
}


