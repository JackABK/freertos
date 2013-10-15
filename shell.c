#include "string-util.h"
#include "fio.h"
void shell_task(void *pvParameters)
{
    //serial_str_msg msg;
	char str[30];
    char ch;
    int curr_char;
    int done;
    char immediate_ch[2];
    while (1) {
        curr_char = 0;
        done = 0;
		my_puts("$:");
        do {
            /* Receive a byte from the RS232 port (this call will
             * block). */
             fio_read(0 ,&ch , 1 );

            /* If the byte is an end-of-line type character, then
             * finish the string and inidcate we are done.
             */
            if ((ch == '\r') || (ch == '\n')) {
                str[curr_char] = '\n';
                str[curr_char+1] = '\0';
                done = -1;
                /* Otherwise, add the character to the
                 * response string. */
            }
            else {
                str[curr_char++] = ch;
				/*immediate output ch to stdout*/
		        immediate_ch[0] = ch;
				fio_write(1,immediate_ch , 1);
            }
        } while (!done);

        /* Once we are done building the response string, queue the
         * response to be sent to the RS232 port.
         */
	  	 my_puts("\r\n");
		 proc_cmd(str);
    }
}   
void proc_cmd(char *cmd)                                                     {
  int i;
  char string_tmp[10];
  if (!strncmp(cmd , "help",4)){
    my_puts("\rhelp -- Display all command explanation\n");
    my_puts("\recho -- Output the string\n");
    my_puts("\rhello -- Display 'Hello JackABK' \n");
    my_puts("\rps -- Display all tasks\r\n");
  }
  else if(!strncmp(cmd , "echo" , 4)){
    if( (!strncmp(cmd,"echo ",5) && (cmd[5]!=' '))){
      my_puts(&cmd[5]);
      my_puts("\r\n");
    }
    else{
     // print_msg("Please input ");
    }
  }
  else if(!strncmp(cmd ,"hello" , 4)){
    my_puts("Hello! this is JackABK Homework\r\n");
  }
  else{
   // my_puts(cmd);
    my_puts("command not found !!\r\n");
  }
}
