/* 	$Id: Sweep.h,v 1.1 2007/09/22 21:08:21 dixon Exp $	 */

# ifndef INCSweeph
# define INCSweeph

#include <dataport/port_types.h>

struct sweepinfo_d {
    char sweep_des[4];	      /* Comment descriptor identifier: ASCII */
			      /* characters "SWIB" stand for sweep info */
			      /* block Descriptor. */
    si32  sweep_des_length;   /* Sweep  descriptor length in bytes. */
    char  radar_name[8];      /* comment*/
    si32  sweep_num;          /*Sweep number from the beginning of the volume*/
    si32  num_rays;            /*number of rays recorded in this sweep*/
    fl32  start_angle;         /*true start angle [deg]*/
    fl32  stop_angle;          /*true stop angle  [deg]*/
    fl32  fixed_angle;
    si32  filter_flag;

}; /* End of Structure */



typedef struct sweepinfo_d sweepinfo_d;
typedef struct sweepinfo_d SWEEPINFO;

# endif /* ifndef INCSweeph */
