#include "Bitmap.h"

#include <cstdio>

int main(void)
{
    Bitmap<Pixel::RGB24> bmp;
    bmp.load("bmp_24.bmp");
    BMPError err;

    for (int i = 0; i < bmp.width(); i++)
    {
        for (int j = 0; j < bmp.height(); j++)
        {
            Pixel::RGB24 p;
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

    return 0;
}