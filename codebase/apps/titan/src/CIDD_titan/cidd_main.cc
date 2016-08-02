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
/*********************************************************************
 * CIDD_MAIN.C : Configurable Interactive Data Display
 *
 * Frank Hage    July 1991 NCAR, Research Applications Program
 *
 *  Note: Some information on how this beast works
 *  can be found in the comment header of data_io.cc
 */

#define    CIDD_MAIN    1        /* This is the main module */

#include "cidd.h"

/********************************************************************
 * MAIN: Process arguments, initialize and begin application
 */
 

int main(int  argc, char **argv)
{

    ZERO_STRUCT (&gd);

    process_args(argc,argv);    /* process command line arguments */

    /* initialize globals, get/set defaults, establish data sources etc. */
    init_data_space();
 
    init_xview(&argc,argv); /* create all Xview objects */    

    setup_colorscales(gd.dpy);    /* Establish color table & mappings  */

    // Instantiate Symbolic products
    init_symprods();

    /* make changes to xview objects not available from DevGuide */
    modify_xview_objects();

    gd.finished_init = 1;
     
    reset_display(); // Set to a known starting position.

    start_timer();                /* start up the redraw/movieloop timer */

    xv_main_loop(gd.h_win_horiz_bw->horiz_bw);
    exit(0);
}
