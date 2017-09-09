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

/**
 * @file Main.cc
 *
 * A simple app to do a quick comparison between two Mdv files.
 * This app is based on MdvCompare, but is even simpler.
 *
 * @author P J Prestopnik
 * @version $Id $
 */

// SYSTEM INCLUDES

#include <iostream>
#include <cstdio>
#include <math.h>
#include <cstdlib>

// PROJECT INCLUDES
#include <toolsa/umisc.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxTimes.hh>
#include <Mdv/MdvxField.hh>    


// LOCAL INCLUDES

//Forward declarations
int Compare(const void *a, const void *b);
void printUsage(char *);
void printBadMissingHeader();
void printBadOrMissing(char* file, char* field, int index, fl32 data, fl32 missing, fl32 bad);

using namespace std;

int main(int argc, char *argv[]){

  //need usage?
  for (int i=0; i < argc; i++){
    if (
        (!(strcmp(argv[i], "-h"))) ||
        (!(strcmp(argv[i], "--"))) ||
        (!(strcmp(argv[i], "-?")))
        ){
      printUsage(argv[0]);
      exit(0);
    }
  }

  //
  // See if we have enough command line args.
  //
  if (argc < 5){
      printUsage(argv[0]);
      exit(0);
  }
  char *mdvFile1 = argv[1];
  char *mdvField1 = argv[2];
  char *mdvFile2 = argv[3];
  char *mdvField2 = argv[4];
  float tolerance = 0.0001;
  bool verbose = false;
  if(argc > 5){
    tolerance = atof(argv[5]);
  }
  if(argc > 6){
    verbose = true;
  }


  ///////////////////////////////////////////////////////////////
  //
  // Read in MDV data into object A
  //
  //
  DsMdvx A;
  A.setReadPath( mdvFile1 ); // Set MDV filename (note : not URL)
  A.addReadField( mdvField1 ); // Only read that one field.           

  A.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  A.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (A.readVolume()){
    cerr << "Failed to read field " << mdvField1 << " from file " << mdvFile1 << endl;
    exit (-1);
  }
  
  //
  // Get the data and field header.
  //
  MdvxField *AField = A.getFieldByName( mdvField1 );
  fl32 *AData = (fl32 *) AField->getVol();
  Mdvx::field_header_t AFhdr = AField->getFieldHeader();
  
  //
  // Get master header for A so we can get time_centroid
  //
  //Mdvx::master_header_t AMhdr = A.getMasterHeader();
  
  //
  // Apply transform if needed.
  //
  if (AField->getFieldHeader().transform_type == Mdvx::DATA_TRANSFORM_LOG)  {
    cerr << "found a field that has log transform: " << AField->getFieldName() << endl;
    int ret = AField->transform2Linear();
    if (ret != 0) {
      cerr << "Transform failed for " << AField->getFieldName() << endl;
      cerr << AField->getErrStr() << endl;
      cerr << "Program terminated." << endl;
      exit(-1);
    }
  }


  ///////////////////////////////////////////////////////////////
  //
  // Read in MDV data into object B
  //
  //
  DsMdvx B;
  B.setReadPath( mdvFile2 ); // Set MDV filename (note : not URL)
  B.addReadField( mdvField2 ); // Only read that one field.           

  B.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  B.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (B.readVolume()){
    cerr << "Failed to read field " << mdvField2 << " from file " << mdvFile2 << endl;
    exit (-1);
  }
  //
  // Get the data and field header.
  //
  MdvxField *BField = B.getFieldByName( mdvField2 );
  fl32 *BData = (fl32 *) BField->getVol();
  Mdvx::field_header_t BFhdr = BField->getFieldHeader();
  
  //
  // Apply transform if needed.
  //
  if (BField->getFieldHeader().transform_type == Mdvx::DATA_TRANSFORM_LOG)  {
    cerr << "found a field that has log transform: " << BField->getFieldName() << endl;
    int ret = BField->transform2Linear();
    if (ret != 0) {
      cerr << "Transform failed for " << BField->getFieldName() << endl;
      cerr << BField->getErrStr() << endl;
      cerr << "Program terminated." << endl;
      exit(-1);
    }
  }

  //
  // Gross check of geometry
  //

  if (
      (BFhdr.nx != AFhdr.nx) ||
      (BFhdr.ny != AFhdr.ny) ||
      (BFhdr.nz != AFhdr.nz)
      ){
    cerr << mdvFile1 << ":" << mdvField1 << " and "
	 << mdvFile2 << ":" << mdvField2 << "have different grid sizes.";
    cerr << "Program terminated." << endl;
    exit(-1);
  }



  int totalVLevelsDifferent = 0;



  for (int iz=0; iz < BFhdr.nz; iz++){
    //
    // Initialize for stats calc.
    //
    int Num=0;
    double Min= 0;
    double Max= 0;
    double Mean=0;
    double TopTen = 0;
    double Median = 0;
    double BotTen = 0;
    double STD = 0;
    double RMS = 0;
    double Total = 0.0;


    //
    // Loop through subgrid.
    //
    int gridSize = BFhdr.nx * BFhdr.ny; // * BFhdr.nz;

    fl32 *Diff = (float *)malloc(gridSize*sizeof(float));
    if (Diff==NULL){
      fprintf(stderr,"Malloc failed.\n");
      exit(-1);
    }

    for (int ix=0; ix < BFhdr.nx; ix++){
      for (int iy=0; iy < BFhdr.ny; iy++){   

	//
	// ix and iy point inside grid.
	//
	int index = iy*AFhdr.nx +ix; //iz*AFhdr.nx*AFhdr.ny + iy*AFhdr.nx +ix;
	//
	// Check missing/bad values.
	//
	if (
	    (AData[index] != AFhdr.missing_data_value) &&
	    (AData[index] != AFhdr.bad_data_value) &&
	    (BData[index] != BFhdr.missing_data_value) &&
	    (BData[index] != BFhdr.bad_data_value)
	    ) {
	  Diff[Num]=AData[index] - BData[index];
	  Total = Total + Diff[Num];
	  Num++; // Get ready for the next one.
	}
	else
	  if (AData[index] != BData[index]
	      && !(AData[index] == AFhdr.missing_data_value && 
	           BData[index] == BFhdr.missing_data_value)
	      && !(AData[index] == AFhdr.bad_data_value && 
	           BData[index] == BFhdr.bad_data_value) )
	    {
	      printBadMissingHeader();
	      printBadOrMissing(mdvFile1, mdvField1, index, AData[index], 
				AFhdr.missing_data_value, 
				AFhdr.bad_data_value);
	      printBadOrMissing(mdvFile2, mdvField2, index, BData[index], 
				BFhdr.missing_data_value, 
				BFhdr.bad_data_value);
	    }
      } // iy
    } // ix
    //
    // If we have data, do statistics.
    //
    if (Num > 0){
      //
      // Compute the mean
      //
      Mean = Total / double(Num);
      //
      // Then the standard deviation.
      //
      double D=0.0;
      for (int i=0; i< Num; i++){
        D=D+(Diff[i]-Mean)*(Diff[i]-Mean);
      }
      double MD = D/Num;
      STD = sqrt( MD );
      //
      // Then the RMS error
      //
      double S=0.0;
      for (int i=0; i< Num; i++){
        S=S+Diff[i]*Diff[i];
      }
      double MS = S / Num;
      RMS = sqrt( MS );
      //
      // And then the 10 percentile, median, and 90 percentile.
      //
      int BotTenIndex = (int)rint((Num-1) * 0.1);
      int MidIndex =    (int)rint((Num-1) * 0.5);
      int TopTenIndex = (int)rint((Num-1) * 0.9);
      //
      // Sort the data into order.
      //
      qsort(Diff, Num, sizeof(float), Compare);
      //
      // Pick off min, bottom tenth percentile, median, 
      // top tenth percentile,
      // and max.
      //
      TopTen = Diff[TopTenIndex];
      Median = Diff[MidIndex];
      BotTen = Diff[BotTenIndex];
      Min = Diff[0]; Max = Diff[Num-1];

      //
      // Print to cout.
      //
      cerr << "Z = " << iz << ": " << endl;
      if ( (RMS > tolerance) || (verbose && (RMS > 0.001) ) )
		{
		  cerr << "Differences between " << mdvFile1 << ":" << mdvField1 << " and "
			   << mdvFile2 << ":" << mdvField2;
		  if (RMS < tolerance) 
			cerr << " are within the tolerance." << endl;
		  else{
			totalVLevelsDifferent++;
			cerr << endl;
		  }
		  printf("Minimum: %10.4f\nMean: %10.4f\nMax: %10.4f\nBottom10%%: %10.4f\nMedian: %10.4f\nTop10%%: %10.4f\nRMS: %10.4f\nSTD: %10.4f\n",
				 Min,Mean,Max,BotTen,Median,TopTen,RMS,STD);
		}
      else
        cerr << "No Differences between " << mdvFile1 << ":" << mdvField1 << " and "
			 << mdvFile2 << ":" << mdvField2 << endl;
	}
	else
	  cerr << "No valid data found" << endl; 

	cerr << endl;
	free( Diff );


  } // iz
  
  cerr << "********  SUMMARY ************* " << endl; 
  cerr << "Number of V-Levels with significant differences: " << totalVLevelsDifferent;
 
  return 0;

}

