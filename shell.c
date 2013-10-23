#include "fio.h"
#include <stddef.h>
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "clib.h"
#include "errno.h"
#include "memtest.h"
#include "filedump.h"
#define BACKSPACE (127)
typedef struct 
{
	char *name;
	char *desc;
	void (*cmd_func_handler)(void);	
} cmd_list;

/*pre-define avilable commands list*/
static void help_cmd(void);
static void ps_cmd(void);
static void ls_cmd(void);
static void cat_cmd(int argc , char * argv[]);
void memtest_cmd(void);

/* using preprocessor to beautify the command , refer to: 
 * 1.http://gcc.gnu.org/onlinedocs/cpp/Stringification.html 
 * 2.http://stackoverflow.com/questions/216875/in-macros/216912
*/
#define DECLARE_COMMAND(n, d) {.name=#n, .cmd_func_handler=n ## _cmd, .desc=d}
static cmd_list available_cmds[] = {
	DECLARE_COMMAND(ls,"list directory contents"),
	DECLARE_COMMAND(ps,"Run the ps command"),
	DECLARE_COMMAND(cat,"concatenate files and print on the standard output"),
	DECLARE_COMMAND(help,"help menu"),
	DECLARE_COMMAND(memtest,"malloc testing")
};

static void help_cmd(void)
{
	int i;
	my_puts("available command list :\r\n");
	for (i = 0; i < sizeof(available_cmds)/sizeof(cmd_list); i++) {
		printf("%s\t\t%s\r\n",available_cmds[i].name,available_cmds[i].desc);
	}
}
void ps_cmd(void)
{
	char buf[1024];
	printf("Task List :\n\r");
	vTaskList(buf);
	printf("\n\r%s\n\r",buf);
}
void ls_cmd(void)
{
	/*will complete*/
}
void cat_cmd(int argc , char * argv[])
{
	int argv_index=1;
	if(argc < 2){
		printf("Usage: cat <filename>\n\r");
		return;
	}
	for(;;){
		if(!filedump(argv[argv_index])){
			printf("%s No such file or directory.\n\r",argv[argv_index]);
			return;
		}
		else{ 
			argv_index++;
		}
	}
}
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
			else if (ch == BACKSPACE){ /*backspace*/
				if (curr_char>0) {
					curr_char--;
			    	printf("\b \b");
				}
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
void proc_cmd(char *cmd){
    int i;
    char string_tmp[10];
	for (i = 0; i < sizeof(available_cmds)/sizeof(cmd_list); i++) {          
		if(strncmp(cmd, available_cmds[i].name , strlen(available_cmds[i].name)) == 0){
			available_cmds[i].cmd_func_handler();
			return;
		}      
    }
	/*not found the command from cmd_list*/
    my_puts("command not found !!\r\n");
}
