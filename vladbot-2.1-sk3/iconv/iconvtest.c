#include "stdio.h"
#include "iconv.h"

int main(int argc,char** argv)
{
    char inbuff[5] = { 233,232,224,231,0 } ; /* iso=8859-1: יטאח */
    char *pinbuff=(char *)&inbuff;
    char outbuff[17]= {0}; /* multibyte...probably better to store in a w_char */
    char *poutbuff=(char *)&outbuff;
    size_t inbytes=4;
    size_t outbytes=8;/*4 bytes of 8859-1 input = 8 bytes of UFT-8 output*/

    iconv_t cd;
    cd = iconv_open ("UTF-8","ISO-8859-1");
    if (cd != ((iconv_t) -1))
        {
            if ( iconv(cd,&pinbuff,&inbytes,&poutbuff,&outbytes) != (size_t)(-1))
                {
                    printf("utf-8: %s\n",outbuff);
                }
            else
                {
                    printf("unable to convert\n");
                }
        }
    else
        {
            printf("unable to open iconv\n");
        }
    iconv_close(cd);
    return 0;
}

