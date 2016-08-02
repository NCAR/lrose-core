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
///////////////////////////////////////////////////////////////////////////
// Grib2 to MDV Manager
//
// Using a list of fields to process, converts each field
// from grib2 to Mdv format.
//
// Jason Craig
///////////////////////////////////////////////////////////////////////////

#include <cstring>

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/Path.hh>
#include <euclid/PjgLc1Calc.hh>
#include <euclid/PjgLc2Calc.hh>
#include <euclid/PjgPolarStereoCalc.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>
#include <grib2/GDS.hh>
#include <grib2/PDS.hh>
#include <grib2/DRS.hh>
#include <grib2/DS.hh>
#include <grib2/LatLonProj.hh>
#include <grib2/LambertConfProj.hh>
#include <grib2/PolarStereoProj.hh>
#include <grib2/GausLatLonProj.hh>
#include <grib2/Template5.2.hh>
#include <grib2/Template5.3.hh>

#include "Grib2Mdv.hh"
using namespace std;

// Constants

const double Grib2Mdv::M_TO_KM = 0.001;

// constructor

Grib2Mdv::Grib2Mdv(const Params &params) :
        _params(params)
{

  _grib2File = NULL;
  _gribRecord = NULL;

  MEM_zero(_fieldHeader);
  MEM_zero(_vlevelHeader);

  _grib2File   = new Grib2::Grib2File ();

}

// detructor

Grib2Mdv::~Grib2Mdv()
{
  _gribFields.clear();
  if (_grib2File != NULL) {
    delete _grib2File;
  }
}

  
int Grib2Mdv::readFile(const string &filePath, DsMdvx &out)

