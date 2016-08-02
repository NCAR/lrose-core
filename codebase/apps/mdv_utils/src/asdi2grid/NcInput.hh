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
/*
-------------------------------------------------------------------------------
NcInput.hh
Class for reading input NetCdf files.

Jason Craig, Aug 2007

-------------------------------------------------------------------------------
*/

#ifndef _NC_INPUT_HH_
#define _NC_INPUT_HH_

#include <string>
#include <dirent.h>
#include <netcdf.hh>
#include <toolsa/Path.hh>

typedef struct
{
  char filePath[200];
  long int records;
  double startTime;
  double endTime;
  int *msgType;
  double *msgTime;
  float *latitude;
  float lat_missing;
  float *longitude;
  float lon_missing;
  float *altitude;
  float alt_missing;
  int *altType;
  char *callsign;
  int strLen;
} InputData_t;

class NcInput 
{
public:
 
  NcInput();
  
  ~NcInput();
  
  bool readFile(char *filePath);

  inline InputData_t *getData() { return &_data; };

private:
  
  NcFile *_ncFile;
  
  bool _readOK;

  InputData_t _data;

  void _clearRead();

  int getDimensionSize(char *dimName);
  double getScalarDouble(char *varName);
  int *get1dInt(char *varName, long varSize, int *missing_value);
  float *get1dFloat(char *varName, long varSize, float *missing_value);
  double *get1dDouble(char *varName, long varSize, double *missing_value);
  char *get1DString(char *varName, long varSize, int stringSize);
};

#endif
   
   
