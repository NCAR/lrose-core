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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <toolsa/pmu.h>
#include <stdlib.h>

#include <netcdf.h>

#include "Windtracer2ncd.hh"

//
// Constructor. Copies parameters.
//
Windtracer2ncd::Windtracer2ncd(Params *TDRP_params){

  _startYear=0; _startMonth=0; _startDay=0;
  _startHour=0; _startMinute=0; _startSec=0;

  _numBeams = 0;
  //
  // Reset the volume number and other things.
  //
  // 
  _firstRange=0.0; _deltaRange=0.0; _numGates = 0;
  //
  //
  // Point to the TDRP parameters.
  //
  _params = TDRP_params;
  //
}

//
// Init - first hit at the file. Gets the number of fields,
// the number of beams, and a few other things.
//
void Windtracer2ncd::Windtracer2ncdInit( char *filename){

  //
  // Open the input file.
  //

  if (_params->debug){
    fprintf(stderr,"Initializing for file %s\n", filename);
  }

  FILE *fp = fopen (filename,"r");
  if (fp == NULL){
    fprintf(stderr,"%s not found.\n",filename);
    return;
  }

  int irec = 0;
  int len = 0;

  _numFields = 0;
  int id, ver;
  _maxLen = 0;

  do {

    id = _readShort( &fp );
    ver = _readShort( &fp );
    len = _readLong( &fp );


    irec++;
    
    if (_params->debug){
      fprintf(stderr,"RECORD : %d ID : %x (%d) VER : %d LEN : %d ", 
	      irec, (int)id, (int)id, (int)ver, len);
      
      fprintf(stderr, "%s\n", _idToStr( id ));
    }
  
    //
    // Pick the number of fields and the max len off the
    // first set, ie. _numBeams == 1
    //  
    if (_numBeams == 1){
      if (
	  (id > 0x1d) &&
	  (strcmp("UNKNOWN", _idToStr( id )))
	  ){
	_numFields++;
	_fieldNames.push_back(_idToStr( id ));	
	if (len > _maxLen){
	  _maxLen = len;
	}
      }
    }
    
    switch (id) {
      
    case 0x10 :
      _numBeams++;
      _processSpecial(fp, _params->debug);
      break;

    case 0x6 :
      _processText(fp, len-2);
      break;

    case 0xf :
      _processHeader(fp);
      break;

    default :
      int byteLen = 4*len-8;
      fseek(fp, byteLen, SEEK_CUR);
      break;

    } 

  } while ((id != 0) && (len > 0) && (!(feof(fp))));

  fclose(fp);

  if (_params->debug){
    fprintf(stderr,"NUMBER OF FIELDS : %d  MAX LEN : %d\n", _numFields, _maxLen);

    for (int i=0; i < _numFields; i++){
      fprintf(stderr,"Field %d : %s\n", i+1, _fieldNames[i]);
    }

    fprintf(stderr,"%d beams found.\n", _numBeams);
  }

  return;

}



