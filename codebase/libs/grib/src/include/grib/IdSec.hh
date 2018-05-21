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
/////////////////////////////////////////////
// IdSec - Indicator Section
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
//
////////////////////////////////////////////
#ifndef _ID_SECTION
#define _ID_SECTION

#include <iostream>
#include <grib/GribSection.hh>
using namespace std;

class IdSec : public GribSection {

public:

  IdSec();
  ~IdSec(){};
  
  int unpack( ui08 *idPtr );
  int pack( ui08 *idPtr );
  
  inline int getTotalSize() const { return( _numMsgBytes ); }
  inline int getEditionNum() const { return( _editionNum ); }

  inline void setTotalSize(const int new_size) { _numMsgBytes = new_size; }
  inline void setEditionNum(const int edition_num)
    { _editionNum = edition_num; }
  
  void print(FILE *) const;
  void print(ostream &stream) const;
  
private:
  
  static const int IS_NUM_BYTES_PACKED;
  
  int _numMsgBytes;
  int _editionNum;

};

#endif
