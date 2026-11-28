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
#include "fusionFile.hh"
#include <cstdio>
#include <ctype.h>
#include <vector>
#include <stdlib.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/umisc.h>

//
// Constructor. Does the work of reading the MDV files.
//
fusionFile::fusionFile(Params *TDRP_params,
		       time_t dataTime,
		       char *fusionFileName){

  _OK = false;
  _data = NULL;
  _fieldName = "";

  FILE *fp = fopen(fusionFileName, "r");
  if (fp == NULL){
    cerr << fusionFileName << " not found!" << endl;
    return;
  }

  //
  // Allocate space for the grid and initialze it all to the bad value.
  //
  _data = (float *) malloc(TDRP_params->output_grid.nx *
			   TDRP_params->output_grid.ny *
			   TDRP_params->output_grid.nz *
			   sizeof(float) );

  if (_data == NULL){
    cerr << " Malloc on grid failed!" << endl;
    exit(-1);
  }
  
  for (int il=0; 
       il < TDRP_params->output_grid.nx * TDRP_params->output_grid.ny * TDRP_params->output_grid.nz; 
       il++){
    _data[il] = fusionFile::badVal;
  }



  bool gotFieldName = false;
  static const int lineLen = 2048;

  char fname[256];

  char Line[lineLen];
  do {
    if (NULL==fgets(Line, lineLen, fp)){
      cerr << "Early end of file!" << endl;
      free(_data);
      _data = NULL;
      return;
    }

    if (
	(strlen(Line) < 5) ||
	(Line[0]=='#')
	){
      continue;
    }    
    
    if (!(strncmp(Line,"OUTPUT_FIELD_NAME:", strlen("OUTPUT_FIELD_NAME:")))){

      char *p = Line + strlen("OUTPUT_FIELD_NAME:");
      
      fusionFile::_removeSpaces(p, fname);
      _fieldName = fname;
      if (TDRP_params->Debug){
	cerr << "Field name : " << _fieldName << endl;
      }
      gotFieldName = true;
    }
    

  } while (gotFieldName == false);

  //
  // Now read the entries and
  // push them back into a vector of entries.
  //

  vector<fusionFile::_fuseFieldEntryType> Entries;

  while (NULL != fgets(Line, lineLen, fp)){

    if (
	(strlen(Line) < 5) ||
	(Line[0]=='#')
	){
      continue;
    }

    if (TDRP_params->Debug){
      cerr << Line;
    }

    fusionFile::_fuseFieldEntryType entry;
    //
    // Use the Powerful strtok function suggested by Frank
    // to parse the string. Do environment variable expansion
    // on the URL's.
    //
    char *p;
    char buffer[2048];
    p = strtok(Line, " ");
    if (p == NULL){
      cerr << "Having problems with line " << Line;
      exit(-1);
    }
    sprintf(buffer,"%s", p);
    if (usubstitute_env(buffer, 2048)){
      cerr << "Failed to expand " << buffer << endl;
      exit(-1);
    }
    entry.url = buffer;

    entry.fieldName = strtok(NULL," ");
    entry.lookbackSecs = atoi(strtok(NULL," "));
    entry.weight = atof(strtok(NULL," "));


    p = strtok(NULL, " ");
    if (p == NULL){
      cerr << "Having problems with line " << Line;
      exit(-1);
    }
    sprintf(buffer,"%s", p);
    if (usubstitute_env(buffer, 2048)){
      cerr << "Failed to expand " << buffer << endl;
      exit(-1);
    }
    entry.filterUrl = buffer;

    int icount = 0;
    do {
      p = strtok(NULL," ");
      if (p != NULL){
	entry.planeNums.push_back(atoi( p ));
	icount ++;
      } 
    } while ((icount != TDRP_params->output_grid.nz) && (p != NULL));
      

    if (TDRP_params->Debug){
      cerr << "        URL : " << entry.url << endl;
      cerr << "        FIELD : " << entry.fieldName << endl;
      cerr << "        LOOKBACK : " << entry.lookbackSecs << endl;
      cerr << "        WEIGHT : " << entry.weight << endl;
      cerr << "        FILTER URL : " << entry.filterUrl << endl;
      for (unsigned ip=0; ip < entry.planeNums.size(); ip++){
	cerr << "        LEVEL " << ip+1 << " : " << entry.planeNums[ip] << endl;
      }
    }
    //
    // Check that we have the right number of vertical planes.
    //
    if ((int)entry.planeNums.size() != TDRP_params->output_grid.nz){
      cerr << "ERROR - expected " << TDRP_params->output_grid.nz;
      cerr << " vertical levels, only got " << entry.planeNums.size() << endl;
      exit(-1);
    }

    Entries.push_back( entry );

  }
  //
  // Close the input file, and process the list of
  // grids we have into the _data array.
  //
  fclose(fp);
  fusionFile::_processEntries( Entries, TDRP_params, dataTime );

  _OK = true;

  return;

}


