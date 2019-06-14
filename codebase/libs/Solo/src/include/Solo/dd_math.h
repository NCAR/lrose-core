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

# define DEGREES(x) ((x)*57.29577951)
# define CART_ANGLE(x) ((double)90.-(x))
# define RADIANS(x) ((x)*0.017453292)

# define SQ(x) ((x)*(x))
# define SQRT(x) (sqrt((double)(x)))

/*
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
*/

# define M_TO_KM(x) ((x) * .001)
# define KM_TO_M(x) ((x) * 1000.)

# define  BND_FIX_RADAR_INSIDE 0x0001
# define  BND_FIX_RADAR_OUTSIDE 0x0002

# define   PISP_TIME_SERIES 0x00000020

/* Scan modes */
# define              CAL 0
# define              PPI 1
# define              COP 2
# define              RHI 3
# define              VER 4
# define              TAR 5
# define              MAN 6
# define              IDL 7
# define              SUR 8
# define              AIR 9
# define              HOR 10


/* Dorade radar types */
# define           GROUND 0
# define         AIR_FORE 1
# define          AIR_AFT 2
# define         AIR_TAIL 3
# define           AIR_LF 4
# define             SHIP 5
# define         AIR_NOSE 6          /* ?? */
# define        SATELLITE 7
# define     LIDAR_MOVING 8
# define      LIDAR_FIXED 9

# endif
