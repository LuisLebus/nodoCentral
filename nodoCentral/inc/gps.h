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

#ifndef _GPS_H_
#define _GPS_H_

/*=====[Inclusions of public function dependencies]==========================*/
#include "sapi.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/
typedef struct {
	int32_t lat;
	int32_t lon;
}gpsPoint_t;

/*=====[Prototypes (declarations) of public functions]=======================*/
void gpsSetLatitude(int32_t val);
void gpsSetLongitude(int32_t val);
void gpsSetSpeed(uint8_t val);
void gpsSetValid(uint8_t val);
void gpsSetCourse(uint16_t val);
void gpsSetQuality(uint8_t val);
void gpsSetSat(uint8_t val);

int32_t gpsGetLatitude(void);
int32_t gpsGetLongitude(void);
uint8_t gpsGetSpeed(void);
uint8_t gpsGetvalid(void);
uint16_t gpsGetCourse(void);
uint8_t gpsGetQuality(void);
uint8_t gpsGetSat(void);


/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* _GPS_H_ */