{

  Grib2::Grib2Record::print_sections_t printSec;

  // Create a grib2 Print sections object from the params request

  if(_params.debug > 1 || _params.print_sections) {
    printSec.is = _params.print_sec_is;
    printSec.ids = _params.print_sec_ids;
    printSec.lus = _params.print_sec_lus;
    printSec.gds = _params.print_sec_gds;
    printSec.pds = _params.print_sec_pds;
    printSec.drs = _params.print_sec_drs;
    printSec.bms = _params.print_sec_bms;
    printSec.ds = _params.print_sec_ds;
  }

  // Process file

  Path file_path_obj(filePath);
      
  // Inventory the file

  if(_grib2File->read(filePath) != Grib2::GRIB_SUCCESS) {
    return -1;
  }
   
  // Print the full contents of the grib file

  if(_params.debug > 1 || _params.print_sections ) {
    _grib2File->printContents(stdout, printSec);
  }
   
  if(_params.print_sections ) {
    return 0;
  }

  // Print only a summary of the grib file

  if (_params.debug || _params.print_summary) {
    _grib2File->printSummary(stdout, _params.debug);
  }

  if(_params.print_summary) {
    return 0;
  }
   
  // Get the full list of fields - just read in the inventory process

  _gribFields.erase(_gribFields.begin(), _gribFields.end());
  list <string> fieldList = _grib2File->getFieldList();
  list <string>::const_iterator field;
  
  for (field = fieldList.begin(); field != fieldList.end(); ++field) {
    
    list <string> fieldLevelList = _grib2File->getFieldLevels(*(field));
    list <string>::const_iterator level;
    
    for (level = fieldLevelList.begin(); level != fieldLevelList.end(); ++level) {
      
      grib_field_t out_field(*field, *level, *field, "");
      _gribFields.push_back(out_field);
      
      if(_params.print_var_list) {
        vector<Grib2::Grib2Record::Grib2Sections_t> GribRecords =
          _grib2File->getRecords(out_field.param, out_field.level);
        char name[20];
        sprintf(name, "%s %s", out_field.param.c_str(), out_field.level.c_str());
        fprintf(stderr, "%-20s \t'%s' '%s'\n", name,
                GribRecords[0].summary->longName.c_str(),
                GribRecords[0].summary->levelTypeLong.c_str());
      }
      
    } // level
    
  } // field

  if(_params.print_var_list) {
    return 0;
  }

  // Get the list of forecast times, usually there is just one
  // but not always. Use the first one.

  list <long int> forecastList = _grib2File->getForecastList();
  if (forecastList.size() < 1) {
    cerr << "ERROR - Grib2Mdv::readFile" << endl;
    cerr << "  No times in file: " << filePath << endl;
    return -1;
  }

  // use first lead time
   
  list <long int>::const_iterator leadTime = forecastList.begin();
   
  // check the lead time if required
   
  if (_params.debug) {
    cerr << "Getting fields for lead time: " << *leadTime << " seconds." << endl;
  }
   
  // Loop over the list of fields to process
  // Keep track of the generate and forecast times
     
  time_t lastGenerateTime = 0;
  _field = _gribFields.begin();
  while (_field != _gribFields.end()) {
    
    if (_params.debug) {
      cerr << "Looking for field " <<  _field->param
           << " level  " << _field->level << endl;
    }
     
    vector<Grib2::Grib2Record::Grib2Sections_t> GribRecords =
      _grib2File->getRecords(_field->param, _field->level, *leadTime);
    
    if(GribRecords.size() > 1) {
      _sortByLevel(GribRecords.begin(), GribRecords.end());
    }
       
    fl32 *fieldDataPtr = NULL;
    fl32 *currDataPtr = NULL;
     
    if (_params.debug) {
      cerr << "Found " << GribRecords.size() << " records." << endl;
    }
     
    size_t nLevels = GribRecords.size();
    if (nLevels >= MDV_MAX_VLEVELS) {
      cerr << "WARNING: Too many levels for MDV file: "  << nLevels << endl;
      cerr << "  Reducing to: " << MDV_MAX_VLEVELS << endl;
      nLevels = MDV_MAX_VLEVELS;
    }
     
    // Set requested vertical level bounds
     
    int levelMin = 0;
    int levelMax = nLevels - 1;
    size_t levelDz = 1;
     
    // Loop over requested vertical levels in each field
     
    for(int levelNum = levelMin; levelNum <= levelMax; levelNum+=levelDz) {
       
      _gribRecord = &(GribRecords[levelNum]);
      lastGenerateTime = _gribRecord->ids->getGenerateTime();
       
      if (_params.debug) {
        cerr <<  _gribRecord->summary->name.c_str() << " ";
        cerr <<  _gribRecord->summary->longName.c_str() << " ";
        cerr <<  _gribRecord->summary->units.c_str() << " ";
        cerr <<  _gribRecord->summary->category << " ";
        cerr <<  _gribRecord->summary->paramNumber << " ";
        cerr <<  _gribRecord->summary->levelType.c_str() << " ";
        cerr <<  _gribRecord->summary->levelVal;
        cerr <<  endl;
      }
       
      // Create Mdvx field header for the first level
       
      if (levelNum == levelMin) {
         
        MEM_zero(_fieldHeader);
        MEM_zero(_vlevelHeader);
        _vlevelHeader.struct_id = Mdvx::VLEVEL_HEAD_MAGIC_COOKIE;
	 
        if (_createFieldHdr() != 0) {
          cerr << "WARNING: File " << filePath << " not processed." << endl << flush;
          out.clear();
          _grib2File->clearInventory();
          return( -1 );
        }
         
        // Pre Count the number of levels
         
        for(int ln = levelNum; ln <= levelMax; ln+=levelDz) {
          _fieldHeader.nz++;
        }
         
        if (_params.debug) {
          cerr << "Processing  " << _fieldHeader.nz << " records." << endl;
        }
         
        fieldDataPtr = new 
          fl32[_fieldHeader.nz*_fieldHeader.nx*_fieldHeader.ny];
        currDataPtr = fieldDataPtr;

      } // if (levelNum == levelMin)
         
      // Get and save the data, reordering and remaping if needed.

      fl32 *data = _gribRecord->ds->getData();
      if(_reMapField) {
        data = _reMapReducedOrGaussian(data, _fieldHeader);
      }
      if(_reOrderNS_2_SN) {
        _reOrderNS2SN(data, _fieldHeader.nx, _fieldHeader.ny);
      }
      if(_reOrderAdjacent_Rows) {
        _reOrderAdjacentRows(data, _fieldHeader.nx, _fieldHeader.ny);
      }
         
      memcpy (currDataPtr, data, sizeof(fl32)*_fieldHeader.nx*_fieldHeader.ny);
      currDataPtr += _fieldHeader.nx*_fieldHeader.ny;
      if(_reMapField) {
        delete[] data;
      }
      _gribRecord->ds->freeData();
       
      // fill out the vlevel header
       
      _vlevelHeader.type[(levelNum - levelMin)/levelDz] = 
        _convertGribLevel2MDVLevel( _gribRecord->summary->levelType );
      _vlevelHeader.level[(levelNum - levelMin)/levelDz] =
        _gribRecord->summary->levelVal;

      // Once we have gotten two vertical levels we can calculate a dz
       
      if(_fieldHeader.nz == 2) {
        if(_vlevelHeader.level[1] == _vlevelHeader.level[0] ) {
          _fieldHeader.grid_dz = 0.0;
        } else {
          _fieldHeader.grid_dz = ( _vlevelHeader.level[1] - _vlevelHeader.level[0] );
        }
        lastGenerateTime = _gribRecord->ids->getGenerateTime();
      }
       
      if(GribRecords.size() == 0) {

        cerr << "WARNING: Field "
             <<  _field->param << " level  "
             << _field->level << " not found in grib file." << endl;
         
      } else {
         
        _setFieldName();
        STRncopy(_fieldHeader.units,
                 _gribRecord->summary->units.c_str(),
                 MDV_UNITS_LEN);
        _convertVerticalUnits();
         
        _fieldHeader.volume_size = (si32)_fieldHeader.nx * (si32)_fieldHeader.ny * 
          (si32)_fieldHeader.nz * (si32)_fieldHeader.data_element_nbytes;
         
        if(_fieldHeader.volume_size < 0) {
          cerr << "ERROR: Field " <<  _field->param 
               << " with " << _fieldHeader.nz 
               << " levels is larger than Mdv can handle." << endl;
          cerr << "Use encoding_type of INT8 or INT16 to reduce"
               << " output size to under 2.156 GB" << endl;
          delete [] fieldDataPtr;
          return( -1 );
        }
         
        MdvxField *fieldPtr = new MdvxField(_fieldHeader, _vlevelHeader, fieldDataPtr);
        delete [] fieldDataPtr;
        out.addField(fieldPtr);
         
      }
       
      _field++;
       
    } // Close loop over levelNum
     
    // set time in master header
    
    _setMasterHdr(lastGenerateTime, out);
     
    if(forecastList.size() == 1) {
      _grib2File->clearInventory();
    }
     
  }  // Close loop on field list
  
  _grib2File->clearInventory();
  
  return(0);

}

