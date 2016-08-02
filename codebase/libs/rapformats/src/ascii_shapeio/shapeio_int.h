/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1992 - 2010 */
/* ** University Corporation for Atmospheric Research(UCAR) */
/* ** National Center for Atmospheric Research(NCAR) */
/* ** Research Applications Laboratory(RAL) */
/* ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
/* ** 2010/10/7 23:12:52 */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/************************************************************************
*                                                                       *
*                       shapeio_int.h                                   *
*                                                                       *
*************************************************************************

                        May 1997

       Description:     Internal header file for shapeio software.

       See Also:

       Author:          Nancy Rehak

       Modification History:

*/

#ifndef SHAPEIO_INT_H
#define SHAPEIO_INT_H

/*
 * System include files
 */

/*
 * Local include files
 */

/*
 * Definitions / macros / types
 */

/*
 * Function prototypes
 */

/*
 * Read 1 line of a file, hoping for 1st line of a new product..
 * Return 1 if it is aligned to this.
 * Return 0 otherwise.
 */

extern int read_first_line(FILE *fp, int verbose);

extern char *known_type_subtype_list(char *);

extern char *known_line_type(char *type);

/*
 * Return 1 if file index indiecated by index record aligns nicely
 * with file contents.
 */

extern int file_is_aligned(FILE *fd, SIO_index_data_t *I);



#endif
