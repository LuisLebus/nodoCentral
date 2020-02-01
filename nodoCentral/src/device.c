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
#include "FreeRTOS.h"
#include "task.h"

#include "sapi.h"

#include "device.h"
#include "j1939.h"
#include "bt.h"
#include "gps.h"
#include "impSw.h"
#include "straightLine.h"
#include "curvedLine.h"

/*=====[Inclusions of private function dependencies]=========================*/

/*=====[Definition macros of private constants]==============================*/

/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Prototypes (declarations) of private functions]======================*/
static void devJ1939RxTask(void *pvParameters);
static void devBtRxTask(void *pvParameters);

/*=====[Implementations of public functions]=================================*/
void deviceInit(void)
{
	j1939Init(CAN0, true);
	btInit(UART_232, BT_BAUDRATE_9600BITS);

	xTaskCreate(devJ1939RxTask, (signed char *) "devJ1939RxTask", configMINIMAL_STACK_SIZE * 2, NULL, (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);
	xTaskCreate(devBtRxTask, (signed char *) "devBtRxTask", configMINIMAL_STACK_SIZE * 2, NULL, (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);

	// Iniciar scheduler
	vTaskStartScheduler();
}
/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/
static void devJ1939RxTask(void *pvParameters)
{
	j1939Message_t j1939Message;

	while (1)
	{
		if( j1939Get(&j1939Message, portMAX_DELAY) )
		{
			switch(j1939Message.PDUFormat)
			{
				case DEV_J1939_MSG_GPS_RMC_0:
					gpsSetLatitude( *(int32_t*)j1939Message.data );
					gpsSetLongitude( *(int32_t*)(j1939Message.data + 4) );
					break;
				case DEV_J1939_MSG_GPS_RMC_1:
					gpsSetSpeed( j1939Message.data[0] );
					gpsSetValid( j1939Message.data[1] );
					gpsSetCourse( *(uint16_t*)(j1939Message.data + 2) );
					break;
				case DEV_J1939_MSG_GPS_GGA:
					gpsSetQuality( j1939Message.data[0] );
					gpsSetSat( j1939Message.data[1] );
					break;
				case DEV_J1939_MSG_IMP_SW:
					impSwSetState( j1939Message.data[0] );
					break;
				default:
					stdioPrintf(UART_USB, "PDUFormat invalid!\r\n\r\n");
			}

			gpioToggle(LED1);
		}
	}
}

static void devBtRxTask(void *pvParameters)
{
	btMessage_t btMessage;

	while (1)
	{
		if( btGet(&btMessage, portMAX_DELAY) )
		{
			switch(btMessage.cmd)
			{
				case BT_MSG_CMD_ST_LINE_SET_A:
					if(btMessage.type == BT_MSG_TYPE_WRITE)
					{
						stLineSetA();
					}
					else
					{
						//TODO: enviar error
					}
					break;
				case BT_MSG_CMD_ST_LINE_SET_B:
					if(btMessage.type == BT_MSG_TYPE_WRITE)
					{
						stLineSetB();
					}
					else
					{
						//TODO: enviar error
					}
					break;
				case BT_MSG_CMD_CU_LINE_SET_A:
					if(btMessage.type == BT_MSG_TYPE_WRITE)
					{
						cuLineSetA();
					}
					else
					{
						//TODO: enviar error
					}
					break;
				case BT_MSG_CMD_CU_LINE_SET_B:
					if(btMessage.type == BT_MSG_TYPE_WRITE)
					{
						cuLineSetB();
					}
					else
					{
						//TODO: enviar error
					}
					break;
				default:
					stdioPrintf(UART_USB, "BT invalid!\r\n\r\n");
			}

		}
	}
}
