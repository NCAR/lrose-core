/*
 *	$Id: Ray.h,v 1.2 2019/02/27 02:59:40 dixon Exp $
 *
 *	Module:		 Ray.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date: 2019/02/27 02:59:40 $
 *
 * revision history
 * ----------------
 * $Log: Ray.h,v $
 * Revision 1.2  2019/02/27 02:59:40  dixon
 * Sync with changes and additions in EOL LROSE Git
 *
 * Revision 1.1  2008/01/24 22:22:32  rehak
 * Added dorade module
 *
 * Revision 1.1  2007/01/10 22:29:51  rehak
 * Initial version -- copied from refr1 and then updated to get rid of most compiler warnings and to reflect new location of include files
 *
 * Revision 1.1  2003/12/02 20:59:52  vanandel
 *
 * : Added Files:
 * : 	CellSpacingFP.h CellVector.h Comment.h Correction.h
 * : 	DataInput.hh IndexFields.hh Parameter.h Pdata.h Platform.h
 * : 	Qdata.h RadarDesc.h RadarPktHdr.hh Ray.h Sweep.h Volume.h
 * : 	WhichFloat.hh Xtra_stuff.h controller.hh dd_defines.h
 * : 	dd_math.h dorade_includes.h inline.h message.h super_SWIB.h
 * : 	viraq.h
 * : ----------------------------------------------------------------------
 * "adopted" from rdss/spol/include
 *
 * Revision 1.1  1996/12/20 19:58:33  oye
 * New.
 *
 * Revision 1.2  1994/04/05  15:35:58  case
 * moved an ifdef RPC and changed an else if to make HP compiler happy.
 *
 * Revision 1.4  1992/07/28  17:33:03  thor
 * Added ray_status.
 *
 * Revision 1.3  1992/04/20  17:18:31  thor
 * Latest Eldora/Asterea revisions included.
 *
 * Revision 1.2  1991/10/15  17:56:43  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.1  1991/08/30  18:39:38  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCRayh
#define INCRayh

struct ray_i {
    char  ray_info[4];		/* Identifier for a data ray info. */
				/* block (ascii characters "RYIB"). */
    long ray_info_length;	/* length of a data ray info block in */
				/* bytes. */
    long  sweep_num;		/* sweep number for this radar. */
    long  julian_day;		/* Guess. */
    short hour;			/* Hour in hours. */
    short minute;		/* Minute in minutes. */
    short second;		/* Second in seconds. */
    short millisecond;		/* Millisecond in milliseconds. */
    float azimuth;		/* Azimuth in degrees. */
    float elevation;		/* Elevation in degrees. */
    float peak_power;		/* Last measured peak transmitted */
				/* power in kw. */
    float true_scan_rate;	/* Actual scan rate in degrees/second. */
    long  ray_status;		/* 0 = normal, 1 = transition, 2 = bad. */
}; /* End of Structure */


typedef struct ray_i ray_i;
typedef struct ray_i RAY;

#endif /* INCRayh */

