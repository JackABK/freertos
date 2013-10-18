#include "clib.h"
#include <stdarg.h>  /*need to using the va_list and some var.*/
#define ITOA_FLAG 1
#define UTOA_FLAG 0
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
char *itoa_utoa_str_conv(unsigned int num , char * buf , unsigned int base , int flags)
{
	int i, negative=0;

	if (flags==ITOA_FLAG) {
		negative= (int)num<0 ;    
        if(negative) num = -(int)num;
	}
	/*UTOA_FLAG is dont need to set*/

        /******common itoa and utoa*******/
		if(num==0){
    	    buf[30]='0';
        	return &buf[30];
    	}	
		for(i=30; i>=0&&num; --i, num/=base)
	        buf[i] = "0123456789ABCDEF" [num % base];
		if(negative){
   		     buf[i]='-';
   	    	 --i;
   		}
	return buf+i+1;
}
char *itoa(int num, unsigned int base){
    static char buf[32]={0};
    return itoa_utoa_str_conv(num , buf , base ,ITOA_FLAG);	
}
char *utoa(unsigned int num, unsigned int base){
    static char buf[32]={0};
    return itoa_utoa_str_conv(num , buf , base , UTOA_FLAG);	
} 

/*this print can common used for printf and sprintf*/
static int print(char * dest , const char *format, va_list args )
{
  int int_tmp , i;
  char ch_tmp[2] = {0 , 0}; /*second byte is as a stop character */
  char *str_tmp=0;
  char str_out_buf[100];

    for(i=0; format[i]!=0; i++){
        if(format[i]=='%'){
            switch(format[i+1]){
                case 'c':
                case 'C':
                    {
                        ch_tmp[0]    = (char) va_arg(args,int);
                        str_tmp = ch_tmp;
                    } 
                    break;
                case 'd':
                case 'D':
                    {
                        int_tmp = va_arg(args, int);
                        str_tmp = itoa(int_tmp,10);
                    }
                    break;  
                case 'x':
                case 'X':
                case 'S':
                case 's':
                    {
                        str_tmp = va_arg(args, char *);
                    }
                    break;
                case '%':
                    send_byte('%'); break;
                default:
                    {   
                    
                    }
            }
          /* next to char */
          i++;
        }
        else{
             ch_tmp[0] = format[i];
             str_tmp = ch_tmp;
        }
      my_puts(str_tmp);
      strcat(dest , str_tmp);
    }
    va_end(args);
	return i ;
}
int printf(const char *format, ...)
{
    va_list args;
    va_start( args, format );
    /*first argv is 0 since dont return string buffer*/
    return print( 0 , format, args );
}
int sprintf(char *dest , const char *format , ...)
{
    va_list args;
    va_start( args, format );
    return print( &dest, format, args );
}