// Returns the Mdv level appropriate to a Grib level

int Grib2Mdv::_convertGribLevel2MDVLevel(const string &GribLevel)
{
  const char *GribLevelCStr = GribLevel.c_str();
  // GRIB2 Code Table 4.5 
  // ?? Means I'm not confident this is the right mdv level
  if (!strcmp(GribLevelCStr, "SFC"))
    return Mdvx::VERT_TYPE_SURFACE;
  else if (!strcmp(GribLevelCStr, "SURFACE"))
    return Mdvx::VERT_TYPE_SURFACE;
  else if (!strcmp(GribLevelCStr, "CBL"))
    return Mdvx::VERT_FIELDS_VAR_ELEV;
  else if (!strcmp(GribLevelCStr, "CTL"))
    return Mdvx::VERT_FIELDS_VAR_ELEV;
  else if (!strcmp(GribLevelCStr, "0DEG"))
    return Mdvx::VERT_FIELDS_VAR_ELEV;
  else if (!strcmp(GribLevelCStr, "0_ISO"))
    return Mdvx::VERT_FIELDS_VAR_ELEV;
  else if (!strcmp(GribLevelCStr, "ADCL"))
    return Mdvx::VERT_FIELDS_VAR_ELEV; //??
  else if (!strcmp(GribLevelCStr, "MWSL"))
    return Mdvx::VERT_FIELDS_VAR_ELEV;
  else if (!strcmp(GribLevelCStr, "TRO"))
    return Mdvx::VERT_FIELDS_VAR_ELEV;
  else if (!strcmp(GribLevelCStr, "NTAT"))
    return Mdvx::VERT_FIELDS_VAR_ELEV;
  else if (!strcmp(GribLevelCStr, "SEAB"))
    return Mdvx::VERT_TYPE_SURFACE;    //??
  else if (!strcmp(GribLevelCStr, "TMPL"))
    return Mdvx::VERT_TYPE_Z;          //??
  else if (!strcmp(GribLevelCStr, "ISBL"))
    return Mdvx::VERT_TYPE_PRESSURE;
  else if (!strcmp(GribLevelCStr, "ISOB"))
    return Mdvx::VERT_TYPE_PRESSURE;
  else if (!strcmp(GribLevelCStr, "MSL"))
    return Mdvx::VERT_TYPE_SURFACE;
  else if (!strcmp(GribLevelCStr, "GPML"))
    return Mdvx::VERT_TYPE_Z;
  else if (!strcmp(GribLevelCStr, "HTGL"))
    return Mdvx::VERT_TYPE_Z;
  else if (!strcmp(GribLevelCStr, "SPH_ABV_GRD"))
    return Mdvx::VERT_TYPE_Z;
  else if (!strcmp(GribLevelCStr, "SIGL"))
    return Mdvx::VERT_TYPE_SIGMA_Z;
  else if (!strcmp(GribLevelCStr, "SIGMA"))
    return Mdvx::VERT_TYPE_SIGMA_Z;
  else if (!strcmp(GribLevelCStr, "HYBL"))
    return Mdvx::VERT_TYPE_SIGMA_Z;
  else if (!strcmp(GribLevelCStr, "DBLL"))
    return Mdvx::VERT_TYPE_Z;
  else if (!strcmp(GribLevelCStr, "THEL"))
    return Mdvx::VERT_TYPE_Z;          //??
  else if (!strcmp(GribLevelCStr, "SPDL"))
    return Mdvx::VERT_TYPE_PRESSURE;
  else if (!strcmp(GribLevelCStr, "PVL"))
    return Mdvx::VERT_TYPE_SURFACE;
  else if (!strcmp(GribLevelCStr, "EtaL"))
    return Mdvx::VERT_TYPE_Z;          //??
  else if (!strcmp(GribLevelCStr, "DBSL"))
    return Mdvx::VERT_TYPE_Z;
  else
    return Mdvx::VERT_TYPE_SURFACE;
}

