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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 *
 * RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/04 02:22:14 $
 *   $Revision: 1.2 $
 *
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/******************************************************************************

Class for reading input NetCdf files.

Jason Craig, Aug 2007

******************************************************************************/

#include <string.h>

#include "NcInput.hh"

NcInput::NcInput()
{
  _ncFile = NULL;
  _data.msgType = NULL;
  _data.msgTime = NULL;
  _data.latitude = NULL;
  _data.longitude = NULL;
  _data.altitude = NULL;
  _data.altType = NULL;
  _data.callsign = NULL;

  NcError ncError( NcError::silent_nonfatal );
}

NcInput::~NcInput()
{
  _clearRead();
}

void NcInput::_clearRead()
{
  _data.records = 0;
  if(_data.msgType)
    delete _data.msgType;
  _data.msgType = NULL;
  if(_data.msgTime)
    delete _data.msgTime;
  _data.msgTime = NULL;
  if(_data.latitude)
    delete _data.latitude;
  _data.latitude = NULL;
  if(_data.longitude)
    delete _data.longitude;
  _data.longitude = NULL;
  if(_data.altitude)
    delete _data.altitude;
  _data.altitude = NULL;
  if(_data.altType)
    delete _data.altType;
  _data.altType = NULL;
  if(_data.callsign)
    delete _data.callsign;
  _data.callsign = NULL;

  _data.filePath[0] = char(0);
}

bool NcInput::readFile(char *filePath ) 
{
  _clearRead();
  _readOK = true;

  NcError ncError( NcError::silent_nonfatal );

  _ncFile = new NcFile( filePath, NcFile::ReadOnly );
  if( !_ncFile || !_ncFile->is_valid() ) {
    printf("Failed to open file '%s'\n", filePath);
    return 1;
  }
  
  strncpy(_data.filePath, filePath, 199);
  
  _data.records     = getDimensionSize("records");
  if(!_readOK)
    return 1;
  _data.strLen     = getDimensionSize("callsign_len");
  if(!_readOK)
    return 1;
  
  _data.startTime   = getScalarDouble("start_time");
  if(!_readOK)
    return 1;
  _data.endTime     = getScalarDouble("end_time");
  if(!_readOK)
    return 1;

  _data.msgTime = get1dDouble("msg_time", _data.records, NULL);
  if(!_readOK)
    return 1;

  _data.msgType = get1dInt("msg_type", _data.records, NULL);
  if(!_readOK)
    return 1;

  _data.latitude = get1dFloat("latitude", _data.records, &(_data.lat_missing));
  if(!_readOK)
    return 1;

  _data.longitude = get1dFloat("longitude", _data.records, &(_data.lon_missing));
  if(!_readOK)
    return 1;

  _data.altitude = get1dFloat("altitude", _data.records, &(_data.alt_missing));
  if(!_readOK)
    return 1;

  _data.altType = get1dInt("alt_type", _data.records, NULL);
  if(!_readOK)
    return 1;

  _data.callsign = get1DString("callsign", _data.records, _data.strLen);
  if(!_readOK)
    return 1;

  delete _ncFile;
  _ncFile = NULL;

  return 0;
}

int NcInput::getDimensionSize(char *dimName)
{
  NcDim *dim = _ncFile->get_dim(dimName);
  if( !dim || !dim->is_valid() ) {
    printf("Could not get '%s' dimension from file '%s'\n", dimName, _data.filePath);
    _readOK = false;
    return -1;
  }
  return dim->size();
}

double NcInput::getScalarDouble(char *varName)
{
  NcVar *var = _ncFile->get_var(varName);
  if( !var || !var->is_valid() ) {
    printf("Could not get '%s' variable from file '%s'\n", varName, _data.filePath);
    _readOK = false;
    return -1.0;
  }
  return var->as_double(0);
}

