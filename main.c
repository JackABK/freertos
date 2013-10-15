#define USE_STDPERIPH_DRIVER
#define FORMAT_CONVERT_ERROR -1  
#define STDOUT_FROM_UART 1  //Write buffer to fd 1 (stdout, through uart)

#include "stm32f10x.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Filesystem includes */
#include "filesystem.h"
#include "fio.h"
#include "shell.h"
extern const char _sromfs;

static void setup_hardware();

volatile xSemaphoreHandle serial_tx_wait_sem = NULL;
volatile xQueueHandle serial_rx_queue = NULL ; 
volatile xQueueHandle serial_str_queue = NULL ; 

/* Queue structure used for passing messages. */
typedef struct {
    char str[100];
} serial_str_msg;

/* Queue structure used for passing characters. */
typedef struct {
    char ch;
} serial_ch_msg;

/* IRQ handler to handle USART2 interruptss (both transmit and receive
 * interrupts). */
void USART2_IRQHandler()
{
	static signed portBASE_TYPE xHigherPriorityTaskWoken;
    serial_ch_msg rx_msg;
	/* If this interrupt is for a transmit... */
	if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
		/* "give" the serial_tx_wait_sem semaphore to notfiy processes
		 * that the buffer has a spot free for the next byte.
		 */
		xSemaphoreGiveFromISR(serial_tx_wait_sem, &xHigherPriorityTaskWoken);

		/* Diables the transmit interrupt. */
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		/* If this interrupt is for a receive... */
	}
	else if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        /* Receive the byte from the buffer. */
        rx_msg.ch = USART_ReceiveData(USART2);                                                                                                            

        /* Queue the received byte. */
        if(!xQueueSendToBackFromISR(serial_rx_queue, &rx_msg, &xHigherPriorityTaskWoken)) {
            /* If there was an error queueing the received byte,
             * freeze. */
            while(1);
        }
    }
	else {
		/* Only transmit and receive interrupts should be enabled.
		 * If this is another type of interrupt, freeze.
		 */
		while(1);
	}
	if (xHigherPriorityTaskWoken) {
		taskYIELD();
	}
}

void send_byte(char ch)
{
	/* Wait until the RS232 port can receive another byte (this semaphore
	 * is "given" by the RS232 port interrupt when the buffer has room for
	 * another byte.
	 */
	while (!xSemaphoreTake(serial_tx_wait_sem, portMAX_DELAY));

	/* Send the byte and enable the transmit interrupt (it is disabled by
	 * the interrupt).
	 */
	USART_SendData(USART2, ch);
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}
char receive_byte()
{
    serial_ch_msg msg ; 

    /* Wait for a byte to be queued by the receive interrupts handler. */
    while (!xQueueReceive(serial_rx_queue, &msg, portMAX_DELAY));
    return msg.ch ; 

}
void testing_unit_task (void *pvParameters)
{
        char ch;
        char str_tmp[20] = "testing unit task\n";
		fio_write(STDOUT_FROM_UART, str_tmp, 19);
}
int main()
{
	init_rs232();
	enable_rs232_interrupts();
	enable_rs232();
	
	fs_init();
	fio_init();
	register_romfs("romfs", &_sromfs);

	/* Create the queue used by the serial task.  Messages for write to
	 * the RS232. 
	 * the queue create usage : 
     * xQueueHandle xQueueCreate xQueueCreate
                                   (
                                     unsigned portBASE_TYPE uxQueueLength , 
                                     unsigned portBASE_TYPE uxItemSize
                                   );
     */
	vSemaphoreCreateBinary(serial_tx_wait_sem);
	serial_str_queue = xQueueCreate(10, sizeof(serial_str_msg));
    serial_rx_queue = xQueueCreate(1, sizeof(char));

    /*Create a task to testing_unit_task*/ 
	//xTaskCreate(testing_unit_task,
	//            (signed portCHAR *) "Testing_unit_task",
	//            512 /* stack size */, NULL, tskIDLE_PRIORITY+3 , NULL);

	/*Create a task to shell task*/
	xTaskCreate(shell_task,
	            (signed portCHAR *) "Shell_task",
	            512 /* stack size */, NULL, tskIDLE_PRIORITY+10 , NULL);


	/* Start running the tasks. */
	vTaskStartScheduler();
    
	return 0;
}

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
void vApplicationTickHook()
{
}
