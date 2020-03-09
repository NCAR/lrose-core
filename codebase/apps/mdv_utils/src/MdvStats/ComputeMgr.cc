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
//////////////////////////////////////////////////////////
// ComputeMgr.cc
//
// Manages the computations for statistics
//
// Yan Chen, RAL, NCAR
//
// Dec. 2007
//
//////////////////////////////////////////////////////////

#include <map>
#include <sstream>
#include <cmath>
#include <toolsa/pmu.h>
#include <Mdv/MdvxChunk.hh>
#include "ComputeMgr.hh"
using namespace std;

const string STDDEV = "STDDEV";
const string MEAN = "MEAN";
const string MIN = "MIN";
const string MAX = "MAX";
const string SUM = "SUM";
const string COV = "COV";

// constructor

ComputeMgr::ComputeMgr(const Params &params) : _params(params) {

  _startTime = 0;
  _endTime = 0;

  _need_setup = TRUE;

  // set up fields to read
  // using set will guarantee no duplicates
  for (
    int field_index = 0;
    field_index < _params.fields_stats_n;
    field_index++
  ) {
    _fieldNameSet.insert(_params._fields_stats[field_index].field_name);
  }
  for (
    int cov_index = 0;
    cov_index < _params.covariance_fields_n;
    cov_index++
  ) {
    _fieldNameSet.insert(_params._covariance_fields[cov_index].field_name1);
    _fieldNameSet.insert(_params._covariance_fields[cov_index].field_name2);
  }

}

// destructor

ComputeMgr::~ComputeMgr()
{
  
}

//////////////////////////////////////////////////
// Compute the statistics - FILELIST mode

int ComputeMgr::computeStatistics() {

  // register with procmap
  PMU_auto_register("ComputeMgr::computeStatistics");

  for (
    int file_index = 0;
    file_index < _params.input_files_n;
    file_index++
  ) {

    if (_params.debug)
      cerr << "Input file: " << _params._input_files[file_index] << endl;

    DsMdvx inMdvx;
    inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    inMdvx.setReadPath(_params._input_files[file_index]);
    inMdvx.setReadNoChunks();

    processOneInput(inMdvx);

  }

  // write output

  PMU_auto_register("Before write");

  if (_params.debug)
    cerr << "Write output..." << endl;

  int iret = writeOutput();

  if (_params.debug)
    cerr << "write done." << endl;

  return iret;
}

//////////////////////////////////////////////////
// Compute the statistics

int ComputeMgr::computeStatistics(vector<time_t>& inputTimeList) {

  // register with procmap
  PMU_auto_register("ComputeMgr::computeStatistics");

  size_t nFiles = inputTimeList.size();
  if (_params.debug)
    cerr << "Processing " << nFiles << " files." << endl;

  PMU_auto_register("Before calculation");

  // loop through files

  for (size_t file_index = 0; file_index < nFiles; file_index++) {

    if (_params.debug) {
      cerr << "Input file: " << utimstr(inputTimeList[file_index]) << endl;
    }
 
    DsMdvx inMdvx;
    inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);

    inMdvx.setReadTime(
      Mdvx::READ_CLOSEST,
      _params.input_url_dir,
      0, // search margin
      inputTimeList[file_index], // search time
      0 // forecast lead time
    );

    inMdvx.setReadNoChunks();

    processOneInput(inMdvx);

  }// end of loop files

  // write output

  PMU_auto_register("Before write");

  if (_params.debug)
    cerr << "Write output..." << endl;

  int iret = writeOutput();

  if (_params.debug)
    cerr << "write done." << endl;

  return iret;

}

//////////////////////////////////////////////////
// Process one input file

int ComputeMgr::processOneInput(DsMdvx& inMdvx) {

  // set up fields needed to be read in

  set<string>::iterator pos;
  for (pos = _fieldNameSet.begin(); pos != _fieldNameSet.end(); pos++) {
    inMdvx.addReadField((*pos).c_str());
  }

  // read volume for all fields needed
  if (inMdvx.readVolume()) {
    cerr << "ERROR: readVolume()" << endl;
    cerr << inMdvx.getErrStr() << endl;
    return -1; // continue to next file
  }

  if (_params.debug >= Params::DEBUG_VERBOSE)
    cerr << "Read in fields: " << inMdvx.getNFields() << endl;

  if (_params.debug)
    cerr << "Read done." << endl;

  // set start time & end time

  Mdvx::master_header_t mhdr = inMdvx.getMasterHeader();
  if (_startTime == 0 || _startTime > mhdr.time_begin)
    _startTime = mhdr.time_begin;
  if (_endTime == 0 || _endTime < mhdr.time_end)
    _endTime = mhdr.time_end;

  string chunkInfo = "Master header of " + inMdvx.getPathInUse();
  _addChunk(mhdr, chunkInfo, stats_StdDev);
  _addChunk(mhdr, chunkInfo, stats_Mean);
  _addChunk(mhdr, chunkInfo, stats_Min);
  _addChunk(mhdr, chunkInfo, stats_Max);
  _addChunk(mhdr, chunkInfo, stats_Sum);
  _addChunk(mhdr, chunkInfo, stats_Cov);

  if (_need_setup) {
    stats_StdDev.setMasterHeader(mhdr);
    stats_Mean.setMasterHeader(mhdr);
    stats_Min.setMasterHeader(mhdr);
    stats_Max.setMasterHeader(mhdr);
    stats_Sum.setMasterHeader(mhdr);
    stats_Cov.setMasterHeader(mhdr);
  }

  // set up output from the first file

  if (_need_setup) {

    int iret = _setupOutput(inMdvx);
    if (iret)
      return(iret);

    _need_setup = FALSE;

  } else {

    _updateOutput(inMdvx);
  }

  if (_params.debug)
    cerr << "process done." << endl;

  return 0;
}

//////////////////////////////////////////////////
// add chunk data

inline void ComputeMgr::_addChunk(
  const Mdvx::master_header_t &mhdr,
  const string &chunkInfo,
  DsMdvx &outMdvx
) {

  MemBuf buf;
  buf.add(&mhdr, sizeof(Mdvx::master_header_t));

  MdvxChunk *chunk = new MdvxChunk;
  chunk->setId(Mdvx::CHUNK_CLIMO_INFO);
  chunk->setInfo(chunkInfo.c_str());
  chunk->setData(buf.getPtr(), buf.getLen());
  outMdvx.addChunk(chunk);

}

