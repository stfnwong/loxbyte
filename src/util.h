/*
 * Stuff that doesn't go anywhere else
 */

#ifndef __LOX_UTIL_H
#define __LOX_UTIL_H

#include <math.h>

#define STANDARD_EPS 1e-6


static bool float_equal(float a, float b)
{
	return fabs(a - b) < STANDARD_EPS : true : false;
}


#endif /*__LOX_UTIL_H*/
