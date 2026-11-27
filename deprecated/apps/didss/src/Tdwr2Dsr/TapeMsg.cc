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
///////////////////////////////////////////////////////////////
// TapeMsg.cc
//
// TapeMsg object
//
// Gary Blackburn, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2001
//
///////////////////////////////////////////////////////////////
#include <stdlib.h>
#include "TapeMsg.hh"
using namespace std;

TapeMsg::TapeMsg( string tapeDevice ) 
{
   _tape_fd           = -1;
   _tape_device       = tapeDevice;
   _offset            = 0;
   _left              = 0;

   //
   // Open the tape Device
   //
   PMU_auto_register( "Opening tape device" );

   if ((_tape_fd = open (_tape_device.c_str(), O_RDONLY)) < 0) {
      cerr << "ERROR, opening tape device " << _tape_device << endl;
      exit (0);
   }

   cerr << "opened tape device, fd = " << _tape_fd << endl;

}

// destructor
TapeMsg::~TapeMsg() 
{
   if( _tape_fd >= 0 )
      close( _tape_fd );
}


////////////////////////////////////////////////////////////////////

int
TapeMsg::get_tdwr_data (unsigned char** tdwr_rec ) 
{
  int  nread = 0;
  int log_rec_size = 0;
   
  PMU_auto_register( "Getting tape data" );
  if (_left == 0) {
    PMU_auto_register( "Reading new physical_rec  tape data" );
    nread = read (_tape_fd, (char *) _physical_rec, RECSIZE);
    // cerr << "read " << nread << " bytes " << endl;

    if (nread == 0) {
      nread = read (_tape_fd, (char *) _physical_rec, RECSIZE);
      cerr << "2nd read " << nread << " bytes " << endl;
    }

    _offset = 0;
    _left = nread;
  }
  if (_left == 0) {
    cerr << "Reached end of tape" << endl;
    return (_left);
  }

  if (_left < 0) {
    cerr << "Tape read error" << endl;
    *tdwr_rec = NULL;
    log_rec_size = _left;
  }
  else {
    *tdwr_rec = _physical_rec + _offset;

    // big endian/little endian stuff
    TDWRadial::BE_to_tdwr_data_hdr ((TDWR_data_header_t *) *tdwr_rec);

    log_rec_size = ((TDWR_data_header_t *) *tdwr_rec)->message_length;
    // cerr << "logical rec size " << log_rec_size << endl;
    if (log_rec_size == 64) {
      _offset += 6144;
      _left -= 6144;
    }
    else {
      _offset += log_rec_size;
      _left -= log_rec_size;
    }

    // cerr << "left " << _left << endl;

    if (_left < log_rec_size) _left = 0;
  }
  return log_rec_size;
}

      
////////////////////////////////////////////////////////////////////



   
   
   