//////////////////////////////////////////////////
// setup output fields

inline int ComputeMgr::_setupOutput(const DsMdvx &inMdvx) {

  int nFieldStats = _params.fields_stats_n;
  Params::field_stats_t *fieldStats = _params._fields_stats;

  int nCovs = _params.covariance_fields_n;
  Params::covariance_t *covFields = _params._covariance_fields;

  // field_stats

  for (int field_index = 0; field_index < nFieldStats; field_index++) {

    MdvxField *inField = inMdvx.getField(fieldStats[field_index].field_name);

    Mdvx::field_header_t infhdr = inField->getFieldHeader();

    if (infhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
      // transform to linear
      inField->transform2Linear();
    }

    string numFieldNameLong = _getNumFieldName(
      inField->getFieldNameLong(),
      false
    );
    string numFieldName = _getNumFieldName(inField->getFieldName(), true);

    // get data volume
    fl32 *inDataVol = (fl32 *)inField->getVol();

    int grid_size = infhdr.nx * infhdr.ny * infhdr.nz;

    // set up std dev

    if (fieldStats[field_index].std_dev) {
      // always calc Mean if need to calc Std_Dev
      MdvxField *curr_mean_field = stats_Mean.getField(
        fieldStats[field_index].field_name
      );
      MdvxField *curr_std_field = stats_StdDev.getField(
        fieldStats[field_index].field_name
      );
      MdvxField *curr_num_mean_field = stats_Mean.getField(
        numFieldName.c_str()
      );
      MdvxField *curr_num_std_field = stats_StdDev.getField(
        numFieldName.c_str()
      );

      if (curr_mean_field == 0) {

        // create new fields
        curr_mean_field = _createField(
          *inField,
          Mdvx::ENCODING_FLOAT32,
          Mdvx::DATA_TRANSFORM_MEAN
        );
        curr_std_field = _createField(
          *inField,
          Mdvx::ENCODING_FLOAT32,
          Mdvx::DATA_TRANSFORM_STDDEV
        );
        curr_num_mean_field = _createField(
          *inField,
          Mdvx::ENCODING_INT16,
          Mdvx::DATA_TRANSFORM_NONE
        );
        curr_num_std_field = _createField(
          *inField,
          Mdvx::ENCODING_INT16,
          Mdvx::DATA_TRANSFORM_NONE
        );
        if (
          curr_mean_field == NULL ||
          curr_std_field == NULL ||
          curr_num_mean_field == NULL ||
          curr_num_std_field == NULL
        ) {
          cerr << "ERROR: cannot create fields in output files." << endl;
          return(-1);
	}

        curr_num_mean_field->setFieldName(numFieldName.c_str());
        curr_num_std_field->setFieldName(numFieldName.c_str());
        curr_num_mean_field->setFieldNameLong(numFieldNameLong.c_str());
        curr_num_std_field->setFieldNameLong(numFieldNameLong.c_str());

        fl32 *mean_data = (fl32 *)curr_mean_field->getVol();
        fl32 *std_data = (fl32 *)curr_std_field->getVol();
        ui16 *num_mean_data = (ui16 *)curr_num_mean_field->getVol();
        ui16 *num_std_data = (ui16 *)curr_num_std_field->getVol();

        for (int i = 0; i < grid_size; i++) {
          if (_params.use_absolute_values) {
            mean_data[i] = fabs(inDataVol[i]);
          } else {
            mean_data[i] = inDataVol[i];
	  }
          std_data[i] = 0.0;
          if (
            inDataVol[i] == infhdr.missing_data_value ||
            inDataVol[i] == infhdr.bad_data_value
	  ) {
            num_std_data[i] = 0;
            num_mean_data[i] = 0;
	  } else {
            num_std_data[i] = 1;
            num_mean_data[i] = 1;
	  }

	}
        stats_Mean.addField(curr_mean_field);
        stats_Mean.addField(curr_num_mean_field);
        stats_StdDev.addField(curr_std_field);
        stats_StdDev.addField(curr_num_std_field);
      }

    } // std dev

    // set up mean if std_dev is not required

    if (
      (fieldStats[field_index].mean) &&
      !(fieldStats[field_index].std_dev)
    ) {
      MdvxField *curr_mean_field = stats_Mean.getField(
        fieldStats[field_index].field_name
      );
      MdvxField *curr_num_field = stats_Mean.getField(numFieldName.c_str());

      if (curr_mean_field == 0) {

        // create new fields

        curr_mean_field = _createField(
          *inField,
          Mdvx::ENCODING_FLOAT32,
          Mdvx::DATA_TRANSFORM_MEAN
        );
        curr_num_field = _createField(
          *inField,
          Mdvx::ENCODING_INT16,
          Mdvx::DATA_TRANSFORM_NONE
        );
        if (
          curr_mean_field == NULL ||
          curr_num_field == NULL
        ) {
          cerr << "ERROR: cannot create fields in output files." << endl;
          return(-1);
	}

        curr_num_field->setFieldName(numFieldName.c_str());
        curr_num_field->setFieldNameLong(numFieldNameLong.c_str());
        fl32 *mean_data = (fl32 *)curr_mean_field->getVol();
        ui16 *num_data = (ui16 *)curr_num_field->getVol();

        for (int i = 0; i < grid_size; i++) {
          if (_params.use_absolute_values) {
            mean_data[i] = fabs(inDataVol[i]);
	  } else {
            mean_data[i] = inDataVol[i];
	  }
          if (
            inDataVol[i] == infhdr.missing_data_value ||
            inDataVol[i] == infhdr.bad_data_value
          ) {
            num_data[i] = 0;
          } else {
            num_data[i] = 1;
          }
        }

        stats_Mean.addField(curr_mean_field);
        stats_Mean.addField(curr_num_field);
      }
    }  // mean

    // set up mininum

    if (fieldStats[field_index].min) {
      MdvxField *curr_min_field = stats_Min.getField(
        fieldStats[field_index].field_name
      );

      if (curr_min_field == 0) {
        // create new fields

        curr_min_field = _createField(
          *inField,
          Mdvx::ENCODING_FLOAT32,
          Mdvx::DATA_TRANSFORM_MINIMUM
        );
        if (curr_min_field == NULL) {
          cerr << "ERROR: cannot create fields in output files." << endl;
          return(-1);
	}

        fl32 *min_data = (fl32 *)curr_min_field->getVol();
        Mdvx::field_header_t minFhdr = curr_min_field->getFieldHeader();

        for (int i = 0; i < grid_size; i++) {
          if (inDataVol[i] == infhdr.missing_data_value) {
            min_data[i] = minFhdr.missing_data_value;
          } else if (inDataVol[i] == infhdr.bad_data_value) {
            min_data[i] = minFhdr.bad_data_value;
          } else {
            min_data[i] = inDataVol[i];
	  }
        }

        stats_Min.addField(curr_min_field);
      }
    } // min

    // set up maximum

    if (fieldStats[field_index].max) {
      MdvxField *curr_max_field = stats_Max.getField(
        fieldStats[field_index].field_name
      );

      if (curr_max_field == 0) {
        // create new fields

        curr_max_field = _createField(
          *inField,
          Mdvx::ENCODING_FLOAT32,
          Mdvx::DATA_TRANSFORM_MAXIMUM
        );
        if (curr_max_field == NULL) {
          cerr << "ERROR: cannot create fields in output files." << endl;
          return(-1);
	}

        fl32 *max_data = (fl32 *)curr_max_field->getVol();
	Mdvx::field_header_t maxFhdr = curr_max_field->getFieldHeader();

        for (int i = 0; i < grid_size; i++) {
          if (inDataVol[i] == infhdr.missing_data_value) {
            max_data[i] = maxFhdr.missing_data_value;
	  } else if (inDataVol[i] == infhdr.bad_data_value) {
            max_data[i] = maxFhdr.bad_data_value;
	  } else {
            max_data[i] = inDataVol[i];
	  }
        }

        stats_Max.addField(curr_max_field);
      }
    } // max

    // set up sum

    if (fieldStats[field_index].sum) {
      MdvxField *curr_sum_field = stats_Sum.getField(
        fieldStats[field_index].field_name
      );

      if (curr_sum_field == 0) {
        // create new fields

        curr_sum_field = _createField(
          *inField,
          Mdvx::ENCODING_FLOAT32,
          Mdvx::DATA_TRANSFORM_SUM
        );
        if (curr_sum_field == NULL) {
          cerr << "ERROR: cannot create fields in output files." << endl;
          return(-1);
	}

        fl32 *sum_data = (fl32 *)curr_sum_field->getVol();
	Mdvx::field_header_t sumFhdr = curr_sum_field->getFieldHeader();

        for (int i = 0; i < grid_size; i++) {
          if (inDataVol[i] == infhdr.missing_data_value) {
            sum_data[i] = sumFhdr.missing_data_value;
	  } else if (inDataVol[i] == infhdr.bad_data_value) {
            sum_data[i] = sumFhdr.bad_data_value;
	  } else {
            sum_data[i] = inDataVol[i];
	  }
        }

        stats_Sum.addField(curr_sum_field);
      }
    } // sum

  } // end of loop fields

  // set up covariance

  for (int cov_index = 0; cov_index < nCovs; cov_index++) {
    MdvxField *inField1 = inMdvx.getField(covFields[cov_index].field_name1);
    MdvxField *inField2 = inMdvx.getField(covFields[cov_index].field_name2);

    // check if two fields match in grid

    string err_str = "";
    if (!_fieldsMatch(inField1, inField2, err_str)) {
      cerr << "Fields do not match: " << inField1->getFieldName()
           << ", " << inField2->getFieldName() << endl;
      cerr << err_str << endl;
      continue; // continue to the next covariance calc
    }

    Mdvx::field_header_t infhdr1 = inField1->getFieldHeader();
    Mdvx::field_header_t infhdr2 = inField2->getFieldHeader();

    int grid_size = infhdr1.nx * infhdr1.ny * infhdr1.nz;

    // transform to linear
    if (infhdr1.transform_type == Mdvx::DATA_TRANSFORM_LOG)
      inField1->transform2Linear();
    if (infhdr2.transform_type == Mdvx::DATA_TRANSFORM_LOG)
      inField2->transform2Linear();

    string covFieldNameLong = _getCovFieldName(
      inField1->getFieldName(),
      inField2->getFieldName(),
      false // short name
    );
    string numCovFieldNameLong = _getNumFieldName(
      covFieldNameLong.c_str(),
      true // short name
    );

    string covFieldName = _getCovFieldName(
      inField1->getFieldName(),
      inField2->getFieldName(),
      true // short name
    );
    string numCovFieldName = _getNumFieldName(
      covFieldName.c_str(),
      true // short name
    );

    string numField1NameLong = _getNumFieldName(
      inField1->getFieldNameLong(),
      false // short name
    );
    string numField1Name = _getNumFieldName(
      inField1->getFieldName(),
      true // short name
    );
    string numField2NameLong = _getNumFieldName(
      inField2->getFieldNameLong(),
      false // short name
    );
    string numField2Name = _getNumFieldName(
      inField2->getFieldName(),
      true // short name
    );

    // get data volume
    fl32 *inDataVol1 = (fl32 *)inField1->getVol();
    fl32 *inDataVol2 = (fl32 *)inField2->getVol();


    // always calc Mean if need to calc covariance
    MdvxField *curr_mean_field1 = stats_Mean.getField(
      covFields[cov_index].field_name1
    );
    MdvxField *curr_mean_field2 = stats_Mean.getField(
      covFields[cov_index].field_name2
    );
    MdvxField *curr_cov_field = stats_Cov.getField(
      covFieldName.c_str()
    );
    MdvxField *curr_num_mean_field1 = stats_Mean.getField(
      numField1Name.c_str()
    );
    MdvxField *curr_num_mean_field2 = stats_Mean.getField(
      numField2Name.c_str()
    );
    MdvxField *curr_num_cov_field = stats_Cov.getField(
      numCovFieldName.c_str()
    );

    if ((curr_cov_field == 0 && curr_num_cov_field != 0) ||
        (curr_cov_field != 0 && curr_num_cov_field == 0)) {
      cerr << "fields do not match for covariance calculation." << endl;
      cerr << "cov_field and num_cov_field" << endl;
      return -1;
    }
    if ((curr_mean_field1 == 0 && curr_num_mean_field1 != 0) ||
        (curr_mean_field1 != 0 && curr_num_mean_field1 == 0)) {
      cerr << "fields do not match for covariance calculation." << endl;
      cerr << "mean_field1 and num_mean_field1" << endl;
      return -1;
    }
    if ((curr_mean_field2 == 0 && curr_num_mean_field2 != 0) ||
        (curr_mean_field2 != 0 && curr_num_mean_field2 == 0)) {
      cerr << "fields do not match for covariance calculation." << endl;
      cerr << "mean_field2 and num_mean_field2" << endl;
      return -1;
    }

    if (curr_cov_field == 0) {

      // create new fields

      curr_cov_field = _createField(
        *inField1,
        Mdvx::ENCODING_FLOAT32,
        Mdvx::DATA_TRANSFORM_COVAR
      );
      curr_num_cov_field = _createField(
        *inField1,
        Mdvx::ENCODING_INT16,
        Mdvx::DATA_TRANSFORM_NONE
      );
      if (curr_cov_field == NULL || curr_num_cov_field == NULL) {
        cerr << "ERROR: cannot create fields in output files." << endl;
        return(-1);
      }


      curr_cov_field->setFieldName(covFieldName.c_str());
      curr_cov_field->setFieldNameLong(covFieldNameLong.c_str());
      curr_num_cov_field->setFieldName(numCovFieldName.c_str());
      curr_num_cov_field->setFieldNameLong(numCovFieldNameLong.c_str());

      fl32 *cov_data = (fl32 *) curr_cov_field->getVol();
      ui16 *num_cov_data = (ui16 *) curr_num_cov_field->getVol();

      for (int i = 0; i < grid_size; i++) {
        cov_data[i] = 0.0;
        if (
          inDataVol1[i] == infhdr1.missing_data_value ||
          inDataVol1[i] == infhdr1.bad_data_value ||
          inDataVol2[i] == infhdr2.missing_data_value ||
          inDataVol2[i] == infhdr2.bad_data_value
        ) {
          num_cov_data[i] = 0;
        } else {
          num_cov_data[i] = 1;
        }

      }

      stats_Cov.addField(curr_cov_field);
      stats_Cov.addField(curr_num_cov_field);
    }

    if (curr_mean_field1 == 0) {

      curr_mean_field1 = _createField(
        *inField1,
        Mdvx::ENCODING_FLOAT32,
        Mdvx::DATA_TRANSFORM_MEAN
      );
      curr_num_mean_field1 = _createField(
        *inField1,
        Mdvx::ENCODING_INT16,
        Mdvx::DATA_TRANSFORM_NONE
      );
      if (curr_mean_field1 == NULL || curr_num_mean_field1 == NULL) {
        cerr << "ERROR: cannot create fields in output files." << endl;
        return(-1);
      }

      curr_num_mean_field1->setFieldName(numField1Name.c_str());
      curr_num_mean_field1->setFieldNameLong(numField1NameLong.c_str());

      fl32 *mean_data = (fl32 *)curr_mean_field1->getVol();
      ui16 *num_data = (ui16 *)curr_num_mean_field1->getVol();

      for (int i = 0; i < grid_size; i++) {
        mean_data[i] = inDataVol1[i];
        if (
          inDataVol1[i] == infhdr1.missing_data_value ||
          inDataVol1[i] == infhdr1.bad_data_value
        ) {
          num_data[i] = 0;
        } else {
          num_data[i] = 1;
        }
      }

      stats_Mean.addField(curr_mean_field1);
      stats_Mean.addField(curr_num_mean_field1);
    }

    if (curr_mean_field2 == 0) {

      curr_mean_field2 = _createField(
        *inField2,
        Mdvx::ENCODING_FLOAT32,
        Mdvx::DATA_TRANSFORM_MEAN
      );
      curr_num_mean_field2 = _createField(
        *inField2,
        Mdvx::ENCODING_INT16,
        Mdvx::DATA_TRANSFORM_NONE
      );
      if (curr_mean_field2 == NULL || curr_num_mean_field2 == NULL) {
        cerr << "ERROR: cannot create fields in output files." << endl;
        return(-1);
      }

      curr_num_mean_field2->setFieldName(numField2Name.c_str());
      curr_num_mean_field2->setFieldNameLong(numField2NameLong.c_str());

      fl32 *mean_data = (fl32 *)curr_mean_field2->getVol();
      ui16 *num_data = (ui16 *)curr_num_mean_field2->getVol();

      for (int i = 0; i < grid_size; i++) {
        mean_data[i] = inDataVol2[i];
        if (
          inDataVol2[i] == infhdr2.missing_data_value ||
          inDataVol2[i] == infhdr2.bad_data_value
        ) {
          num_data[i] = 0;
        } else {
          num_data[i] = 1;
        }
      }

      stats_Mean.addField(curr_mean_field2);
      stats_Mean.addField(curr_num_mean_field2);
    }

  } // end of loop covariance fields

  return 0;
}

