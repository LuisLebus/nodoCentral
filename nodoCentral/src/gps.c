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
#include "gps.h"

/*=====[Inclusions of private function dependencies]=========================*/

/*=====[Definition macros of private constants]==============================*/

/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/
static int32_t latitude = 0;
static int32_t longitude = 0;
static uint8_t speed = 0;
static uint8_t valid = 0;
static uint16_t course = 0;
static uint8_t quality = 0;
static uint8_t sat = 0;

/*=====[Prototypes (declarations) of private functions]======================*/

/*=====[Implementations of public functions]=================================*/
void gpsSetLatitude(int32_t val)
{
	latitude = val;
}

void gpsSetLongitude(int32_t val)
{
	longitude = val;
}

void gpsSetSpeed(uint8_t val)
{
	speed = val;
}

void gpsSetValid(uint8_t val)
{
	valid = val;
}

void gpsSetCourse(uint16_t val)
{
	course = val;
}

void gpsSetQuality(uint8_t val)
{
	quality = val;
}

void gpsSetSat(uint8_t val)
{
	sat = val;
}

int32_t gpsGetLatitude(void)
{
	return latitude;
}

int32_t gpsGetLongitude(void)
{
	return longitude;
}

uint8_t gpsGetSpeed(void)
{
	return speed;
}

uint8_t gpsGetvalid(void)
{
	return valid;
}

uint16_t gpsGetCourse(void)
{
	return course;
}

uint8_t gpsGetQuality(void)
{
	return quality;
}

uint8_t gpsGetSat(void)
{
	return sat;
}

void gpsGradeToCm( gpsPoint_t* in, gpsPoint_t* out)
{
	out->lat = (in->lat / 1000000.0) * 111111;
	out->lon = (in->lon / 1000000.0) * 111111 * cos( (in->lat / 1000000.0) * ( ( 2 * PI() ) / 360.0) );
}
