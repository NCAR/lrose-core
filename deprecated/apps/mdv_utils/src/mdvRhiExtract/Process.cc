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
  New.addReadField( P->FieldName );

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     

  //
  // Get the desired field.
  //
  MdvxField *InField;
  
  InField = New.getFieldByName( P->FieldName );
  
  if (InField == NULL){
    cerr << "New field " << P->FieldName << " not found." << endl;
    return -1;
  }

  Mdvx::field_header_t fhdr = InField->getFieldHeader();

  fl32 *data = (fl32 *) InField->getVol();

  char filename[MAX_PATH_LEN];

  Mdvx::master_header_t mhdr = New.getMasterHeader();
  date_time_t dataTime;
  dataTime.unix_time = mhdr.time_centroid;
  uconvert_from_utime( &dataTime );

  sprintf(filename,"%s/%s_%d%02d%02d_%02d%02d%02d.dat",
	  P->OutDir, P->FieldName, 
	  dataTime.year, dataTime.month, dataTime.day, 
	  dataTime.hour, dataTime.min, dataTime.sec);

  ta_makedir_recurse( P->OutDir );

  FILE *ofp = fopen(filename,"w");
  if (ofp == NULL){
    cerr << "Failed to create " << filename << endl;
    return -1;
  }

  sprintf(filename,"%s/%s_%d%02d%02d_%02d%02d%02d.prf",
	  P->OutDir, P->FieldName, 
	  dataTime.year, dataTime.month, dataTime.day, 
	  dataTime.hour, dataTime.min, dataTime.sec);

  FILE *ofp2 = fopen(filename,"w");
  if (ofp2 == NULL){
    cerr << "Failed to create " << filename << endl;
    return -1;
  }

  int first = 1;
  double maxVal = -1.0;
  double maxBheight = -1.0;

  vector <double> backscatter;
  vector <double> heights;

  backscatter.clear(); heights.clear();

  for (double y=P->y.min; y<=P->y.max; y+=P->y.inc){
    double xTotal = 0.0;
    int xNum = 0;
    for (double x=P->x.min; x<=P->x.max; x+=P->x.inc){
    

      // Get the angle
      double angle = atan2(y,x);
      double angleDeg = 180.0*angle/3.1415927;

      double distAlongBeam = x/cos(angle);

      // Go from angleDeg, dist along beam to indices ix, iy

      int ix = (int)rint((distAlongBeam-fhdr.grid_minx)/fhdr.grid_dx);
      int iy = (int)rint((angleDeg-fhdr.grid_miny)/fhdr.grid_dy);

      // Do averaging.
      double total = 0.0;
      int numFound = 0;
      for (int ixx=ix - P->AveragingPoints;
	   ixx < ix - P->AveragingPoints + 1;
	   ixx++){

	for (int iyy=iy - P->AveragingPoints;
	     iyy < iy - P->AveragingPoints + 1;
	     iyy++){
	  
	  if ((ixx > -1) && (iyy > -1) && (ixx < fhdr.nx) && (iyy < fhdr.ny)){
	    
	    int index = iyy*fhdr.nx + ixx;
	    if (
		(data[index] != fhdr.bad_data_value) &&
		(data[index] != fhdr.missing_data_value)
		){
	      numFound++; total+=data[index];
	    }
	  }
	}
      }      
      
      if (numFound == 0){
	fprintf(ofp,"-999 ");
      } else {
	fprintf(ofp,"%g ", total/double(numFound));
	xTotal += (total/double(numFound));
	xNum++;
      }
    }
    fprintf(ofp,"\n");

    if (xNum == 0){
      fprintf(ofp2, "%g\t-999\n",y);
    } else {
       fprintf(ofp2, "%g\t%g\n", y, xTotal/double(xNum));

       heights.push_back(y*1000.0); // Push back height in meters
       backscatter.push_back(xTotal/double(xNum));

       if (first){
	 first = 0;
	 maxBheight = y;
	 maxVal = xTotal/double(xNum);
       } else {
	 if (maxVal < xTotal/double(xNum)){
	   maxBheight = y;
	   maxVal = xTotal/double(xNum);
	 }
       }
    }
  }
  
  fclose(ofp); fclose(ofp2);

  if ((P->Debug) || (P->printInfo)) cerr << endl;

  if (P->printInfo){

    if (first){
      cerr << "No maximum backscatter detected in RHI scan at " << utimstr(dataTime.unix_time) << endl;
    } else {
      cerr << "Maximum backscatter at " << utimstr(dataTime.unix_time) << " Z was ";
      cerr << maxVal << " at "  << maxBheight*1000.0 << " meters" << endl;
    }
    
    
    int numEntries = heights.size();
    if (numEntries < 10){
      cerr << "Insufficient data to perform gradient analysis" << endl;
    } else {
      //
      // Form gradient along vector, print out regions of high gradient
      //
      int numSequentialDetections=0;

      int printOutOn = 1;
      for (unsigned k=1; k < heights.size()-1; k++){
	double gr = fabs((backscatter[k+1]-backscatter[k])/(heights[k+1]-heights[k]));
	// cerr << k << " " << heights[k] << " " << backscatter[k] << " " << gr << " " << endl;
	if (gr < P->gradientPrintThresh){
	  numSequentialDetections=0;
	  printOutOn = 1;
	} else {
	  numSequentialDetections++;
	  if (numSequentialDetections >= P->sequentialDetectionsForPrint){
	    if (printOutOn) {
	      cerr << "   High gradient of " << gr << " backscatter/vertical meter at height " << heights[k] << " meters" << endl;
	    }
	    numSequentialDetections=0;
	    printOutOn = 0;
	  }
	}
      }
    }
    cerr << endl;
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










