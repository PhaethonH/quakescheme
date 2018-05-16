#ifndef _QSPRIMLIB_H_
#define _QSPRIMLIB_H_

#include "qsmach.h"
#include "qsprimreg.h"

/* QuakeScheme Primitives. */

/* available argument types:
   * w: qsword
   * p: qsptr_t (tagged pointer)
   * i: int
   * s: char*
   * f: float

   Indicates what decoder to use for arguments in calling C function (qsobj.*).
*/


int qsprimlib_init(qsprimreg_t * qsprims);


#endif // _QSPRIMLIB_H_
