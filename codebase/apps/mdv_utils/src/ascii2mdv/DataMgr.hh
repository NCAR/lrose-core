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
////////////////////////////////////////////////////////////////////////////////
//
//  Ascii ingest and mdv/grid data output
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  June 1999
//
//  $Id: DataMgr.hh,v 1.8 2016/03/04 02:22:14 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _DATAMGR_INC_
#define _DATAMGR_INC_

#include <string>
#include <vector>

#include <euclid/TypeGrid.hh>
#include <euclid/GridGeom.hh>

#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>

using namespace std;

//
// Forward class declarations
//
class Params;
class MdvFile;

class DataMgr
{
public:
   DataMgr();
  ~DataMgr();

   int              init( Params &params );
   int              format2mdv( const char *inputFile );

private:

   //
   // Constants
   //
   static const int HEADER_LINE_LEN;
  

  // Ascii input

  bool _reverseOrder;
  bool _timeSpecified;
  int _numHeaderLines;
  

  // Field information from parameter file

  string _fieldName;
  string _dataUnits;
  float _missingDataValue;
  
  // Mdv output

  string _outputDir;
  Mdvx _outputMdv;
  MdvxPjg  _projection;
  bool _forceIntegralScaling;
  
  time_t _dataTime;

  // Methods

  static MdvxField *_createField(const string &field_name_long,
				 const string &field_name,
				 const string &units,
				 const time_t data_time,
				 const MdvxPjg &projection,
				 const float missing_data_value);
  
  static void _initMasterHeader(Mdvx::master_header_t &master_hdr,
				const time_t data_time,
				const MdvxPjg &projection);
  
};

#endif
