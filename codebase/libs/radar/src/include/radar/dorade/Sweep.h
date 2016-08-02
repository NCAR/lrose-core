/* 	$Id: Sweep.h,v 1.1 2008/01/24 22:22:32 rehak Exp $	 */

# ifndef INCSweeph
# define INCSweeph

struct sweepinfo_d {
    char sweep_des[4];	      /* Comment descriptor identifier: ASCII */
			      /* characters "SWIB" stand for sweep info */
			      /* block Descriptor. */
    long  sweep_des_length;   /* Sweep  descriptor length in bytes. */
    char  radar_name[8];      /* comment*/
    long  sweep_num;          /*Sweep number from the beginning of the volume*/
    long  num_rays;            /*number of rays recorded in this sweep*/
    float start_angle;         /*true start angle [deg]*/
    float stop_angle;          /*true stop angle  [deg]*/
    float fixed_angle;
    long  filter_flag;

}; /* End of Structure */



typedef struct sweepinfo_d sweepinfo_d;
typedef struct sweepinfo_d SWEEPINFO;

# endif /* ifndef INCSweeph */
