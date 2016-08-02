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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: OutputFile.hh,v 1.5 2016/03/07 01:23:05 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	OutputFile
// 
// Author:	G. M. Cunning
// 
// Date:	Wed Jul 25 20:59:10 2007
// 
// Description:	
// 
// 


# ifndef    OUTPUT_FILE_H
# define    OUTPUT_FILE_H

// C++ include files
#include <string>
#include <ctime>

// System/RAP include files
#include <Mdv/MdvxField.hh>

// Local include files

using namespace std;

//
// Forward class declarations
//
class DsMdvx;
class Params;
class SeviriData;
class Pjg;

class OutputFile {
  
public:

  static const double MISSING_DATA_VALUE = -9999.0;

  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  OutputFile();
  OutputFile(const OutputFile &);

  // destructor
  virtual ~OutputFile();

  bool init(const Params* params);

  bool addData(const SeviriData* seviri_data);

  bool write();


protected:

  ///////////////////////
  // protected members //
  ///////////////////////
  

  ///////////////////////
  // protected methods //
  ///////////////////////

private:

  /////////////////////
  // private members //
  /////////////////////
  static const string _className;

  const Params *_params;

  DsMdvx *_outMdvx;

  const Pjg *_pjg;

  time_t _imageTime;

  time_t _lastImageTime;

  time_t _timestamp;

  string _sourceInfo;

  /////////////////////
  // private methods //
  /////////////////////

  void _copy(const OutputFile &from);
  void _getBandInfo(int band_num, string& name, string& long_name, string& units);
  void _setupProjection(const Pjg* pjg);
  bool _processData(const SeviriData* seviri_data);
  void _setMasterHeader(const time_t& d_time, 
			Mdvx::master_header_t *hdr);
  void _setFieldHeader(const string& long_name, const string& name, 
		       const string& units, Mdvx::field_header_t *hdr);
  void _setTimestamp(const string& path);
  void _remap(MdvxField* field);
};

# endif     /* OUTPUT_FILE_H */
