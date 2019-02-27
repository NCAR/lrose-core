/*
 *	$Id: CellSpacingFP.h,v 1.2 2019/02/27 02:59:40 dixon Exp $
 *
 *	Module:		 CellSpacing.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date: 2019/02/27 02:59:40 $
 *
 * revision history
 * ----------------
 * $Log: CellSpacingFP.h,v $
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
 * Revision 1.1  1999/04/12 20:03:07  oye
 * Cell spacing information similar to ELDORA's "CSPD" but with floating
 * point values rather than short values. Meant to be substituted for the
 * CELLVECTOR and to make sweepfiles smaller for network transfers.
 *
 * Revision 1.2  1994/04/05  15:22:38  case
 * moved an ifdef RPC and changed else if to else and if on another line
 * >> to keep the HP compiler happy.
 * >> .
 *
 * Revision 1.3  1991/10/15  17:54:21  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.2  1991/10/11  15:32:10  thor
 * Added variable for offset to first gate.
 *
 * Revision 1.1  1991/08/30  18:39:19  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCCellSpacingFPh
#define INCCellSpacingFPh

struct cell_spacing_fp_d {
    char   cell_spacing_des[4]; /* Identifier for a cell spacing descriptor */
				/* (ascii characters CSFD). */
    long   cell_spacing_des_len; /* Cell Spacing descriptor length in bytes. */
    long   num_segments;	/* Number of segments that contain cells of */
    float  distToFirst;		/* Distance to first gate in meters. */
    float  spacing[8];		/* Width of cells in each segment in m. */
    short  num_cells[8];	/* Number of cells in each segment. */
				/* equal widths. */
};				/* End of Structure */


typedef struct cell_spacing_fp_d cell_spacing_fp_d;
typedef struct cell_spacing_fp_d CELLSPACINGFP;

#endif /* INCCellSpacingPFh */