//////////////////////////////////////////////////////////////
// set the master header

void Grib2Mdv::_setMasterHdr(time_t validTime,
                             DsMdvx &mdvx)
{

   Mdvx::master_header_t masterHdr;

   // Clear out master header

   memset(&masterHdr, 0, sizeof(Mdvx::master_header_t));
  
   // Fill the master header

   masterHdr.record_len1     = sizeof(Mdvx::master_header_t);
   masterHdr.struct_id       = Mdvx::MASTER_HEAD_MAGIC_COOKIE;
   masterHdr.revision_number = 1;
   masterHdr.num_data_times  = 1;
   masterHdr.index_number    = 0;
   masterHdr.data_dimension  = 3;
    
   masterHdr.data_collection_type = Mdvx::DATA_FORECAST;
   masterHdr.vlevel_included      = TRUE;
   masterHdr.grid_orientation     = Mdvx::ORIENT_SN_WE;
   masterHdr.data_ordering        = Mdvx::ORDER_XYZ;
   masterHdr.sensor_lon           = 0.0;
   masterHdr.sensor_lat           = 0.0;
   masterHdr.sensor_alt           = 0.0;

   masterHdr.time_gen = validTime;
   masterHdr.time_begin = validTime;
   masterHdr.time_end = validTime;
   masterHdr.time_centroid = validTime;
   masterHdr.time_expire = validTime;

   STRncopy(masterHdr.data_set_info, _params.data_set_info, MDV_INFO_LEN);
   STRncopy(masterHdr.data_set_name, _params.data_set_name, MDV_NAME_LEN);
   STRncopy(masterHdr.data_set_source, _params.data_set_source, MDV_NAME_LEN);

   masterHdr.record_len2    = sizeof(Mdvx::master_header_t);

   mdvx.setMasterHeader(masterHdr);
   mdvx.updateMasterHeader();

}

//////////////////////////////////////////////////////////////////////
// Creates/sets the Field Header from the current _gribRecord pointer