int *NcInput::get1dInt(char *varName, long varSize, int *missing_value)
{
  NcVar *var = _ncFile->get_var(varName);
  if( !var || !var->is_valid() ) {
    printf("Could not get '%s' variable from file '%s'\n", varName, _data.filePath);
    _readOK = false;
    return NULL;
  }
  long size = var->num_vals();
  if(var->num_dims() != 1 || size != varSize) {
    printf("Variable '%s' is not 1d in file '%s'\n", varName, _data.filePath);
    _readOK = false;
    return NULL;
  }

  int *values = new int[size];
  if(!var->get(values, size)) {
    printf("Could not get '%s' variable values from file '%s'\n", varName, _data.filePath);
    _readOK = false;
    return NULL;
  }

  NcAtt *att = var->get_att("missing_value");
  if( missing_value && att && att->is_valid() ) {
    *missing_value = att->as_int(0);
  }
  if(att)
    delete att;

  return values;
}

float *NcInput::get1dFloat(char *varName, long varSize, float *missing_value)
{
  NcVar *var = _ncFile->get_var(varName);
  if( !var || !var->is_valid() ) {
    printf("Could not get '%s' variable from file '%s'\n", varName, _data.filePath);
    _readOK = false;
    return NULL;
  }
  long size = var->num_vals();
  if(var->num_dims() != 1 || size != varSize) {
    printf("Variable '%s' is not 1d in file '%s'\n", varName, _data.filePath);
    _readOK = false;
    return NULL;
  }

  float *values = new float[size];
  if(!var->get(values, size)) {
    printf("Could not get '%s' variable values from file '%s'\n", varName, _data.filePath);
    _readOK = false;
    return NULL;
  }

  NcAtt *att = var->get_att("_FillValue");
  if( missing_value && att && att->is_valid() ) {
    *missing_value = att->as_float(0);
  }
  if(att)
    delete att;

  return values;
}

double *NcInput::get1dDouble(char *varName, long varSize, double *missing_value)
{
  const char *FUNCTION_NAME = "get1dDouble";
  NcVar *var = _ncFile->get_var(varName);
  if( !var || !var->is_valid() ) {
    printf("Could not get '%s' variable from file '%s'\n", varName, _data.filePath);
    _readOK = false;
    return NULL;
  }
  long size = var->num_vals();
  if(var->num_dims() != 1 || size != varSize) {
    printf("Variable '%s' is not 1d in file '%s'\n", varName, _data.filePath);
    _readOK = false;
    return NULL;
  }

  double *values = new double[size];
  if(!var->get(values, size)) {
    printf("Could not get '%s' variable values from file '%s'\n", varName, _data.filePath);
    _readOK = false;
    return NULL;
  }

  NcAtt *att = var->get_att("missing_value");
  if( missing_value && att && att->is_valid() ) {
    *missing_value = att->as_double(0);
  }
  if(att)
    delete att;

  return values;
}

char *NcInput::get1DString(char *varName, long varSize, int stringSize)
{
  const char *FUNCTION_NAME = "getVar";
  NcVar *var = _ncFile->get_var(varName);
  if( !var || !var->is_valid() ) {
    printf("Could not get '%s' variable from file '%s'\n", varName, _data.filePath);
    _readOK = false;
    return NULL;
  }
  if(var->num_dims() != 2) {
    sprintf("Variable '%s' is not 2d in file '%s'\n", varName, _data.filePath);
    _readOK = false;
    return NULL;
  }

  long size = 1;  
  long *edges = var->edges();
  for(int a = 0; a < var->num_dims(); a++) {
    size *= edges[a];
  }

  char *data = new char[varSize*stringSize];

  if(var->type() != ncChar) {
    printf("Variable '%s' is not NcChar from file '%s'\n", varName, _data.filePath);
    _readOK = false;
    delete [] edges;
    return NULL;
  } 

  if(!var->get((char*)data, edges[0], edges[1])) {
    printf( "Could not get '%s' variable values from file '%s'\n", varName, _data.filePath);
    _readOK = false;
    delete [] edges;
    return NULL;
  }

  delete [] edges;

  return data;
}