//////////////////////////////////////////////////
// Update output

inline void ComputeMgr::_updateOutput(const DsMdvx &inMdvx) {

  int nFieldStats = _params.fields_stats_n;
  Params::field_stats_t *fieldStats = _params._fields_stats;

  int nCovs = _params.covariance_fields_n;
  Params::covariance_t *covFields = _params._covariance_fields;

  map<string/*field name*/, bool/*already calc'ed*/> meanCalcedMap;

  // loop through fields

  for (int field_index = 0; field_index < nFieldStats; field_index++) {
    MdvxField *inField = inMdvx.getField(fieldStats[field_index].field_name);
    Mdvx::field_header_t infhdr = inField->getFieldHeader();

    // transform to linear if log
    if (infhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG)
      inField->transform2Linear();

    string numFieldName = _getNumFieldName(inField->getFieldName(), true);

    // get data volume
    fl32 *inDataVol = (fl32 *)inField->getVol();

    int grid_size = infhdr.nx * infhdr.ny * infhdr.nz;

    // set flags for covariance calculation.

    if (fieldStats[field_index].std_dev || fieldStats[field_index].mean)
      meanCalcedMap.insert(
        make_pair(fieldStats[field_index].field_name, TRUE)
      );

    // calc standard deviation

    if (fieldStats[field_index].std_dev) {
      // always calc Mean if need to calc Std_Dev
      MdvxField *curr_mean_field = stats_Mean.getField(
        fieldStats[field_index].field_name
      );
      MdvxField *curr_std_field = stats_StdDev.getField(
        fieldStats[field_index].field_name
      );
      MdvxField *curr_num_mean_field = stats_Mean.getField(
        numFieldName.c_str()
      );
      MdvxField *curr_num_std_field = stats_StdDev.getField(
        numFieldName.c_str()
      );

      // check if fields match

      string err_str = "";
      if (!_fieldsMatch(inField, curr_std_field, err_str)) {
        cerr << "Fields do not match: " << inField->getFieldName() << endl;
        cerr << err_str << endl;
        continue; // continue to the next field
      }

      fl32 *curr_std_vol = (fl32 *)curr_std_field->getVol();
      fl32 *curr_mean_vol = (fl32 *)curr_mean_field->getVol();
      ui16 *curr_num_std = (ui16 *)curr_num_std_field->getVol();
      ui16 *curr_num_mean = (ui16 *)curr_num_mean_field->getVol();

      // update existing fields

      for (int i = 0; i < grid_size; i++) {

        if (
          inDataVol[i] != infhdr.missing_data_value &&
          inDataVol[i] != infhdr.bad_data_value
        ) {
          if (curr_num_std[i] == 0) {

            curr_std_vol[i] = 0.0;
            if (_params.use_absolute_values) {
              curr_mean_vol[i] = fabs(inDataVol[i]);
            } else {
              curr_mean_vol[i] = inDataVol[i];
	    }
            curr_num_std[i] = 1;
            curr_num_mean[i] = 1;

          } else {

            ui16 curr_num = curr_num_std[i];
            ui16 new_num = curr_num + 1;
            double curr_mean = curr_mean_vol[i];
            double curr_std = curr_std_vol[i];
            double curr_total = curr_mean * curr_num;
            double new_total;
            if (_params.use_absolute_values) {
              new_total = curr_total + fabs(inDataVol[i]);
	    } else {
              new_total = curr_total + inDataVol[i];
	    }

            double new_std =
              sqrt(
                (
                  ((curr_num - 1.0) * curr_std * curr_std) +
                  (curr_num * curr_mean * curr_mean) +
                  (inDataVol[i] * inDataVol[i]) -
                  ((new_total * new_total) / new_num)
                ) / curr_num
              );
            double new_mean = new_total / new_num;
            
            curr_std_vol[i] = new_std;
            curr_mean_vol[i] = new_mean;
            curr_num_std[i] = new_num;
            curr_num_mean[i] = new_num;

          }

        }
      } // loop grid

    } // std dev

    // calc Mean here if StdDev is not required

    if (
      (fieldStats[field_index].mean) &&
      !(fieldStats[field_index].std_dev)
    ) {
      MdvxField *curr_mean_field = stats_Mean.getField(
        fieldStats[field_index].field_name
      );
      MdvxField *curr_num_field = stats_Mean.getField(numFieldName.c_str());

      // check if fields match

      string err_str = "";
      if (!_fieldsMatch(inField, curr_mean_field, err_str)) {
        cerr << "Fields do not match: " << inField->getFieldName() << endl;
        cerr << err_str << endl;
        continue; // continue to the next field
      }

      fl32 *curr_mean_vol = (fl32 *)curr_mean_field->getVol();
      ui16 *curr_num_data = (ui16 *)curr_num_field->getVol();

      // update existing fields

      for (int i = 0; i < grid_size; i++) {

        if (
          inDataVol[i] != infhdr.missing_data_value &&
          inDataVol[i] != infhdr.bad_data_value
        ) {
          ui16 curr_num = curr_num_data[i];
          ui16 new_num = curr_num + 1;
          double curr_mean = curr_mean_vol[i];
          double curr_total = curr_mean * curr_num;
          double new_total;
          if (_params.use_absolute_values) {
            new_total = curr_total + fabs(inDataVol[i]);
          } else {
            new_total = curr_total + inDataVol[i];
	  }
          double new_mean = new_total / new_num;
            
          curr_mean_vol[i] = new_mean;
          curr_num_data[i] = new_num;

        }

      } // loop grid

    } // mean

    // calc minimum

    if (fieldStats[field_index].min) {
      MdvxField *curr_min_field = stats_Min.getField(
        fieldStats[field_index].field_name
      );
      fl32 *curr_min_vol = (fl32 *)curr_min_field->getVol();

      // check if fields match

      string err_str = "";
      if (!_fieldsMatch(inField, curr_min_field, err_str)) {
        cerr << "Fields do not match: " << inField->getFieldName() << endl;
        cerr << err_str << endl;
        continue; // continue to the next field
      }

      // update existing fields

      Mdvx::field_header_t minFhdr = curr_min_field->getFieldHeader();
      for (int i = 0; i < grid_size; i++) {

        if (
          inDataVol[i] != infhdr.missing_data_value &&
          inDataVol[i] != infhdr.bad_data_value
        ) {

          if (
            curr_min_vol[i] == minFhdr.bad_data_value ||
            curr_min_vol[i] == minFhdr.missing_data_value ||
            inDataVol[i] < curr_min_vol[i]
          ) {
            curr_min_vol[i] = inDataVol[i];

          }
        }

      } // loop grid

    } // min

    // calc maximum

    if (fieldStats[field_index].max) {
      MdvxField *curr_max_field = stats_Max.getField(
        fieldStats[field_index].field_name
      );
      fl32 *curr_max_vol = (fl32 *)curr_max_field->getVol();

      // check if fields match

      string err_str = "";
      if (!_fieldsMatch(inField, curr_max_field, err_str)) {
        cerr << "Fields do not match: " << inField->getFieldName() << endl;
        cerr << err_str << endl;
        continue; // continue to the next field
      }

      // update existing fields

      Mdvx::field_header_t maxFhdr = curr_max_field->getFieldHeader();
      for (int i = 0; i < grid_size; i++) {

        if (
          inDataVol[i] != infhdr.missing_data_value &&
          inDataVol[i] != infhdr.bad_data_value
        ) {

          if (
            curr_max_vol[i] == maxFhdr.bad_data_value ||
            curr_max_vol[i] == maxFhdr.missing_data_value ||
            inDataVol[i] > curr_max_vol[i]
          ) {
            
            curr_max_vol[i] = inDataVol[i];
          }
        }

      } // loop grid

    } // max

    // calc sum

    if (fieldStats[field_index].sum) {
      MdvxField *curr_sum_field = stats_Sum.getField(
        fieldStats[field_index].field_name
      );
      fl32 *curr_sum_vol = (fl32 *)curr_sum_field->getVol();

      // check if fields match

      string err_str = "";
      if (!_fieldsMatch(inField, curr_sum_field, err_str)) {
        cerr << "Fields do not match: " << inField->getFieldName() << endl;
        cerr << err_str << endl;
        continue; // continue to the next field
      }

      // update existing fields

      Mdvx::field_header_t sumFhdr = curr_sum_field->getFieldHeader();
      for (int i = 0; i < grid_size; i++) {

        if (
          inDataVol[i] != infhdr.missing_data_value &&
          inDataVol[i] != infhdr.bad_data_value
        ) {

          if (
            curr_sum_vol[i] == sumFhdr.bad_data_value ||
            curr_sum_vol[i] == sumFhdr.missing_data_value
          ) {
            curr_sum_vol[i] = inDataVol[i];
          } else {
            curr_sum_vol[i] += inDataVol[i];
	  }
        }

      } // loop grid

    } // sum

  } // end of loop fields


  // loop through covariance

  for (int cov_index = 0; cov_index < nCovs; cov_index++) {

    MdvxField *inField1 = inMdvx.getField(covFields[cov_index].field_name1);
    MdvxField *inField2 = inMdvx.getField(covFields[cov_index].field_name2);

    // check if two fields match in grid

    string err_str = "";
    if (!_fieldsMatch(inField1, inField2, err_str)) {
      cerr << "Fields do not match: " << inField1->getFieldName()
           << ", " << inField2->getFieldName() << endl;
      cerr << err_str << endl;
      continue; // continue to the next covariance calc
    }

    Mdvx::field_header_t infhdr1 = inField1->getFieldHeader();
    Mdvx::field_header_t infhdr2 = inField2->getFieldHeader();

    int grid_size = infhdr1.nx * infhdr1.ny * infhdr1.nz;

    // transform to linear
    if (infhdr1.transform_type == Mdvx::DATA_TRANSFORM_LOG)
      inField1->transform2Linear();
    if (infhdr2.transform_type == Mdvx::DATA_TRANSFORM_LOG)
      inField2->transform2Linear();

    // check if mean value has already been calculated.

    bool mean1Calced = FALSE;
    bool mean2Calced = FALSE;
    map<string, bool>::iterator it;

    it = meanCalcedMap.find(covFields[cov_index].field_name1);
    if (it == meanCalcedMap.end()) {
      meanCalcedMap.insert(make_pair(covFields[cov_index].field_name1, TRUE));
    } else {
      mean1Calced = it->second;
    }

    it = meanCalcedMap.find(covFields[cov_index].field_name2);
    if (it == meanCalcedMap.end()) {
      meanCalcedMap.insert(make_pair(covFields[cov_index].field_name2, TRUE));
    } else {
      mean2Calced = it->second;
    }

    string covFieldNameLong = _getCovFieldName(
      inField1->getFieldName(),
      inField2->getFieldName(),
      false // short name
    );
    string numCovFieldNameLong = _getNumFieldName(
      covFieldNameLong.c_str(),
      true // short name
    );

    string covFieldName = _getCovFieldName(
      inField1->getFieldName(),
      inField2->getFieldName(),
      true // short name
    );
    string numCovFieldName = _getNumFieldName(
      covFieldName.c_str(),
      true // short name
    );

    string numField1NameLong = _getNumFieldName(
      inField1->getFieldNameLong(),
      false // short name
    );
    string numField1Name = _getNumFieldName(
      inField1->getFieldName(),
      true // short name
    );
    string numField2NameLong = _getNumFieldName(
      inField2->getFieldNameLong(),
      false // short name
    );
    string numField2Name = _getNumFieldName(
      inField2->getFieldName(),
      true // short name
    );

    // get data volume

    fl32 *inDataVol1 = (fl32 *)inField1->getVol();
    fl32 *inDataVol2 = (fl32 *)inField2->getVol();

    // always calc Mean if need to calc covariance

    MdvxField *curr_mean_field1 = stats_Mean.getField(
      covFields[cov_index].field_name1
    );
    MdvxField *curr_mean_field2 = stats_Mean.getField(
      covFields[cov_index].field_name2
    );
    MdvxField *curr_cov_field = stats_Cov.getField(
      covFieldName.c_str()
    );
    MdvxField *curr_num_mean_field1 = stats_Mean.getField(
      numField1Name.c_str()
    );
    MdvxField *curr_num_mean_field2 = stats_Mean.getField(
      numField2Name.c_str()
    );
    MdvxField *curr_num_cov_field = stats_Cov.getField(
      numCovFieldName.c_str()
    );

    // check if fields match

    err_str = "";
    if (!_fieldsMatch(inField1, curr_cov_field, err_str)) {
      cerr << "Fields do not match: " << inField1->getFieldName() << endl;
      cerr << err_str << endl;
      continue; // continue to the next field
    }
    err_str = "";
    if (!_fieldsMatch(inField2, curr_cov_field, err_str)) {
      cerr << "Fields do not match: " << inField2->getFieldName() << endl;
      cerr << err_str << endl;
      continue; // continue to the next field
    }

    fl32 *curr_mean_vol1 = (fl32 *)curr_mean_field1->getVol();
    fl32 *curr_mean_vol2 = (fl32 *)curr_mean_field2->getVol();
    fl32 *curr_cov_vol = (fl32 *)curr_cov_field->getVol();
    ui16 *curr_num_mean1 = (ui16 *)curr_num_mean_field1->getVol();
    ui16 *curr_num_mean2 = (ui16 *)curr_num_mean_field2->getVol();
    ui16 *curr_num_cov = (ui16 *)curr_num_cov_field->getVol();

    // update existing fields

    for (int i = 0; i < grid_size; i++) {

      if (
        inDataVol1[i] != infhdr1.missing_data_value &&
        inDataVol1[i] != infhdr1.bad_data_value &&
        inDataVol2[i] != infhdr2.missing_data_value &&
        inDataVol2[i] != infhdr2.bad_data_value
      ) {

        if (curr_num_cov[i] == 0) {
          curr_cov_vol[i] = 0.0;
          curr_num_cov[i] = 1;
          if (!mean1Calced) {
            curr_mean_vol1[i] = inDataVol1[i];
            curr_num_mean1[i] = 1;
	  }
          if (!mean2Calced) {
            curr_mean_vol2[i] = inDataVol2[i];
            curr_num_mean2[i] = 1;
	  }

        } else {

          ui16 curr_num = curr_num_cov[i];
          ui16 new_num = curr_num + 1;
          double curr_mean1, curr_mean2;
          double new_mean1, new_mean2;
          double new_total1, new_total2;

          if (mean1Calced) {
            new_mean1 = curr_mean_vol1[i];
            new_total1 = new_mean1 * new_num;
            curr_mean1 = (new_total1 - inDataVol1[i]) / curr_num;
	  } else {
            curr_mean1 = curr_mean_vol1[i];
            new_total1 = curr_mean1 * curr_num + inDataVol1[i];
            new_mean1 = new_total1 / new_num;
	  }

          if (mean2Calced) {
            new_mean2 = curr_mean_vol2[i];
            new_total2 = new_mean2 * new_num;
            curr_mean2 = (new_total2 - inDataVol2[i]) / curr_num;
	  } else {
            curr_mean2 = curr_mean_vol2[i];
            new_total2 = curr_mean2 * curr_num + inDataVol2[i];
            new_mean2 = new_total2 / new_num;
	  }

          double curr_cov = curr_cov_vol[i];

          double new_cov =
            (
              ((curr_num - 1.0) * curr_cov) +
              (curr_num * curr_mean1 * curr_mean2) +
              (inDataVol1[i] * inDataVol2[i]) -
              ((new_total1 * new_total2) / new_num)
            ) / curr_num;

          curr_cov_vol[i] = new_cov;
          curr_mean_vol1[i] = new_mean1;
          curr_mean_vol2[i] = new_mean2;
          curr_num_cov[i] = new_num;
          curr_num_mean1[i] = new_num;
          curr_num_mean2[i] = new_num;

        }

      }

    } // end of loop grids

  } // end of loop covariance

}