void fusionFile::_processEntries( vector<fusionFile::_fuseFieldEntryType> &Entries,
			     Params *TDRP_params, time_t dataTime ){

  //
  // This is the real innards of this program - where the various fields
  // are combined by linear combination of inputs. This is done by adding
  // the weighted values in the '_data' array and keeping the sum of the
  // weights in a local 'sumWeights' array. After the summation is complete
  // the data are divided by the sum of the weights.
  //

  float *sumWeights = (float *) malloc(TDRP_params->output_grid.nx *
				       TDRP_params->output_grid.ny *
				       TDRP_params->output_grid.nz *
				       sizeof(float) );


  if (sumWeights == NULL){
    cerr << " Malloc on grid failed!" << endl;
    exit(-1);
  }
  //
  // Set the sum of weights to 0.
  //
  for (int il=0; 
       il < TDRP_params->output_grid.nx * TDRP_params->output_grid.ny * TDRP_params->output_grid.nz; 
       il++){
    sumWeights[il] = 0.0;
  }
  //
  // Loop through the entries and incorporate them as we go.
  //
  for (unsigned ie=0; ie < Entries.size(); ie++){

    // Read the data.

    DsMdvx mdvMgr;

    Mdvx::read_search_mode_t readMode = Mdvx::READ_FIRST_BEFORE;

    switch (TDRP_params->mdv_read_method){

    case Params::MDV_READ_FIRST_BEFORE :
      readMode = Mdvx::READ_FIRST_BEFORE;
      break;
      
    case Params::MDV_READ_CLOSEST :
      readMode = Mdvx::READ_CLOSEST;
      break;

    default :
      cerr << "Undefined MDV read mode!" << endl;
      exit(-1); // Highly unlikely.
      break;
      
    }

    mdvMgr.setReadTime(readMode, 
		       Entries[ie].url, 
		       Entries[ie].lookbackSecs,
		       dataTime );
    mdvMgr.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    mdvMgr.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    //
    // Remap to the specified grid.
    //

    switch ( TDRP_params->output_projection){

    case Params::PROJ_FLAT:
      mdvMgr.setReadRemapFlat(TDRP_params->output_grid.nx,
			      TDRP_params->output_grid.ny,
			      TDRP_params->output_grid.minx, TDRP_params->output_grid.miny,
			      TDRP_params->output_grid.dx, TDRP_params->output_grid.dy,
			      TDRP_params->output_grid.latOrig, 
			      TDRP_params->output_grid.lonOrig,
			      TDRP_params->output_grid.flatEarthRotation );
      
      break;
      
    case Params::PROJ_LATLON:
      mdvMgr.setReadRemapLatlon(TDRP_params->output_grid.nx, TDRP_params->output_grid.ny,
				TDRP_params->output_grid.minx, TDRP_params->output_grid.miny,
				TDRP_params->output_grid.dx, TDRP_params->output_grid.dy);

      break;

    case Params::PROJ_LAMBERT:
      mdvMgr.setReadRemapLc2(TDRP_params->output_grid.nx, TDRP_params->output_grid.ny,
			     TDRP_params->output_grid.minx, TDRP_params->output_grid.miny,
			     TDRP_params->output_grid.dx, TDRP_params->output_grid.dy,
			     TDRP_params->output_grid.latOrig,
			     TDRP_params->output_grid.lonOrig,
			     TDRP_params->output_grid.lambertLat1,
			     TDRP_params->output_grid.lambertLat2);

      break;
      
    default:
      cerr << "Unsupported projection." << endl;
      exit(-1);
      break;
      
    }

    //
    // Specify the field we want to read.
    //
    mdvMgr.addReadField( Entries[ie].fieldName );
    //
    // If the read fails, skip this entry.
    //
    if (mdvMgr.readVolume()){
      cerr << "Read failed at " << utimstr(dataTime) << " from ";
      cerr << Entries[ie].url  << endl;
      cerr << mdvMgr.getErrStr() << endl;
      continue;
    }

    if (TDRP_params->Debug){
      cerr << "Path in use : " << endl;
      cerr << mdvMgr.getPathInUse() << endl;
    }

    MdvxField *Field = mdvMgr.getFieldByNum( 0 );
    if (Field == NULL){
      cerr << "Unable to locate selected field!" << endl;
      continue;
    }

    float *fieldData = (float *)Field->getVol();
    Mdvx::field_header_t Fhdr = Field->getFieldHeader();
    
    ///////////////////////////////////////////////////////////////
    //
    // Having read the data, do the same for the weight.
    //


   DsMdvx weightMgr;

   weightMgr.setReadPath(Entries[ie].filterUrl); 
   weightMgr.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
   weightMgr.setReadCompressionType(Mdvx::COMPRESSION_NONE);
   //
   // Remap to the specified grid.
   //

   switch ( TDRP_params->output_projection){
      
   case Params::PROJ_FLAT:
     weightMgr.setReadRemapFlat(TDRP_params->output_grid.nx,
				TDRP_params->output_grid.ny,
				TDRP_params->output_grid.minx, TDRP_params->output_grid.miny,
				TDRP_params->output_grid.dx, TDRP_params->output_grid.dy,
				TDRP_params->output_grid.latOrig, 
				TDRP_params->output_grid.lonOrig,
				TDRP_params->output_grid.flatEarthRotation );
      
     break;
      
   case Params::PROJ_LATLON:
     weightMgr.setReadRemapLatlon(TDRP_params->output_grid.nx, TDRP_params->output_grid.ny,
				  TDRP_params->output_grid.minx, TDRP_params->output_grid.miny,
				  TDRP_params->output_grid.dx, TDRP_params->output_grid.dy);

     break;

   case Params::PROJ_LAMBERT:
     weightMgr.setReadRemapLc2(TDRP_params->output_grid.nx, TDRP_params->output_grid.ny,
			       TDRP_params->output_grid.minx, TDRP_params->output_grid.miny,
			       TDRP_params->output_grid.dx, TDRP_params->output_grid.dy,
			       TDRP_params->output_grid.latOrig,
			       TDRP_params->output_grid.lonOrig,
			       TDRP_params->output_grid.lambertLat1,
			       TDRP_params->output_grid.lambertLat2);
     
     break;
     
   default:
     cerr << "Unsupported projection." << endl;
     exit(-1);
     break;
     
   }

    //
    // Specify the field we want to read - number 0, there is only one.
    //
    weightMgr.addReadField( 0 );

    //
    // If the read fails, skip this entry.
    //
    if (weightMgr.readVolume()){
      cerr << "Read failed from ";
      cerr << Entries[ie].filterUrl  << endl;
      cerr << weightMgr.getErrStr() << endl;
      continue;
    }

    if (TDRP_params->Debug){
      cerr << "Weighting path in use : " << endl;
      cerr << weightMgr.getPathInUse() << endl;
    }

    MdvxField *WField = weightMgr.getFieldByNum( 0 );
    if (WField == NULL){
      cerr << "Unable to locate selected weighting field!" << endl;
      continue;
    }

    float *weightData = (float *)WField->getVol();
    Mdvx::field_header_t WFhdr = WField->getFieldHeader();

    //
    // Scale the weight data by the overall weighting specified
    // in the fusion file.
    //    
    for (int ik = 0; 
	 ik < TDRP_params->output_grid.nx * TDRP_params->output_grid.ny;
	 ik++){
           if (
	  (weightData[ik] != WFhdr.missing_data_value) &&
	  (weightData[ik] != WFhdr.bad_data_value)
	  ){
	     weightData[ik] *= Entries[ie].weight;
	   }
    }

    ////////////////////////////////////////////////////////
    //
    // Done with I/O. Loop through the planes. Have to do this since planes
    // in our data are not the same as planes in the input data set.
    //

    for (int iz = 0; iz < TDRP_params->output_grid.nz; iz++){

      int dataIZ = Entries[ie].planeNums[ iz ];

      if (dataIZ > Fhdr.nz-1){
	cerr << "ERROR - Plane number " << dataIZ;
	cerr << " specified, but range is 0 to " <<  Fhdr.nz-1 << endl;
	exit(-1); // Not sure if I should exit here or not - Niles.
      }

      ///////////////////////////////////////////////////////////////
      //
      // Now, integrate the data.
      //
      for (int ik = 0; 
	   ik < TDRP_params->output_grid.nx * TDRP_params->output_grid.ny;
	   ik++){
	//
	// If we have no data, or no weight, here forget it.
	//
	int index = ik + iz * 
	  TDRP_params->output_grid.nx * 
	  TDRP_params->output_grid.ny;
	
	int fieldDataIndex = ik + dataIZ *
	  TDRP_params->output_grid.nx * 
	  TDRP_params->output_grid.ny;
	
	if (
	    (fieldData[fieldDataIndex] == Fhdr.missing_data_value) ||
	    (fieldData[fieldDataIndex] == Fhdr.bad_data_value) ||
	    (weightData[ik] == WFhdr.missing_data_value) ||
	    (weightData[ik] == WFhdr.bad_data_value)
	    ){
	  continue;
	}
	//
	// We have data here, incorporate it.
	//
	if (_data[index] == fusionFile::badVal){
	  //
	  // First one - initialize.
	  //
	  sumWeights[index] = weightData[ik];
	  _data[index] = fieldData[fieldDataIndex] * weightData[ik];
	} else {
	  //
	  // Not first one - increment by weighted data.
	  //
	  sumWeights[index] += weightData[ik];
	  _data[index] += fieldData[fieldDataIndex] * weightData[ik];
	}
      } // End of loop in ik
    } // End of loop in Z
  } // End of loop through entries.
  //
  // Now, divide out the weights.
  //
  for (int ik = 0; 
       ik < TDRP_params->output_grid.nx * TDRP_params->output_grid.ny * TDRP_params->output_grid.nz;
       ik++){
    //	
    //
    // If we have no data, or the sum of weights is zero, forget it.
    //
    if ((_data[ik] != fusionFile::badVal) && (sumWeights[ik] != 0.0)) {
      //
      // Divide out the weights.
      //
      _data[ik] = _data[ik] / sumWeights[ik];
    }
  }
  
  free(sumWeights);

  return;

}




//////////////////////////////////////////////////
//
// Small private method to remove spaces from a string. Used
// to parse out the field names as read from the file.
//
void fusionFile::_removeSpaces(char *inStr, char*outStr){

  int inIndex = 0;
  int outIndex = 0;
  int go = 1;
  do {
    if (
	(isalnum(inStr[inIndex])) ||
	((int)inStr[inIndex] == 0 )
	){
      outStr[outIndex] = inStr[inIndex];
      if (outStr[outIndex] == char(0)){
	go=0;
      }
      outIndex++;
    }

    inIndex++;
  } while ( go );

  return;

}

//
// Get methods. Returns the results of reading the mdv file.
//

bool fusionFile::isOK(){
  // returns TRUE if MDV files were read OK
  return _OK;
}

float *fusionFile::getData(){
  // returns pointer to the data.
  return _data;
}

string fusionFile::getFieldName(){
  // Returns field name.
  return _fieldName;
}

//
// Destructor. Frees up space.
//
fusionFile::~fusionFile(){

  if (_data != NULL)  free(_data);

  return;
}

