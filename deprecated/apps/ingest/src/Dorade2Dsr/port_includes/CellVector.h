/*
 *	$Id: CellVector.h,v 1.1 2007/09/22 21:08:21 dixon Exp $
 *
 *	Module:		 CellVector.h
 *	Original Author: Richard E. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date: 2007/09/22 21:08:21 $
 *
 * revision history
 * ----------------
 * $Log: CellVector.h,v $
 * Revision 1.1  2007/09/22 21:08:21  dixon
 * adding
 *
 * Revision 1.1  2002/02/07 18:31:34  dixon
 * added
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

#include <dataport/port_types.h>

#define MAXCVGATES 1500

# ifndef MAXGATES
# define MAXGATES MAXCVGATES
# endif

struct cell_d {
    char cell_spacing_des[4];	/* Cell descriptor identifier: ASCII */
				/* characters "CELV" stand for cell*/
				/* vector. */
    si32  cell_des_len   ;	/* Comment descriptor length in bytes*/
    si32  number_cells   ;	/*Number of sample volumes*/
    fl32  dist_cells[MAXCVGATES]; /*Distance from the radar to cell*/
				/* n in meters*/

}; /* End of Structure */

typedef struct cell_d cell_d;
typedef struct cell_d CELLVECTOR;

#endif /* INCCellVector_h */
