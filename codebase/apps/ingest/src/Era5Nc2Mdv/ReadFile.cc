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
#include <limits>

#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <dsdata/TriggerInfo.hh>
#include <euclid/Pjg.hh>
#include <euclid/PjgLc1Calc.hh>
#include <euclid/PjgLc2Calc.hh>
#include <euclid/PjgPolarStereoCalc.hh>
#include <Mdv/Mdvx.hh>
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

#include "ReadFile.hh"
#include "Era5Nc2Mdv.hh"
using namespace std;

//
// Constants
//
const int    ReadFile::MAX_NPTS = 6000000;
const double ReadFile::M_TO_KM = 0.001;
const double ReadFile::M_TO_FT = 3.2808399;
const double ReadFile::M_TO_KFT = .00328; 
const double ReadFile::M_TO_MI = 0.000621371192;
const double ReadFile::M_TO_100FT = .0328;
const double ReadFile::MPS_TO_KNOTS = 1.94;
const double ReadFile::PASCALS_TO_MBARS = 0.01;
const double ReadFile::KELVIN_TO_CELCIUS = -273.15;
const double ReadFile::KG_TO_G = 1000.0;
const double ReadFile::PERCENT_TO_FRAC = 0.01;
const double ReadFile::PASCAL_TO_HECTOPASCAL = 0.01;
const double ReadFile::MM_S_TO_MM_HR = 3600.0;


ReadFile::ReadFile (Params &params)
{
  _paramsPtr = &params;
  _dataTrigger = NULL;
  _outputFile = NULL;
  _Grib2File = NULL;
  _GribRecord = NULL;

  memset( (void *) &_fieldHeader, (int) 0, sizeof(Mdvx::field_header_t) );
  memset( (void *) &_vlevelHeader, (int) 0, sizeof(Mdvx::vlevel_header_t) );
}

ReadFile::~ReadFile()
{
  _gribFields.erase(_gribFields.begin(), _gribFields.end() );
  delete _dataTrigger;
  if (_outputFile != (OutputFile *)NULL)
    delete _outputFile;
  if (_Grib2File != (Grib2::Grib2File *)NULL)
    delete _Grib2File;
}

int ReadFile::init(int nFiles, char** fileList, bool printVarList, 
		   bool printsummary , bool printsections)
{

  if (nFiles > 0) {

    // filelist mode
    
    vector< string > file_list;
    for (int i = 0; i < nFiles; ++i)
      file_list.push_back(fileList[i]);
    
    DsFileListTrigger *data_trigger = new DsFileListTrigger();
    if (_paramsPtr->debug >= 2) {
      data_trigger->setVerbose(true);
    } else if (_paramsPtr->debug) {
      data_trigger->setDebug(true);
    }
    if (data_trigger->init(file_list) != 0) {
      cerr << "ERROR: Error initializing file list trigger" << endl;
      return RI_FAILURE;
    }
    _dataTrigger = data_trigger;
    _inputSuffix = "";
    
   } else {

    if (_paramsPtr->latest_data_info_avail) {

      // realtime with latest data info available

      DsLdataTrigger *data_trigger = new DsLdataTrigger();
      if (_paramsPtr->debug >= 2) {
        data_trigger->setVerbose(true);
      } else if (_paramsPtr->debug) {
        data_trigger->setDebug(true);
      }
      if (data_trigger->init(_paramsPtr->input_dir,
                             _paramsPtr->max_input_data_age,
                             PMU_auto_register) != 0) {
        cerr << "ERROR: Error initializing realtime trigger" << endl;
        return RI_FAILURE;
      }
      _dataTrigger = data_trigger;
      _inputSuffix = _paramsPtr->input_suffix;
      if (strlen(_paramsPtr->input_substring) > 0) {
        _inputSubstrings.push_back(_paramsPtr->input_substring);
      } else {
        for (int i = 0; i < _paramsPtr->input_substrings_n; ++i) {
          if (strlen(_paramsPtr->_input_substrings[i]) > 0) {
            _inputSubstrings.push_back(_paramsPtr->_input_substrings[i]);
          }
        }
      }
      
    } else {

      // realtime with no latest data info

      DsInputDirTrigger *data_trigger = new DsInputDirTrigger();
      if (_paramsPtr->debug >= 2) {
        data_trigger->setVerbose(true);
      } else if (_paramsPtr->debug) {
        data_trigger->setDebug(true);
      }
      if (data_trigger->init(_paramsPtr->input_dir,
                             _paramsPtr->input_substring,
                             !_paramsPtr->latest_file_only,
                             PMU_auto_register,
                             _paramsPtr->recursive_search,
                             "",
                             _paramsPtr->data_check_interval_secs) != 0) {
        cerr << "ERROR: Error initializing realtime trigger" << endl;
        return RI_FAILURE;
      }
      _dataTrigger = data_trigger;
      _inputSuffix = _paramsPtr->input_suffix;
      if (strlen(_paramsPtr->input_substring) > 0) {
        _inputSubstrings.push_back(_paramsPtr->input_substring);
      } else {
        for (int i = 0; i < _paramsPtr->input_substrings_n; ++i) {
          if (strlen(_paramsPtr->_input_substrings[i]) > 0) {
            _inputSubstrings.push_back(_paramsPtr->_input_substrings[i]);
          }
        }
      }
      
    } // if (_paramsPtr->latest_data_info_avail) {
   
   } // if( nFiles > 0 ) {

   _Grib2File   = new Grib2::Grib2File ();
   _printVarList = printVarList;
   _printSummary = printsummary;
   _printSections = printsections;

   if( _paramsPtr->write_forecast || _paramsPtr->write_non_forecast ) {
     if( _mdvInit() != RI_SUCCESS ) {
       return( RI_FAILURE );
     }
   }

   //
   // Create a list of variables to process from the params request
   if (!_paramsPtr->process_everything) {
     _gribFields.insert( _gribFields.begin(), _paramsPtr->_output_fields,
			 (_paramsPtr->_output_fields)+(_paramsPtr->output_fields_n));
   }
   return( RI_SUCCESS );
   
}

