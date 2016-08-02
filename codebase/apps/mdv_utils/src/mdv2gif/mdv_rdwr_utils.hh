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
#ifndef MDV_RDWR_UTILS_HH
#define MDV_RDWR_UTILS_HH

#include <mdv/mdv_read.h>
#include <mdv/mdv_write.h>
#include <mdv/mdv_user.h>
#include <mdv/mdv_handle.h>
#include <mdv/mdv_macros.h>
#include <mdv/mdv_file.h>
#include <toolsa/udatetime.h>

/***************************************************
* structure for the mdv info and a data float array *
***************************************************/
typedef struct {
   MDV_handle_t mdv;
   float ***f_array;
   ui08 ***c_array;
   char filename[100];
   char pathname[100];
   char year[5], month[3], day[3], hour[3], min[3], sec[3];
   date_time_t dtime;
} mdv_float_handle;

/*******************************************************
* allocate the memory for the float array for the data *
*******************************************************/
extern void alloc_arrays(mdv_float_handle *mdv_f);

/************************************* 
* fill the data into the float array *
*************************************/
extern void fill_float_array(mdv_float_handle *mdv_f);

/************************
* free the mdv_f handle * 
************************/
extern void free_float_handle(mdv_float_handle *mdv_f);

/********************************************
* convert the float array into a byte array *
********************************************/
extern void float2byte(mdv_float_handle *mdv_f);

/*******************************************************
* Fill the headers and allocate the space for the data *
*******************************************************/
extern void Set_Vol_Hdrs(mdv_float_handle *mdv_f);

/*******************************************************
* Write out completed volume in mdv format             *
*******************************************************/
extern int Write_Complete_Vol(mdv_float_handle *mdv_f, char *dirout);

/****************************
* Set the Header Time       *
*****************************/
extern void Set_Header_Time(mdv_float_handle *mdv_f, int timetostart);

#endif

