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

#include <ctime>
#include <Mdv/DsMdvx.hh>
#include <toolsa/udatetime.h>
#include <toolsa/pmu.h>
#include "Detect.hh"

#include "Reader.hh"
using namespace std;

//
// Constructor. Actually does the reads.
//
Reader::Reader(time_t dataTime,
	       Params *P,
	       int UrlNumber,
	       bool isTruth){

  //
  // Make local copies of stuff not in the param struct.
  //
  _isTruth = isTruth;
  _UrlNum = UrlNumber;
  _dataTime = dataTime;

  if (_isTruth){
    Url = P->_TruthUrls[UrlNumber];
    OtherUrl = P->_ForecastUrls[UrlNumber];
  } else {
    OtherUrl = P->_TruthUrls[UrlNumber];
    Url = P->_ForecastUrls[UrlNumber];
  }

}

//
// Destructor. Cleans up after the reads.
//
Reader::~Reader(){


}

//
// Process.
//
int Reader::Process(Params *P){
  //
  // Read the URL.
  //
  DsMdvx a,b;
  PMU_auto_register("Reading");
 
  //
  // Set the times on the URLs.
  //
  a.setReadTime( Mdvx::READ_CLOSEST, Url, 
		 0, _dataTime );

  if (P->Debug){
    cerr << "URL : " << Url << endl;
    cerr << "Other URL " << OtherUrl << endl;
  }

  b.setReadTime( Mdvx::READ_FIRST_BEFORE, OtherUrl, 
		 P->LookBack, _dataTime );

  //
  // Set compositing.
  //
  if (_isTruth){
    a.setReadVlevelLimits(P->CompositeMin, P->CompositeMax);
    a.setReadComposite();
  } else {
    b.setReadVlevelLimits(P->CompositeMin, P->CompositeMax);
    b.setReadComposite();
  }

  //
  // Read the actual data into uncompressed floats.
  //
  a.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  a.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  b.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  b.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  //
  // And re-map while we're at it.
  //
  a.setReadRemapFlat(P->GridNx, P->GridNy,
		     P->GridMinx, P->GridMiny,
		     P->GridDx, P->GridDy,
		     P->GridOriginY, P->GridOriginX,
		     0.0);

  b.setReadRemapFlat(P->GridNx, P->GridNy,
		     P->GridMinx, P->GridMiny,
		     P->GridDx, P->GridDy,
		     P->GridOriginY, P->GridOriginX,
		     0.0); //  0.0 is the rotation.

  if (a.readVolume()){
    cerr << "Mdv read failed from URL " << Url << " at ";
    cerr << utimstr(_dataTime) << endl;
    return -1;
  }

  if (b.readVolume()){
    cerr << "Mdv read failed from other URL " << OtherUrl << " at ";
    cerr << utimstr(_dataTime) << endl;
    return -1;
  }
  //
  // Get the field headers.
  //
  MdvxField *afield, *bfield;

  if (_isTruth){
    afield = a.getFieldByName(P->TruthFieldName);
    bfield = b.getFieldByName(P->ForecastFieldName);
  } else {
    bfield = b.getFieldByName(P->TruthFieldName);
    afield = a.getFieldByName(P->ForecastFieldName);
  }

  //
  // Check that it went well.
  //
  if (afield == NULL){
    cerr << "Could not find any field named ";
    if (_isTruth)
      cerr << P->TruthFieldName;
    else
      cerr << P->ForecastFieldName;
    cerr << " at URL " << Url << endl;
    return -1;
  }

  if (bfield == NULL){
    cerr << "Could not find any field named ";
    if (!(_isTruth))
      cerr << P->TruthFieldName;
    else
      cerr << P->ForecastFieldName;
    cerr << " at URL " << OtherUrl << endl;
    return -1;
  }
  //
  // Later we may want to add an option so that the
  // user can re-map on reads to a given grid. For now, we'll
  // just check that Nx and Ny are equal.
  //

  int ret;
  Detect D;

  if (_isTruth){
    ret=D.Threshold(afield, bfield, P, _UrlNum, _dataTime);
  } else {
    ret=D.Threshold(bfield, afield, P, _UrlNum, _dataTime);
  }


  return ret;

}