int ReadFile::getData()
{
   string filePath;
   Grib2::Grib2Record::print_sections_t printSec;

   //
   // Create a grib2 Print sections object from the params request
   if(_paramsPtr->debug > 1 || _printSections ) {
     printSec.is = _paramsPtr->printSec_is;
     printSec.ids = _paramsPtr->printSec_ids;
     printSec.lus = _paramsPtr->printSec_lus;
     printSec.gds = _paramsPtr->printSec_gds;
     printSec.pds = _paramsPtr->printSec_pds;
     printSec.drs = _paramsPtr->printSec_drs;
     printSec.bms = _paramsPtr->printSec_bms;
     printSec.ds = _paramsPtr->printSec_ds;
   }

   //
   // Process each file
   TriggerInfo trigger_info;
   while (_dataTrigger->next(trigger_info) == 0) {

      filePath = trigger_info.getFilePath();
      Path file_path_obj(filePath);
      
      PMU_auto_register(filePath.c_str());

      // Check for appropriate substring and extension

      bool substring_found = false;
      for (vector< string >::const_iterator substr = _inputSubstrings.begin();
           substr != _inputSubstrings.end(); ++substr)
      {
        if (file_path_obj.getFile().find(*substr, 0) != string::npos)
        {
          substring_found = true;
          break;
        }
      }
      if (_inputSubstrings.size() > 0 && !substring_found)
        continue;
      
      if (!_inputSuffix.empty() &&
	  file_path_obj.getExt() != _inputSuffix)
	continue;
      
      //
      // Inventory the file
      PMU_auto_register( "Reading grib2 file" );
      cerr << "Reading file " << filePath << endl;
      if(_Grib2File->read(filePath) != Grib2::GRIB_SUCCESS)
	continue;

      //
      // Print the full contents of the grib file
      if(_paramsPtr->debug > 1 || _printSections ) 
	_Grib2File->printContents(stdout, printSec);

      if(_printSections ) 
	continue;

      //
      // Print only a summary of the grib file
      if (_paramsPtr->debug || _printSummary)
	_Grib2File->printSummary(stdout, _paramsPtr->debug);

      if(_printSummary)
	continue;

      //
      // Get the full list of fields just read in the inventory process
      if (_paramsPtr->process_everything || _printVarList) {
        _gribFields.erase(_gribFields.begin(), _gribFields.end());
	list <string> fieldList = _Grib2File->getFieldList();
	list <string>::const_iterator field;

	for (field = fieldList.begin(); field != fieldList.end(); ++field) {
          PMU_auto_register(filePath.c_str());
	  list <string> fieldLevelList = _Grib2File->getFieldLevels(*(field));
	  list <string>::const_iterator level;

	  for (level = fieldLevelList.begin(); level != fieldLevelList.end(); ++level) {
	    Params::out_field_t out_field;
	    out_field.param = new char[50];
	    out_field.level = new char[50];
	    memcpy(out_field.param, (*(field)).c_str(), strlen((*(field)).c_str())+1);
	    memcpy(out_field.level, (*(level)).c_str(), strlen((*(level)).c_str())+1);
            out_field.vert_level_min = -1;
            out_field.vert_level_max = -1;
	    out_field.vert_level_dz = 1;
	    out_field.use_additional_bad_data_value = pFALSE;
	    out_field.use_additional_missing_data_value = pFALSE;
	    out_field.additional_bad_data_value = -9999.99;
	    out_field.additional_missing_data_value = -999.99;
	    _gribFields.push_back(out_field);

	    if(_printVarList) {
	      vector<Grib2::Grib2Record::Grib2Sections_t> GribRecords = _Grib2File->getRecords(out_field.param, out_field.level);
	      char name[20];
	      sprintf(name, "%s %s", out_field.param, out_field.level);
	      printf("%-20s \t'%s' '%s'\n", name,
		     GribRecords[0].summary->longName.c_str(), GribRecords[0].summary->levelTypeLong.c_str());
	    }
	  }
	}
	if(_printVarList)
	  continue;
      }

      // Get the list of forecast times, usually there is just one
      // but not always.
      list <long int> forecastList = _Grib2File->getForecastList();
      list <long int>::const_iterator leadTime;

      // Loop over the lead times, each time will become a mdv file
      for (leadTime = forecastList.begin(); leadTime != forecastList.end(); ++leadTime) {
        
        PMU_auto_register(filePath.c_str());

        // check the lead time if required

        if (_paramsPtr->check_lead_time &&
            *leadTime > _paramsPtr->max_lead_time_secs) {
          continue;
        }


        if (_paramsPtr->lead_time_subsampling) {
	  bool proccess_lead = false;
	  for(int i = 0; i < _paramsPtr->subsample_lead_times_n; i++) {
	    if (*leadTime == _paramsPtr->_subsample_lead_times[i]) {
	      proccess_lead = true;
	    }
	  }

	  if (proccess_lead == false)   
	    continue;
        }

	if (_paramsPtr->debug)
	  cerr << "Getting fields for forecast time of "
               << *leadTime << " seconds." << endl;

	// Loop over the list of fields to process
	// Keep track of the generate and forecast times

	time_t lastGenerateTime = 0;
	PMU_auto_register( "Reading grib2 file" );
	_field = _gribFields.begin();
	while (_field != _gribFields.end()) {

          PMU_auto_register(filePath.c_str());

	  if (_paramsPtr->debug)
	    cerr << "Looking for field " <<  _field->param
                 << " level  " << _field->level << endl;
	  
	  vector<Grib2::Grib2Record::Grib2Sections_t>
            GribRecords = _Grib2File->getRecords(_field->param, _field->level, *leadTime);
	  
	  if(GribRecords.size() > 1)
	    _sortByLevel(GribRecords.begin(), GribRecords.end());
	  
	  //MemBuf fieldData;
	  fl32 *fieldDataPtr = NULL;
	  fl32 *currDataPtr = NULL;

	  if (_paramsPtr->debug)
	    cerr << "Found " << GribRecords.size() << " records." << endl;
	  if(GribRecords.size() >= MDV_MAX_VLEVELS) {
	    cerr << "ERROR: Too many fields for one record! "
                 << GribRecords.size() << " > Max Mdv Vlevels" << endl;
	    return( RI_FAILURE );
	  }
          
	  //
	  // Set requested vertical level bounds
	  int levelMin = 0;
	  if(_field->vert_level_min >= 0)
	    levelMin = _field->vert_level_min;

	  int levelMax;
	  if( _field->vert_level_max < 0 ) {
	    levelMax = GribRecords.size() - 1;
          } else if( _field->vert_level_max  < (int) GribRecords.size() - 1) {
	    levelMax = _field->vert_level_max;
	  } else {
	    levelMax = GribRecords.size() - 1;
          }

	  size_t levelDz = 1;
	  if( _field->vert_level_dz > 1)
	    levelDz =  _field->vert_level_dz;

	  //
	  // Loop over requested vertical levels in each field
	  for(int levelNum = levelMin; levelNum <= levelMax; levelNum+=levelDz) {
	    
	    _GribRecord = &(GribRecords[levelNum]);

	    if (_paramsPtr->debug) {
	      cerr <<  _GribRecord->summary->name.c_str() << " ";
	      cerr <<  _GribRecord->summary->longName.c_str() << " ";
	      cerr <<  _GribRecord->summary->units.c_str() << " ";
	      cerr <<  _GribRecord->summary->category << " ";
	      cerr <<  _GribRecord->summary->paramNumber << " ";
	      cerr <<  _GribRecord->summary->levelType.c_str() << " ";
	      cerr <<  _GribRecord->summary->levelVal;
	      cerr <<  endl;
	    }

	    //
	    // Create Mdvx field header for the first level
	    if (levelNum == levelMin) {

	      memset( (void *) &_fieldHeader, (int) 0, sizeof(Mdvx::field_header_t) );
	      memset( (void *) &_vlevelHeader, (int) 0, sizeof(Mdvx::vlevel_header_t) );
	      _vlevelHeader.struct_id = Mdvx::VLEVEL_HEAD_MAGIC_COOKIE_64;
	      
	      if ( _createFieldHdr() != RI_SUCCESS ) {
		cerr << "WARNING: File " << filePath
                     << " not processed." << endl;
		_outputFile->clear();
		_Grib2File->clearInventory();
		return( RI_FAILURE );
	      }

	      // Pre Count the number of levels
	      _fieldHeader.nz = 0;
	      for(int ln = levelNum; ln <= levelMax; ln+=levelDz)
		_fieldHeader.nz ++;

	      if (_paramsPtr->debug) {
		cerr << "Processing  " << _fieldHeader.nz << " records." << endl;
	      }
	  
	      fieldDataPtr = new fl32[(size_t)_fieldHeader.nz*(size_t)_fieldHeader.nx*(size_t)_fieldHeader.ny];
	      currDataPtr = fieldDataPtr;
	    }

	    //
	    // Generation time changed. This shouldn't happen and we can't handle it correctly.
	    if (currDataPtr != fieldDataPtr && _GribRecord->ids->getGenerateTime() != lastGenerateTime) {
	      
	      cerr << "ERROR: File containes multiple gen times." << endl;
	      cerr << "       currently unsupported." << endl;
	      _outputFile->clear();
	      _Grib2File->clearInventory();
	      return( RI_FAILURE );
	    }

	    //
	    // Get and save the data, reordering and remaping if needed.
	    fl32 *data = _GribRecord->ds->getData();
	    if(data == NULL) {
	      cerr << "ERROR: Failed to get field "
                   <<  _field->param << " level " << _field->level << endl;
	      return( RI_FAILURE );
	    }

            if (_field->use_additional_bad_data_value || _field->use_additional_missing_data_value)
              _replaceAdditionalBadMissing(data, _fieldHeader,
                                           _field->use_additional_bad_data_value,
                                           _field->additional_bad_data_value,
                                           _field->use_additional_missing_data_value,
                                           _field->additional_missing_data_value);
	    if(_reMapField)
	      data = _reMapReducedOrGaussian(data, _fieldHeader);
	    if(_reOrderNS_2_SN)
	      _reOrderNS2SN(data, _fieldHeader.nx, _fieldHeader.ny);
	    if(_reOrderAdjacent_Rows)
	      _reOrderAdjacentRows(data, _fieldHeader.nx, _fieldHeader.ny);
	    
	    //fieldDataPtr = fieldData.add(data, sizeof(fl32)*_fieldHeader.nx*_fieldHeader.ny );
	    memcpy ( currDataPtr, data, sizeof(fl32)*_fieldHeader.nx*_fieldHeader.ny );
	    currDataPtr += _fieldHeader.nx*_fieldHeader.ny;
	    if(_reMapField)
	      delete[] data;
	    _GribRecord->ds->freeData();

	    //
	    // fill out the vlevel header
	    _vlevelHeader.type[(levelNum - levelMin)/levelDz] = _convertGribLevel2MDVLevel( _GribRecord->summary->levelType );
	    _vlevelHeader.level[(levelNum - levelMin)/levelDz] = _GribRecord->summary->levelVal;
	    //_fieldHeader.nz ++;

	    //
	    // Once we have gotten two vertical levels we can calculate a dz
	    if((levelNum - levelMin)/levelDz == 2)
	    {
	      if(_vlevelHeader.level[1] == _vlevelHeader.level[0] )
		_fieldHeader.grid_dz = 0.0;
	      else
		_fieldHeader.grid_dz = ( _vlevelHeader.level[1] - _vlevelHeader.level[0] );
	    }
	    lastGenerateTime = _GribRecord->ids->getGenerateTime();
	  }
	  
	  if(GribRecords.size() == 0) {
	    cerr << "WARNING: Field " <<  _field->param
                 << " level  " << _field->level
                 << " not found in grib file." << endl;
	  } else {
            
	    Mdvx::encoding_type_t encoding = Mdvx::ENCODING_FLOAT32;    
	    if(_paramsPtr->process_everything) {
	      _setFieldNames(-1);
	      _convertUnits(-1,fieldDataPtr);
	      _convertVerticalUnits(-1);
	      encoding = OutputFile::mdvEncoding(_paramsPtr->encoding_type);
	    } else {
	      for (int i = 0; i < _paramsPtr->output_fields_n; i++) {
		if (STRequal_exact(_paramsPtr->_output_fields[i].param,
                                   _GribRecord->summary->name.c_str()) &&
		    STRequal_exact(_paramsPtr->_output_fields[i].level,
                                   _GribRecord->summary->levelType.c_str()) ) {
		  _setFieldNames(i);
		  _limitDataRange(i,fieldDataPtr);
		  _convertUnits(i,fieldDataPtr);
		  _convertVerticalUnits(i);
		  encoding = OutputFile::mdvEncoding(_paramsPtr->_output_fields[i].encoding_type);
		}
	      }
	    }

	    _fieldHeader.volume_size = (si32)_fieldHeader.nx * (si32)_fieldHeader.ny * 
	      (si32)_fieldHeader.nz * (si32)_fieldHeader.data_element_nbytes;

	    if(_fieldHeader.volume_size < 0) {
	      cerr << "ERROR: Field " <<  _field->param << " with " 
                   << _fieldHeader.nz << " levels is larger than Mdv can handle." << endl;
	      cerr << "Use encoding_type of INT8 or INT16 to reduce output size to under 2.156 GB" << endl;
	      delete [] fieldDataPtr;
	      return( RI_FAILURE );
	    }

	    MdvxField *fieldPtr = new MdvxField(_fieldHeader, _vlevelHeader, fieldDataPtr );
	    delete [] fieldDataPtr;
	    _outputFile->addField(fieldPtr, encoding);

	  }

	  _field++;

	}  // Close loop over field list

	if(forecastList.size() == 1)
	  _Grib2File->clearInventory();

	if (_outputFile->numFields() == 0) {
	  cerr << "WARNING: No fields found in grib2 file." << endl;
	  cerr << "         No output MDV file created." << endl;
	} else {
	  PMU_auto_register( "Writing mdv file" );
          if( _writeMdvFile(lastGenerateTime, *leadTime) != RI_SUCCESS ) {
            cerr << "ERROR: Could not write MDV file." << endl;
            _outputFile->clear();
            _Grib2File->clearInventory();
            return( RI_FAILURE );
          }
	  _outputFile->clear();
	}
	
      }  // Close loop on forecast leadTime
      
      _Grib2File->clearInventory();

   } // Close loop over input grib2 files
   
   return(RI_SUCCESS);
}