//
// Destructor. Does little.
//
Windtracer2ncd::~Windtracer2ncd(){


}
//
// Main routine.
//
void Windtracer2ncd::Windtracer2ncdFile( char *filename ){
  //
  // Allocate space for what we have to do.
  //
  for (int i=0; i < _numFields; i++){
    float *data = (float *)calloc(_maxLen * _numBeams, sizeof(float));
    if (data == NULL){
      fprintf(stderr,"Malloc failed.\n");
      exit(-1);
    }
    _fields.push_back(data);
  }

  _elevations = (float *)malloc(_numBeams * sizeof(float));
  _azimuths   = (float *)malloc(_numBeams * sizeof(float));
  _years =      (float *)malloc(_numBeams * sizeof(float));
  _months =      (float *)malloc(_numBeams * sizeof(float));
  _days =      (float *)malloc(_numBeams * sizeof(float));
  _hours =      (float *)malloc(_numBeams * sizeof(float));
  _mins =      (float *)malloc(_numBeams * sizeof(float));
  _secs =      (float *)malloc(_numBeams * sizeof(float));
  _msecs =      (float *)malloc(_numBeams * sizeof(float));

  //
  // Open the input file.
  //

  if (_params->debug){
    fprintf(stderr,"Initializing for file %s\n", filename);
  }

  FILE *fp = fopen (filename,"r");
  if (fp == NULL){
    fprintf(stderr,"%s not found.\n",filename);
    return;
  }

  int len = 0;
  int id, ver;
  int iTime = 0;
  int iDir = 0;
  int iFld = 0;
  int iBeam = 0;

  do {

    id = _readShort( &fp );
    ver = _readShort( &fp );
    len = _readLong( &fp );

    date_time_t T;
    
    switch (id) {
      
    case 0x10 :
      _processSpecial(fp, false);
      _elevations[iDir] = _el;
      _azimuths[iDir] = _az;
      iDir++;
      break;

    case 0xf :
      _processHeader(fp);
      T.year = _year; T.month = _month; T.day = _day;
      T.hour = _hour; T.min = _min; T.sec = _sec;
      uconvert_to_utime( &T );
      _years[iTime] = _year;
      _months[iTime] = _month;
      _days[iTime] = _day;
      _hours[iTime] = _hour;
      _mins[iTime] = _min;
      _secs[iTime] = _sec;
      _msecs[iTime] = _msec;       
      iTime++;
      //
      // If these are the start time, record them for the filename.
      //
      if (_startYear == 0){
	_startYear = _year; _startMonth = _month; _startDay = _day;
	_startHour = _hour; _startMinute = _min; _startSec = _sec;
      }

      break;

    default :
      if (
	  (id > 0x1d) &&
	  (strcmp("UNKNOWN", _idToStr( id )))
	  ){
	//
	// Valid field.
	//
	float *data = _fields[iFld];
	
	int offset = iBeam * _numGates;
	fread(data + offset, sizeof(float), _numGates, fp);
	
	for (int i=0; i <  _numGates; i++){
	  _byteSwap4(&data[offset + i]);
	}
	//
	// int byteLen = 4*len-8;
	// fseek(fp, byteLen, SEEK_CUR);
	//
	iFld++;
	if (iFld == _numFields){
	  iFld = 0;
	  iBeam++;
	}
      } else {
	//
	// Not a valid field, skip it.
	//
	int byteLen = 4*len-8;
	fseek(fp, byteLen, SEEK_CUR);
      }
      break;

    } 

  } while ((id != 0) && (len > 0) && (!(feof(fp))));

  fclose(fp);

  if (_params->debug >= Params::DEBUG_DATA){
    for (int i=0; i < _numBeams; i++){
      fprintf(stderr,"TIME : %d : EL: %g AZ: %g\n", i, 
	      _elevations[i], _azimuths[i]);
      for (int j=0; j < _numFields; j++){
	fprintf(stderr,"%s : ", _fieldNames[j]);
	
	float *data =  _fields[j];
	int offset = i * _numGates;
	
	for (int k=0; k < _numGates; k++){
	  fprintf(stderr,"%g, ", data[offset + k]);
	  if ((k % 5) == 0) fprintf(stderr,"\n    ");
	}
	fprintf(stderr,"\n");
      }
    }
  }
  //
  // OK - we now have the data resident in memory - save
  // it out as a netcdf file.
  //
  char outFileName[MAX_PATH_LEN];

  if (_params->retainFilenames){
    sprintf(outFileName,"%s/%s_%4d%02d%02d_%02d%02d%02d.nc", 
	    _params->OutDir, _params->baseName,
	    _startYear, _startMonth, _startDay,
	    _startHour, _startMinute, _startSec);
  } else {
    sprintf(outFileName,"%s.nc", filename);
  }

  int NcID;
  if (nc_create(outFileName, NC_CLOBBER, &NcID) != NC_NOERR ){
    fprintf(stderr,"Failed to create %s\n",outFileName);
    exit(-1);
  }  
  //
  // Define the time and gate dimensions.
  //
  int dimID[2];
  if (nc_def_dim(NcID, "time", _numBeams, &dimID[0])){
    fprintf(stderr,"Failed to define the time dimension.\n");
    exit(-1);
  }
  //
  if (nc_def_dim(NcID, "gates", _numGates, &dimID[1])){
    fprintf(stderr,"Failed to define the gate dimension.\n");
    exit(-1);
  }
  //
  // Define time, az, el variables.
  //
  int yearVarID;
  if (nc_def_var(NcID, "year",
		 NC_FLOAT, 1, &dimID[0], &yearVarID) != NC_NOERR ){
    fprintf(stderr,"Failed on nc_def_var for year variable.\n");
    exit(-1);
  }
  //
  int monthVarID;
  if (nc_def_var(NcID, "month",
		 NC_FLOAT, 1, &dimID[0], &monthVarID) != NC_NOERR ){
    fprintf(stderr,"Failed on nc_def_var for month variable.\n");
    exit(-1);
  }
  //
  int dayVarID;
  if (nc_def_var(NcID, "day",
		 NC_FLOAT, 1, &dimID[0], &dayVarID) != NC_NOERR ){
    fprintf(stderr,"Failed on nc_def_var for day variable.\n");
    exit(-1);
  }
  //
  int hourVarID;
  if (nc_def_var(NcID, "hour",
		 NC_FLOAT, 1, &dimID[0], &hourVarID) != NC_NOERR ){
    fprintf(stderr,"Failed on nc_def_var for hour variable.\n");
    exit(-1);
  }
  //
  int minVarID;
  if (nc_def_var(NcID, "min",
		 NC_FLOAT, 1, &dimID[0], &minVarID) != NC_NOERR ){
    fprintf(stderr,"Failed on nc_def_var for min variable.\n");
    exit(-1);
  }
  //
  int secVarID;
  if (nc_def_var(NcID, "sec",
		 NC_FLOAT, 1, &dimID[0], &secVarID) != NC_NOERR ){
    fprintf(stderr,"Failed on nc_def_var for sec variable.\n");
    exit(-1);
  }
  //
  int msecVarID;
  if (nc_def_var(NcID, "msec",
		 NC_FLOAT, 1, &dimID[0], &msecVarID) != NC_NOERR ){
    fprintf(stderr,"Failed on nc_def_var for msec variable.\n");
    exit(-1);
  }



  //
  int azVarID;
  if (nc_def_var(NcID, "azimuth",
		 NC_FLOAT, 1, &dimID[0], &azVarID) != NC_NOERR ){
    fprintf(stderr,"Failed on nc_def_var for azimuth variable.\n");
    exit(-1);
  }
  //
  int elVarID;
  if (nc_def_var(NcID, "elevation",
		 NC_FLOAT, 1, &dimID[0], &elVarID) != NC_NOERR ){
    fprintf(stderr,"Failed on nc_def_var for elevation variable.\n");
    exit(-1);
  }
  //
  // Now define the variables for the fields.
  //
  int *fieldID = (int *) malloc(_numFields * sizeof(int));
  if (fieldID == NULL){
    fprintf(stderr,"Malloc failed.\n");
    exit(-1);
  }
  //
  for (int i=0; i < _numFields; i++){
    if (nc_def_var(NcID, _fieldNames[i],
		   NC_FLOAT, 2, dimID, &fieldID[i]) != NC_NOERR ){
      fprintf(stderr,"Failed on nc_def_var for %s variable.\n", _fieldNames[i]);
      exit(-1);
    }
  }
  //
  // Add a few global attributes.
  //
  float nTimes = float(_numBeams);
  float nGates = float(_numGates);
  float nFields = float(_numFields);
  nc_put_att_float(NcID, NC_GLOBAL, "firstRange", NC_FLOAT, 1, &_firstRange);
  nc_put_att_float(NcID, NC_GLOBAL, "deltaRange", NC_FLOAT, 1, &_deltaRange);
  nc_put_att_float(NcID, NC_GLOBAL, "nTimes", NC_FLOAT, 1, &nTimes);
  nc_put_att_float(NcID, NC_GLOBAL, "nGates", NC_FLOAT, 1, &nGates);
  nc_put_att_float(NcID, NC_GLOBAL, "nFields", NC_FLOAT, 1, &nFields);
  nc_put_att_text(NcID, NC_GLOBAL, "Comment",
		  strlen(_params->commentString), 
		  _params->commentString);
  //
  // So much for the definitions - now the actual writing of data.
  //
  nc_enddef(NcID);
  //
  //
  nc_put_var_float(NcID, yearVarID, _years);
  nc_put_var_float(NcID, monthVarID, _months);
  nc_put_var_float(NcID, dayVarID, _days);
  nc_put_var_float(NcID, hourVarID, _hours);
  nc_put_var_float(NcID, minVarID, _mins);
  nc_put_var_float(NcID, secVarID, _secs);
  nc_put_var_float(NcID, msecVarID, _msecs);
  nc_put_var_float(NcID, azVarID, _azimuths);
  nc_put_var_float(NcID, elVarID, _elevations);
  //
  // Add a couple of extras.
  //
  // nc_put_att_float(NcID, NC_GLOBAL, "firstRange", NC_FLOAT, 1, &_firstRange);
  // nc_put_att_float(NcID, NC_GLOBAL, "deltaRange", NC_FLOAT, 1, &_deltaRange);
  //
  // Now add the data for the fields.
  //
  for (int i=0; i < _numFields; i++){
    nc_put_var_float(NcID, fieldID[i], _fields[i]);
  }
  //
  if (nc_close(NcID)){
    fprintf(stderr,"Failed to close %s\n", outFileName);
    exit(-1);
  }
  //
  // Zip the output, if desired. No check is done on if this
  // goes OK or not.
  //
  if (_params->zip_output){
    char com[1024];
    sprintf(com,"gzip -f %s", outFileName);
    system(com);
  }

  //
  // Free the data arrays.
  //
  for (int i=0; i < _numFields; i++){
    free(_fields[i]);
  }
  //
  free(fieldID);
  free(_elevations);  
  free(_azimuths);  
  free(_years); free(_months);  free(_days); 
  free(_hours); free(_mins);  free(_secs); 
  free(_msecs);
  //
  return;

}
//
// Read a short int, byte swapped.
//
int Windtracer2ncd::_readShort(FILE **fp){

  unsigned char a,b;
  fread(&a, sizeof(unsigned char), 1, *fp);
  fread(&b, sizeof(unsigned char), 1, *fp);
  
  return a*256+b;

}
//
// Read a long int, byte swapped.
//
int Windtracer2ncd::_readLong(FILE **fp){

  unsigned char a,b,c,d;

  fread(&a, sizeof(unsigned char), 1, *fp);
  fread(&b, sizeof(unsigned char), 1, *fp);
  fread(&c, sizeof(unsigned char), 1, *fp);
  fread(&d, sizeof(unsigned char), 1, *fp);

  return a*256*256*256 + b*256*256 + c*256 + d;

}

