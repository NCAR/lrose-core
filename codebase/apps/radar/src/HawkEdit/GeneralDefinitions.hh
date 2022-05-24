#ifndef GENERALDEFINITIONS_H
#define GENERALDEFINITIONS_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// use DoradeData.hh
//# define CART_ANGLE(x) ((double)90.-(x))

#define dd_isnanf(x)       (((*(long *)&(x) & 0x7f800000L) == 0x7f800000L) && \
                            ((*(long *)&(x) & 0x007fffffL) != 0x00000000L))
# define SQRT(x) (sqrt((double)(x)))
# define SQ(x) ((x)*(x))
// # define RADIANS(x)  ((x)*0.017453292)
#ifndef PI
# define PI 3.141592654
#endif
# define PIOVR2 1.570796327
# define TWOPI 6.283185307
# define DD_SCALE(x,scale,offset) ((x)*(scale)+(offset)+.5)

// use DoradeData.hh radar_type_t or lidar_type_t
/* Dorade radar types */
/*
# define           GROUND 0
# define         AIR_FORE 1
# define          AIR_AFT 2
# define         AIR_TAIL 3
# define           AIR_LF 4
# define             SHIP 5
# define         AIR_NOSE 6         
# define        SATELLITE 7
# define     LIDAR_MOVING 8
# define      LIDAR_FIXED 9
*/

#endif
