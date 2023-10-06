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
///////////////////////////////////////////////////
// OutputFile - adapted from code by Mike Dixon for
// MM5Ingest
//
///////////////////////////////////////////////////
#ifndef _OUTPUT_FILE_
#define _OUTPUT_FILE_

//
// Forward class declarations
//
#include "Params.hh"
class MdvxField;
class DsMdvx;
class HtInterp;

class OutputFile {
  
public:

  OutputFile( Params *params );
 
  ~OutputFile();

  inline void setVerticalType( const int& vt  ){_verticalType = vt;}

  void addField(MdvxField* inputField, Mdvx::encoding_type_t encoding);

  int  writeVol(time_t gen_time, long int lead_secs );

  void clear();

  int numFields();

  static Mdvx::encoding_type_t 
    mdvEncoding(Params::encoding_type_t paramEncoding);
  
  static Mdvx::compression_type_t 
    mdvCompression(Params::compression_type_t paramCompression);

protected:
  
private:

  typedef struct {
    double ht;
    int indexLower;
    int indexUpper;
    double ghtLower;
    double ghtUpper;
    double wtLower;
    double wtUpper;
  } interp_pt_t;

  Params *_paramsPtr;

  DsMdvx *_mdvObj;

  HtInterp *_htInterp;

  int _verticalType;

  void _remap(MdvxField* inputField);

  void _setMasterHdr( time_t genTime, long int leadSecs, bool isObs );

  void _remapLambertLambert(MdvxField* inputField);

  float _interp2(Mdvx::field_header_t *fieldHdr, 
                 double x, double y, int z, float *field);

};

#endif
