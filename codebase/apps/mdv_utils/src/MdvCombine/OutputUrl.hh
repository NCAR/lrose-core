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
 *  $Id: OutputUrl.hh,v 1.5 2016/03/04 02:22:10 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header: OutputUrl
// 
// Author: G M Cunning
// 
// Date:	Wed Jan 17 17:02:29 2001
// 
// Description:	This class manages the output MDV data.
// 
// 
// 
// 


# ifndef    OUTPUT_URL_H
# define    OUTPUT_URL_H

// C++ include files
#include <string>
using namespace std;

// System/RAP include files

// Local include files

/////////////////////////////
// forward class declaration
class Mdvx;
class DsMdvx;
class MdvxField;
class Params;

class OutputUrl {
  
public:

  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  OutputUrl(Params *params);

  // destructor
  virtual ~OutputUrl();

  // isOK -- check on class
  bool isOK() const { return _isOk; }

  // getErrStr -- returns error message
  string getErrStr() const { return _errStr; }

  // clear out the current information in the output URL
  void clear(void);
  
  // initialize the headers
  void initMasterHeader(const Mdvx::master_header_t *in_mhdr);

  // add string to data set info
  void addToInfo(const string& info_str);
  void addToInfo(const char* info_str);

  // initialize the headers
  void addField(MdvxField* in_field);

  // write out merged volume
  bool writeVol(const time_t& merge_time, const time_t& start_time, 
		const time_t& end_time);



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
  bool _isOk;
  string _errStr;
  static const string _className;

  Params *_params;
  DsMdvx *_mdvx;
  string _path;
  time_t _mergeTime;
  time_t _startTime;
  time_t _endTime;

  /////////////////////
  // private methods //
  /////////////////////


};



# endif     /* OUTPUT_URL_H */