int Grib2Mdv::_createFieldHdr ()
{

  _reOrderNS_2_SN = false;
  _reOrderAdjacent_Rows = false;
  _reMapField = false;

  //
  // fill out the field header
  _fieldHeader.record_len1         = sizeof( Mdvx::field_header_t );
  _fieldHeader.struct_id           = Mdvx::FIELD_HEAD_MAGIC_COOKIE;
  _fieldHeader.field_code          = _gribRecord->summary->paramNumber;
  _fieldHeader.forecast_delta      = 0;
  _fieldHeader.forecast_time       = 0;
  _fieldHeader.data_element_nbytes = sizeof(fl32);
  _fieldHeader.encoding_type       = Mdvx::ENCODING_FLOAT32;    
  _fieldHeader.field_data_offset   = 0;
  _fieldHeader.compression_type    = Mdvx::COMPRESSION_NONE;
  _fieldHeader.transform_type      = Mdvx::DATA_TRANSFORM_NONE;
  _fieldHeader.scaling_type        = Mdvx::SCALING_NONE;
  _fieldHeader.native_vlevel_type  =
    _convertGribLevel2MDVLevel( _gribRecord->summary->levelType );
  _fieldHeader.vlevel_type =
    _convertGribLevel2MDVLevel( _gribRecord->summary->levelType );
  _fieldHeader.dz_constant         = 1;
  _fieldHeader.grid_dz             = 0.0;
  _fieldHeader.grid_minz           = _gribRecord->summary->levelVal;
  _fieldHeader.proj_rotation       = 0.0;
  _fieldHeader.scale               = 1.0;
  _fieldHeader.bias                = 0.0;

  // We set nz to zero then count the number of levels we find
  _fieldHeader.nz                  = 0;   

  Grib2::DataRepTemp::data_representation_t drsConstants =
    _gribRecord->drs->getDrsConstants();
  Grib2::DataRepTemp *drsTemplate =
    _gribRecord->drs->getDrsTemplate();

  // Generic missing values if not specified in Grib
  _fieldHeader.bad_data_value      = -9998.0;
  _fieldHeader.missing_data_value  = -9999.0;  
  
  if(drsConstants.templateNumber == 2) {
    
    Grib2::Template5_pt_2 *Template5_2 = (Grib2::Template5_pt_2 *)drsTemplate;
    
    if(Template5_2->_missingType != 0 && Template5_2->_missingType != 255) {
      _fieldHeader.bad_data_value      = Template5_2->_primaryMissingVal;
      _fieldHeader.missing_data_value  = Template5_2->_secondaryMissingVal;
    }

  } else if(drsConstants.templateNumber == 3) {

    Grib2::Template5_pt_3 *Template5_3 = (Grib2::Template5_pt_3 *)drsTemplate;

    if(Template5_3->_missingType != 0 && Template5_3->_missingType != 255) {
      _fieldHeader.bad_data_value      = Template5_3->_primaryMissingVal;
      _fieldHeader.missing_data_value  = Template5_3->_secondaryMissingVal;
    }

  }

  si32 projID = _gribRecord->gds->getGridID();
  Grib2::GribProj *proj = _gribRecord->gds->getProjection();
  
  if(projID == Grib2::GDS::EQUIDISTANT_CYL_PROJ_ID) {

    Grib2::LatLonProj *latlonProj = (Grib2::LatLonProj *)proj;
    
    _fieldHeader.proj_type = Mdvx::PROJ_LATLON;
    _fieldHeader.nx        = latlonProj->_ni;
    _fieldHeader.ny        = latlonProj->_nj;
    _fieldHeader.grid_minx = latlonProj->_lo1;
    _fieldHeader.grid_miny = latlonProj->_la1;
    _fieldHeader.grid_dx   = latlonProj->_di;
    _fieldHeader.grid_dy   = latlonProj->_dj;

    // If _resolutionFlag & 32 == true
    // i direction increments not given, need to calculate

    if ((latlonProj->_resolutionFlag & 32) == 0) {
      _fieldHeader.grid_dx = (latlonProj->_lo2 - latlonProj->_lo1) / (latlonProj->_ni -1);
      if(_fieldHeader.grid_dx < 0.0)
	_fieldHeader.grid_dx *= -1.0;
    }

    // If _resolutionFlag & 16 == true
    // j direction increments not given, need to calculate

    if ((latlonProj->_resolutionFlag & 16) == 0) {
      _fieldHeader.grid_dy = (latlonProj->_la2 - latlonProj->_la1) / (latlonProj->_nj -1);
      if(_fieldHeader.grid_dy < 0.0)
	_fieldHeader.grid_dy *= -1.0;
    }

    if(_fieldHeader.grid_minx > 180.0)
      _fieldHeader.grid_minx -= 360.0;

    // If _scanModeFlag & 64 == true
    // Data is order north to south
    // MDV expects south to north so switch la1 with la2 
    // and reorder the data

    if ((latlonProj->_scanModeFlag & 64) == 0) {
      _fieldHeader.grid_miny = latlonProj->_la2;
      _reOrderNS_2_SN = true;  // flag to reorder the data
    }

    // If _scanModeFlag & 16 == true
    // Adjacent rows scan in opposite direction
    // reorder to scan in the same direction

    if (latlonProj->_scanModeFlag & 16) {
      _reOrderAdjacent_Rows = true;  // flag to reorder the data
    }

    _fieldHeader.proj_origin_lat     = _fieldHeader.grid_miny;
    _fieldHeader.proj_origin_lon     = _fieldHeader.grid_minx;

  } else if(projID == Grib2::GDS::LAMBERT_CONFORMAL_PROJ_ID) {

    Grib2::LambertConfProj *lambertProj = (Grib2::LambertConfProj *)proj;
    
    _fieldHeader.proj_type = Mdvx::PROJ_LAMBERT_CONF;
    _fieldHeader.nx        = lambertProj->_nx;
    _fieldHeader.ny        = lambertProj->_ny;
    _fieldHeader.proj_origin_lat = lambertProj->_latin1;
    _fieldHeader.proj_origin_lon = lambertProj->_lov;
    _fieldHeader.grid_dx = lambertProj->_dx;
    _fieldHeader.grid_dy = lambertProj->_dy;
    _fieldHeader.proj_param[0] = lambertProj->_latin1;
    _fieldHeader.proj_param[1] = lambertProj->_latin2;
    
    if(_fieldHeader.proj_origin_lon > 180.0 && _fieldHeader.proj_origin_lon <= 360.0)
      _fieldHeader.proj_origin_lon -= 360.0;

    if ((lambertProj->_scanModeFlag & 64) == 0) {
      _reOrderNS_2_SN = true;
    }
    if (lambertProj->_scanModeFlag & 16) {
      _reOrderAdjacent_Rows = true;
    }

    // calculate min_x and min_y
    double min_x, min_y;
     
    PjgCalc *calculator;
    
    if (lambertProj->_latin1 == lambertProj->_latin2)
      calculator = new PjgLc1Calc(lambertProj->_latin1,
                                  lambertProj->_lov,
                                  lambertProj->_latin1);
    else
      calculator = new PjgLc2Calc(lambertProj->_latin1,
                                  lambertProj->_lov,
                                  lambertProj->_latin1,
                                  lambertProj->_latin2);

    calculator->latlon2xy(lambertProj->_la1, lambertProj->_lo1, min_x, min_y);
    delete(calculator);

    _fieldHeader.grid_minx = min_x;
    _fieldHeader.grid_miny = min_y;

  } else if(projID == Grib2::GDS::POLAR_STEREOGRAPHIC_PROJ_ID) {

    Grib2::PolarStereoProj *polarProj = (Grib2::PolarStereoProj *)proj;

    _fieldHeader.proj_type = Mdvx::PROJ_POLAR_STEREO;
    _fieldHeader.nx        = polarProj->_nx;
    _fieldHeader.ny        = polarProj->_ny;
    _fieldHeader.proj_origin_lat = polarProj->_la1;
    _fieldHeader.proj_origin_lon = polarProj->_lo1;

    // Our projection libraries assume that the projection plane 
    // cuts the earth at 90 degrees north. The projection plane 
    // in Grib2 can be user defined. Therefore, the grid length 
    // (cell size) must be converted to a 90 degree projection plane.

    double polarStereoAdjustment = 2.0 / (1.0 + sin(polarProj->_lad * PI / 180.0));

    _fieldHeader.grid_dx = polarProj->_dx * polarStereoAdjustment;
    _fieldHeader.grid_dy = polarProj->_dy * polarStereoAdjustment;
    _fieldHeader.proj_param[0] = polarProj->_lov;
    _fieldHeader.proj_param[1] = polarProj->_projCtrFlag;

    if ((polarProj->_scanModeFlag & 64) == 0) {
      _reOrderNS_2_SN = true;
    }
    if (polarProj->_scanModeFlag & 16) {
      _reOrderAdjacent_Rows = true;
    }

  } else if(projID == Grib2::GDS::GAUSSIAN_LAT_LON_PROJ_ID) {

    Grib2::GausLatLonProj *latlonProj = (Grib2::GausLatLonProj *)proj;
    
    _fieldHeader.proj_type = Mdvx::PROJ_LATLON;
    _fieldHeader.nx        = latlonProj->_maxNi;
    _fieldHeader.ny        = latlonProj->_nj;
    _fieldHeader.grid_minx = latlonProj->_lo1;
    _fieldHeader.grid_miny = latlonProj->_la1;
    _fieldHeader.grid_dx   = latlonProj->_di;
    _fieldHeader.grid_dy   = 180. / (latlonProj->_nParalells*2);
    _fieldHeader.proj_origin_lat     = _fieldHeader.grid_miny;
    _fieldHeader.proj_origin_lon     = _fieldHeader.grid_minx;
    if(_fieldHeader.grid_dx == -1)
      _fieldHeader.grid_dx =
        (latlonProj->_lo2 +
         (latlonProj->_lo2 / (double)(latlonProj->_maxNi-1)) -
         latlonProj->_lo1) / latlonProj->_maxNi;
    
    _reMapField = true;

    // If _scanModeFlag & 64 == true
    // Data is order north to south
    // MDV expects south to north so switch la1 with la2 
    // and reorder the data

    if ((latlonProj->_scanModeFlag & 64) == 0) {
      _fieldHeader.grid_miny = latlonProj->_la2;
      _reOrderNS_2_SN = true;  // flag to reorder the data
    }

    // If _scanModeFlag & 16 == true
    // Adjacent rows scan in opposite direction
    // reorder to scan in the same direction

    if (latlonProj->_scanModeFlag & 16) {
      _reOrderAdjacent_Rows = true;  // flag to reorder the data
    }

  } else {

    cerr << "ERROR: Unimplemented projection type " << projID << endl << flush;
    return( -1 );

  }
  
  return( 0 );

}

