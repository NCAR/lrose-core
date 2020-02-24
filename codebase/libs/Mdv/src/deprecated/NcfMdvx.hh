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

///////////////////////////////////////////////////////////////
// NcfMdvx.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008
//
///////////////////////////////////////////////////////////////
//
// The NcfMdvx extends the DsMdvx class, adding the capability
// to locally convert to/from netCDF CF.
//
///////////////////////////////////////////////////////////////

#ifndef NcfMdvx_HH
#define NcfMdvx_HH

#include <Mdv/DsMdvx.hh>
using namespace std;

class DsMdvxMsg;
class Mdv2NcfTrans;

///////////////////////////////////////////////////////////////
// class definition

class NcfMdvx : public DsMdvx

{

  friend class DsMdvxMsg;
  friend class Mdv2NcfTrans;

public:
  
  // constructor

  NcfMdvx();

  // copy constructor
  
  NcfMdvx(const NcfMdvx &rhs);
  
  // destructor
  
  virtual ~NcfMdvx();

  // assignment
  
  NcfMdvx & operator=(const NcfMdvx &rhs);

  // convert MDV to NETCDF CF
  // stores the NetCdf version in _ncfBuf
  // returns 0 on success, -1 on failure

  virtual int convertMdv2Ncf(const string &url);

  // convert NETCDF CF to MDV
  // given an object containing a netcdf file buffer
  // returns 0 on success, -1 on failure

  virtual int convertNcf2Mdv(const string &url);

  // constrain NETCDF CF using read qualifiers
  // returns 0 on success, -1 on failure

  virtual int constrainNcf(const string &url);

  // Read the headers from a NETCDF CF file into MDV, given the file path
  // Convert to NCF at the end if required
  // Currently we read all of the data, which will include the headers.
  // returns 0 on success, -1 on failure
  
  virtual int readAllHeadersNcf(const string &url);

  // Read a NETCDF CF file into MDV, given the file path
  // Convert to NCF at the end if required
  // returns 0 on success, -1 on failure

  virtual int readNcf(const string &url);

  // Read the metadata from a RADX file, given the file path
  // Fill out the Mdv file headers
  // returns 0 on success, -1 on failure

  virtual int readAllHeadersRadx(const string &url);

  // Read a RADX-type file, convert to MDV
  // Convert to RADX at the end if required
  // returns 0 on success, -1 on failure

  virtual int readRadx(const string &url);

  // write to directory
  // returns 0 on success, -1 on failure
  
  virtual int writeToDir(const string &output_url);

protected:

  // functions

  NcfMdvx &_copy(const NcfMdvx &rhs);

private:

  int _convertNcfToMdvAndWrite(const string &url);
  int _convertMdvToNcfAndWrite(const string &url);
  int _constrainNcfAndWrite(const string &url);
  bool _getWriteAsForecast();
  string _computeNcfOutputPath(const string &outputDir);
  void _doWriteLdataInfo(const string &outputDir,
                         const string &outputPath,
                         const string &dataType);

};

#endif


