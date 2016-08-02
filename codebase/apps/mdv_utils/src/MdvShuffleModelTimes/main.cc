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
// Shuffle the times of MDV data. Useful as a precursor to MdvRepeatDay.
// Niles Oien June 2005.
//
#include <cstdio>
#include <iostream>
#include <toolsa/pjg_flat.h>
#include "Params.hh"
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxChunk.hh>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>

using namespace std;



int main( int argc, char *argv[] )
{
  //
  // Get params and check in.
  //  
  Params params;

  if (params.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }

  //
  // Get the start and end generation times.
  //
  date_time_t genStart, genEnd, inGen;

  if (
      ( 6 != sscanf(params.startGenTime, "%d/%d/%d %d:%d:%d",
		    &genStart.year, &genStart.month, &genStart.day,
		    &genStart.hour, &genStart.min, &genStart.sec)) ||
      ( 6 != sscanf(params.endGenTime, "%d/%d/%d %d:%d:%d",
		    &genEnd.year, &genEnd.month, &genEnd.day,
		    &genEnd.hour, &genEnd.min, &genEnd.sec)) ||
      ( 6 != sscanf(params.inputGenTime, "%d/%d/%d %d:%d:%d",
		    &inGen.year, &inGen.month, &inGen.day,
		    &inGen.hour, &inGen.min, &inGen.sec))
      ){
    cerr << "Error in formatting of start or end gen time" << endl;
    exit(-1);
  }
  uconvert_to_utime( &genEnd ); uconvert_to_utime( &genStart ); uconvert_to_utime( &inGen );

  time_t genTime = genStart.unix_time;

  while (genTime <= genEnd.unix_time){
    //
    // Loop through the lead times for this generation time.
    //
    for (int ilead = 0; ilead < params.numLeadTimes; ilead++){
      //
      time_t validTime = genTime + ilead * params.delLeadTime;

      if (params.debug){
	cerr << "Generation time " << utimstr(genTime);
	cerr << " lead time " << ilead * params.delLeadTime;
	cerr << " => valid time " << utimstr(validTime) << endl;
      }
      //
      // Read the data.
      //
      DsMdvx inMdvMgr;

      time_t inLead = validTime - inGen.unix_time;

      inMdvMgr.setReadTime(Mdvx::READ_FIRST_BEFORE, params.InUrl, 
			   864000, validTime, inLead);

      if (inMdvMgr.readVolume()){
	cerr << "   Read failed for genTime " << utimstr(genTime);
	cerr << " valid time " << utimstr(validTime) << " from ";
	cerr << params.InUrl  << endl;
	cerr << "   specified input lead time of " << inLead << " seconds, cold start ";
	cerr << utimstr(inGen.unix_time) << endl << endl;
	cerr << inMdvMgr.getErrStr() << endl << endl;
	continue;
      }

      Mdvx::master_header_t Mhdr = inMdvMgr.getMasterHeader();
      //
      // Set the gen time.
      //
      Mhdr.time_gen = genTime;
      //
      // Change the name so it's clear that these are mucked with data.
      //
      sprintf(Mhdr.data_set_info, "MdvShuffleModelTimes");
      sprintf(Mhdr.data_set_name, "MdvShuffleModelTimes");
      sprintf(Mhdr.data_set_source, "MdvShuffleModelTimes");

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

	Fhdr.forecast_time = validTime;
	Fhdr.forecast_delta = validTime - genTime;

	if (params.debug >= Params::DEBUG_VERBOSE){
	  cerr << "      Field " << ifld << " " << Fhdr.field_name << endl;
	}

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
      Out.setWriteAsForecast();
      //
      if (Out.writeToDir(params.OutUrl)) {
	cerr << "Failed to wite to " << params.OutUrl << endl;
	continue;
      }

    }

    genTime += params.delGenTime;

  }


  return 0;

}



