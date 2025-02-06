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
/**********************************************************************
 * CIDD_DATA_IO.H:  Data IO Status and Defns for CIDD
 */

#ifndef CIDD_DATA_IO_H
#define CIDD_DATA_IO_H

#include "Product.hh"

typedef struct {    // 
    int    request_type; //  0 = none, 1 = horizontal,
			 // 2 = vertical data orientation, 3 = product 
    int    expire_time;  // unix time when request times out  
    int    outstanding_request;      // 1 =  data request is outstanding 
    int    page;         // page being rendered 

    // CDATA PROTOCOL Support variables
    int    mode;    // Cdatap: LIVE_DATA or STATIC_DATA or  
    int    busy_status; // 0 - Safe to read data.  1 =Avoid reading data - Replacement in progress.
    int    fd;          // file discriptor for io 
    unsigned short *incoming_data_pointer;

    // IO timing variables
    long last_read;              // Number of bytes read 
    struct timeval request_time; // time the data  request was submitted
    struct timeval last_time;    // The last time data was read

    // Pointers to objests from where data was being requested.
    MdvReader    *mr;   // pointer to the meterological data object 
    Product *prod;         // Pointer to the Product data object *
} io_info_t;
 
#endif