//////////////////////////////////////////
//
// Comparison routine for ascending order.
//
int Compare(const void *a, const void *b){

  float *x = (float *)a;  float *y = (float *)b;

  if (*x >  *y) return 1;
  if (*x <  *y) return -1;
  return 0;

}

void printUsage(char *progName)
{
  cerr << progName << " is a simple app to compare mdv fields" << endl;
  cerr << "Usage: " << endl;
  cerr << progName << " file1 field1 file2 field2 [tolerance verbose]" << endl;
  cerr << "eg. MdvCompare myDir/20100926/013000.mdv div myOtherDir/20100926/013000.mdv div" << endl;
  cerr << "eg. MdvCompare myDir/20100926/013000.mdv div myOtherDir/20100926/013000.mdv div 0.0001" << endl;
  cerr << "eg. MdvCompare myDir/20100926/013000.mdv div myOtherDir/20100926/013000.mdv div 0.0001 verbose" << endl;

}


void printBadMissingHeader()
{
  static bool hasPrinted = false;
  
  if (!hasPrinted)
    {
      cerr << "Inconsistant Bad/Missing Data Found" << endl;
      cerr << "-----------------------------------" << endl;
      hasPrinted = true;
    }
}


void printBadOrMissing(char* file, char* field, int index, fl32 data, fl32 missing, fl32 bad)
{
  if (data == missing)
    cerr << "In " << file << ":" << field << " @ " << index << " the data is missing." << endl;

  if (data == bad)
    cerr << "In " << file << ":" << field << " @ " << index << " the data is bad." << endl;
}
