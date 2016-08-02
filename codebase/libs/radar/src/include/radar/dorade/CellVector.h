/*
 *	$Id: CellVector.h,v 1.1 2008/01/24 22:22:32 rehak Exp $
 *
 *	Module:		 CellVector.h
 *	Original Author: Richard E. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date: 2008/01/24 22:22:32 $
 *
 * revision history
 * ----------------
 * $Log: CellVector.h,v $
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
 * Revision 1.1  1996/12/20 19:52:40  oye
 * New.
 *
 * Revision 1.3  1993/11/18  16:54:59  oye
 * *** empty log message ***
 *
 * Revision 1.2  1993/11/18  16:53:31  oye
 * Add MAXGATES definition to be compatable with Oye's code.
 *
 * Revision 1.1  1993/10/04  22:45:45  nettletn
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCCellVector_h
#define INCCellVector_h

#define MAXCVGATES 1500

# ifndef MAXGATES
# define MAXGATES MAXCVGATES
# endif

struct cell_d {
    char cell_spacing_des[4];	/* Cell descriptor identifier: ASCII */
				/* characters "CELV" stand for cell*/
				/* vector. */
    long  cell_des_len   ;	/* Comment descriptor length in bytes*/
    long  number_cells   ;	/*Number of sample volumes*/
    float dist_cells[MAXCVGATES]; /*Distance from the radar to cell*/
				/* n in meters*/

}; /* End of Structure */

typedef struct cell_d cell_d;
typedef struct cell_d CELLVECTOR;

#endif /* INCCellVector_h */