int ReadFile::_mdvInit() 
{

  _outputFile = new OutputFile( _paramsPtr );

  return( RI_SUCCESS );
}

//
// Takes the list of _outputFields and writes them out to Mdv
int ReadFile::_writeMdvFile(time_t generateTime, long int forecastTime)
{

  if( (generateTime < 0) || (forecastTime < 0)) {
    cerr << " WARNING: File times don't make sense" << endl;
    cerr << "    Generate time = " << generateTime << endl;
    cerr << "    Forecast time = " << forecastTime << endl;
    return( RI_SUCCESS );
  }

  DateTime genTime( generateTime );
  cerr << "Writing grid output file at "
       << genTime.dtime() << " for a forecast time of "
       << (forecastTime/3600) << " hours" << endl << flush;

  if ( _outputFile->writeVol( generateTime, forecastTime ) != 0 )
    return( RI_FAILURE );

  return( RI_SUCCESS );
}

//
// Returns the Mdv level appropriate to a Grib level
int ReadFile::_convertGribLevel2MDVLevel(const string &GribLevel)
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

//
// Creates/sets the Field Header from the current _GribRecord pointer
int ReadFile::_createFieldHdr ()
{

  _reOrderNS_2_SN = false;
  _reOrderAdjacent_Rows = false;
  _reMapField = false;

  //
  // fill out the field header
  _fieldHeader.record_len1         = sizeof( Mdvx::field_header_t );
  _fieldHeader.struct_id           = Mdvx::FIELD_HEAD_MAGIC_COOKIE_64;
  _fieldHeader.field_code          = _GribRecord->summary->paramNumber;
  _fieldHeader.forecast_delta      = _GribRecord->pds->getForecastTime();
  _fieldHeader.forecast_time       = _GribRecord->ids->getGenerateTime() + _GribRecord->pds->getForecastTime();
  _fieldHeader.data_element_nbytes = sizeof(fl32);
  _fieldHeader.encoding_type       = Mdvx::ENCODING_FLOAT32;    
  _fieldHeader.field_data_offset   = 0;
  _fieldHeader.compression_type    = Mdvx::COMPRESSION_NONE;
  _fieldHeader.transform_type      = Mdvx::DATA_TRANSFORM_NONE;
  _fieldHeader.scaling_type        = Mdvx::SCALING_NONE;
  _fieldHeader.native_vlevel_type  = _convertGribLevel2MDVLevel( _GribRecord->summary->levelType );
  _fieldHeader.vlevel_type         = _convertGribLevel2MDVLevel( _GribRecord->summary->levelType );
  _fieldHeader.dz_constant         = 1;
  _fieldHeader.grid_dz             = 0.0;   // Will be calculated after we get a second level
  _fieldHeader.grid_minz           = _GribRecord->summary->levelVal;
  _fieldHeader.proj_rotation       = 0.0;
  _fieldHeader.scale               = 1.0;
  _fieldHeader.bias                = 0.0;

  // We set nz to zero then count the number of levels we find
  _fieldHeader.nz                  = 0;   

  Grib2::DataRepTemp::data_representation_t drsConstants = _GribRecord->drs->getDrsConstants();
  Grib2::DataRepTemp *drsTemplate = _GribRecord->drs->getDrsTemplate();

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

  float earth_major_axis, earth_minor_axis;
  float earth_radius = _GribRecord->gds->getEarthRadius(earth_major_axis, earth_minor_axis) / 1000.0;
  if(earth_radius != 0.0)
    Pjg::setEarthRadiusKm(earth_radius);

  si32 projID = _GribRecord->gds->getGridID();
  Grib2::GribProj *proj = _GribRecord->gds->getProjection();
  
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

     // 
     // calculate min_x and min_y
     double min_x, min_y;
     
     PjgCalc *calculator;
     
     if (lambertProj->_latin1 == lambertProj->_latin2)
       calculator = new PjgLc1Calc(lambertProj->_latin1, lambertProj->_lov, lambertProj->_latin1);
     else
       calculator = new PjgLc2Calc(lambertProj->_latin1, lambertProj->_lov,
				   lambertProj->_latin1, lambertProj->_latin2);

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
    //_fieldHeader.proj_param[2] = 1.0;  //Set Central scale?
    
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
      _fieldHeader.grid_dx = (latlonProj->_lo2 + (latlonProj->_lo2 / (double)(latlonProj->_maxNi-1)) - latlonProj->_lo1) / latlonProj->_maxNi;

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
    cerr << "ERROR: Unimplemented projection type " << projID << endl;
    return( RI_FAILURE );
  }
  
  return( RI_SUCCESS );
}