//////////////////////////////////////////////////
// Create a new field from the given field

inline MdvxField *ComputeMgr::_createField(
  const MdvxField &inField,
  const Mdvx::encoding_type_t encodingType,
  const Mdvx::transform_type_t transformType
) const
{

  // set field header

  const Mdvx::field_header_t infhdr = inField.getFieldHeader();
  Mdvx::field_header_t newFHdr = infhdr;

  STRcopy(
    newFHdr.transform,
    _params.data_transformation_type,
    MDV_TRANSFORM_LEN
  );
  newFHdr.encoding_type = encodingType;
  newFHdr.transform_type = transformType;
  newFHdr.compression_type = Mdvx::COMPRESSION_NONE;
  newFHdr.bad_data_value = -9999.0;
  newFHdr.missing_data_value = -9999.0;
  newFHdr.min_value = 0.0;
  newFHdr.max_value = 0.0;
  newFHdr.min_value_orig_vol = 0.0;
  newFHdr.max_value_orig_vol = 0.0;

  int vol_size = newFHdr.nx * newFHdr.ny * newFHdr.nz;

  if (encodingType == Mdvx::ENCODING_FLOAT32) {
    newFHdr.volume_size = vol_size * sizeof(fl32);

    // create a new field

    MdvxField *newField = new MdvxField(
      newFHdr,
      inField.getVlevelHeader()
    );

    return newField;

  } else if (encodingType == Mdvx::ENCODING_INT8) {
    newFHdr.volume_size = vol_size * sizeof(ui08);
    newFHdr.data_element_nbytes = 1;
    newFHdr.scaling_type = Mdvx::SCALING_ROUNDED;
    newFHdr.scale = 1.0;
    newFHdr.bias = 0.0;

    MdvxField *newField = new MdvxField(
      newFHdr,
      inField.getVlevelHeader()
    );

    return newField;

  } else if (encodingType == Mdvx::ENCODING_INT16) {
    newFHdr.volume_size = vol_size * sizeof(ui16);
    newFHdr.data_element_nbytes = 2;
    newFHdr.scaling_type = Mdvx::SCALING_ROUNDED;
    newFHdr.scale = 1.0;
    newFHdr.bias = 0.0;

    MdvxField *newField = new MdvxField(
      newFHdr,
      inField.getVlevelHeader()
    );

    return newField;
  }

  return NULL;
}

