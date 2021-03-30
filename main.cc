#include "Bitmap.h"

#include <cstdio>

int main(void)
{
    Bitmap<Pixel::BGR24> bmp;
    bmp.load("bmp_24.bmp");
    BMPError err;

    for (int i = 0; i < bmp.width(); i++)
    {
        for (int j = 0; j < bmp.height(); j++)
        {
            Pixel::BGR24 p;
            bmp.get(i, j, p);
            p.r = 0;

            err = bmp.set(i, j, p);
            if (err != BMP_SUCCESS)
            {
                printf("Error: %d - err=0x%X\n", __LINE__, err);
            }
        }
    }
    bmp.write("testtest.bmp");

    Bitmap<Pixel::BGR24> bmp2;
    bmp2.create(1024,1024);
    bmp2.write("writetest.bmp");

    return 0;
}