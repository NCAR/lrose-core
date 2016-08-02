/* 	$Id: dd_math.h,v 1.1 2008/01/24 22:22:32 rehak Exp $	 */

# ifndef DDMATHH
# define DDMATHH

# include <math.h>

#define dd_isnanf(x)       (((*(long *)&(x) & 0x7f800000L) == 0x7f800000L) && \
                            ((*(long *)&(x) & 0x007fffffL) != 0x00000000L))

# define FMOD360(x) (fmod((double)((x)+720.), (double)360.))
# define EXP2(x) (pow((double)2.0, (double)(x)))
# define EXPN(x) (pow((double)2.718281828, (double)(x)))
# define EXP10(x) (pow((double)10.0, (double)(x)))

# define WATTZ(x) (pow((double)10.0, (double)((x) * .1)))
# define W_TO_DBM(x) (10. * log10((double)(x)))

# define LOG10(x) (log10((double)(x)))
# define LOGN(x) (log((double)(x)))
# define SQRT(x) (sqrt((double)(x)))
# define FABS(x) (fabs((double)(x)))
# define SIN(x) (sin((double)(x)))
# define COS(x) (cos((double)(x)))
# define TAN(x) (tan((double)(x)))
# define ATAN2(y,x) (atan2((double)(y), (double)(x)))
# define ASIN(x) (asin((double)(x)))
# define ACOS(x) (acos((double)(x)))
# define ATAN(x) (atan((double)(x)))

# define M_TO_KM(x) ((x) * .001)
# define KM_TO_M(x) ((x) * 1000.)

# endif
