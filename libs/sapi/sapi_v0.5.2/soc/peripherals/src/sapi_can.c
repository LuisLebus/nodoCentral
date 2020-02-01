/* Copyright ---.
 * Copyright ---.
 * Copyright ---.
 * All rights reserved.
 *
 * This file is part sAPI library for microcontrollers.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* Date: 2016-02-26 */

/*==================[inclusions]=============================================*/
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "sapi_can.h"

/*==================[macros]=================================================*/
#define CAN_QUEUE_SIZE		5

/*==================[typedef]================================================*/
typedef struct {
	LPC_CCAN_T*      	addr;
	lpc4337ScuPin_t 	txPin;
	lpc4337ScuPin_t 	rxPin;
	IRQn_Type        	irqAddr;
}canModule_t;

typedef struct {
	callBackFuncPtr_t 	rxIsrCallback;
	void* 				rxIsrCallbackParams;
} canCallback_t;

/*==================[internal data declaration]==============================*/
static const canModule_t canModule[CAN_MAX] = {
		// { canAddr, { txPort, txpin, txfunc }, { rxPort, rxpin, rxfunc }, canIrqAddr },

		// CAN0 (P3_2 = TX, P3_1 = RX)
		{ LPC_C_CAN0, { 3, 2, FUNC2}, { 3, 1, FUNC2}, C_CAN0_IRQn },	// CAN0
};

static canCallback_t canCallback[CAN_MAX] = {
		// { rxCallback, rxCallbackParam },
		{ 0, NULL },	// CAN0
};

static xQueueHandle canRxQueue[CAN_MAX] = { NULL };
static xQueueHandle canTxQueue[CAN_MAX] = { NULL };

/*==================[internal functions declaration]=========================*/
static void canProcessIRQ(canMap_t can);