//
// Print a string to stderr describing the field.
//
char *Windtracer2ncd::_idToStr( int ID ){

  switch (ID) {
 
  case 0x0 :
    return "EOF";
    break;

  case 0xf :
    return "CREC HDR FOR PRODUCT RECORD";
    break;

  case 0x10 :
    return "CREC SPECIAL FOR PRODUCT RECORD";
    break;

  case 0x11 :
    return "CREC RESULTS FOR PRODUCT RECORD";
    break;

  case 0x6 :
    return "TEXT";
    break;

  case 0x15 :
    return "TEXT";
    break;

  case 0x1e :
    return "VEL";
    break;

  case 0x1f :
    return "SNR";
    break;

  case 0x20 :
    return "CFAR";
    break;

  case 0x21 :
    return "SW";
    break;
   
  case 0x55 :
    return "FVEL";
    break;

  case 0x56 :
    return "FSNR";
    break;

  case 0x58 :
    return "MCFAR";
    break;

  case 0x59 :
    return "QUALITY";
    break;

  case 0x5a :
    return "BACKSCAT";
    break;

  default :
    return "UNKNOWN";
    break;

  }


}
//
// Process the special header - for us, this holds the azimuth
// and the elevation.
//
void Windtracer2ncd::_processSpecial(FILE *fp, bool noisy){


  for (int i=0; i < 18; i++){
    float h;
    fread(&h, sizeof(float), 1, fp); 
    _byteSwap4(&h);
    if (i == 11) _az = h;
    if (i == 12) _el = h;
  }
  //
  // The time should be set by now as well, so
  // print time and pointing vector if requested.
  //
  if (noisy){
    if (_params->debug){
      fprintf(stderr,"[%d] TIME FROM FILE : %d/%02d/%02d %0d:%02d:%02d AZ : %g EL : %g\n",
	      _numBeams, _year, _month, _day, _hour, _min, _sec, _az, _el);
    }
  }
  //
  // Add elevation, azimuth offset.
  //
  _az = _az + _params->azimuth_offset;
  _el = _el + _params->elevation_offset;

  // Make sure the angles are still on the desired range.

  do {
    if (_az > 360.0) _az = _az - 360.0;
    if (_az < 0.0) _az = _az + 360.0;
    if (_el > 90.0){
      if (_el > 180.0){
	fprintf(stderr,"Silly elevation of %g, I cannot cope.\n",_el);
	exit(-1);
      }
      _el = 180.0 - _el;
      _az = _az + 180.0;
    }
  } while ((_el > 90.0) || (_az > 360.0) || (_az < 0.0));

  //
  return;

}
//
// Process the header - for us, holds the hour, minute and second.
//
void Windtracer2ncd::_processHeader(FILE *fp){

  int hour=0, min=0, sec=0, msec=0;

  for (int i=0; i < 6; i++){
    int k;
    fread(&k, sizeof(int), 1, fp); 
    _byteSwap4(&k);
   
    if (i == 1) hour = k;
    if (i == 2) min = k;
    if (i == 3) sec = k;
    if (i == 4) msec = k;

  }

  //
  // Store these in the global section.
  //
  _hour = hour; _min = min; 
  _sec = sec;  _msec = msec;

  return;

}

