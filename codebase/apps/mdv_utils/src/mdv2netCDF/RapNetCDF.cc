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
/////////////////////////////////////////////////////////////
//
// RapNetCDF.hh - class that takes RAP data and
// writes it to netCDF files in the COARDS convention.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1999
//
/////////////////////////////////////////////////////////////


#include "RapNetCDF.hh"

#include <Mdv/DsMdvx.hh>
#include <cstdio>
#include <cstring>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <netcdf.h>                                                            
#include <unistd.h>
#include <ctype.h>
using namespace std;

// We have our methods.

// Constructor - init a netCDF file. Filenames under the COARDS convention
// must end in '.nc' - if this is not the case, '.nc' will be 
// appended to the filename.

RapNetCDF::RapNetCDF(const Params &params, date_time_t &data_time,
		     const string &out_path) :
  _params(params),
  _dataTime(data_time),
  _outPath(out_path)

{

  IsOpen = false;
  FirstVarDefined = false;

  if (nc_create(_outPath.c_str(),NC_CLOBBER, &NcID) != NC_NOERR ){
    cerr << "ERROR - RapNetCDF::RapNetCDF" << endl;
    cerr << "  Failed to create output file: " << _outPath << endl;
  } else {
    IsOpen = true; // Spiffing!
  }

}

///////////////////////////////////////////

// Destructor - closes the NetCDF file.
// Also attempts to zip it if requested.
RapNetCDF::~RapNetCDF(){


  if (!(IsOpen)) {
    fprintf(stderr,"RapNetCDF::~RapNetCDF - NetCDF object is not open!\n");
  } else {
    nc_close(NcID);
    IsOpen=0;
  }
  if (_params.CompressAfterWrite){
    if (_params.mode == Params::REALTIME) sleep(1);
    char comStr[512];
    sprintf(comStr,"gzip -f %s",_outPath.c_str());
    int z=system(comStr);
    if (_params.debug){
      fprintf(stderr,"Command : %s\nReturned : %d\n",comStr,z);
    }
  }
}
  
////////////////////////////////////////////////////////////////


// GetNCDFromMDVFieldHeader - returns a RapNCD_t
// structure given an MDV field header.
void RapNetCDF::Mdv2NCD(MdvxField *field,
			date_time_t dataTime,
			RapNCD_t *R)

{

  Mdvx::field_header_t Fhdr=field->getFieldHeader();
  Mdvx::vlevel_header_t V=  field->getVlevelHeader();  

  R->Nx = Fhdr.nx;
  R->Ny = Fhdr.ny;
  R->Nz = Fhdr.nz;

  R->OriginX = Fhdr.proj_origin_lon;
  R->OriginY = Fhdr.proj_origin_lat;

  R->missing = Fhdr.missing_data_value;
  R-> fill = R->missing;

  R->x = (float *)malloc(R->Nx * sizeof(float));
  R->y = (float *)malloc(R->Ny * sizeof(float));
  R->z = (float *)malloc(R->Nz * sizeof(float));

  for (size_t i=0; i < R->Nx; i++){
    R->x[i] = Fhdr.grid_minx + i * Fhdr.grid_dx;
  }

  for (size_t i=0; i < R->Ny; i++){
    R->y[i] = Fhdr.grid_miny + i * Fhdr.grid_dy;
  }

  for (size_t i=0; i < R->Nz; i++){
    R->z[i] = V.level[i];
  }

  R->zName = STRdup("height"); R->zUnits = STRdup(_params.zUnitLabel); R->zUp = STRdup("up");

  if (Fhdr.proj_type == Mdvx::PROJ_LATLON){
    R->yName = STRdup("lat"); R->yUnits = STRdup("degrees_north"); 
    R->xName = STRdup("lon"); R->xUnits = STRdup("degrees_east"); 
  } else {
    if (Fhdr.proj_type == Mdvx::PROJ_FLAT) {
      R->yName = STRdup("Y"); R->yUnits = STRdup("km"); 
      R->xName = STRdup("X"); R->xUnits = STRdup("km");
    } else {
      R->yName = STRdup("Y"); R->yUnits = STRdup("None"); 
      R->xName = STRdup("X"); R->xUnits = STRdup("None");
    }
  }

  //
  // See if we have substitutions for field names, unit.
  //
  int subsIndex = -1;
  for (int j=0; j < _params.substitutions_n; j++){
    if (0==strcmp(_params._substitutions[j].mdvFieldname,
		  Fhdr.field_name)){
      subsIndex = j;
      break;
    }
  }

  //
  // OK - at this point, if subsIndex == -1 then we
  // are NOT doing substitutions for name, units etc. Otherwise
  // we are.
  //

  if (subsIndex == -1){
    R->dataName = STRdup(Fhdr.field_name); 
  } else {
    R->dataName = STRdup(_params._substitutions[subsIndex].netCDFfieldname);
  }

  for (size_t i=0; i < strlen(R->dataName); i++){
    if (!(isalnum(R->dataName[i])))
      R->dataName[i]='_';
  }

  if (subsIndex == -1){
    R->dataUnits = STRdup(Fhdr.units);
  } else {
    R->dataUnits = STRdup(_params._substitutions[subsIndex].units);
  }

  if (subsIndex == -1){
    R->longDataName = STRdup( R->dataName );
  } else {
    R->longDataName = STRdup( _params._substitutions[subsIndex].longName );
  }

  R->history = STRdup("Generated from MDV file, NCAR (RAP division) by Niles Oien (oien@ucar.edu)");

  R->dataTime = dataTime;

  R->data = (float *) field->getVol();
  for (int i=0; i< Fhdr.nx * Fhdr.ny * Fhdr.nz; i++){
    if (R->data[i] == Fhdr.bad_data_value) R->data[i]=Fhdr.missing_data_value;
  }


}

