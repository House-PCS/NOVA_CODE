#ifndef _BASE64_H_
#define _BASE64_H_

/* Very simple library for base 64 encodenig/decoding. 
No malloc, no unnecessary checks
based on sources from internet
2020 kiltum@kiltum.tech
*/

// return how many bytes need for storing result WITH ending 0
int base64_encode_length (unsigned char *input, int len);
// retunr how many byes wroted to output WITHOUT ending 0
int base64_encode( unsigned char *input, int len, unsigned char *output );
// return how many bytes wited to output WITHOUT ending 0. Rerun 0 in base64 are broken
int base64_decode( unsigned char *input, int len, unsigned char *output );

#endif