/*==================[internal functions definition]==========================*/
static void canProcessIRQ(canMap_t can)
{
	canMessage_t canMessage;
	uint32_t int_reg;
	uint32_t *pData;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	while( 0 != ( int_reg = Chip_CCAN_GetIntID(canModule[can].addr) ) )
	{
		if( (CCAN_INT_STATUS == int_reg) && ( CCAN_STAT_RXOK & Chip_CCAN_GetStatus(canModule[can].addr) ) )
		{
			if(canModule[can].addr->IF[1].MCTRL & CCAN_IF_MCTRL_NEWD)
			{
				canMessage.id = (canModule[can].addr->IF[1].ARB1) | (canModule[can].addr->IF[1].ARB2 << 16);
				canMessage.dlc = canModule[can].addr->IF[1].MCTRL & CCAN_IF_MCTRL_DLC_MSK;

				pData = (uint32_t*)canMessage.data;
				*pData++ = (canModule[can].addr->IF[1].DA2 << 16) | canModule[can].addr->IF[1].DA1;
				*pData = (canModule[can].addr->IF[1].DB2 << 16) | canModule[can].addr->IF[1].DB1;

				if (canMessage.id & ( 0x1 << 30) )
				{
					canMessage.id &= CCAN_MSG_ID_EXT_MASK;
				}
				else
				{
					canMessage.id >>= 18;
					canMessage.id &= CCAN_MSG_ID_STD_MASK;
				}
			}

			if(canRxQueue[can] != NULL)
				xQueueSendFromISR(canRxQueue[can], &canMessage, &xHigherPriorityTaskWoken);

			// Execute callback
			if(canCallback[can].rxIsrCallback != 0)
				(*canCallback[can].rxIsrCallback)(0);

			Chip_CCAN_ClearStatus(canModule[can].addr, CCAN_STAT_RXOK);
		}
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void canTxTask(void *pvParameters)
{
	canMessage_t canMessage;
	CCAN_MSG_OBJ_T send_obj;
	uint8_t i;

	while (1)
	{
		for(i=0; i<CAN_MAX; i++)
		{
			if(canTxQueue[i] != NULL)
			{
				if( xQueueReceive(canTxQueue[i], &canMessage, 0) == pdTRUE)
				{
					send_obj.dlc = canMessage.dlc;
					memcpy(&send_obj.data, &canMessage.data, send_obj.dlc);
					send_obj.id = canMessage.id;

					if(canMessage.idType == CAN_EXTENDED_ID)
						send_obj.id |= CAN_EXTEND_ID_USAGE;

					Chip_CCAN_Send(canModule[i].addr, CCAN_MSG_IF1, false, &send_obj);
					Chip_CCAN_ClearStatus(canModule[i].addr, CCAN_STAT_TXOK);
				}
			}
		}

		taskYIELD();
	}
}

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
// CAN Global Interrupt Enable/Disable
void canInterrupt( canMap_t can, bool_t enable )
{
	if(enable)
	{
		// Interrupt Priority for CAN channel
		NVIC_SetPriority( canModule[can].irqAddr, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY ); // FreeRTOS Requiere prioridad >= 5 (numero mas alto, mas baja prioridad)

		// Enable Interrupt for CAN channel
		NVIC_EnableIRQ(canModule[can].irqAddr);
	}
	else
	{
		// Disable Interrupt for CAN channel
		NVIC_DisableIRQ(canModule[can].irqAddr);
	}
}

// CAN Interrupt event Enable and set a callback
void canCallbackSet(canMap_t can, canEvents_t event, callBackFuncPtr_t callbackFunc, void* callbackParam)
{   
	uint32_t intMask = 0;

	switch(event)
	{
	case CAN_RECEIVE:
		// Enable CAN Receiver Buffer Register Interrupt
		intMask |= (CCAN_CTRL_IE | CCAN_CTRL_SIE | CCAN_CTRL_EIE);

		if(callbackFunc != 0)
		{
			// Set callback
			canCallback[can].rxIsrCallback = callbackFunc;
			canCallback[can].rxIsrCallbackParams = callbackParam;
		}

		// Enable CAN Interrupt
		Chip_CCAN_EnableInt(canModule[can].addr, intMask);

		break;
	}
}

// CAN Interrupt event Disable
void canCallbackClr(canMap_t can, canEvents_t event)
{
	uint32_t intMask = 0;

	switch(event)
	{
	case CAN_RECEIVE:
		// Disable CAN Receiver Buffer Register Interrupt
		intMask |= (CCAN_CTRL_IE | CCAN_CTRL_SIE | CCAN_CTRL_EIE);

		// Set callback
		canCallback[can].rxIsrCallback = 0;
		canCallback[can].rxIsrCallbackParams = NULL;

		// Disable CAN Interrupt
		Chip_CCAN_DisableInt(canModule[can].addr, intMask);
		break;
	}
}

void canInit(canMap_t can, uint32_t baudRate)
{
	// Configure CANn_TXD uartPin
	Chip_SCU_PinMuxSet(
			canModule[can].txPin.lpcScuPort,
			canModule[can].txPin.lpcScuPin,
			SCU_MODE_INACT | canModule[can].txPin.lpcScuFunc );

	// Configure CANn_RXD uartPin
	Chip_SCU_PinMuxSet(
			canModule[can].rxPin.lpcScuPort,
			canModule[can].rxPin.lpcScuPin,
			SCU_MODE_INACT | SCU_MODE_INBUFF_EN | canModule[can].rxPin.lpcScuFunc );

	/* Set CCAN peripheral clock under 100Mhz for working stable */
	Chip_Clock_SetBaseClock(CLK_BASE_APB3, CLKIN_IDIVC, true, false);
	Chip_CCAN_Init(canModule[can].addr);
	Chip_CCAN_SetBitRate(canModule[can].addr, baudRate);

	canRxQueue[can] = xQueueCreate(CAN_QUEUE_SIZE, sizeof(canMessage_t));
	canTxQueue[can] = xQueueCreate(CAN_QUEUE_SIZE, sizeof(canMessage_t));

	xTaskCreate(canTxTask, (signed char *) "canTxTask", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);
}

bool_t canPut(canMap_t can, canMessage_t* canMessage, uint32_t msToWait)
{
	bool_t retVal = false;

	if( xQueueSend(canTxQueue[can], canMessage, msToWait / portTICK_RATE_MS) == pdTRUE)
		retVal = true;

	return retVal;
}

bool_t canGet(canMap_t can, canMessage_t* canMessage, uint32_t msToWait)
{
	bool_t retVal = false;

	if( xQueueReceive(canRxQueue[can], canMessage, msToWait / portTICK_RATE_MS) == pdTRUE)
		retVal = true;

	return retVal;
}

void canSetFilter(canMap_t can, uint32_t filter)
{
	Chip_CCAN_AddReceiveID(canModule[can].addr, CCAN_MSG_IF1, filter);
}

void canClearFilter(canMap_t can, uint32_t filter)
{
	Chip_CCAN_DeleteReceiveID(canModule[can].addr, CCAN_MSG_IF1, filter);
}

void canDisableFilter(canMap_t can)
{
	Chip_CCAN_EnableTestMode(canModule[can].addr);
	Chip_CCAN_ConfigTestMode(canModule[can].addr, CCAN_TEST_BASIC_MODE);
}

/*==================[ISR external functions definition]======================*/

//__attribute__ ((section(".after_vectors")))

void CAN0_IRQHandler(void)
{
	canProcessIRQ(CAN0);
}

/*==================[end of file]============================================*/
