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
//
//
//
#include <cstdio>
#include <iostream>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxChunk.hh>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>

using namespace std;


int main( int argc, char *argv[] )
{

  if (argc != 3){
    cerr << "USAGE : " << argv[0] << " <input MDV file> <output MDV url>" << endl;
    return -1;
  }

  char *inFile = argv[1];
  char *outUrl = argv[2];


  //
  // Read the data in, add the time offset, write it out.
  //

  DsMdvx inMdvMgr;

  inMdvMgr.setReadPath( inFile );

  if (inMdvMgr.readVolume()){
    cerr << "Read failed from " << inFile << endl;
    return -1;
  }

  Mdvx::master_header_t Mhdr = inMdvMgr.getMasterHeader();
  //
  //

  // Calculate the time offset we want to add.

  time_t thisDayStart = 86400*(time(NULL)/86400);
  time_t thenDayStart = 86400*(Mhdr.time_centroid/86400);

  time_t offset = thisDayStart - thenDayStart;

  if ( Mhdr.time_gen )  Mhdr.time_gen += offset;
  if ( Mhdr.time_begin ) Mhdr.time_begin  += offset;
  if ( Mhdr.time_end ) Mhdr.time_end += offset;
  if ( Mhdr.time_centroid ) Mhdr.time_centroid += offset;
  if ( Mhdr.time_expire ) Mhdr.time_expire += offset;
  //
  // Change the name so it's clear that these are replayed data.
  //
  sprintf(Mhdr.data_set_info, "MdvMakeCurrent");
  sprintf(Mhdr.data_set_name, "MdvMakeCurrent");
  sprintf(Mhdr.data_set_source, "MdvMakeCurrent");

  DsMdvx Out;
  Out.setMasterHeader(Mhdr);
  Out.clearFields();
  //
  // Loop throught the fields, adjust the time in the
  // field header (only one entry) and add it to the output
  // object.
  //
  for (int ifld=0; ifld < Mhdr.n_fields; ifld++){
    MdvxField *InField = inMdvMgr.getFieldByNum( ifld );

    if (InField == NULL){
      cerr << "Field " << ifld << " not found." << endl;
      continue;
    }

    Mdvx::field_header_t Fhdr = InField->getFieldHeader();
    
    if (Fhdr.forecast_time) Fhdr.forecast_time += offset;
    Mdvx::vlevel_header_t Vhdr = InField->getVlevelHeader();
    
    MdvxField *fld = new MdvxField(Fhdr, Vhdr, InField->getVol() );

    Out.addField(fld);

  }

  //
  // Add any chunks from the input.
  //
  for (int ic=0; ic < inMdvMgr.getNChunks(); ic++){
    MdvxChunk *inChunk = inMdvMgr.getChunkByNum( ic );
    //
    // The chunk must be created using 'new'.
    // The chunk object becomes 'owned' by the Mdvx object, and will be
    // freed by it.
    //
    MdvxChunk *outChunk = new MdvxChunk( *inChunk );
    Out.addChunk( outChunk );
  }
  //
  // Write it out.
  //
  if (Out.writeToDir(outUrl)) {
    cerr << "Failed to wite to " << outUrl << endl;
    return -1;
  }


  return 0;

}









