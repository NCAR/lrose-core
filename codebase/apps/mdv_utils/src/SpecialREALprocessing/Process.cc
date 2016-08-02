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
  OutputUrl = (char *)NULL;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t T){

  OutputUrl = STRdup(P->OutUrl);

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

  //
  // Set up the output.
  //
  Mdvx::master_header_t InMhdr = New.getMasterHeader();
  Mdvx::master_header_t OutMhdr = InMhdr;


  Out.setMasterHeader(OutMhdr);
  Out.clearFields();     


  MdvxField *InField;
  InField = New.getFieldByNum( 0 ); 


  Mdvx::field_header_t InFhdr = InField->getFieldHeader();
  Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

  MdvxProj Proj(InMhdr, InFhdr);

  fl32 *InData = (fl32 *) InField->getVol();

  fl32 *OutData = (fl32 *) malloc(sizeof(float) * 
				  InFhdr.nx *
				  InFhdr.ny *
				  InFhdr.nz);

  if (OutData == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  //
  // Init to missing.
  //

  for (int i=0; i < InFhdr.nx * InFhdr.ny * InFhdr.nz; i++){
    OutData[i] = InFhdr.missing_data_value;
  }

  int ll_Ix, ll_Iy, ur_Ix, ur_Iy;



  if (
      (Proj.latlon2xyIndex(P->ur_lat, P->ur_lon, ur_Ix, ur_Iy))
      ){
    cerr << "UR-Region is outside of grid." << endl;
    exit(-1);
  }

 if (
      (Proj.latlon2xyIndex(P->ll_lat, P->ll_lon, ll_Ix, ll_Iy))
      ){
    cerr << "ll-Region is outside of grid." << endl;
    exit(-1);
  }

 if (ll_Ix > ur_Ix){
   int temp = ur_Ix;
   ur_Ix = ll_Ix;
   ll_Ix = temp;
 }

 if (ll_Iy > ur_Iy){
   int temp = ur_Iy;
   ur_Iy = ll_Iy;
   ll_Iy = temp;
 }



 if (P->Debug){
   cerr << "X : " << ll_Ix << " to " << ur_Ix << endl;
   cerr << "Y : " << ll_Iy << " to " << ur_Iy << endl;
   
 }

 for (int iz = 0; iz < InFhdr.nz; iz++){
   for (int ix = ll_Ix; ix < ur_Ix+1; ix++){
     for (int iy = ll_Iy; iy < ur_Iy+1; iy++){
       
       int index = iz * InFhdr.ny * InFhdr.nx +
	 iy * InFhdr.nx    + ix;
       
       OutData[index] = InData[index];
       
     }
   }
 }



  MdvxField *fld = new MdvxField(InFhdr, InVhdr, (void *)OutData);

  /*
  fld->setFieldName("B");
  fld->setUnits("none");
  fld->setFieldNameLong("B");
  */    
  if (fld->convertRounded(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }  
  Out.addField(fld);

 //
 // Undo the log relationship that was done in the ingest.
 //
 for (int i=0; i < InFhdr.nx * InFhdr.ny * InFhdr.nz; i++){
   if (
       (OutData[i] != InFhdr.missing_data_value) &&
       (OutData[i] != InFhdr.bad_data_value)
       ){
     OutData[i] = pow(10.0, OutData[i]/10.0);
   }
 }
 
 
 //
 // Normalize in X (ie. along each beam for Polar data) if desired.
 //
 if (P->deMeanInX){
   
   for (int iz = 0; iz < InFhdr.nz; iz++){
     for (int iy = ll_Iy; iy < ur_Iy+1; iy++){
       double totalInBeam = 0.0;
       int numInBeam = 0;
       for (int ix = ll_Ix; ix < ur_Ix+1; ix++){
	 
	 int index = iz * InFhdr.ny * InFhdr.nx +
	   iy * InFhdr.nx    + ix;
	 
	 if ( 
	     (OutData[index] != InFhdr.missing_data_value) &&
	     (OutData[index] != InFhdr.bad_data_value)
	     ){
	   numInBeam++; totalInBeam += OutData[index];	    
	 }
       }
       double meanInBeam = totalInBeam / double (numInBeam);
       //
       // De-trend the beam.
       //
       for (int ix = ll_Ix; ix < ur_Ix+1; ix++){
	 
	 int index = iz * InFhdr.ny * InFhdr.nx +
	   iy * InFhdr.nx    + ix;
	 
	 if ( 
	     (OutData[index] != InFhdr.missing_data_value) &&
	     (OutData[index] != InFhdr.bad_data_value)
	     ){
	   OutData[index] =  OutData[index]/meanInBeam - 1.0;	    
	 }
       }
     }
   }
   /*    
   MdvxField *deMeanXfld = new MdvxField(InFhdr, InVhdr, (void *)OutData);
   
   deMeanXfld->setFieldName("deMeanXfld");
   deMeanXfld->setUnits("none");
   deMeanXfld->setFieldNameLong("deMeanXfld");
     
   if (deMeanXfld->convertRounded(Mdvx::ENCODING_INT8,
				  Mdvx::COMPRESSION_ZLIB)){
     cerr << "convertRounded failed - I cannot go on." << endl;
     exit(-1);
   }  
   Out.addField(deMeanXfld);
   */
 }


 if (!(P->deMeanInX)){

   //
   // Get the mean.
   //
   
   double total = 0.0;
   int num = 0;
   for (int i=0; i < InFhdr.nx * InFhdr.ny * InFhdr.nz; i++){
     if (
	 (OutData[i] != InFhdr.missing_data_value) &&
	 (OutData[i] != InFhdr.bad_data_value)
	 ){
       total += OutData[i];
       num++;
     }
   }
   double mean = total / double(num);
   
   //
   // Mean adjust.
   //
   for (int i=0; i < InFhdr.nx * InFhdr.ny * InFhdr.nz; i++){
     if (
	 (OutData[i] != InFhdr.missing_data_value) &&
	 (OutData[i] != InFhdr.bad_data_value)
	 ){
       OutData[i] = OutData[i]/mean - 1.0;
     }
   }
 }

 MdvxField *db_fld = new MdvxField(InFhdr, InVhdr, (void *)OutData);

 db_fld->setFieldName("deltaBeta");
 db_fld->setUnits("none");
 db_fld->setFieldNameLong("deltaBeta");
 
 if (db_fld->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_ZLIB)){
   cerr << "convertRounded failed - I cannot go on." << endl;
   exit(-1);
 }  
 Out.addField(db_fld);

  //
  // Compute the root mean of deltaBeta squared.
  //
  double total = 0.0;
  int num = 0;
  for (int i=0; i < InFhdr.nx * InFhdr.ny * InFhdr.nz; i++){
    if (
	(OutData[i] != InFhdr.missing_data_value) &&
	(OutData[i] != InFhdr.bad_data_value)
	){
      total += OutData[i] * OutData[i];
      num++;
    }
  }
  double mean = sqrt(total / double(num));
  //
  // OK - now do the auto correlation function in X and Y.
  //
  double *xcorr = (double *) malloc(sizeof(double) * P->numAutoCorr );
  double *ycorr = (double *) malloc(sizeof(double) * P->numAutoCorr );

  if ((xcorr==NULL) || (ycorr == NULL)){
    cerr << "Malloc failure!" << endl;
    exit(-1);
  }
  

  for (int k=0; k < P->numAutoCorr; k++){
    //
    xcorr[k] = -9999.0; ycorr[k] = -9999.0;
    //
    double totalX = 0.0; double totalY = 0.0;
    int numX = 0; int numY = 0;
    //
    for (int ix = ll_Ix; ix < ur_Ix+1; ix++){
      for (int iy = ll_Iy; iy < ur_Iy+1; iy++){
	//
	int index = iy * InFhdr.nx + ix;
	//
	if (ix < ur_Ix - P->numAutoCorr){
	  //
	  // We have space to do it in X. See if we have two good values.
	  //
	  int indexPlusX = iy * InFhdr.nx + ix + k;
	  //
	  if (
	      (OutData[index] != InFhdr.missing_data_value) &&
	      (OutData[index] != InFhdr.bad_data_value) &&
	      (OutData[indexPlusX] != InFhdr.missing_data_value) &&
	      (OutData[indexPlusX] != InFhdr.bad_data_value)
	      ){
	    totalX = totalX + OutData[index] * OutData[indexPlusX];
	    numX++;
	  }
	}
	//
	if (iy < ur_Iy - P->numAutoCorr){
	  //
	  // We have space to do it in Y. See if we have two good values.
	  //
	  int indexPlusY = (iy + k) * InFhdr.nx + ix;
	  //
	  if (
	      (OutData[index] != InFhdr.missing_data_value) &&
	      (OutData[index] != InFhdr.bad_data_value) &&
	      (OutData[indexPlusY] != InFhdr.missing_data_value) &&
	      (OutData[indexPlusY] != InFhdr.bad_data_value)
	      ){
	    totalY = totalY + OutData[index] * OutData[indexPlusY];
	    numY++;
	  }
	  
	}
	
      }
    }
    if (numX > 0) xcorr[k] = totalX / double(numX);
    if (numY > 0) ycorr[k] = totalY / double(numY);
  }

  // Normalize - a little wonky.

  if (P->normalize){
    for (int k = P->numAutoCorr-1; k > -1; k--){
      if (xcorr[k] != -9999.0) xcorr[k] = xcorr[k] / xcorr[0];
      if (ycorr[k] != -9999.0) ycorr[k] = ycorr[k] / ycorr[0];
    }
  }

  //
  // Bang these out to an ASCII file.
  //
  date_time_t dataTime, tStart;
  dataTime.unix_time = T;
  //
  uconvert_from_utime( &dataTime );
  //
  tStart = dataTime;
  tStart.hour = 0; tStart.min = 0; tStart.sec = 0;
  uconvert_to_utime( &tStart );
  //
  char outCorrFileName[MAX_PATH_LEN];

  if (ta_makedir_recurse( P->asciiOutDir )){
    cerr << "Failed to create directory " << P->asciiOutDir << endl;
  }
  //
  // Write the RMS Delta Beta out - only one number - to an ASCII file.
  //
  sprintf(outCorrFileName,
	  "%s/RootMeanSquaredDeltaBeta_%d%02d%02d_%02d%02d%02d.dat",
	  P->asciiOutDir, dataTime.year, dataTime.month, dataTime.day, 
	  dataTime.hour, dataTime.min, dataTime.sec);

  FILE *ofp = fopen(outCorrFileName,"w");
  if (ofp == NULL){
    cerr << "Failed to open " << outCorrFileName << endl;
    exit(-1);
  }

  fprintf(ofp,"%d\t%g\n", int(dataTime.unix_time - tStart.unix_time), mean);

  fclose( ofp );



  sprintf(outCorrFileName,
	  "%s/AutoCorr_%d%02d%02d_%02d%02d%02d.dat",
	  P->asciiOutDir, dataTime.year, dataTime.month, dataTime.day, 
	  dataTime.hour, dataTime.min, dataTime.sec);

  ofp = fopen(outCorrFileName,"w");
  if (ofp == NULL){
    cerr << "Failed to open " << outCorrFileName << endl;
    exit(-1);
  }

  for (int k=0; k < P->numAutoCorr; k++){
    fprintf(ofp, "%g\t%g\t%g\t%g\n",
	    1000.0*k*InFhdr.grid_dx,
	    xcorr[k],
	    1000.0*k*InFhdr.grid_dy,
	    ycorr[k]);
  }

  fclose(ofp);


  if (P->Debug){
    cerr << "Root mean of delta beta squared is " << mean << endl;
  }

  if (P->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  free(OutData); free(xcorr); free(ycorr);

  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  //
  // Only do the write if fields were added.
  //

  Mdvx::master_header_t Mhdr = Out.getMasterHeader();

  if (Mhdr.n_fields > 0){
    if (Out.writeToDir(OutputUrl)) {
      cerr << "Failed to wite to " << OutputUrl << endl;
      exit(-1);
    }      
  }
  free(OutputUrl);
}










