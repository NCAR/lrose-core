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
/************************************************************************
 * RunForest: 
 *
 * Jason Craig
 *
 * RAP, NCAR, Boulder CO
 *
 * Dec, 2009
 ************************************************************************/

#ifndef InputField_HH
#define InputField_HH

#include <Mdv/MdvxField.hh>
#include "Params.hh"

using namespace std;

class InputField
{
 public:

  InputField(Params *params, string name);

  ~InputField();

  void clear();

  void setDummy();

  void setField(Mdvx::field_header_t *header, Mdvx::vlevel_header_t *vHeader, float *data);

  void setMissing(float miss, float no_data);

  void setRemap(Params::remap_option_t remap_type) { _remap_type = remap_type; };

  void setZIndex(int z, float val) { if(z < _params->output_nz) _zIndexs[z] = val; };

  float *getzIndexs() { return _zIndexs; };

  float getVal(const int &i, const int &j, const int &k);

  float *getData() { return _data; };

  string getName() { return _name; };

  Mdvx::field_header_t *getHeader() { return _header; };

  Mdvx::vlevel_header_t *getVHeader() { return _vHeader; };

 private:

  Params *_params;

  string _name;

  bool _isDummy;

  float *_data;

  float _missing_val;
  
  float _no_data_val;

  Params::remap_option_t _remap_type;
  
  Mdvx::field_header_t *_header;
  
  Mdvx::vlevel_header_t *_vHeader;

  MdvxProj _inproj;

  MdvxProj _outproj;

  float *_zIndexs;

};

#endif

  
