/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* 	$Id: Sweep.h,v 1.3 2016/03/07 01:23:00 dixon Exp $	 */

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
