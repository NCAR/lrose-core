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


#include "Process.hh"

#include <iostream>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxProj.hh>
using namespace std;

//
// Constructor
//
Process::Process(){
  return;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t T){


  if (P->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }


  //
  // Set up for the new data.
  //
  DsMdvx New;


  New.setDebug( P->Debug);

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     


  Mdvx::master_header_t InMhdr = New.getMasterHeader();
  //
  // Loop through the fields doing QC checks.
  //
  vector <string> failedFields;
  for (int ifld=0; ifld < P->field_n; ifld++){

    MdvxField *InField = New.getFieldByName( P->_field[ifld].fieldName );
    if (InField == NULL){
      cerr << "Failed to find field " << P->_field[ifld].fieldName << endl;
      return -1; // Arguably should exit here.
    }

    unsigned long numMissing = 0;
    unsigned long numOutsideMinMax = 0;
    unsigned numPlanesConsidered = 0;

    fl32 *InData = (fl32 *) InField->getVol();

    Mdvx::field_header_t InFhdr = InField->getFieldHeader();
    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

    for (int iz=0; iz < InFhdr.nz; iz++){

      //
      // See if we consider this plane.
      //
      bool doThisPlane = true;
      if (  P->_field[ifld].limitVertical ) {
	doThisPlane = false;
	if (  P->_field[ifld].usePlaneNumbers ){
	  if (
	      ( (int)rint( iz ) <= P->_field[ifld].vertMax ) &&
	      ( (int)rint( iz ) >= P->_field[ifld].vertMin )
	      ){
	    doThisPlane = true;
	  }
	} else {
	  if (
	      ( InVhdr.level[iz] <= P->_field[ifld].vertMax ) &&
	      ( InVhdr.level[iz] >= P->_field[ifld].vertMin )
	      ){
	    doThisPlane = true;
	  }
	}
      }
      
      if (doThisPlane){
	numPlanesConsidered++;
	for (int iy = 0; iy < InFhdr.ny; iy++){
	  for (int ix = 0; ix < InFhdr.nx; ix++){
	    int index = iz * InFhdr.nx * InFhdr.ny + iy * InFhdr.nx + ix;
	    if (
		(InData[index] == InFhdr.missing_data_value) ||
		(InData[index] == InFhdr.bad_data_value)
		){
	      numMissing++;
	    } else {
	      if ( (InData[index] < P->_field[ifld].min) || (InData[index] < P->_field[ifld].max))
		numOutsideMinMax++;
	    }
	  }
	}
      }
    }

    if (0==numPlanesConsidered*InFhdr.nx*InFhdr.ny){
      cerr << "WARNING : field with no data - were vertical limits mis-set?" << endl;
      return -1; // Unlikely. Probably bad setting of vert limits.
    }

    double pm = 100.0 * numMissing / double(numPlanesConsidered*InFhdr.nx*InFhdr.ny);
    double pmAfterMinMax = 100.0 * (numMissing + numOutsideMinMax) / double(numPlanesConsidered*InFhdr.nx*InFhdr.ny);

    bool fail = false;

    if ( pm > P->_field[ifld].pmBeforeMinMax ) fail = true;


    if ( P->_field[ifld].applyMinMax ){
      if ( pmAfterMinMax > P->_field[ifld].pmAfterMinMax ) fail = true;
      if ( (!(P->_field[ifld].allowOutsideRange)) && (numOutsideMinMax > 0)) fail = true;
    }

    //
    // Append this info to the log file. Delete it first
    // if we are on the first field.
    //
    if (ifld == 0) {
      unlink( P->logFilename );
    }
    FILE *fp = fopen( P->logFilename, "a" );
    if (fp == NULL){
      cerr << "ERROR : Unable to open " << P->logFilename << " in append mode." << endl;
      return -1;
    }

    if (ifld == 0) {
      fprintf(fp,"<mdvDataQC>\n\n");
      date_time_t T;
      T.unix_time = InMhdr.time_centroid;
      uconvert_from_utime( &T );

      fprintf(fp," <unix_time>%ld</unix_time>\n", T.unix_time);
      fprintf(fp," <year>%d</year>\n", T.year);
      fprintf(fp," <month>%02d</month>\n", T.month);
      fprintf(fp," <day>%02d</day>\n", T.day);
      fprintf(fp," <hour>%02d</hour>\n", T.hour);
      fprintf(fp," <min>%02d</min>\n", T.min);
      fprintf(fp," <sec>%02d</sec>\n", T.sec);
      
    }

    fprintf(fp,"\n <%s>\n", P->_field[ifld].fieldName);


    fprintf(fp,"  <pmBeforeMinMax>%g</pmBeforeMinMax>\n", pm);

    if ( P->_field[ifld].applyMinMax ){
      fprintf(fp,"  <numOutsideMinMax>%ld</numOutsideMinMax>\n", numOutsideMinMax);
      fprintf(fp,"  <pmAfterMinMax>%g</pmAfterMinMax>\n", pmAfterMinMax );
    }

    if (fail){
      fprintf(fp," <qcChecks> fail </qcChecks>\n");
    } else {
      fprintf(fp," <qcChecks> pass </qcChecks>\n");
    }

    fprintf(fp," </%s>\n", P->_field[ifld].fieldName);


    fclose(fp);

    if (fail) failedFields.push_back( P->_field[ifld].fieldName );

    if (P->Debug){
      cerr << "Field : " << P->_field[ifld].fieldName << endl;
      cerr << " Percent missing : " << pm << endl;
      if ( P->_field[ifld].applyMinMax ){
	cerr << " Number outside of min/max range : " << numOutsideMinMax << endl;
	cerr << " Percent missing after min/max exclusion : " << pmAfterMinMax << endl;
      }
      cerr << " QC check : ";
      if (fail)
	cerr << "Failed";
      else
	cerr << "Passed";
      cerr << endl;
    }

  }


  FILE *fp = fopen( P->logFilename, "a" );
  if (fp == NULL){
    cerr << "ERROR : Unable to open " << P->logFilename << " in append mode." << endl;
    return -1;
  }
  
  fprintf(fp,"\n</mdvDataQC>\n");

  fclose(fp);

  //
  // If we are set up so that if when one field fails, all must
  // fail, then return - generate no output.
  //
  if ((failedFields.size() > 0) && (P->allFieldsMustPass)) return 0;

  //
  // Otherwise, output those fields that have passed.
  //

  //
  // Set up the output.
  //
  Mdvx::master_header_t OutMhdr = InMhdr;

  DsMdvx Out;
  Out.setMasterHeader(OutMhdr);
  Out.clearFields();     

  for (int ifld=0; ifld < InMhdr.n_fields; ifld++){

    MdvxField *InField =  New.getFieldByNum( ifld );
    if (InField == NULL){
      cerr << "Something has gone horribly wrong..." << endl;
      exit(-1);
    }

    Mdvx::field_header_t InFhdr = InField->getFieldHeader();

    bool failedField = false;
    for (unsigned k=0; k < failedFields.size(); k++){
      if (
	  (0 == strcmp(InFhdr.field_name, failedFields[k].c_str())) ||
	  (0 == strcmp(InFhdr.field_name_long, failedFields[k].c_str()))
	  ){
	failedField = true; // Count this field out, it failed QC.
	break;
      }
    }
    if (failedField) continue;

    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();
    fl32 *InData = (fl32 *) InField->getVol();
    
    MdvxField *fld = new MdvxField(InFhdr, InVhdr, InData);
    if (fld->convertRounded(Mdvx::ENCODING_INT16,
                            Mdvx::COMPRESSION_ZLIB)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }  
    
    Out.addField(fld);

  }

  //
  // Add any chunks from the input.
  //
  for (int ic=0; ic < New.getNChunks(); ic++){
    MdvxChunk *inChunk = New.getChunkByNum( ic );
    //
    // The chunk must be created using 'new'.
    // The chunk object becomes 'owned' by the Mdvx object, and will be
    // freed by it.
    //
    MdvxChunk *outChunk = new MdvxChunk( *inChunk );
    Out.addChunk( outChunk );
  }

  if (P->forecastData) Out.setWriteAsForecast();

  if (Out.writeToDir(P->OutUrl)) {
    cerr << "Failed to write to " << P->OutUrl << endl;
    exit(-1);
  }      

  if (P->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  return;
}