//
// Sorts a list of Grib2Records by level
void ReadFile::_sortByLevel(vector<Grib2::Grib2Record::Grib2Sections_t>::iterator begin, 
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
  for (IPos = end-1; IPos != begin-1; IPos--)
  {
    for (JPos = begin+1; JPos != IPos+1; JPos++)
    {
      if ((!reverse && (JPos-1)->summary->levelVal > JPos->summary->levelVal) ||
	  (reverse && (JPos-1)->summary->levelVal < JPos->summary->levelVal))
      {
        temp = *(JPos-1);
        *(JPos-1) = *(JPos);
        *(JPos) = temp;
      }
    }
  }

}

// reorders data from North to South to South to North
// East/West increments remain the same
// For GFS data this means  that the data starts in the 
// lower left hand corner instead of the upper left hand 
// corner. 
void ReadFile::_reOrderNS2SN (fl32 *data, int numX, int numY)
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

//
// Reorders rows that scan in the opposite direction
// to rows that scan in the same direction.
// Grid size remains the same.
void ReadFile::_reOrderAdjacentRows (fl32 *data, int numX, int numY)
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

//
// Selects the Mdv field name. Uses the abbreviated name from the Grib2 product table along
// with the abbreviated level name to form a unique name for this field.
// If there is an mdv field name specfied in the parameter file it is used instead.
void ReadFile::_setFieldNames(int paramsIndex)
{
  char defaultName[100];
  STRncopy(defaultName, _GribRecord->summary->name.c_str(), 100);
  if(strcmp(_GribRecord->summary->levelType.c_str(), "unknown primary surface type") != 0) {
    strcat(defaultName, "_");
    strcat(defaultName, _GribRecord->summary->levelType.c_str());
  }
  if(_paramsPtr->process_everything || paramsIndex < 0 || paramsIndex >= _paramsPtr->output_fields_n) {
    STRncopy(_fieldHeader.field_name, defaultName, MDV_SHORT_FIELD_LEN);
  } else {

    if (string(_paramsPtr->_output_fields[paramsIndex].mdv_name).size() > 0) {
      STRncopy(_fieldHeader.field_name, _paramsPtr->_output_fields[paramsIndex].mdv_name, MDV_SHORT_FIELD_LEN);
    }
    else {
      STRncopy(_fieldHeader.field_name, defaultName, MDV_SHORT_FIELD_LEN);
    }
  }
  
  STRncopy(_fieldHeader.field_name_long, _GribRecord->summary->longName.c_str(), MDV_LONG_FIELD_LEN);
}

