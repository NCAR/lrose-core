// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#ifndef _UMALLOC_HH
#define _UMALLOC_HH

#include <sys/types.h>
#include <cstdlib>

class Umalloc {

public:

/*******************************************************************
 * umalloc()
 *
 * normal malloc
 *
 *******************************************************************/

static void *umalloc(size_t size);

/*******************************************************************
 * ucalloc()
 *
 * malloc of buffer initialized with zeros
 *
 ********************************************************************/

static void *ucalloc(size_t num, size_t size);

/*******************************************************************
 * urealloc()
 *
 * reallocation
 *
 ********************************************************************/

static void *urealloc(void *ptr, size_t size);

/*******************************************************************
 * ufree()
 *
 * normal free
 *
 *******************************************************************/

static void ufree(void *ptr);

/************************************************************
 * ufree_non_null()
 *
 * Frees pointer if non_null, sets pointer to null after free.
 *
 * The arg passed in is a pointer to the pointer to be freed.
 * 
 ************************************************************/

static void ufree_non_null(void **ptr_p);

/*******************************************************************
 * umalloc2()
 *
 * malloc of two-dimensional array
 *
 * The data array is contiguously malloc'd in a buffer, and the
 * two_d_pointer array points to different parts of the data array.
 *
 *******************************************************************/

static void **umalloc2(size_t m, size_t n, size_t item_size);

/*******************************************************************
 * umalloc3()
 *
 * malloc of three-dimensional array
 *
 * The data array is contiguously malloc'd in a buffer, and the
 * two_d_pointer array points to different parts of the data array.
 *
 * The three_d_pointer points to different parts of the two_d_pointer
 * array.
 *
 *******************************************************************/

static void ***umalloc3(size_t l, size_t m, size_t n, size_t item_size);

/*******************************************************************
 * ucalloc2()
 *
 * calloc of two-dimensional array
 *
 * The data array is contiguously calloc'd in a buffer, and the
 * two_d_pointer array points to different parts of the data array.
 *
 *******************************************************************/

static void **ucalloc2(size_t m, size_t n, size_t item_size);

/*******************************************************************
 * ucalloc3()
 *
 * calloc of three-dimensional array
 *
 * The data array is contiguously calloc'd in a buffer, and the
 * two_d_pointer array points to different parts of the data array.
 *
 * The three_d_pointer points to different parts of the two_d_pointer
 * array.
 *
 *******************************************************************/

static void ***ucalloc3(size_t l, size_t m, size_t n, size_t item_size);

/*******************************************************************
 * urealloc2()
 *
 * realloc of two-dimensional array
 *
 * The data structure used in umalloc2() is realloc'd to include
 * a different number for the m dimension.  Note that if you try
 * to change the n dimension, this will mess up all of your pointers
 * because of the data structures used.
 *
 * Can only be used on arrays allocated using umalloc2() or ucalloc2().
 *
 *******************************************************************/

static void **urealloc2(void **user_addr,
			size_t m, size_t n, size_t item_size);

/*******************************************************************
 * ufree2()
 *
 * frees 2 dimensional arrau allocated with umalloc2 or ucalloc2
 *
 *******************************************************************/

static void ufree2(void **two_d_pointers);

/*******************************************************************
 * ufree3()
 *
 * frees 3 dimensional arrau allocated with umalloc3 or ucalloc3
 *
 *******************************************************************/

static void ufree3(void ***three_d_pointers);

};

#endif /* _UMALLOC_HH */