/////////////////////////////////////////
// Sorts a list of Grib2Records by level

void Grib2Mdv::_sortByLevel(vector<Grib2::Grib2Record::Grib2Sections_t>::iterator begin, 
			    vector<Grib2::Grib2Record::Grib2Sections_t>::iterator end)
{

  bool reverse = false;
  const char *GribLevelCStr = (begin)->summary->levelType.c_str();
  // Pressure levels go in descending order
  if(!strcmp(GribLevelCStr, "SPDL") || !strcmp(GribLevelCStr, "ISBL"))
    reverse = true;
  vector<Grib2::Grib2Record::Grib2Sections_t>::iterator IPos;
  vector<Grib2::Grib2Record::Grib2Sections_t>::iterator JPos;
  Grib2::Grib2Record::Grib2Sections_t temp;
  for (IPos = end-1; IPos != begin-1; IPos--) {
    for (JPos = begin+1; JPos != IPos+1; JPos++) {
      if ((!reverse && (JPos-1)->summary->levelVal > JPos->summary->levelVal) ||
	  (reverse && (JPos-1)->summary->levelVal < JPos->summary->levelVal)) {
        temp = *(JPos-1);
        *(JPos-1) = *(JPos);
        *(JPos) = temp;
      }
    }
  }

}

////////////////////////////////////////////////////////
// reorders data from North to South to South to North
// East/West increments remain the same
// For GFS data this means  that the data starts in the 
// lower left hand corner instead of the upper left hand 
// corner. 
void Grib2Mdv::_reOrderNS2SN (fl32 *data, int numX, int numY)
{

  int j = 0;
  MemBuf *gribData = new MemBuf();
  
  fl32 *bufPtr = (fl32 *) gribData->load 
    ((void *) data, numX * numY * sizeof (fl32));

  for (int y = numY - 1; y >= 0; y--) {
    for (int x = 0;  x < numX; x++) {
      data[j] = bufPtr[(y * numX) + x];
      j++;
    }
  }

  delete gribData;
  
}