//
// Applies limits on range of values in _data, specified by the upper_range_limit 
// and lower_range_limit values from the parameter file. 
void ReadFile::_limitDataRange(int paramsIndex, fl32 *dataPtr)
{
  if(paramsIndex < 0 || paramsIndex >= _paramsPtr->output_fields_n)
    return;

  fl32 upperLimit = _paramsPtr->_output_fields[paramsIndex].upper_range_limit;
  fl32 lowerLimit = _paramsPtr->_output_fields[paramsIndex].lower_range_limit;

  if(lowerLimit > upperLimit) {
    fl32 tmp = lowerLimit;
    lowerLimit = upperLimit;
    upperLimit = tmp;
  }

  fl32 replacementValue = _fieldHeader.bad_data_value;
  Params::qc_default_t qcDefaultType = 
    _paramsPtr->_output_fields[paramsIndex].qc_default_type;

  if (qcDefaultType == Params::UNKNOWN_VALUE)
    replacementValue = _fieldHeader.missing_data_value;
  else if (qcDefaultType == Params::USER_DEFINED)
    replacementValue = _paramsPtr->_output_fields[paramsIndex].qc_default_value;

  //
  // no limits on this data
  if ((upperLimit == 0.0) && (lowerLimit == 0.0)) {
    return;
  }
  
  fl32* flPtr = (fl32*) dataPtr;
  size_t numPts = _fieldHeader.nx*_fieldHeader.ny*_fieldHeader.nz;
  
  for (size_t j = 0; j < numPts; j++,  flPtr++) {
    if((*flPtr < lowerLimit) || (*flPtr > upperLimit)) {
      *flPtr = replacementValue;
    }
  }
  
}

