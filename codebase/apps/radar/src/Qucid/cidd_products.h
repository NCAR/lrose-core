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
#ifndef CIDD_PRODUCTS_H
#define CIDD_PRODUCTS_H
/*************************************************************************
 * CIDD_PRODUCTS.H : Definitions and control structures/variables
 *
 */

#define NUM_PRODUCT_DETAIL_THRESHOLDS 3

typedef enum {
      LAST_FRAME_PRODUCTS = 0,
      EACH_FRAME_PRODUCTS
} select_type_t;

typedef struct  {
    double threshold;
    int adjustment;
} prod_detail_thresh_t;

typedef struct  {  
    int    products_on;         /* Flag to turn products on/off globally */
    int    prod_line_width;    /* How wide to make the lines */
    int    prod_font_num;    

    prod_detail_thresh_t detail[NUM_PRODUCT_DETAIL_THRESHOLDS];  /* Threshold and adjustment to font size */
} prod_info_t;

#endif