//////////////////////////////////////////////////////////
// Reorders rows that scan in the opposite direction
// to rows that scan in the same direction.
// Grid size remains the same.
void Grib2Mdv::_reOrderAdjacentRows (fl32 *data, int numX, int numY)
{

  MemBuf *gribData = new MemBuf();
  
  fl32 *bufPtr = (fl32 *) gribData->load 
    ((void *) data, numX * numY * sizeof (fl32));
  int halfX = (numX / 2)+1;
  for (int y = 1; y < numY; y += 2) {
    for (int x = 0;  x < halfX; x++) {
      data[(y * numX) + x + ((halfX - x - 1)*2)] = bufPtr[(y * numX) + x];
    }
    for (int x = halfX+1;  x < numX; x++) {
      data[(y * numX) + x - ((x - halfX + 1)*2)] = bufPtr[(y * numX) + x];
    }
  }

  delete gribData;

}

///////////////////////////////////////////////////////////////////////////////
// Selects the Mdv field name. Uses the abbreviated name from the Grib2 product
// table along with the abbreviated level name to form a unique name for this
// field. If there is an mdv field name specfied in the parameter file it
//  is used instead.

void Grib2Mdv::_setFieldName()
{

  string defaultName = _gribRecord->summary->name;
  if(_gribRecord->summary->levelType ==
     "unknown primary surface type") {
    defaultName += "_";
    defaultName += _gribRecord->summary->levelType;
  }
  
  if (_params.rename_fields) {
    for (int ii = 0; ii < _params.field_rename_n; ii++) {
      string gribName = _params._field_rename[ii].grib_name;
      string outputName = _params._field_rename[ii].output_name;
      if (defaultName == gribName) {
        defaultName = outputName;
      }
    }
  }
  
  STRncopy(_fieldHeader.field_name, defaultName.c_str(), MDV_SHORT_FIELD_LEN);

  STRncopy(_fieldHeader.field_name_long,
           _gribRecord->summary->longName.c_str(), MDV_LONG_FIELD_LEN);

}

////////////////////////////////////////////////////////////
// Performs simple unit conversions on the vertical level

void Grib2Mdv::_convertVerticalUnits()
{

  // Grib2 vertical levels come in m, convert to km for Mdv

  if(_vlevelHeader.type[0] == Mdvx::VERT_TYPE_Z) {
    _fieldHeader.grid_minz *= M_TO_KM;;
    _fieldHeader.grid_dz *= M_TO_KM;;
    for(int a = 0; a < _fieldHeader.nz; a++) {
      _vlevelHeader.level[a] *= M_TO_KM;;
    }
  }
  
}

float inline getInd(double out, double in1, double in2)
{
  float ind, diff;
  if(in1 < in2) {
    double tmp = in1;
    in1 = in2;
    in2 = tmp;
  }
  diff = (in1 - out) + (out - in2);
  ind = (in1 - out) / diff;
  return ind;
}


