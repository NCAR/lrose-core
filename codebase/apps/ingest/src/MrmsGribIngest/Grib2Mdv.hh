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
////////////////////////////////////////////////
// Grib2 to MDV Manager
//
// Jason Craig
////////////////////////////////////////////////

#ifndef _GRIB2MDV
#define _GRIB2MDV

#include <string>
#include <vector>
#include <list>

#include <toolsa/utim.h>
#include <grib2/Grib2File.hh>
#include <grib2/Grib2Record.hh>

#include "Params.hh"
using namespace std;

// Forward class declarations

class Grib2File;
class GribRecord;
class DsMdvx;
class MdvxProj;
class MdvxField;

class Grib2Mdv {
public:
   
  Grib2Mdv(const Params &params);
  ~Grib2Mdv();
  
  int readFile(const string &filePath, DsMdvx &out);

private:

  // unit conversion constants

  static const double M_TO_KM;

  // Parameters

  const Params &_params;

  // Data ingest

  Grib2::Grib2File *_grib2File;
  Grib2::Grib2Record::Grib2Sections_t *_gribRecord;
  fl32 _missingVal;
  
  // list of GRIB fields to be processed

  class grib_field_t {
  public:
    string param;
    string level;
    string mdv_name;
    string mdv_units;
    grib_field_t(const string &param_,
                 const string &level_,
                 const string &mdv_name_,
                 const string &mdv_units_) :
            param(param_),
            level(level_),
            mdv_name(mdv_name_),
            mdv_units(mdv_units_) {}
  };
  
  list<grib_field_t> _gribFields;
  list<grib_field_t>::iterator _field;

  // Mdv output

  Mdvx::field_header_t _fieldHeader;
  Mdvx::vlevel_header_t _vlevelHeader;

  bool _reOrderNS_2_SN;
  bool _reOrderAdjacent_Rows;
  bool _reMapField;
  vector<MdvxField*> _outputFields;

  void _convertVerticalUnits();
  int _convertGribLevel2MDVLevel(const string &GribLevel);
  int _createFieldHdr(); 
  void _setMasterHdr(time_t validTime, DsMdvx &mdvx);
  int _writeMdvFile(time_t generateTime, long int forecastTime);

  void _sortByLevel(vector<Grib2::Grib2Record::Grib2Sections_t>::iterator begin, 
		    vector<Grib2::Grib2Record::Grib2Sections_t>::iterator end);
  void _reOrderNS2SN(fl32 *data, int numX, int numY);
  void _reOrderAdjacentRows(fl32 *data, int numX, int numY);
  fl32 *_reMapReducedOrGaussian(fl32 *data, Mdvx::field_header_t fhdr);
  void _setFieldName();

};

#endif
