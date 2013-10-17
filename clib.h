#ifndef __CLIB_H__
#define __CLIB_H__
#define FORMAT_CONVERT_ERROR -1  
#define STDOUT_FROM_UART 1  //Write buffer to fd 1 (stdout, through uart)
void my_puts(char *msg);
int str2int(char *str);
void int2str(int in , char*out );
char *itoa(int i, unsigned int base);
char *utoa(unsigned int i, unsigned int base);
int printf(const char *format, ...);
#endif
