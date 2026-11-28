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
/*
 *   Module: MdvReader.hh
 *
 *   Author: Jason Craig
 *
 */

#ifndef MDVREADER_HH
#define MDVREADER_HH

#define MYSQLPP_SSQLS_NO_STATICS

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "Params.hh"
#include "InputField.hh"

class MdvReader {

public:

  typedef enum {
    SUCCESS = 0,
    NO_MDV_AT_TIME = -1,
    READ_FAILURE = -3
  } MdvReaderReturn_t;

  // constructor

  MdvReader(Params *params,  Params::input_t mdvInfo);

  // destructor

  ~MdvReader();

  void addReadField(int mdv_number) { if(_mdvx) _mdvx->addReadField(mdv_number); };
  void addReadField(char *fieldName){ if(_mdvx) _mdvx->addReadField(fieldName); };

  int setTime(time_t loadTime);

  time_t getLoadedTime();
  int getForecastDelta();
  string getPathInUse();

  void clearRead();

  bool getField(char *fieldName, InputField *ifield, Params::remap_option_t remap_type, float vert_level, 
		Mdvx::vlevel_header_t *outVHeader);

  bool getField(int fieldNumber, InputField *ifield, Params::remap_option_t remap_type, float vert_level, 
		Mdvx::vlevel_header_t *outVHeader);

  Mdvx::master_header_t getMasterHeader() { return _master_header; };

  // Static members

  static float interp3(Mdvx::field_header_t *fieldHdr, double x, double y, double z, float *field);
  static float interp2(Mdvx::field_header_t *fieldHdr, double x, double y, float *field);
  static float interp2(Mdvx::field_header_t *fieldHdr, double x, double y, int z, float *field);

private:


  bool _getField(MdvxField *field, InputField *ifield, Params::remap_option_t remap_type, float vert_level, 
			  Mdvx::vlevel_header_t *outVHeader);
  float _getZIndex(float altitude, int out_type, Mdvx::field_header_t fieldHdr, Mdvx::vlevel_header_t lvlHdr);
  void _remapBasic(MdvxField* inputField, Params::remap_option_t remap_type, float *zIndexs, int zType);
  void _remapMeanMaxMin(MdvxField* inputField, Params::remap_option_t remap_type, float *zIndexs, int zType);

  Params *_params;
  char *_url;
  int _loadMargin; // minutes
  int _loadOffset;

  DsMdvx *_mdvx;
  Mdvx::master_header_t _master_header;

  int _state;
  time_t _fileTime;
  int _leadTime;
  float _vert_min;
  float _vert_max;
  bool _do_comp;

  string _path_in_use;
};

#endif /* MDVREADER_HH */