//////////////////////////////////////////////////
// getNumFieldName()

inline string ComputeMgr::_getNumFieldName(
  const char*data_field_name,
  bool short_name
) {

  if (short_name) {
    return("N_" + string(data_field_name));
  } else {
    return("Num of " + string(data_field_name));
  }
}

//////////////////////////////////////////////////
// getCovFieldName()

inline string ComputeMgr::_getCovFieldName(
  const char* data_field1_name,
  const char* data_field2_name,
  bool short_name
) {

  if (short_name) {
    return(
      "cov_" +
      string(data_field1_name) + "_" + string(data_field2_name)
    );
  } else {
    return(
      "Covariance of " +
      string(data_field1_name) + "," + string(data_field2_name)
    );
  }
}

//////////////////////////////////////////////////////
// check if the given fields match based on their headers

bool ComputeMgr::_fieldsMatch(
  MdvxField *field1,
  MdvxField *field2,
  string &err_str
) {

  bool iret = true;

  const Mdvx::field_header_t &fhdr1 = field1->getFieldHeader();
  const Mdvx::field_header_t &fhdr2 = field2->getFieldHeader();

  MdvxProj proj1(fhdr1);
  MdvxProj proj2(fhdr2);
  if (proj1 != proj2) {
    err_str += "Field projections do not match:\n";
    err_str += fhdr1.field_name_long;
    err_str += ":\n";
    ostringstream oss;
    proj2.print(oss);
    err_str += oss.str();
    err_str += fhdr2.field_name_long;
    err_str += ":\n";
    oss.flush();
    proj2.print(oss);
    err_str += oss.str();

    iret = false;
  }

  if (fhdr1.vlevel_type != fhdr2.vlevel_type) {
    stringstream ss;
    ss << "Field vlevel types do not match:" << endl;
    ss << fhdr1.field_name_long << endl;
    ss << "   vert_type = "
       << Mdvx::vertType2Str(fhdr1.vlevel_type) << endl;
    ss << fhdr2.field_name_long << endl;
    ss << "   vert_type = "
       << Mdvx::vertType2Str(fhdr2.vlevel_type) << endl;
    err_str += ss.str();

    iret = false;
  }

  // Only check the vertical levels if they aren't constant

  if (!fhdr1.dz_constant || !fhdr2.dz_constant) {
    const Mdvx::vlevel_header_t &vlevel_hdr1 = field1->getVlevelHeader();
    const Mdvx::vlevel_header_t &vlevel_hdr2 = field2->getVlevelHeader();

    for (int z = 0; z < fhdr1.nz; ++z) {
      // Make sure all of the v levels match.
      //  I'm assuming that if the type is set to 0,
      // then it really wasn't set at all and the check should be ignored.

      if ((vlevel_hdr1.type[z] != 0 &&
           vlevel_hdr2.type[z] != 0 &&
           vlevel_hdr1.type[z] != vlevel_hdr2.type[z]) ||
          (vlevel_hdr1.level[z] != vlevel_hdr2.level[z])) {
        stringstream ss;
        ss << "Field vertical levels do not match:" << endl;
        ss << fhdr1.field_name_long << endl;
        ss << "   type[" << z << "] = "
           << Mdvx::vertType2Str(vlevel_hdr1.type[z]) << endl;
        ss << "   level[" << z << "] = " << vlevel_hdr1.level[z] << endl;
        ss << fhdr2.field_name_long << endl;
        ss << "   type[" << z << "] = "
           << Mdvx::vertType2Str(vlevel_hdr2.type[z]) << endl;
        ss << "   level[" << z << "] = " << vlevel_hdr2.level[z] << endl;
        err_str += ss.str();

        iret = false;
      }
    } /* endfor - z */
  }

  return(iret);
}

