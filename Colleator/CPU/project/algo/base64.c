/*
Simple base64 decoder/encoder. 
No any dynamically memory allocating
based on sources from internet
2020 kiltum@kiltum.tech
*/

#include "base64.h"

static const unsigned char b64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;

static const unsigned char ub64[] = {   
    0,   0,   0,  62,   0,   0,   0,  63,  52,  53, 
    54,  55,  56,  57,  58,  59,  60,  61,   0,   0, 
    0,   0,   0,   0,   0,   0,   1,   2,   3,   4, 
    5,   6,   7,   8,   9,  10,  11,  12,  13,  14, 
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24, 
    25,   0,   0,   0,   0,   0,   0,  26,  27,  28, 
    29,  30,  31,  32,  33,  34,  35,  36,  37,  38, 
    39,  40,  41,  42,  43,  44,  45,  46,  47,  48, 
    49,  50,  51,   0,   0,   0,   0,   0,   0,   0    
}; 

int base64_encode_length (unsigned char *input, int len)
{
    int mod = len % 3 ;
    int pad = ((mod & 1) << 1) + ((mod & 2) >> 1);
    return (4 * (len + pad) / 3) + 1;
}

int base64_encode( unsigned char *input, int len, unsigned char *output )
{
    int rc = 0 ; 
    int bn ; 

    int mod = len % 3 ;
    int pad = ((mod & 1) << 1) + ((mod & 2) >> 1);

    unsigned char b0, b1,b2;

    for ( bn = 0 ; bn <= len - 3 ; bn += 3 ) {
        b0 = input[bn];
        b1 = input[bn + 1];
        b2 = input[bn + 2];
        output[rc++]  = b64[ b0 >> 2 ] ;
        output[rc++]  = b64[ ((0x3 & b0) << 4) + (b1 >> 4) ] ;
        output[rc++]  = b64[ ((0x0f & b1) << 2) + (b2 >> 6) ] ;
        output[rc++]  = b64[ 0x3f & b2 ] ;
    }

    if ( pad == 2 ) {
        output[rc++] = b64[ input[bn] >> 2 ] ;
        output[rc++] = b64[ (0x3 & input[bn]) << 4 ] ;
        output[rc++] = '=';
        output[rc++] = '=';
    } else if ( pad == 1 ) {
        output[rc++]  = b64[ input[bn] >> 2 ] ;
        output[rc++]  = b64[ ((0x3 & input[bn]) << 4)   +   (input[bn + 1] >> 4) ] ;
        output[rc++]  = b64[ (0x0f & input[bn + 1]) << 2 ] ;
        output[rc++] = '=';
    }

    output[rc] = 0; 
    return rc ;
}

int strexist( unsigned char t)
{
    for(int i=0; i< sizeof(b64); i++)
    {
        if(b64[i]==t) return 0;
    }
    return 1;
}

int base64_decode( unsigned char *input, int len, unsigned char *output )
{
    int cb = 0;
    int cn;
    int pad = 0 ;
    int a,b,c,d;

    // validting input string

    for ( int i = 0; i < len; i++ ) {
        if (input[i] == '=') {
            ++pad;
            if (pad > 2) { // invalid padding!
                return 0;
            }
        } else if (strexist(input[i]) == 1) { // bad input char
            return 0;
        } else if (pad) { // bad input 
            return 0;
        }
    }

   // lets decode

    for ( cn = 0; cn <= len - 4 - pad ; cn += 4 ) {
        a = ub64[input[cn] - 40];
        b = ub64[input[cn + 1] - 40];
        c = ub64[input[cn + 2] - 40];
        d = ub64[input[cn + 3] - 40];

        output[cb++] = (a << 2) | (b >> 4) ;
        output[cb++] = (b << 4) | (c >> 2) ;
        output[cb++] = (c << 6) | (d) ;
    }

    if ( pad == 1 ) {
        a = ub64[input[cn] - 40];
        b = ub64[input[cn + 1] - 40];
        c = ub64[input[cn + 2] - 40];

        output[cb++] = (a << 2) | (b >> 4) ;
        output[cb++] = (b << 4) | (c >> 2) ;
    } else if ( pad == 2 ) {
        a = ub64[input[cn] - 40];
        b = ub64[input[cn + 1] - 40];

        output[cb++] = (a << 2) | (b >> 4) ;
    }

    return cb ;
}
