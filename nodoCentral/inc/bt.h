/*=====[Module Name]===========================================================
 * Copyright YYYY Author Compelte Name <author@mail.com>
 * All rights reserved.
 * License: license text or at least name and link 
         (example: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Creation Date: YYYY/MM/DD
 */

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef _BT_H_
#define _BT_H_

/*=====[Inclusions of public function dependencies]==========================*/
#include "sapi.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/
#define BT_DATA_LENGTH	32

#define BT_MSG_TYPE_WAIT	0
#define BT_MSG_TYPE_ERROR	1
#define BT_MSG_TYPE_REPLY	2
#define BT_MSG_TYPE_READ	3
#define BT_MSG_TYPE_WRITE	4

#define BT_MSG_CMD_ST_LINE_SET_A			1
#define BT_MSG_CMD_ST_LINE_SET_B			2
#define BT_MSG_CMD_CU_LINE_SET_A			3
#define BT_MSG_CMD_CU_LINE_SET_B			4

#define BT_BAUDRATE_4800BITS		4800
#define BT_BAUDRATE_9600BITS		9600
#define BT_BAUDRATE_19200BITS		19200
#define BT_BAUDRATE_115200BITS		115200

#define BT_QUEUE_SIZE		5

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/
typedef struct {
	uint8_t cmd;
	uint8_t type;
	char data[BT_DATA_LENGTH];
}btMessage_t;

/*=====[Prototypes (declarations) of public functions]=======================*/
bool_t btInit(uartMap_t uart, uint32_t baudRate );
bool_t btPut(btMessage_t* btMessage, uint32_t msToWait);
bool_t btGet(btMessage_t* btMessage, uint32_t msToWait);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* _BT_H_ */