//
// Routine to process the text part of the file. The text is pulled
// out and written to a temporary file, and then that temporary file is processed.
//
void Windtracer2ncd::_processText(FILE *fp, int len){

  //
  // Pull the text section of the input file out and
  // write it to a file.
  //
  FILE *ofp = fopen(_params->temp_file,"w");
  if (ofp == NULL){
    fprintf(stderr,"Could not create %s\n", _params->temp_file);
    exit(-1);
  }
  
  int byteLen = len * 4;
  for(int i=0; i < byteLen; i++){
    char c;
    fread(&c, sizeof(c), 1, fp);
    //
    // Leave out control code 13 - windows file.
    //
    if ((int)c != 13)    fprintf(ofp,"%c", c);
  }
  
  fclose(ofp);
  //
  //  Process the file we have just written.
  //
  _processExistingText();

}

void Windtracer2ncd::_processExistingText(){
  //
  // Process the file to read the year, month, date and the
  // parameters we will need to calulate range gate spacing.
  //
  FILE *ifp = fopen(_params->temp_file,"r");
  if (ifp == NULL){
    fprintf(stderr,"Could not read file %s\n", _params->temp_file);
    exit(-1);
  }

  double year, month, day;
  double sample_frequency;
  double raw_data_offset_meters;
  double raw_data_first_sample;
  double raw_data_sample_count;
  double range_gates;
  double samples_per_gate;
  double gates_to_merge;

  //
  // Read the year, month, day and sample frequency.
  // Must be done in this order (the order in which they appear
  // in the file).

  _readKeyword(ifp, "YEAR", &year, 1);
  _readKeyword(ifp, "MONTH", &month, 1);
  _readKeyword(ifp, "DAY", &day, 1);

  _year = (int)rint(year);
  _month = (int)rint(month);
  _day = (int)rint(day);


  _readKeyword(ifp, "SAMPLE_FREQUENCY", &sample_frequency, 1);

  //
  // Seek ahead to the tag line prior to reading the other parameters.
  //
  // 
  _readKeyword(ifp, _params->tag_line, NULL, 0);

  //
  // From this section, read the rest of the parameters we need and close the file.
  //
  _readKeyword(ifp, "RAW_DATA_OFFSET_METERS", &raw_data_offset_meters, 1);

  _readKeyword(ifp, "RAW_DATA_FIRST_SAMPLE", &raw_data_first_sample, 1);
  _readKeyword(ifp, "RAW_DATA_SAMPLE_COUNT", &raw_data_sample_count, 1);
  _readKeyword(ifp, "RANGE_GATES", &range_gates, 1);
  _readKeyword(ifp, "SAMPLES_PER_GATE", &samples_per_gate, 1);
  _readKeyword(ifp, "GATES_TO_MERGE", &gates_to_merge, 1);

  fclose(ifp);

  if ((int)rint(gates_to_merge) != _params->expected_merged_gates){
    if (_params->exit_on_unexpected_merge){
      cerr << "Gates to merge : " << (int)rint(gates_to_merge) << endl;
      cerr << "Expected : " << _params->expected_merged_gates << endl;
      exit(0);
    }
  }

  //
  // Print these out.
  //

  if (_params->debug){
    fprintf(stderr,"\nTEXT DATA FOR %d/%02d/%02d :\n", (int)year, (int)month, (int)day);

    fprintf(stderr,"Sample frequency : %g\n", sample_frequency);
    fprintf(stderr,"Raw data offset (meters) : %g\n", raw_data_offset_meters);
    fprintf(stderr,"Raw data first sample : %g\n", raw_data_first_sample);
    fprintf(stderr,"Raw data sample count : %g\n", raw_data_sample_count);
    fprintf(stderr,"Range gates : %g\n", range_gates);
    fprintf(stderr,"Samples per gate : %g\n", samples_per_gate);
    fprintf(stderr,"Gates to merge : %g\n", gates_to_merge);
  }
  //
  // Now, do the calculation of range gate spacing. This is
  // right out of the text I have from CLR.
  //
  double RangePerSample = 1.5e+08/sample_frequency;

  int SamplesBetweenRangeGateCenters = (int)rint((raw_data_sample_count-samples_per_gate)/(range_gates-1.0));

  double RangeBetweenGateCenters = SamplesBetweenRangeGateCenters * RangePerSample;

  double RangePerGate = samples_per_gate * RangePerSample;

  double FirstRange = raw_data_offset_meters + (raw_data_first_sample + samples_per_gate/2.0)*RangePerSample;

  double CorrectedFirstRange = FirstRange + ((gates_to_merge-1.0)/2.0) * RangePerGate;
  //
  // Print out the gate spacing parameters.
  //
  if (_params->debug){
    fprintf(stderr,"First range : %gm\n", CorrectedFirstRange);
    fprintf(stderr,"Range step : %gm\n", RangeBetweenGateCenters);
    fprintf(stderr,"Last range : %gm\n\n", 
	    CorrectedFirstRange+range_gates*RangeBetweenGateCenters);
  }
  //
  // Store them in some class parameters.
  //
  _firstRange = CorrectedFirstRange;
  _deltaRange = RangeBetweenGateCenters;
  //  _numGates = (int)rint(range_gates);
   _numGates = (int)rint(range_gates) - (int)rint(gates_to_merge) + 1;

  return;

}

