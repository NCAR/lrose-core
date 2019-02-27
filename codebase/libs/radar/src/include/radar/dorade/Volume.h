/*
 *	$Id: Volume.h,v 1.2 2019/02/27 02:59:40 dixon Exp $
 *
 *	Module:		 Volume.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date: 2019/02/27 02:59:40 $
 *
 * revision history
 * ----------------
 * $Log: Volume.h,v $
 * Revision 1.2  2019/02/27 02:59:40  dixon
 * Sync with changes and additions in EOL LROSE Git
 *
 * Revision 1.1  2008/01/24 22:22:32  rehak
 * Added dorade module
 *
 * Revision 1.1  2007/01/10 22:29:51  rehak
 * Initial version -- copied from refr1 and then updated to get rid of most compiler warnings and to reflect new location of include files
 *
 * Revision 1.1  2003/12/02 20:59:53  vanandel
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
 * Revision 1.1  1996/12/20 20:00:09  oye
 * New.
 *
 * Revision 1.2  1994/04/05  15:36:47  case
 * moved an ifdef RPC and changed an else if to make HP compiler happy.
 *
 * Revision 1.4  1992/04/20  17:18:31  thor
 * Latest Eldora/Asterea revisions included.
 *
 * Revision 1.3  1991/10/15  17:57:07  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.2  1991/10/09  15:24:44  thor
 * Fixed incorrect flight number variable and added needed padding.
 *
 * Revision 1.1  1991/08/30  18:39:40  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCVolumeh
#define INCVolumeh

struct volume_d {
    char  volume_des[4];	/* Volume descriptor identifier: ASCII */
				/* characters "VOLD" stand for Volume */
				/* Descriptor. */
    long  volume_des_length;	/* Volume descriptor length in bytes. */
    short format_version;	/* ELDORA/ASTRAEA field tape format */
				/* revision number. */
    short volume_num;		/* Volume Number into current tape. */
    long  maximum_bytes;	/* Maximum number of bytes in any. */
				/* physical record in this volume. */
    char  proj_name[20];		/* Project number or name. */
    short year;			/* Year data taken in years. */
    short month;		/* Month data taken in months. */
    short day;			/* Day data taken in days. */
    short data_set_hour;	/* hour data taken in hours. */
    short data_set_minute;	/* minute data taken in minutes. */
    short data_set_second;	/* second data taken in seconds. */
    char  flight_num[8];	/* Flight number. */
    char  gen_facility[8];	/* identifier of facility that */
				/* generated this recording. */
    short gen_year;		/* year this recording was generated */
				/* in years. */
    short gen_month;		/* month this recording was generated */
				/* in months. */
    short gen_day;		/* day this recording was generated in days. */
    short number_sensor_des;	/* Total Number of sensor descriptors */
				/* that follow. */
}; /* End of Structure */



typedef struct volume_d volume_d;
typedef struct volume_d VOLUME;

#endif /* INCVolumeh */

