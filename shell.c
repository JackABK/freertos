#include "fio.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "clib.h"
#include "errno.h"
#include "memtest.h"
#define BACKSPACE (127)
typedef struct 
{
	char *name;
	char *desc;
	void (*cmd_func_handler)(void);	
} cmd_list;

/*pre-define avilable commands list*/
static void help_menu(void);
static void ps_cmd(void);
static void ls_cmd(void);
static void cat_cmd(void);
void memtest(void);

/*command list table*/
static cmd_list available_cmds[] = {
	{
		.name = "help" ,
		.desc = "help menu:" , 
		.cmd_func_handler = help_menu 
	},
	{
		.name = "ps" ,
		.desc = "Run the ps command" , 
		.cmd_func_handler = ps_cmd 

	}, 
	{
		.name = "ls" ,
		.desc = "list directory contents" , 
		.cmd_func_handler = ls_cmd
	},
	{
		.name = "cat" ,
		.desc = "concatenate files and print on the standard output" ,
		.cmd_func_handler = cat_cmd
	},
	{
		.name = "memtest" ,
		.desc = "malloc testing" ,
		.cmd_func_handler = memtest
	}
};
static void help_menu(void)
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
void cat_cmd(void)
{
	/*will complete*/
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
