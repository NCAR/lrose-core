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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: usort_stl.h,v 1.2 2016/03/03 19:24:05 dixon Exp $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/


/************************************************************************

Header: usort_stl.h

Author: C S Morse

Date:	Mon Nov 26 10:51:51 2007

Description:	sorting routines utilizing C++ STL classes

*************************************************************************/

# ifndef    RAPMATH_SORT_STL_H
# define    RAPMATH_SORT_STL_H

/* System include files / Local include files */

#include <vector>


/* Constant definitions / Macro definitions / Type definitions */


/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */

using namespace std;


/************************************************************************

Function Name: 	usort

Description:	sorts an array of doubles in place using a heap sort algorithm

Returns:	none

Globals:	none

Notes:	Adapted (and debugged!) from source found at
        http://linux.wku.edu/~lamonml/algor/sort/heap.html
        
************************************************************************/

extern void usort( vector<double>& array );
extern void usort( vector<float>& array );
extern void usort( vector<int>& array );


/************************************************************************

Function Name: 	usort_index

Description:	indexes an input array so that the indexed array is 
                sorted in ascending order

Returns:	none

Globals:	none

Notes:	Adapted from usort.

************************************************************************/

extern void usort_index( vector<double>& array, int size, vector<int>& index );
extern void usort_index( vector<float>& array, int size, vector<int>& index );
extern void usort_index( vector<int>& array, int size, vector<int>& index );

# endif     /* RAPMATH_SORT_STL_H */

