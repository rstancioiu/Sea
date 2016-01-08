#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "bitmap_image.hpp"
#define N 7
#define M 4

using namespace std;

int main()
{
  bitmap_image image("./input.bmp");
  if(!image)
  {
    printf("Error -failed to open: input.bmp\n");
    return 1;
  }

  unsigned char red,green,blue;
  const unsigned int height=image.height();
  const unsigned int width =image.width();
  int k=0;
  for(int i=0;i<N;++i)
  {
    for(int j=0;j<M;++j)
    {
      size_t a = i*width/N;
      size_t b = (i+1)*width/N;
      size_t c = j*height/M;
      size_t d = (j+1)*height/M;
      int pos = j*N+i;
      if(pos!=21 && pos!=27)
      {
        string s="test/";
        s+=(char)('A'+k);
        s+=".out";
        const char * filename = s.c_str();
        freopen(filename,"w",stdout);
        for(size_t y=c;y<d;++y)
        {
          for(size_t x=a;x<b;++x)
          {
            image.get_pixel(x,y,red,green,blue);
            cout<<(int)red<<" "<<(int)green<<" "<<(int)blue<<endl;
          }
        }
        k++;
      }
    }
  }
  return 0;
}