//
// Performs simple unit conversions on the vertical level
//
void ReadFile::_convertVerticalUnits(int paramsIndex)
{
  // Set to User defined value if set in parameter file
  if(strlen(_paramsPtr->override_vlevels) > 0 &&
     STRequal_exact(_GribRecord->summary->levelType.c_str(), _paramsPtr->override_vlevels) ) 
  {  
    if(_fieldHeader.nz != _paramsPtr->vlevel_array_n)
      cerr << "WARNING: Override_vlevels size does not match parameter " << _GribRecord->summary->name.c_str() << endl;
    else {
      for(int a = 0; a < _fieldHeader.nz && a < _paramsPtr->vlevel_array_n; a++)
	_vlevelHeader.level[a] = _paramsPtr->_vlevel_array[a];
      return;
    }
  }
 
  // Grib2 pressure levels come in Pa, convert to hPa for Mdv
  if(_vlevelHeader.type[0] == Mdvx::VERT_TYPE_PRESSURE) {
    _fieldHeader.grid_minz   *= PASCAL_TO_HECTOPASCAL;
    _fieldHeader.grid_dz     *= PASCAL_TO_HECTOPASCAL;
    for(int a = 0; a < _fieldHeader.nz; a++)
      _vlevelHeader.level[a] *= PASCAL_TO_HECTOPASCAL;
  }
  // Grib2 vertical levels come in m, convert to km for Mdv
  if(_vlevelHeader.type[0] == Mdvx::VERT_TYPE_Z) {
    _fieldHeader.grid_minz   *= M_TO_KM;;
    _fieldHeader.grid_dz     *= M_TO_KM;;
    for(int a = 0; a < _fieldHeader.nz; a++)
      _vlevelHeader.level[a] *= M_TO_KM;;
  }

}

