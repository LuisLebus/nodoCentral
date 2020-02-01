/*=====[Module Name]===========================================================
 * Copyright YYYY Author Compelte Name <author@mail.com>
 * All rights reserved.
 * License: license text or at least name and link 
         (example: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Creation Date: YYYY/MM/DD
 */

/*=====[Inclusion of own header]=============================================*/
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "bt.h"
#include "utils.h"

/*=====[Inclusions of private function dependencies]=========================*/

/*=====[Definition macros of private constants]==============================*/
#define BT_MAX_LENGHT	32

/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/
static uartMap_t uartUsed = UART_232;

static xQueueHandle btRxQueue;
static xQueueHandle btTxQueue;


/*=====[Prototypes (declarations) of private functions]======================*/
static void btOnRx(void* param);
static void btTxTask(void *pvParameters);
static bool_t btControlChecksum(char* line);
static uint8_t btCalculateChecksum(char* line);

/*=====[Implementations of public functions]=================================*/
bool_t btInit(uartMap_t uart, uint32_t baudRate )
{
	uartUsed = uart;

	uartInit(uartUsed, baudRate);
	uartCallbackSet(uartUsed, UART_RECEIVE, btOnRx, NULL);
	uartInterrupt(uartUsed, true);

	btTxQueue = xQueueCreate(BT_QUEUE_SIZE, sizeof(btMessage_t));
	if(btTxQueue == NULL)
		return false;

	btRxQueue = xQueueCreate(BT_QUEUE_SIZE, sizeof(btMessage_t));
	if(btRxQueue == NULL)
		return false;

	xTaskCreate(btTxTask, (signed char *) "btTxTask", configMINIMAL_STACK_SIZE * 2, NULL, (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);

	return true;
}

bool_t btPut(btMessage_t* btMessage, uint32_t msToWait)
{
	bool_t retVal = false;

	if( xQueueSend(btTxQueue, btMessage, msToWait / portTICK_RATE_MS) == pdTRUE)
		retVal = true;

	return retVal;
}

bool_t btGet(btMessage_t* btMessage, uint32_t msToWait)
{
	bool_t retVal = false;

	if( xQueueReceive(btRxQueue, btMessage, msToWait / portTICK_RATE_MS) == pdTRUE)
		retVal = true;

	return retVal;
}

/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/
static void btOnRx(void* param)
{
	static volatile char arrayLine[BT_MAX_LENGHT] = {0};
	static volatile char* line = arrayLine;
	static volatile uint8_t i = 0;
	char val = 0;
	char strAux[16] = {0};

	btMessage_t btMessage = {0};

	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	val = uartRxRead(uartUsed);

	if(val == '$')
	{
		i = 0;
		line[i] = val;
		i++;
	}
	else
	{
		line[i] = val;

		if( ( (val == '\n') || (val == '\r') ) && (line[0] == '$') )
		{
			line[i + 1] = 0;

			if( btControlChecksum( (char*)line ) )
			{
				line++;
				line += getSubStr( strAux, (char*)line ) + 1;
				btMessage.cmd = (uint8_t)strtol(strAux, NULL, 10);

				line += getSubStr( strAux, (char*)line ) + 1;
				btMessage.type = (uint8_t)strtol(strAux, NULL, 10);

				strncpy((char*)&btMessage.data, (char*)line, strlen( (char*)line ) - 4);

				xQueueSendFromISR(btRxQueue, &btMessage, &xHigherPriorityTaskWoken);
			}
		}

		if(i < (BT_MAX_LENGHT - 1))
			i++;
		else
			i = 0;
	}

}

static void btTxTask(void *pvParameters)
{
	char line[BT_MAX_LENGHT] = {0};
	btMessage_t btMessage;
	uint8_t checksum;

	while (1)
	{
		if( xQueueReceive(btTxQueue, &btMessage, portMAX_DELAY) == pdTRUE)
		{
			stdioSprintf(line, "$%u,%u,%s,*", btMessage.cmd, btMessage.type, btMessage.data);
			checksum = btCalculateChecksum(line);
			stdioSprintf(line, "%s%02x", line, checksum);
			uartWriteString(uartUsed, line);
		}
	}
}


static bool_t btControlChecksum(char* line)
{
	bool_t retVal = false;
	uint8_t checksumOk = 0;
	uint8_t checksumTest = 0;

	line++;

	while( (*line != '*') && (*line != '\r') && (*line != '\n') && (*line != '\0'))
	{
		checksumOk ^= *line;
		line++;
	}

	line++;
	if( (*line >= '0') && (*line <= '9') )
	{
		checksumTest = *line - '0';
	}
	else if( (*line = 'A') && (*line <= 'F') )
	{
		checksumTest = *line - 'A' + 10;
	}

	line++;
	if( (*line >= '0') && (*line <= '9') )
	{
		checksumTest = (checksumTest * 0x10) + (*line - '0');
	}
	else if( (*line = 'A') && (*line <= 'F') )
	{
		checksumTest = (checksumTest * 0x10) + (*line - 'A' + 10);
	}

	if(checksumOk == checksumTest)
		retVal = true;

	return retVal;
}


static uint8_t btCalculateChecksum(char* line)
{
	uint8_t checksum = 0;

	line++;

	while( (*line != '*') && (*line != '\r') && (*line != '\n') && (*line != '\0'))
	{
		checksum ^= *line;
		line++;
	}

	return checksum;
}