//////////////////////////////////////////////////
// Output for each file

int ComputeMgr::writeOutput() {

  int iret = 0;

  size_t stdNFields = stats_StdDev.getNFields();
  size_t meanNFields = stats_Mean.getNFields();
  size_t minNFields = stats_Min.getNFields();
  size_t maxNFields = stats_Max.getNFields();
  size_t sumNFields = stats_Sum.getNFields();
  size_t covNFields = stats_Cov.getNFields();

  if (stdNFields > 0) {

    if (_params.debug >= Params::DEBUG_VERBOSE)
      cerr << "Standard deviation output" << endl;

    iret |= _write(stats_StdDev, STDDEV);
  }

  if (meanNFields > 0) {

    if (_params.debug >= Params::DEBUG_VERBOSE)
      cerr << "Mean output" << endl;

    iret |= _write(stats_Mean, MEAN);
  }

  if (minNFields > 0) {

    if (_params.debug >= Params::DEBUG_VERBOSE)
      cerr << "Minimum output" << endl;

    iret |= _write(stats_Min, MIN);
  }

  if (maxNFields > 0) {

    if (_params.debug >= Params::DEBUG_VERBOSE)
      cerr << "Maximum output" << endl;

    iret |= _write(stats_Max, MAX);
  }

  if (sumNFields > 0) {

    if (_params.debug >= Params::DEBUG_VERBOSE)
      cerr << "Sum output" << endl;

    iret |= _write(stats_Sum, SUM);
  }
  
  if (covNFields > 0) {

    if (_params.debug >= Params::DEBUG_VERBOSE)
      cerr << "Covariance output" << endl;

    iret |= _write(stats_Cov, COV);
  }

  return (iret);
}