fl32 *Grib2Mdv::_reMapReducedOrGaussian(fl32 *data, Mdvx::field_header_t fhdr)
{

  fl32 *lats;
  int nx, ny;
  float dx, dy, minx, miny;
  fl32 **lons;
  int *nlons;
  int *tlons;

  si32 projID = _gribRecord->gds->getGridID();

  if(projID == Grib2::GDS::GAUSSIAN_LAT_LON_PROJ_ID) {
    
    Grib2::GausLatLonProj *latlonProj = (Grib2::GausLatLonProj *) _gribRecord->gds->getProjection();
    latlonProj->getGaussianLats(&lats);
    if(lats == NULL) {
      cerr << "ERROR: Calculation of Gaussian Latitude failure." << endl << flush;
      return NULL;
    }
    nx = latlonProj->_maxNi;
    ny = latlonProj->_nj;
    dx = latlonProj->_di;
    if(dx == -1)
      dx =  (latlonProj->_lo2 + (latlonProj->_lo2 / (double)(latlonProj->_maxNi-1)) - latlonProj->_lo1) / latlonProj->_maxNi;
    dy = 180. / (latlonProj->_nParalells*2);
    minx = latlonProj->_lo1;
    miny = latlonProj->_la1;
    if(latlonProj->_lo2 < latlonProj->_lo1)
      dx *= -1.0;
    if(latlonProj->_la2 < latlonProj->_la1)
      dy *= -1.0;

    lons = new fl32*[ny];
    nlons = new int[ny];
    tlons = new int[ny];
    for(int y = 0; y < ny; y++) {
      nlons[y] = latlonProj->getQuasiLons(&(lons[y]), y);
      if(y > 0) 
	tlons[y] = nlons[y-1] + tlons[y-1];
      else
	tlons[0] = 0;
    }

  }  else {
    cerr << "ERROR: Unsupported Reduced grid type, cannot remap to regular lat/lon grid." << endl << flush;
    return NULL;
  }

  MdvxProj outproj;
  outproj.initLatlon();
  outproj.setGrid(nx, ny, dx, dy, minx, miny);

  float *odata = new float[nx*ny];

  double lat, lon, x;
  int jlat = 0, ilon1, ilon2;
  double p1, p2, p3, p4;

  for(int j = 0; j < fhdr.ny; j++) 
  {
    outproj.xyIndex2latlon(0, j, lat, lon);
    if(jlat < fhdr.ny-1)
      x = getInd(lat, lats[jlat], lats[jlat+1]);
    else
      x = 0.0;

    for(int i = 0; i < fhdr.nx; i++)
    {
      outproj.xyIndex2latlon(i, j, lat, lon);
      double y1, y2;

      odata[(j*nx)+i] = 0.0;

      ilon1 = 0;
      for(int ii = 0; ii < nlons[jlat]; ii++)
	if(lons[jlat][ii] <= lon)
	  ilon1++;
      ilon1--;

      p1 = data[ilon1+tlons[jlat]];

      if(ilon1 == nlons[jlat]-1) 
      {
	p2 = 0.0;
	y1 = 1.0;
      } else {
	p2 = data[ilon1+1+tlons[jlat]];
	y1 = getInd(lon, lons[jlat][ilon1], lons[jlat][ilon1+1]);
      } 
      
      y2 = 0.0;
      if(jlat == fhdr.ny-1) 
      {
	p3 = 0.0;
	p4 = 0.0;
      } else {
	ilon2 = 0;
	for(int ii = 0; ii < nlons[jlat+1]; ii++)
	  if(lons[jlat+1][ii] <= lon)
	    ilon2++;
	ilon2--;
	
	p3 = data[ilon2+tlons[jlat+1]];
	if(ilon2 == nlons[jlat+1]-1) 
	{
	  p4 = 0.0;
	  y2 = 1.0;
	} else {
	  p4 = data[ilon2+1+tlons[jlat+1]];
	  y2 = getInd(lon, lons[jlat+1][ilon2], lons[jlat+1][ilon2+1]);
	}
	
	
      }


      if(p1 == fhdr.missing_data_value || p1 == fhdr.bad_data_value ||
	 p2 == fhdr.missing_data_value || p2 == fhdr.bad_data_value ||
	 p3 == fhdr.missing_data_value || p3 == fhdr.bad_data_value ||
	 p4 == fhdr.missing_data_value || p4 == fhdr.bad_data_value)
	odata[(j*nx)+i] = fhdr.missing_data_value;
      else
	odata[(j*nx)+i] = (p1 * y1 + p2 * (1-y1)) * (1-x) + (p3 * y2 + p4 * (1-y2)) * x;      

    }
    jlat++;
  }

  delete [] lats;
  for(int y = 0; y < ny; y++)
    delete [] lons[y];
  delete[] lons;
  delete[] nlons;
  delete[] tlons;

  return odata;
}
