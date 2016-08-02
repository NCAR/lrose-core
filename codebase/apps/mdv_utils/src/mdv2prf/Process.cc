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

const double Process::badVal = 999.0000;

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

  date_time_t dataTime;
  dataTime.unix_time = T;
  uconvert_from_utime( &dataTime );

  if (P->Debug){
    cerr << "Triggered with time " << utimstr(T) << endl;
  }

  // Make sure the output directory exists.

  if (ta_makedir_recurse(P->output.outDir)){
    cerr << "Failed to create directory " << P->output.outDir << endl;
    return -1;
  }

  //
  // Read the input file specifying URLs, lookback, field names
  // and lat/lon points. We do this every time so it is a dynamically
  // changing file, ie. we don't need to restart the program to run it.
  //
  FILE *fp = fopen(P->inputPointsFile, "r");
  if (fp == NULL){
    cerr << "Could not open " << P->inputPointsFile << endl;
    return -1;
  }

  int lineNum = 0;
  char Line[1024];
  bool error = false;
  int inputNum = 0;
  char *defaultDataName = "MDV";

  while (NULL != fgets(Line, 1024, fp)){

    _input_t i;

    // Keep track of what line we're on for error reporting.
    lineNum++;

    //
    // Ignore comments, blank lines.
    // set data name to what follows comment character WARNING could have unexpected results :-)

    if (Line[0] == '#') continue;
    if (strlen(Line) < 5) continue;


    // First line should be URL. Save it, trim off trailing return.
    //sprintf(i.url, "%s", Line);
    //i.url[strlen(i.url)-1] = char(0);

    Line[strlen(Line)-1] = char(0);

    char *p = strtok(Line, " ");
    if (p == NULL){
      error = true;
      cerr << P->inputPointsFile << " line " << lineNum << " : ";
      cerr << "Failed to parse first mdv url name." << endl;
      break;
    }
    // url name
    sprintf(i.url, "%s", p);
    p = strtok(NULL, " ");
    // data name
    if (p == NULL) {
    	sprintf(i.dataName, "%3.3s", defaultDataName);
    } else {
        sprintf(i.dataName, "%3.3s", p);
    }
//end get url and data name

    // Next line should be lookback.
    if (NULL == fgets(Line, 1024, fp)){
      error = true;
      cerr << P->inputPointsFile << " line " << lineNum << " : ";
      cerr << "Failed to get line to read lookback time." << endl;
      break;
    }
    lineNum++;

    if (1 != sscanf(Line, "%d", &i.lookback)){
      error = true;
      cerr << P->inputPointsFile << " line " << lineNum << " : ";
      cerr << "Failed to decode lookback from : " << Line;
      break;
    }



    // Next line should be input mdv field names, in order.
    if (NULL == fgets(Line, 1024, fp)){
      error = true;
      cerr << P->inputPointsFile << " line " << lineNum << " : ";
      cerr << "Failed to get line to read mdv field names." << endl;
      break;
    }
    Line[strlen(Line)-1] = char(0);
    lineNum++;

    p = strtok(Line, " ");
    if (p == NULL){
      error = true;
      cerr << P->inputPointsFile << " line " << lineNum << " : ";
      cerr << "Failed to parse first mdv field name." << endl;
      break;
    }

    do {
      string name(p);
      i.mdvFieldNames.push_back(name);
      p = strtok(NULL, " ");
    } while (p != NULL);





    // Next line should be data scale factors, in order.
    if (NULL == fgets(Line, 1024, fp)){
      error = true;
      cerr << P->inputPointsFile << " line " << lineNum << " : ";
      cerr << "Failed to get line to read scale factors." << endl;
      break;
    }
    Line[strlen(Line)-1] = char(0);
    lineNum++;

    p = strtok(Line, " ");
    if (p == NULL){
      error = true;
      cerr << P->inputPointsFile << " line " << lineNum << " : ";
      cerr << "Failed to parse first scale factor." << endl;
      break;
    }

    do {
      double scale = atof(p);
      i.scale.push_back(scale);
      p = strtok(NULL, " ");
    } while (p != NULL);




   // Next line should be data bias offsets, in order.
    if (NULL == fgets(Line, 1024, fp)){
      error = true;
      cerr << P->inputPointsFile << " line " << lineNum << " : ";
      cerr << "Failed to get line to read bias offsets." << endl;
      break;
    }
    Line[strlen(Line)-1] = char(0);
    lineNum++;

    p = strtok(Line, " ");
    if (p == NULL){
      error = true;
      cerr << P->inputPointsFile << " line " << lineNum << " : ";
      cerr << "Failed to parse first bias offset." << endl;
      break;
    }

    do {
      double bias = atof(p);
      i.bias.push_back(bias);
      p = strtok(NULL, " ");
    } while (p != NULL);



    //
    // If this is the first input, the the next two lines
    // should be the output field names and the output units, respectively.
    //

    if (inputNum == 0){
      // This line should be output field names, in order.
      if (NULL == fgets(Line, 1024, fp)){
	error = true;
	cerr << P->inputPointsFile << " line " << lineNum << " : ";
	cerr << "Failed to get line to read output field names." << endl;
	break;
      }
      Line[strlen(Line)-1] = char(0);
      lineNum++;

      p = strtok(Line, " ");
      if (p == NULL){
	error = true;
	cerr << P->inputPointsFile << " line " << lineNum << " : ";
	cerr << "Failed to parse first output field name." << endl;
	break;
      }

      do {
	string name(p);
	_outFieldNames.push_back(name);
	p = strtok(NULL, " ");
      } while (p != NULL);




      // Next line should be output units, in order.
      if (NULL == fgets(Line, 1024, fp)){
	error = true;
	cerr << P->inputPointsFile << " line " << lineNum << " : ";
	cerr << "Failed to get line to read output units." << endl;
	break;
      }
      Line[strlen(Line)-1] = char(0);
      lineNum++;

      p = strtok(Line, " ");
      if (p == NULL){
	error = true;
	cerr << P->inputPointsFile << " line " << lineNum << " : ";
	cerr << "Failed to parse first output field units." << endl;
	break;
      }

      do {
	string name(p);
	_outUnits.push_back(name);
	p = strtok(NULL, " ");
      } while (p != NULL);
    } // End of if this is the first input

    //
    // Check we got the same number of inpput mdv field
    // names, output field names and units.
    //
    if (
	(_outUnits.size() != i.mdvFieldNames.size()) ||
	(_outUnits.size() != _outFieldNames.size()) ||
	(_outUnits.size() != i.scale.size()) ||
	(_outUnits.size() != i.bias.size())
	){
      cerr << P->inputPointsFile << " line " << lineNum << endl;
      cerr << "Inconsistent number of input field names, output field names, scale factors, offset biases, or units :" << endl;

      cerr << _outUnits.size() << " units :" << endl;
      for (unsigned j=0; j < _outUnits.size(); j++){
	cerr << " " << j+1 << " : " << _outUnits[j] << endl;
      }

      cerr << i.mdvFieldNames.size() << " input MDV field names :" << endl;
      for (unsigned j=0; j < i.mdvFieldNames.size(); j++){
	cerr << " " << j+1 << " : " << i.mdvFieldNames[j] << endl;
      }

      cerr << _outFieldNames.size() <<  " output field names :" << endl;
      for (unsigned j=0; j < _outFieldNames.size(); j++){
	cerr << " " << j+1 << " : " << _outFieldNames[j] << endl;
      }

      cerr << i.scale.size() << " scale factors :" << endl;
      for (unsigned j=0; j < i.scale.size(); j++){
	cerr << " " << j+1 << " : " << i.scale[j] << endl;
      }

      cerr << i.bias.size() << " bias offsets :" << endl;
      for (unsigned j=0; j < i.bias.size(); j++){
	cerr << " " << j+1 << " : " << i.bias[j] << endl;
      }

      error = true;
      break;
    }

    //
    // Read the list of lat/lon points with optional booleans.
    //
    while (NULL != fgets(Line, 1024, fp)){
      lineNum++;
      if (strlen(Line) < 5) continue;
      if (Line[0] == '#') continue;

      //
      // See if this is the entry that gives us the height.
      //
      if (!(strncmp(Line,"height", strlen("height")))){
	if (!(strncmp(Line,"heightName_", strlen("heightName_")))){
	  i.heightFromName = true;

	  int l = strlen("heightName_"); int m=0;
	  char buf[100];
	  do {
	    if (isalnum((int)Line[l])){
	      buf[m] = Line[l];
	      m++; l++;
	    } else {
	      buf[m]=char(0);
	      break;
	    }
	  } while(1);

	  i.heightName = buf;

	  if (2 != sscanf(Line + l, "%lf %lf", &i.heightScale, &i.heightBias)){
	    cerr << "Failed to decode scale and bias from " << P->inputPointsFile << " line " << lineNum << endl;
	    return -1;
	  }


	} else {
	  i.heightFromName = false;
	  i.heightName = "NONE";
	  if (2 != sscanf(Line + strlen("height"), "%lf %lf", &i.heightScale, &i.heightBias)){
	    cerr << "Failed to decode scale and bias from " << P->inputPointsFile << " line " << lineNum << endl;
	    return -1;
	  }
	}
	break;
      }

      //
      // It should be another lat/lon point, then.
      //
      double lat, lon;
      if (2 != sscanf(Line, "%lf %lf", &lat, &lon)){
	cerr << "Failed to decode lat/lon from " << P->inputPointsFile << " entry " << Line;
	return -1;
      } else {
	bool doIfLastUrlOK = true;
	if (inputNum > 0){
	  if (NULL != strstr(Line, "FALSE")) doIfLastUrlOK = false;
	  if (NULL != strstr(Line, "false")) doIfLastUrlOK = false;
	}
	i.lat.push_back( lat );
	i.lon.push_back( lon );
	i.doIfLastUrlOK.push_back( doIfLastUrlOK );
      }
    }
    _inputs.push_back(i);
    inputNum++;
    i.lat.clear(); i.lon.clear();
    i.doIfLastUrlOK.clear(); i.mdvFieldNames.clear();
    i.bias.clear(); i.scale.clear();

  }

  fclose(fp);


  if (error) return -1;

  if (P->Debug){
    cerr << _inputs.size() << " input urls :" << endl;
    for (unsigned i=0; i < _inputs.size(); i++){
      cerr << " Input " << i+1 << " url " << _inputs[i].url << " lookback seconds " << _inputs[i].lookback << endl;
      for (unsigned j=0; j < _inputs[i].mdvFieldNames.size(); j++){
	cerr << "  Field name " << j+1 << " is " <<  _inputs[i].mdvFieldNames[j];
	cerr << ", in output will be " << _outFieldNames[j] << " units " << _outUnits[j] << endl;
	cerr << "  after applying a scale factor of " << _inputs[i].scale[j];
	cerr << " and a bias offset of " << _inputs[i].bias[j]<< endl;
      }

      cerr << "   Input " << i+1 << " has " << _inputs[i].lat.size() << " points : " << endl;
      for (unsigned j=0; j < _inputs[i].lat.size(); j++){
	cerr << "   Point " << j+1 << " is " <<  _inputs[i].lat[j] << ", " << _inputs[i].lon[j] << ", Do if last URL ok : ";
	if (_inputs[i].doIfLastUrlOK[j])
	  cerr << "TRUE";
	else
	  cerr << "FALSE";
	cerr << endl;

      }
      if (_inputs[i].heightFromName)
	cerr << "  Heights for input " << i+1 << " will be taken from mdv field " << _inputs[i].heightName;
      else
	cerr << "  Heights for input " << i+1 << " will be taken from vlevel header";
      cerr << " with scale " << _inputs[i].heightScale << " and bias " << _inputs[i].heightBias << " applied." << endl << endl;
    }
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // Finally done parsing file that specifies input sources. Now, on to generating output.
  //
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  //
  // Open the output file.
  //
  char outFileName[MAX_PATH_LEN];
  sprintf(outFileName,"%s/%s", P->output.outDir, P->output.outFileName);
  if (P->output.timestampOutput){
    sprintf(outFileName,"%s_%d%02d%02d_%02d%02d%02d.prf",
	     outFileName, dataTime.year, dataTime.month, dataTime.day,
	     dataTime.hour, dataTime.min, dataTime.sec);
  }

  if (P->Debug){
    cerr << "Output file is " << outFileName << endl;
  }

  fp = fopen(outFileName, "w");
  if (fp == NULL){
    cerr << "Failed to create " << outFileName << endl;
    return -1;
  }

  unsigned long stationNum = 1; // Used to keep tabs on the
                                // station number in the file

  error = false;
  bool lastUrlOk = true;

  //
  // Put the header on the output file.
  //
  double deciHour = dataTime.hour + dataTime.min/60.0 + dataTime.sec / 3600.0;
  fprintf(fp, "# CREATOR:       mdv2prf\n");
  fprintf(fp, "# DATE:          %d-%02d-%02d ",
	  dataTime.year, dataTime.month, dataTime.day);
  fprintf(fp, "%02d", dataTime.hour); fprintf(fp, "%s", ":");
  fprintf(fp, "%02d", dataTime.min);  fprintf(fp, "%s", ":");
  fprintf(fp, "%02d UTC\n", dataTime.sec);
  fprintf(fp, "# SOURCE:        mdv\n");
  fprintf(fp, "# REFERENCE:     agl\n");
  fprintf(fp, "# TYPE:          OBSERVATION\n");
  fprintf(fp, "# START:         %d %02d %02d %12.4f\n", dataTime.year, dataTime.month, dataTime.day, deciHour);
  fprintf(fp, "# END:           %d %02d %02d %12.4f\n",dataTime.year, dataTime.month, dataTime.day, deciHour);
  fprintf(fp, "# TIMEREFERENCE: UTC\n");
  fprintf(fp, "# MODE:          profile scattered\n");
  fprintf(fp, "PROFILE\n");
  const static int numSurfaceFields = 5;
  fprintf(fp, "%d %d\n", numSurfaceFields, _outFieldNames.size()+1);
  fprintf(fp, "ID      YYYYMMDDHOUR    LAT     LON\n"); // Surface names

  fprintf(fp, "                HOURS   N       E\n"); // Surface units

  // Upper air names.
  fprintf(fp, "%-8s", "Z");
  for (unsigned ifld = 0; ifld < _outFieldNames.size(); ifld++){
    fprintf(fp, "%-8s", _outFieldNames[ifld].c_str() );
  }
  fprintf(fp, "\n");

  // Upper air units.
  fprintf(fp, "%-8s", "M");
  for (unsigned ifld = 0; ifld < _outUnits.size(); ifld++){
    fprintf(fp, "%-8s", _outUnits[ifld].c_str() );
  }
  fprintf(fp, "\n");

  fprintf(fp, "999.00\n");


  //
  // Start looping through the input sources.
  //

  int numUrlsResponded = 0;

  for (unsigned i=0; i < _inputs.size(); i++){

    if (P->Debug){
      cerr << "Dealing with url " << _inputs[i].url << endl;
      cerr << "lookback " << _inputs[i].lookback << " seconds " << endl;
      cerr << "Data from " << _inputs[i].dataName << endl;
    }
    //
    // Set up for the new data.
    //
    DsMdvx New;

    New.setReadTime(Mdvx::READ_FIRST_BEFORE, _inputs[i].url, _inputs[i].lookback, T);
    New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    New.setReadCompressionType(Mdvx::COMPRESSION_NONE);

    for (unsigned j=0; j < _inputs[i].mdvFieldNames.size(); j++){
      if (!(strcmp("missingField", _inputs[i].mdvFieldNames[j].c_str()))) continue;
      New.addReadField(_inputs[i].mdvFieldNames[j]);
      if (P->Debug){
	cerr << "Added field " << _inputs[i].mdvFieldNames[j] << " to the read." << endl;
      }
    }

    if (_inputs[i].heightFromName){
      New.addReadField(_inputs[i].heightName);
      if (P->Debug){
	cerr << "Added height field " << _inputs[i].heightName << " to the read." << endl;
      }
    }

    if (New.readVolume()){
      if (P->Debug){
	cerr << "Read failed at " << utimstr(T) << " from ";
	cerr << _inputs[i].url << " lookback " << _inputs[i].lookback<< endl;
      }
      lastUrlOk = false;
      continue;
    }

    numUrlsResponded++;

    //
    // Loop through the points for this input.
    //
    for (unsigned j=0; j < _inputs[i].lat.size(); j++){

      //
      // If the last URL went OK and we are not doing this point
      // if the last URL went OK, then skip it.
      //
      if ((lastUrlOk) && (!(_inputs[i].doIfLastUrlOK[j]))) continue;

      //
      // Loop through the fields we want to extract.
      //
      fl32 *heights = NULL;
      fl32 *outData = NULL;
      int Nz = -1;

      bool dataMissing = false;

      for (unsigned k=0; k < _inputs[i].mdvFieldNames.size(); k++){

	if (!(strcmp("missingField", _inputs[i].mdvFieldNames[k].c_str()))){
	  //
	  // This input does not have this field name. Just skip
	  // it. The values are initialized to missing, which is
	  // appropriate.
	  //
	  continue;
	}

	MdvxField *field = New.getFieldByName( _inputs[i].mdvFieldNames[k] );
	if (field == NULL){
	  cerr << "Field " << _inputs[i].mdvFieldNames[k] << " not found at " << _inputs[i].url << endl;
	  exit(-1);
	}

	Mdvx::master_header_t mhdr = New.getMasterHeader();
	Mdvx::field_header_t fhdr = field->getFieldHeader();
	Mdvx::vlevel_header_t vhdr = field->getVlevelHeader();

	MdvxProj proj(mhdr, fhdr);

	fl32 *mdvData = (fl32 *) field->getVol();

	if (heights == NULL){
	  Nz = fhdr.nz;
	  //
	  // If this is the first field, we need to fill
	  // up the heights, and allocate enough space
	  // for the data in memory.
	  //
	  heights = (fl32 *)malloc(sizeof(fl32)*fhdr.nz);
	  if (heights == NULL){
	    cerr << "Malloc failed!" << endl;
	    exit(-1);
	  }

	  if (_inputs[i].heightFromName){
	    //
	    // Take heights from mdv field, apply scale and bias.
	    //
	    MdvxField *heightField =  New.getFieldByName( _inputs[i].heightName );
	    if (heightField == NULL){
	      cerr << "Height field " << _inputs[i].heightName << " not found at " << _inputs[i].url << endl;
	      exit(-1);
	    }

	    Mdvx::field_header_t heightFhdr = heightField->getFieldHeader();

	    if (Nz != heightFhdr.nz){
	      cerr << "FATAL : " << heightFhdr.nz << " vertical levels for height field " << _inputs[i].heightName;
	      cerr << " from " << _inputs[i].url << ", expected " << Nz << endl;
	      exit(-1);
	    }

	    Mdvx::vlevel_header_t heightVhdr = heightField->getVlevelHeader();
	    MdvxProj heightProj(mhdr, heightFhdr);
	    fl32 *mdvHeightData = (fl32 *) heightField->getVol();

	    int ix, iy;
	    if (heightProj.latlon2xyIndex(_inputs[i].lat[j], _inputs[i].lon[j], ix, iy)){
	      for (int iz=0; iz < heightFhdr.nz; iz++){
		heights[iz] = badVal;
	      }
	    } else {
	      for (int iz=0; iz < heightFhdr.nz; iz++){
		int index = heightFhdr.nx * heightFhdr.ny * iz + heightFhdr.nx * iy + ix;
		if (
		    (mdvHeightData[index] == heightFhdr.bad_data_value) ||
		    (mdvHeightData[index] == heightFhdr.missing_data_value)
		    ){
		  heights[iz] = badVal;
		} else {
		  heights[iz] = (mdvHeightData[index] + _inputs[i].heightBias) * _inputs[i].heightScale;
		}
	      }
	    }
	  } else {
	    //
	    // Take heights from vlevel header, apply scale and bias.
	    //
	    for (int iz=0; iz < fhdr.nz; iz++){
	      heights[iz] = (vhdr.level[iz] + _inputs[i].heightBias) * _inputs[i].heightScale;
	    }
	  } // End of if we are taking heights from MDV field or from vlevel header.

	  //
	  // May as well allocate space for the array we're going to print here.
	  //
	  outData = (fl32 *) malloc(sizeof(fl32) * fhdr.nz * _inputs[i].mdvFieldNames.size());
	  if (outData == NULL){
	    cerr << "Malloc failed !" << endl;
	    exit(-1);
	  }

	  //
	  // Init to missing, that way if we skip a field it will
	  // be handled appropriately.
	  //
	  for (unsigned n=0; n < fhdr.nz * _inputs[i].mdvFieldNames.size(); n++){
	    outData[n] = badVal;
	  }

	} // End of if this is the first field.

	if (Nz != fhdr.nz){
	  cerr << "FATAL : " << fhdr.nz << " vertical levels for field " << _inputs[i].mdvFieldNames[k];
	  cerr << " from " << _inputs[i].url << ", expected " << Nz << endl;
	  exit(-1);
	}

	//
	// Take data from mdv field, apply scale and bias, store in outData array.
	//

        dataMissing = false;
	int ix, iy;
	if (proj.latlon2xyIndex(_inputs[i].lat[j], _inputs[i].lon[j], ix, iy)){
	  for (int iz=0; iz < fhdr.nz; iz++){
	    outData[fhdr.nz * k + iz] = badVal;
	  }
	} else {
	  for (int iz=0; iz < fhdr.nz; iz++){
	    int index = fhdr.nx * fhdr.ny * iz + fhdr.nx * iy + ix;
	    if (
		(mdvData[index] == fhdr.bad_data_value) ||
		(mdvData[index] == fhdr.missing_data_value)
		){
	      outData[fhdr.nz * k + iz] = badVal;
	      dataMissing = true;
	    } else {
	      outData[fhdr.nz * k + iz] = (mdvData[index] + _inputs[i].bias[k]) * _inputs[i].scale[k];
	    }
	  }
	}

      } // End of loop though fields.


      //
      // If there are missing data and we've decided not to have that in the output,
      // then continue.
      //
      if ((dataMissing) && (P->noMissingData)){
	free( heights ); free(outData);
	continue;
      }

      //
      // Print the outData array out to file as a sounding.
      //
      fprintf(fp, "ID: %3s%05ld %d%02d%02d %12.4f %12.4f %12.4f\n", _inputs[i].dataName,
	      stationNum,
	      dataTime.year, dataTime.month, dataTime.day, deciHour,
	      _inputs[i].lat[j],
	      _inputs[i].lon[j]);

      stationNum++;


      for (int iz = 0; iz < Nz; iz++){
	fprintf(fp,"%12.2f ", heights[iz]);
	for (unsigned ifld = 0; ifld < _inputs[i].mdvFieldNames.size(); ifld++){
	  if (fabs(outData[Nz * ifld + iz ]) < 400.0){
	    fprintf(fp,"%12.2f ", outData[Nz * ifld + iz ]);
	  } else {
	    fprintf(fp,"%12.2f ", badVal);
	  }
	}
	fprintf(fp,"\n");
      }

      free( heights ); free(outData);

    } // End of loop through points.


    //
    // End of loop - update the last URL OK boolean. If we got here it went OK.
    //
    lastUrlOk = true;

  }

  fclose (fp);
  if (error) return -1;

  //
  // Run a script, if requested.
  //
  if (P->script.runScript){
    if (numUrlsResponded > 0){
      char com[1024];
      sprintf(com,"%s %s", P->script.scriptName, outFileName);
      if (P->Debug){
	cerr << "Executing " << com << endl;
      }

      system(com);
    } else {
      cerr << "WARNING : No URLS responded at time " << utimstr(time(NULL)) << " not running script " << P->script.scriptName << endl;
    }
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