//
// Performs simple unit conversions, which are prescribed in the parameter file
//
void ReadFile::_convertUnits(int paramsIndex, fl32 *dataPtr)
{

  float scaleFactor = 1.0;
  float offsetFactor = 0.0;
  string theUnits = "";

  // provide the default unit if we're not converting
  STRncopy(_fieldHeader.units, _GribRecord->summary->units.c_str(), MDV_UNITS_LEN);

  // Attempt to get missing data value correct.
  // Grib2 does not allow a missing value to be specified unless complex packing is used.
  if (  _paramsPtr->autoset_missing_value){
     fl32 firstVal = ((fl32*)dataPtr)[0];
     if(_fieldHeader.missing_data_value == -9999.0 && 
       (firstVal == -1 || firstVal == -99 || firstVal == -999) ) {
        _fieldHeader.missing_data_value = firstVal;
     }
  } 

  if(paramsIndex >= 0 && paramsIndex < _paramsPtr->output_fields_n) {

    switch (_paramsPtr->_output_fields[paramsIndex].units) {
      case Params::MPS_TO_KNOTS:
	scaleFactor = MPS_TO_KNOTS;
	theUnits = "kts";
	break;
      case Params::M_TO_KM:
	scaleFactor = M_TO_KM;
	theUnits = "km";
	break;
      case Params::M_TO_FT:
	scaleFactor = M_TO_FT;
	theUnits = "ft";
	break;
      case Params::M_TO_KFT:
        scaleFactor = M_TO_KFT;
        theUnits = "kft";
        break;
      case Params::M_TO_MI:
	scaleFactor = M_TO_MI;
	theUnits = "miles";
	break;
      case Params::M_TO_100FT:
	scaleFactor = M_TO_100FT;
	theUnits = "ft(x100)";
	break;
      case Params::PASCALS_TO_MBAR:
	scaleFactor = PASCALS_TO_MBARS;
	theUnits = "mbar";
	break;
      case Params::KELVIN_TO_CELCIUS:
	offsetFactor = KELVIN_TO_CELCIUS;
	theUnits = "C";
	break;
      case Params::KGPKG_TO_GPKG:
	scaleFactor = KG_TO_G;
	theUnits = "g/kg";
	break;
      case Params::PERCENT_TO_FRACTION:
	scaleFactor = PERCENT_TO_FRAC;
	theUnits = "%";
	break;
      case Params::FRACTION_TO_PERCENT:
	scaleFactor = 1.0/PERCENT_TO_FRAC;
	theUnits = "%";
	break;
      case Params::MM_S_TO_MM_HR:
	scaleFactor = MM_S_TO_MM_HR;
	theUnits = "mm/hr";
	break;
      case Params::NO_CHANGE:
      default:
	theUnits = _GribRecord->summary->units;
	break;
    }
    
    fl32* flPtr = (fl32*)dataPtr;
    size_t numPts = _fieldHeader.nx*_fieldHeader.ny*_fieldHeader.nz;

    if (scaleFactor != 1.0) {
      
      for (size_t j = 0; j < numPts; j++,  flPtr++) {
	if((*flPtr != _fieldHeader.bad_data_value) && (*flPtr != _fieldHeader.missing_data_value)) {
	  *flPtr *= scaleFactor;
	}
      }
    
    }
    
    if (offsetFactor != 0.0) {
      
      for (size_t j = 0; j < numPts; j++,  flPtr++) {
	if((*flPtr != _fieldHeader.bad_data_value) && (*flPtr != _fieldHeader.missing_data_value)) {
	  *flPtr += offsetFactor;
	}
      }
      
    }
    
    STRncopy(_fieldHeader.units, theUnits.c_str(), MDV_UNITS_LEN);
    
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


void ReadFile::_replaceAdditionalBadMissing(fl32 *data, Mdvx::field_header_t fhdr,
                                            bool use_bad_value, fl32 bad_value,
                                            bool use_missing_value, fl32 missing_value)
{
  long vol_size = fhdr.nx * fhdr.ny;
  for (long i = 0; i < vol_size; ++i)
  {
    if (use_bad_value && data[i] == bad_value)
      data[i] = fhdr.bad_data_value;
    else if (use_missing_value && data[i] == missing_value)
      data[i] = fhdr.missing_data_value;
  }
}


fl32 *ReadFile::_reMapReducedOrGaussian(fl32 *data, Mdvx::field_header_t fhdr)
{

  fl32 *lats;
  int nx, ny;
  float dx, dy, minx, miny;
  fl32 **lons;
  int *nlons;
  int *tlons;

  si32 projID = _GribRecord->gds->getGridID();

  if(projID == Grib2::GDS::GAUSSIAN_LAT_LON_PROJ_ID) {
    
    Grib2::GausLatLonProj *latlonProj = (Grib2::GausLatLonProj *)_GribRecord->gds->getProjection();
    latlonProj->getGaussianLats(&lats);
    if(lats == NULL) {
      cerr << "ERROR: Calculation of Gaussian Latitude failure." << endl;
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
    cerr << "ERROR: Unsupported Reduced grid type." << endl;
    cerr << "  Cannot remap to regular lat/lon grid." << endl;
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

    // int ix = floor(x);

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
