#include "clib.h"
#include <stdarg.h>  /*need to using the va_list and some var.*/
void my_puts(char *msg)
{
  if (!msg) return;
  fio_write(STDOUT_FROM_UART , msg , strlen(msg));
}

int str2int(char *str)
{
 int i=0,tmp=0;
 while(str[i]!='\0')
 {
  if(str[i]>='0'&&str[i]<='9')  tmp=tmp*10+(str[i]-'0');
  else return FORMAT_CONVERT_ERROR;

  i++;
 }
 return tmp;
}

void int2str(int in , char*out )
{
  int i, number_len=0;
  char out_tmp[10];

  if(in == 0)
  {
    out[0] = '0';
    out[1] = '\0';
    return ;
  }

  while(in > 0){
     out_tmp[number_len] = '0' +  (in % 10);
     in /= 10;
     number_len++;
  }
  
  for(i=0; i<number_len; i++){
     out[i] = out_tmp[number_len-1-i] ;
  }    

  out[number_len] = '\0';
}
char *itoa(int num, unsigned int base){
    static char buf[32]={0};
    int i;
    if(num==0){
        buf[30]='0';
        return &buf[30];
    }
    int negative=(num<0);
    if(negative) num=-num;
    for(i=30; i>=0&&num; --i, num/=base)
        buf[i] = "0123456789ABCDEF" [num % base];
    if(negative){
        buf[i]='-';
        --i;
    }
    return buf+i+1;
}

char *utoa(unsigned int num, unsigned int base){
    static char buf[32]={0};
    int i;
    if(num==0){
        buf[30]='0';
        return &buf[30];
    }
    for(i=30; i>=0&&num; --i, num/=base)
        buf[i] = "0123456789ABCDEF" [num % base];
    return buf+i+1;
} 