//////////////////////////////////////////////////////////////


// NCDWriteFirstVar - Write the first variable to the file.
// Returns 0 if all went well. Call once only.
int RapNetCDF::NCDWriteFirstVar( RapNCD_t N ){

  if (!(IsOpen)) {
    fprintf(stderr,"RapNetCDF::AddNCD - NetCDF object is not ready!\n");
    return -1;
  }

  if (FirstVarDefined) {
    fprintf(stderr,
	    "RapNetCDF::AddNCD - First variable already defined, exiting\n");
    return -1;
  }

     
  // Add the global parameters.
  
  nc_put_att_text(NcID, NC_GLOBAL, "history",
		  strlen(N.history),
		  N.history);

  
  char ConventionString[64];
  sprintf(ConventionString,"%s","COARDS");

  nc_put_att_text(NcID, NC_GLOBAL, "Conventions",
		  strlen(ConventionString),
		  ConventionString);

  char GenStr[512],Host[64];
  date_time_t Now;
  ugmtime(&Now);
  if (gethostname(Host,64)){
    fprintf(stderr,"Cannot determine host name.\n");
    return -1;
  }
  sprintf(GenStr,
	  "File %s generated %4d/%02d/%02d %02d:%02d:%02d UTC on host %s,"
	  " by Niles Oien, RAP, NCAR (oien@ucar.edu)",
	  _outPath.c_str(),
	  Now.year, Now.month, Now.day, Now.hour, Now.min, Now.sec,
	  Host);

  nc_put_att_text(NcID, NC_GLOBAL, "FileOrigins",
		  strlen(GenStr), GenStr);

  char TimeStr[64];
  sprintf(TimeStr,"%4d/%02d/%02d %02d:%02d:%02d",
	  N.dataTime.year, N.dataTime.month, N.dataTime.day, 
	  N.dataTime.hour, N.dataTime.min, N.dataTime.sec);
  nc_put_att_text(NcID, NC_GLOBAL, "DataTime",
		  strlen(TimeStr), TimeStr);
                                 
  float f;
  f = (float) N.dataTime.year;
  nc_put_att_float(NcID, NC_GLOBAL, "Year", NC_FLOAT, 1, &f);

  f = (float) N.dataTime.month;
  nc_put_att_float(NcID, NC_GLOBAL, "Month", NC_FLOAT, 1, &f);

  f = (float) N.dataTime.day;
  nc_put_att_float(NcID, NC_GLOBAL, "Day", NC_FLOAT, 1, &f);

  f = (float) N.dataTime.hour;
  nc_put_att_float(NcID, NC_GLOBAL, "Hour", NC_FLOAT, 1, &f);

  f = (float) N.dataTime.min;
  nc_put_att_float(NcID, NC_GLOBAL, "Minute", NC_FLOAT, 1, &f);

  f = (float) N.dataTime.sec;
  nc_put_att_float(NcID, NC_GLOBAL, "Second", NC_FLOAT, 1, &f);

  f = (float) N.dataTime.unix_time;
  nc_put_att_float(NcID, NC_GLOBAL, "UNIX_Time", NC_FLOAT, 1, &f);

  nc_put_att_float(NcID, NC_GLOBAL, "OriginX", NC_FLOAT, 1, &N.OriginX);
  nc_put_att_float(NcID, NC_GLOBAL, "OriginY", NC_FLOAT, 1, &N.OriginY);


  //
  // Add a specified ascii file as a set of text attributes, 
  // line by line, if requested.
  //
  if (_params.includeFile){

    FILE *fp = fopen(_params.includeFilename,"r");
    if (fp == NULL){
      //
      // The file does not exist - print a warning and carry on.
      //
      cerr << "WARNING : Include file " << _params.includeFilename << " not found." << endl;
    } else {

      const int LineLen = 1024;
      char Line[LineLen];
      int lineNum = 0;

      while (NULL != fgets(Line, LineLen, fp)){

	lineNum++;
	//
	// Chomp off the last carriage return.
	//
	int len = strlen(Line);
	if (len > 0){
	  if (Line[len-1] < 32) { // It's a control code
	    Line[len-1] = char(0);
	  }
	}
	//
	// Put together the tag string - ie. the attribute name.
	//
	char tagString[256];
	sprintf(tagString,"Include_%d", lineNum);
	//
	// Add this to the netCDF output.
	//
	nc_put_att_text(NcID, NC_GLOBAL, tagString,
			strlen(Line), Line);
	//
	// Print some debugging.
	//
	if (_params.debug >= Params::DEBUG_NORM){
	  cerr << "Added file include line attribute " << tagString;
	  cerr << " : " << Line << endl;
	}

      } // End of loop through the file.

      fclose(fp);

    }
  }


  //
  // Add the UNIX time as a dimensioned variable - requested by ATD.
  //

  // Now add the dimensions.
  if (nc_def_dim(NcID, "time", 1, &DimID[0]) != NC_NOERR){
    fprintf(stderr,"Failed on nc_def_dim for time variable.\n");
    return -1;
  }


  if ((_params.DataOrderingZYX) && (N.Nz > 1) ){

    // ZYX ordering - only supported if Nz > 1

    if (nc_def_dim(NcID, N.zName, N.Nz, &DimID[1]) != NC_NOERR ){
      fprintf(stderr,"Failed to set Z dimension.\n");
      return -1;
    }
    
    if (nc_def_dim(NcID, N.yName, N.Ny, &DimID[2]) != NC_NOERR ){
      fprintf(stderr,"Failed to set Y dimension.\n");
      return -1;
    }
    
    
    if (nc_def_dim(NcID, N.xName, N.Nx, &DimID[3]) != NC_NOERR ){
      fprintf(stderr,"Failed to set X dimension.\n");
      return -1;
    }

  } else {

    // YXZ ordering

    if (nc_def_dim(NcID, N.yName, N.Ny, &DimID[1]) != NC_NOERR ){
      fprintf(stderr,"Failed to set Y dimension.\n");
      return -1;
    }
    
    
    if (nc_def_dim(NcID, N.xName, N.Nx, &DimID[2]) != NC_NOERR ){
      fprintf(stderr,"Failed to set X dimension.\n");
      return -1;
    }

    if (N.Nz > 1) {
      if (nc_def_dim(NcID, N.zName, N.Nz, &DimID[3]) != NC_NOERR ){
        fprintf(stderr,"Failed to set Z dimension.\n");
        return -1;
      }
    }

  }

  // Now give these dimensions names and assign values.
  // These are co-ordinate variables, so under COARDS they have the
  // same name as their dimensions.



  // Time

  int timeVarID;
  if (nc_def_var(NcID, "time",
		 NC_LONG, 1, &DimID[0], &timeVarID) != NC_NOERR ){
    fprintf(stderr,"Failed on nc_def_var for time variable.\n");
    return -1;
  }

  nc_put_att_text(NcID, timeVarID, "units",
		  strlen("seconds since 1970-1-1 0:00:00 0:00"), 
		  "seconds since 1970-1-1 0:00:00 0:00");

  int YvarID;
  int XvarID;
  int ZvarID;
  
  if ((_params.DataOrderingZYX) &&  (N.Nz > 1)){
    
    // ZYX ordering

    if (nc_def_var(NcID, N.zName,
		   NC_FLOAT, 1, &DimID[1], &ZvarID) != NC_NOERR ){
      fprintf(stderr,"Failed on nc_def_var for Z\n");
      return -1;
    }
    
    // Y
    if (nc_def_var(NcID, N.yName,
                   NC_FLOAT, 1, &DimID[2], &YvarID) != NC_NOERR ){
      fprintf(stderr,"Failed on nc_def_var for Y\n");
      return -1;
    }
    
    // X
    if (nc_def_var(NcID, N.xName,
                   NC_FLOAT, 1, &DimID[3], &XvarID) != NC_NOERR ){
      fprintf(stderr,"Failed on nc_def_var for X\n");
      return -1;
    }
    
  } else {
    
    // YXZ ordering
    
    // Y
    if (nc_def_var(NcID, N.yName,
                   NC_FLOAT, 1, &DimID[1], &YvarID) != NC_NOERR ){
      fprintf(stderr,"Failed on nc_def_var for y\n");
      return -1;
    }
    
    // X
    if (nc_def_var(NcID, N.xName,
                   NC_FLOAT, 1, &DimID[2], &XvarID) != NC_NOERR ){
      fprintf(stderr,"Failed on nc_def_var for x\n");
      return -1;
    }
    
    // Z - if required.
    if (N.Nz > 1) {
      if (nc_def_var(NcID, N.zName,
                     NC_FLOAT, 1, &DimID[3], &ZvarID) != NC_NOERR ){
        fprintf(stderr,"Failed on nc_def_var for z\n");
        return -1;
      }
    }

  }

  // set attributes

  if (N.Nz > 1) {
    
    nc_put_att_text(NcID, ZvarID, "units",
                    strlen(N.zUnits), N.zUnits);
    
    nc_put_att_float(NcID, ZvarID, "_FillValue",
                     NC_FLOAT, 1, &N.fill);     
    
    //
    // The 'positive' attribute is specific to the Z axis.
    // It is either 'up' or 'down'.
    //
    nc_put_att_text(NcID, ZvarID, "positive",
                    strlen(N.zUp), N.zUp);
    
  }
  
  nc_put_att_text(NcID, YvarID, "units",
                  strlen(N.yUnits), N.yUnits);
  
  nc_put_att_float(NcID, YvarID, "_FillValue",
                   NC_FLOAT, 1, &N.fill);     
  
  nc_put_att_text(NcID, XvarID, "units",
                  strlen(N.xUnits), N.xUnits);
  
  nc_put_att_float(NcID, XvarID, "_FillValue",
                   NC_FLOAT, 1, &N.fill);     
  
  //
  // Now add the actual variable.
  //

  if (N.Nz > 1){
    if (nc_def_var(NcID, N.dataName,
		   NC_FLOAT, 4, DimID, &varID) != NC_NOERR ){
      fprintf(stderr,"Failed on 4d nc_def_var\n");
      return -1;
    }
  } else {
    if (nc_def_var(NcID, N.dataName,
		   NC_FLOAT, 3, DimID, &varID) != NC_NOERR ){
      fprintf(stderr,"Failed on 3d nc_def_var\n");
      return -1;
    }
  }     

  nc_put_att_text(NcID, varID, "units",
		  strlen(N.dataUnits),
		  N.dataUnits);

  nc_put_att_text(NcID, varID, "longname",
		  strlen(N.longDataName),
		  N.longDataName);

  nc_put_att_float(NcID, varID, "_FillValue",
		   NC_FLOAT, 1, &N.fill);
   
  nc_put_att_float(NcID, varID, "missing_value",
		   NC_FLOAT, 1, &N.missing);
 

  //
  // End of defintion mode - now we can actually
  // write the data.
  //
  nc_enddef(NcID);   

  //
  // Add the time as a variable - requested by ATD.
  //
  f = (float) N.dataTime.unix_time;
  nc_put_var_long(NcID, timeVarID, &N.dataTime.unix_time);

  // Write the co-ordinate axes.
  nc_put_var_float(NcID, XvarID, N.x);  
  nc_put_var_float(NcID, YvarID, N.y);  
  if (N.Nz > 1)  nc_put_var_float(NcID, ZvarID, N.z);  

  // Then the actual thing itself.
  nc_put_var_float(NcID, varID, N.data);

  //
  // Make local copies in case of a call
  // to AddAnother.
  //
  _Nz = N.Nz;


  FirstVarDefined = 1;
  return 0;

}