//////////////////////////////////////////////////
// Output

int ComputeMgr::_write(DsMdvx& outMdvx, const string& stats) {

  // set the master header

  Mdvx::master_header_t mhdr = outMdvx.getMasterHeader();
  mhdr.time_gen = time(NULL);
  mhdr.time_begin = _startTime;
  mhdr.time_centroid = _endTime;
  mhdr.time_end = _endTime;
  mhdr.time_expire = _endTime;
  mhdr.data_collection_type = Mdvx::DATA_CLIMO_ANA;

  outMdvx.setMasterHeader(mhdr);
  outMdvx.setDataSetSource(_params.input_url_dir);
  outMdvx.setDataSetInfo(_params.output_data_set_info);
  outMdvx.setDataSetName(_params.output_data_set_info);

  size_t nFields = outMdvx.getNFields();
  for (size_t i = 0; i < nFields; i++) {

    MdvxField *field = outMdvx.getField(i);
    field->computeMinAndMax(true);

    if (_params.compress_stats_fields) {
      field->requestCompression(Mdvx::COMPRESSION_ZLIB);
    }
  }

  // write out the file

  outMdvx.setDebug(_params.debug);
  outMdvx.setWriteLdataInfo();

  string path(_params.stats_url_dir);
  path += PATH_DELIM;
  path += stats;

  if (outMdvx.writeToDir(path.c_str())) {
    cerr << "ERROR - ComputeMgr::_write()" << endl;
    cerr << "  Cannot write file to dir:" << path << endl;
    cerr << outMdvx.getErrStr() << endl;
    return -1;
  }

  return 0;
}

