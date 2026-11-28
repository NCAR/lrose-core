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
/*
 *	$Id: Parameter.h,v 1.3 2016/03/07 01:23:00 dixon Exp $
 *
 *	Module:		 Parameter.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date: 2016/03/07 01:23:00 $
 *
 * revision history
 * ----------------
 * $Log: Parameter.h,v $
 * Revision 1.3  2016/03/07 01:23:00  dixon
 * Changing copyright/license to BSD
 *
 * Revision 1.2  2012/05/17 01:49:05  dixon
 * Moving port_includes up into main source dir
 *
 * Revision 1.1  2007-09-22 21:08:21  dixon
 * adding
 *
 * Revision 1.1  2002/02/07 18:31:35  dixon
 * added
 *
 * Revision 1.1  1996/12/20 19:55:32  oye
 * New.
 *
 * Revision 1.5  1996/01/08  16:41:54  oye
 * Added some comments and cell vector information unique to this parameter.
 *
 * Revision 1.4  1995/12/13  17:10:34  oye
 * Added an eff_unamb_vel (Nyquist velocity) for each parameter.
 *
 * Revision 1.3  1995/10/25  17:00:32  oye
 * New extended parameter descriptor. This header also includes the older
 * parameter descriptor named "struct parameter_d_v00".
 *
 * Revision 1.2  1994/04/05  15:32:47  case
 * moved an ifdef RPC and changed an else if to make HP compiler happy.
 *
 * Revision 1.5  1992/07/28  17:31:33  thor
 * Removed filter_flag.
 *
 * Revision 1.4  1992/04/20  17:18:31  thor
 * Latest Eldora/Asterea revisions included.
 *
 * Revision 1.3  1991/11/25  20:07:45  thor
 * Added filter flag.
 *
 * Revision 1.2  1991/10/15  17:56:03  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.1  1991/08/30  18:39:32  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCParameterh
#define INCParameterh

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else
#if defined(WRS)
#include "rpc/rpc.h"
#endif
#endif /* UNIX */

#endif /* OK_RPC */

struct parameter_d {
    char  parameter_des[4];	/* Parameter Descriptor identifier */
				/* (ascii characters "PARM"). */
    si32 parameter_des_length;	/* Parameter Descriptor length in */
				/* bytes.*/
    char  parameter_name[8];	/* Name of parameter being described. */
    char  param_description[40]; /* Detailed description of this parameter. */
    char  param_units[8];	/* Units parameter is written in. */
    si16 interpulse_time;	/* Inter-pulse periods used. bits 0-1 */
				/* = frequencies 1-2. */
    si16 xmitted_freq;		/* Frequencies used for this */
				/* parameter. */
    fl32 recvr_bandwidth;	/* Effective receiver bandwidth for */
				/* this parameter in MHz.*/
    si16 pulse_width;		/* Effective pulse width of parameter */
				/* in m. */
    si16 polarization;		/* Polarization of the radar beam for */
				/* this parameter (0 Horizontal,1 */
				/* Vertical,2 Circular,3 Elliptical) in na. */
    si16 num_samples;		/* Number of samples used in estimate */
				/* for this parameter. */
    si16 binary_format;	/* Binary format of radar data. */
    char  threshold_field[8];	/* Name of parameter upon which this */
				/* parameter is thresholded (ascii */
				/* characters NONE if not */
				/* thresholded). */
    fl32 threshold_value;	/* Value of threshold in ? */
    fl32 parameter_scale;	/* Scale factor for parameter. */
    fl32 parameter_bias;	/* Bias factor for parameter. */
    si32  bad_data;		/* Bad data flag. */
    
				/* 1995 extension #1 */

    si32 extension_num;
    char  config_name[8];	/* used to identify this set of
				 * unique radar characteristics */
    si32  config_num;
    si32 offset_to_data;	/* bytes added to the data struct pointer
				 * to point to the first datum whether it's
				 * an RDAT or a QDAT
				 */
    fl32 mks_conversion;
    si32 num_qnames;		
    char qdata_names[32];	/* each of 4 names occupies 8 characters
				 * of this space
				 * and is blank filled. Each name identifies
				 * some interesting segment of data in a
				 * particular ray for this parameter.
				 */
    si32 num_criteria;
    char criteria_names[32];	/* each of 4 names occupies 8 characters
				 * and is blank filled. These names identify
				 * a single interesting floating point value
				 * that is associated with a particular ray
				 * for a this parameter. Examples might
				 * be a brightness temperature or
				 * the percentage of cells above or
				 * below a certain value */
    si32 number_cells;
    fl32 meters_to_first_cell;	/* center! */
    fl32 meters_between_cells;
    fl32 eff_unamb_vel;	/* Effective unambiguous velocity, m/s. */

}; /* End of Structure */


struct parameter_d_v00 {
    char  parameter_des[4];	/* Parameter Descriptor identifier */
				/* (ascii characters "PARM"). */
    si32 parameter_des_length;	/* Parameter Descriptor length in */
				/* bytes.*/
    char  parameter_name[8];	/* Name of parameter being described. */
    char  param_description[40]; /* Detailed description of this parameter. */
    char  param_units[8];	/* Units parameter is written in. */
    si16 interpulse_time;	/* Inter-pulse periods used. bits 0-1 */
				/* = frequencies 1-2. */
    si16 xmitted_freq;		/* Frequencies used for this */
				/* parameter. */
    fl32 recvr_bandwidth;	/* Effective receiver bandwidth for */
				/* this parameter in MHz.*/
    si16 pulse_width;		/* Effective pulse width of parameter */
				/* in m. */
    si16 polarization;		/* Polarization of the radar beam for */
				/* this parameter (0 Horizontal,1 */
				/* Vertical,2 Circular,3 Elliptical) in na. */
    si16 num_samples;		/* Number of samples used in estimate */
				/* for this parameter. */
    si16 binary_format;	/* Binary format of radar data. */
    char  threshold_field[8];	/* Name of parameter upon which this */
				/* parameter is thresholded (ascii */
				/* characters NONE if not */
				/* thresholded). */
    fl32 threshold_value;	/* Value of threshold in ? */
    fl32 parameter_scale;	/* Scale factor for parameter. */
    fl32 parameter_bias;	/* Bias factor for parameter. */
    si32  bad_data;		/* Bad data flag. */
}; /* End of Structure */


typedef struct parameter_d parameter_d;
typedef struct parameter_d PARAMETER;

#ifdef OK_RPC
#if defined(sun) || defined(WRS)
bool_t xdr_parameter_d(XDR *, PARAMETER *);
#endif

#endif /* OK_RPC */

#endif /* INCParameterh */