///////////////////////////////////////////////////////////////////

int RapNetCDF::NCDAddAnother(char *dataName,
			     char *dataUnits,
			     char *dataLongName,
			     float fill, float missing,
			     float *data){

  nc_redef(NcID);

  if (_Nz > 1){
    if (nc_def_var(NcID, dataName,
		   NC_FLOAT, 4, DimID, &varID) != NC_NOERR ){
      fprintf(stderr,"Failed on 4d nc_def_var in AddAnother\n");
      return -1;
    }
  } else {
    if (nc_def_var(NcID, dataName,
		   NC_FLOAT, 3, DimID, &varID) != NC_NOERR ){
      fprintf(stderr,"Failed on 3d nc_def_var in AddAnother\n");
      return -1;
    }
  }     

  nc_put_att_text(NcID, varID, "units",
		  strlen(dataUnits),
		  dataUnits);

  nc_put_att_text(NcID, varID, "longname",
		  strlen(dataLongName),
		  dataLongName);

  nc_put_att_float(NcID, varID, "_FillValue",
		   NC_FLOAT, 1, &fill);
   
  nc_put_att_float(NcID, varID, "missing_value",
		   NC_FLOAT, 1, &missing);
 
  //
  // End of defintion mode - now we can again actually
  // write the data.
  //
  nc_enddef(NcID);   

  // Then the actual thing itself.
  nc_put_var_float(NcID, varID, data);

  return 0;

}