//
// Small routine to read values from the text file. If tryRead is
// set, then a value is read into the double pointed at by val - otherwise the
// stream is positioned just after the keyword.
//
void Windtracer2ncd::_readKeyword(FILE *fp, char *key, double *val, int tryRead){

  const int lineLen = 1024;
  char Line[lineLen];

  do {
    if (NULL == fgets(Line, lineLen, fp)){
      //
      // Must have hit the end of the file - should not have happened.
      //
      fprintf(stderr,"Could not locate keyword %s\n", key);
      exit(-1);
    }
    if (!(strncmp(Line, key, strlen(key)))){
      //
      // Found the key word.
      // If we don't have to read a value, return.
      //
      if (!(tryRead)) return;
      //
      // Otherwise, read the value.
      //
      char *p = Line + strlen(key);
      if (1 != sscanf(p, "%lf", val)){
	fprintf(stderr,"Could not decode keyword %s\n", key);
	exit(-1);
      }
      return;
    }
  } while(1);


}



void Windtracer2ncd::_byteSwap4(void *p){

  unsigned char *b = (unsigned char *)p;

  unsigned char b1 = *b;
  unsigned char b2 = *(b+1);
  unsigned char b3 = *(b+2);
  unsigned char b4 = *(b+3);


  *(b+3) = b1;
  *(b+2) = b2;
  *(b+1) = b3;
  *b = b4;

  return;

}
//
