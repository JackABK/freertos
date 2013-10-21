#define USE_STDPERIPH_DRIVER

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
#include "clib.h"

/*The task priority*/
#define SHELL_TASK_PRIORITY                (tskIDLE_PRIORITY + 3)
#define TEST_UNIT_TASK_PRIORITY            (tskIDLE_PRIORITY+2) 

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
	//            512 /* stack size */, NULL, TEST_UNIT_TASK_PRIORITY , NULL);

	/*Create a task to shell task*/
	xTaskCreate(shell_task,
	            (signed portCHAR *) "Shell_task",
	            512 /* stack size */, NULL, SHELL_TASK_PRIORITY , NULL);


	/* Start running the tasks. */
	vTaskStartScheduler();
    
	return 0;
}
void vApplicationTickHook()
{
}