///////////////////////////////////////////////////

// Add global text and float attributes
// returns 0 if OK
int RapNetCDF::RapNCDAddGlobalText(char *AttName, char *Att){

  nc_redef(NcID);

  if (nc_put_att_text(NcID, NC_GLOBAL, AttName,
		      strlen(Att),Att) != NC_NOERR){
    return -1;
  } else {
    return 0;
  }

  nc_enddef(NcID);

}

int RapNetCDF::RapNCDAddGlobalFloat(char *AttName, float *Att, int NumFloats){

  nc_redef(NcID);

  if (nc_put_att_float(NcID, NC_GLOBAL, AttName,
		      NC_FLOAT, NumFloats, Att) != NC_NOERR){
    return -1;
  } else {
    return 0;
  }

  nc_enddef(NcID);

}


// Add text and float attributes - local to variable
// Call after AddNCD
// returns 0 if OK

int RapNetCDF::RapNCDAddLocalText(char *AttName, char *Att){

  nc_redef(NcID);

  if (nc_put_att_text(NcID, varID, AttName,
		      strlen(Att),Att) != NC_NOERR){
    return -1;
  } else {
    return 0;
  }

  nc_enddef(NcID);

}

int RapNetCDF::RapNCDAddLocalFloat(char *AttName, float *Att, int NumFloats){

  nc_redef(NcID);

  if (nc_put_att_float(NcID, varID, AttName,
		      NC_FLOAT, NumFloats, Att) != NC_NOERR){
    return -1;
  } else {
    return 0;
  }

  nc_enddef(NcID);

}




///////////////////////////////////////////////////////////////////
//
// Frees up a RapNCD_t structure that was obtained 
// from Mdv2NCD
void RapNetCDF::RapNCDFree( RapNCD_t N){

  free(N.x); free(N.y); free(N.z);

  free(N.history);
  free(N.xName); free(N.xUnits);
  free(N.yName); free(N.yUnits);
  free(N.zName); free(N.zUnits); free(N.zUp);
  free(N.dataName); free(N.dataUnits);
  free( N.longDataName );

}













