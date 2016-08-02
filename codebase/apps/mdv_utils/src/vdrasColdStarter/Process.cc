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
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
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

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  for (int ifld=0; ifld < P->fieldList_n; ifld++){
    New.addReadField(P->_fieldList[ifld].fieldName);
  }

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     

  //
  // Commence loop through fields.
  //

  bool failed = false;

  for (int ifld=0; ifld < P->fieldList_n; ifld++){
    //
    // Get the desired field.
    //
    MdvxField *InField = New.getFieldByName( P->_fieldList[ifld].fieldName );



    if (InField == NULL){
      cerr << "Field " << P->_fieldList[ifld].fieldName << " not found, skipping that test." << endl;
      continue;
    }

    Mdvx::field_header_t InFhdr = InField->getFieldHeader();

    fl32 *InData = (fl32 *) InField->getVol();

    //
    // Apply the range limit, if requested
    //
    if (P->_fieldList[ifld].applyMinMaxLimits){
      for (int i=0; i < InFhdr.nx * InFhdr.ny * InFhdr.nz; i++){
	if ((InData[i] != InFhdr.bad_data_value) && (InData[i] != InFhdr.missing_data_value)){
	  if (InData[i] < P->_fieldList[ifld].minLimit) InData[i] = InFhdr.bad_data_value;
	  if (InData[i] > P->_fieldList[ifld].maxLimit) InData[i] = InFhdr.bad_data_value;
	}
      }
    }
    //
    // Get the percent bad
    //
    unsigned long numBad = 0;
    for (int i=0; i < InFhdr.nx * InFhdr.ny * InFhdr.nz; i++){
      if ((InData[i] == InFhdr.bad_data_value) || (InData[i] == InFhdr.missing_data_value)) numBad++;
    }

    double percentBad = 100.0*double(numBad)/double(InFhdr.nx * InFhdr.ny * InFhdr.nz);

    if (P->Debug){
      cerr << "Field " << P->_fieldList[ifld].fieldName << " was " << percentBad << " percent bad,";
      cerr << " require less than " << P->_fieldList[ifld].maxPercentBad << " percent bad." << endl;
    }

    if (percentBad > P->_fieldList[ifld].maxPercentBad){

      failed = true;
    
      if (P->Debug)
	cerr << "Spawning script " << P->failScript << endl;
   
      system(P->failScript);

      //
      // No need to do other tests in this case
      //
      break;

    }
      
  }

  if (P->Debug){
    if (failed)
      cerr << "Data at " << utimstr(T) << " failed QC tests." << endl << endl;
    else
      cerr << "Data at " << utimstr(T) << " passed QC tests." << endl << endl;
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










