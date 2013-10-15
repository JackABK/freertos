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
        //while (!xQueueSendToBack(serial_str_queue, &msg, portMAX_DELAY));
    }
}   
