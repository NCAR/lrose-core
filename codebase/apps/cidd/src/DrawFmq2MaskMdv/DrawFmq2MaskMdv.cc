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

#include "DrawFmq2MaskMdv.hh"
#include "Process.hh"

#include <iostream>

#include <Fmq/DrawQueue.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

//
// Constructor and destructor - these do little.
//
DrawFmq2MaskMdv::DrawFmq2MaskMdv(){
  _initCalled = false;
}

DrawFmq2MaskMdv::~DrawFmq2MaskMdv(){
  
}

//
// init function. Reads TDRP. Must be called before run.
//
int DrawFmq2MaskMdv::init(int argc, char *argv[]){
  //
  // See if this is a cry for help.
  //
  for( int i=1; i < argc; i++ ) {
    //
    // request for usage information
    //
    if ( !strcmp(argv[i], "--" ) ||
	 !strcmp(argv[i], "-h" ) ||
	 !strcmp(argv[i], "-help" ) ||
	 !strcmp(argv[i], "-man" )) {
      cerr << "Use the -print_params option for more information." << endl;
    }
  }
  //
  // Load TDRP.
  //
  if (_params.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }
  //
  // Init with PMU.
  //
  PMU_auto_init( "DrawFmq2MaskMdv", _params.instance,
		 PROCMAP_REGISTER_INTERVAL );
  PMU_auto_register( "Starting up application" );
  //
  // It went OK.
  //
  _initCalled = true;
  return 0;
}

//
// Main method - runs app.
//
int DrawFmq2MaskMdv::run(){
  //
  // Make sure init was called.
  //
  if (!_initCalled){
    cerr << "run called before init - exiting ..." << endl;
    exit(-1);
  }
  //
  // Init the FMQ.
  //
  DrawQueue    drawqueue;
  if ( drawqueue.initReadBlocking( _params.draw_fmq_url,
				   "DrawFmq2MaskMdv", 
				   _params.debug ) != 0 ) {
    cerr << "Failed to init queue " << _params.draw_fmq_url << endl;
    exit(-1);
  }
  //
  // Enter the eternal loop - keep reading from queue.
  //
  while (true) {

    PMU_auto_register( "Looking for data" );

    int status = 0;
    const Human_Drawn_Data_t& h_prod = drawqueue.nextProduct( status );

    if ( status == 0 ) { // success
      //
      //
      if (_params.debug)
      {
	cerr << "Got a Message - Product_ID_Label: ";
	cerr << h_prod.id_label << endl;  
      }
      //
      // See if the label matches.
      //
      bool eraseOutside = false;

      if (strncmp(h_prod.id_label.c_str(), _params.eraseOutsideLabel,
		 DrawQueue::ID_label_Len) == 0)
      {
	eraseOutside = true;
      }
      else if (strncmp(h_prod.id_label.c_str(), _params.eraseInsideLabel,
		       DrawQueue::ID_label_Len) == 0)
      {
	eraseOutside = false;
      }
      else
	continue;
      

      if (_params.debug){
	cerr << "The label matches one of ours." << endl;
	cerr << "Product ID_NO: " << h_prod.id_no;
	cerr << " Valid: " << h_prod.valid_seconds << " seconds" << endl;
	cerr << "Product issued: " << utimstr(h_prod.issueTime) << endl;
	cerr << "Product data time: " << utimstr(h_prod.data_time) << endl;
	cerr << "Product label text: " << h_prod.prod_label << endl;
	cerr << "Product sender: " << h_prod.sender << endl;
	cerr << h_prod.num_points << " points in message." << endl;
	  
	  
	for(int i=0; i < h_prod.num_points; i++) {
	  cerr << "LAT, LON: " << h_prod.lat_points[i] << ", " << h_prod.lon_points[i] << endl;
	}
      }
      //
      // Process these data.
      //
      Process P;
      P.Derive(&_params, eraseOutside, h_prod.data_time,
	       h_prod.lat_points, h_prod.lon_points, h_prod.num_points);
      
    } // end of if status is zero
    
  } // end of eternal loop.

}


