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
int Process::Derive(Params *tdrpParams, time_t T){

  if (tdrpParams->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }

  //
  // Set up for the new data.
  //
  DsMdvx New;


  New.setReadTime(Mdvx::READ_FIRST_BEFORE, tdrpParams->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  New.addReadField( tdrpParams->missingSpec.fieldName );

  if ((New.readVolume())){
    cerr << "Failed to read volume at " << utimstr(T) << ", field " <<  tdrpParams->missingSpec.fieldName << " may not be present." << endl;
    return -1;
  } else {
    MdvxField *field= New.getFieldByNum( 0 );
    if (field == NULL){
      cerr << "Did not find field " << tdrpParams->missingSpec.fieldName << " in volume at " << utimstr(T) << endl;
      return -1;
    } else {
      Mdvx::field_header_t fhdr = field->getFieldHeader();
      fl32 *data = (fl32 *) field->getVol();
      unsigned long num = fhdr.nx * fhdr.ny * fhdr.nz;
      unsigned long numBad = 0L;
      unsigned long numBadThisAz;
      if (tdrpParams->LimitAz){
        unsigned long count = 0;
        unsigned long minAzInd = floor(tdrpParams->MinAz/fhdr.grid_dy);
        unsigned long maxAzInd = ceil(tdrpParams->MaxAz/fhdr.grid_dy);
        if (tdrpParams->Debug){
          cerr << "minAzInd: " << minAzInd << " maxAzInd: " << maxAzInd << endl;
        }
      /* loop through the elevations, then the azimuths, then out the radials 
         this way a statistic for each azimuth can be calculated if desired */
        for (int ii=0; ii < fhdr.nz; ii++){
          for (int jj=0; jj < fhdr.ny; jj++){
            numBadThisAz = 0;
            for (int kk=0; kk < fhdr.nx; kk++){
              if ((jj >= (int)minAzInd) && (jj <= (int)maxAzInd)){ 
	         if ((data[count] == fhdr.bad_data_value) || (data[count] == fhdr.missing_data_value)) {
                    numBad++;
                    numBadThisAz++;
                 }
              }
              count += 1;
            }
         // if (tdrpParams->Debug){
         //   if (jj >= minAzInd & jj <= maxAzInd){ 
         //      percentBadThisAz = 100.0*double(numBadThisAz)/double(fhdr.ny);
	 //      cerr << jj << " " <<  percentBadThisAz << endl;
         //   }
         // }
          }
        }
      } else {
         for (unsigned long k=0; k < num; k++){
	   if ((data[k] == fhdr.bad_data_value) || (data[k] == fhdr.missing_data_value)) numBad++;
         }
      }

      //
      // Now we have percent bad
      //
      double percentBad = 100.0*double(numBad)/double(num);
      if (tdrpParams->Debug){
	cerr << "At " <<  utimstr(T) << " field " <<  tdrpParams->missingSpec.fieldName << " was " <<  percentBad << " percent missing." << endl;
      }
      if ((percentBad > tdrpParams->missingSpec.percentMissingMax) && (tdrpParams->missingSpec.deleteData)){
	  string filename( New.getPathInUse());
// run a script if we need to
     if (tdrpParams->script.runScript){
	   char com[1024];
	   sprintf(com, "%s %s", tdrpParams->script.scriptName, filename.c_str() );
	   if (tdrpParams->Debug){
		 cerr << "Running " << com << endl;
       }
	   system(com);
    }

	if (unlink( filename.c_str() )){
	  cerr << "Problems deleting " << filename << endl;
	  cerr << strerror(errno) << endl;
	}

	if (tdrpParams->Debug){
	  cerr << "Deleted " << filename << endl;
	}
      }
      
      if ((percentBad > tdrpParams->missingSpec.percentMissingMax) && (!(tdrpParams->missingSpec.deleteData)) && tdrpParams->Debug){
	cerr << "File " << New.getPathInUse() << " would have been deleted." << endl;
      }
      
    }
  }


  if (tdrpParams->Debug){
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










