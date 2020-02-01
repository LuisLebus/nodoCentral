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

#include "straightLine.h"

/*=====[Inclusions of private function dependencies]=========================*/

/*=====[Definition macros of private constants]==============================*/

/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/
static gpsPoint_t pointA;
static gpsPoint_t pointB;
static uint32_t distanceAB = 0;

static xTaskHandle distanceABTaskHandle = NULL;

/*=====[Prototypes (declarations) of private functions]======================*/

/*=====[Implementations of public functions]=================================*/
bool_t stLineInit(void)
{
	return true;
}

void stLineSetA(void)
{
	gpsPoint_t currentPoint;

	currentPoint.lat = gpsGetLatitude();
	currentPoint.lat = gpsGetLongitude();

	gpsGradeToCm(&currentPoint, &pointA);

	if(distanceABTaskHandle != NULL)
	{
		vTaskDelete(distanceABTaskHandle);
		distanceABTaskHandle = NULL;
	}

	xTaskCreate(distanceABTask, (signed char *) "distanceABTask", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), &distanceABTaskHandle);
}

void stLineSetB(void)
{
	gpsPoint_t currentPoint;

	if(distanceAB > ST_LINE_MINIMAL_DISTANCE_AB)
	{
		currentPoint.lat = gpsGetLatitude();
		currentPoint.lat = gpsGetLongitude();

		gpsGradeToCm(&currentPoint, &pointB);


		//TODO: calcular el angulo de la recta, eliminar la tarea que mide la distancia AB,
		//lanzar la tarea que calcula el error


	}
	else
	{
		//TODO: enviar un error
	}

}

uint32_t stLineGetDistanceAB(void)
{
	return distanceAB;
}

/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/
static void distanceABTask(void *pvParameters)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();

	distanceAB = 0;

	while(1)
	{
		vTaskDelayUntil( &xLastWakeTime, 1000 / portTICK_PERIOD_MS );

		distanceAB += ( gpsGetSpeed() / 3.6 );
	}
}

static void processStLineTask(void *pvParameters)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();


	while(1)
	{
		vTaskDelayUntil( &xLastWakeTime, 1000 / portTICK_PERIOD_MS );

		//TODO: tomar el punto actual, pasarlo a cm, calcular la distancia a la recta AB inicial,
		//identificar la pasada o setpoint segun ancho de semabradora,
		//calcular el error en cm, calcular el angulo conrespecto al punto anterior,
		//reservar los resultados, la app los consultará eventualmente



	}
}
