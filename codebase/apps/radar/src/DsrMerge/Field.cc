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
///////////////////////////////////////////////////////////////
// Field.cc
//
// Field object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2010
//
///////////////////////////////////////////////////////////////

#include "Field.hh"
#include <cstring>
#include <cmath>
using namespace std;

////////////////////////////////////////////////////
// constructor

Field::Field(const string &inputName,
             const string &outputName,
             const DsRadarParams radarParams,
             source_t source) :
        _inputName(inputName),
        _outputName(outputName),
        _radarParams(radarParams),
        _source(source)
        
{

  _isOutput = false;
  
}

//////////////////////////////////////////////////////////////////
// destructor

Field::~Field()

{
}

//////////////////////////////////////////////////////////////////
// print

void Field::print(ostream &out) const

{
  out << "========== Field ===========" << endl;
  out << "  inputName: " << _inputName << endl;
  out << "  outputName: " << _outputName << endl;
  out << "  isOutput: " << _isOutput << endl;
  out << "  source: " << sourceToStr(_source) << endl;
  out << "============================" << endl;
}

////////////////////////////////////////
// get string representation of source

string Field::sourceToStr(Field::source_t source)
{
  if (source == CHAN1) {
    return "CHAN1";
  } else if (source == CHAN2) {
    return "CHAN2";
  } else if (source == MEAN) {
    return "MEAN";
  } else {
    return "UNKNOWN";
  }
}

////////////////////////////////////////
// load up missing data

void Field::loadMissing(const DsRadarParams &radarParams)
{
  
  // allocate the data buffer

  int nBytes = radarParams.numGates * _params.byteWidth;
  _dataBuf.reserve(nBytes);
  memset(_dataBuf.getPtr(), 0, nBytes);

  if (_params.byteWidth == 1) {

    ui08 *data = (ui08 *) _dataBuf.getPtr();
    for (int ii = 0; ii < radarParams.numGates; ii++) {
      data[ii] = (ui08) _params.missingDataValue;
    }

  } else if (_params.byteWidth == 2) {

    ui16 *data = (ui16 *) _dataBuf.getPtr();
    for (int ii = 0; ii < radarParams.numGates; ii++) {
      data[ii] = (ui16) _params.missingDataValue;
    }

  } else if (_params.byteWidth == 4) {

    fl32 *data = (fl32 *) _dataBuf.getPtr();
    for (int ii = 0; ii < radarParams.numGates; ii++) {
      data[ii] = (fl32) _params.missingDataValue;
    }

  }

}

/////////////////////////////////////////////
// load up data unchanged from input field

void Field::loadUnchanged(const DsRadarParams &radarParams,
                          const vector<DsFieldParams*> &fp,
                          const DsRadarBeam &beam)
{

  // find the index of the field in the input data

  int nFieldsIn = fp.size();
  int index = -1;
  for (int ii = 0; ii < nFieldsIn; ii++) {
    if (_inputName == fp[ii]->name) {
      index = ii;
      break;
    }
  }
  if (index < 0) {
    // not found, fill with missing
    loadMissing(radarParams);
    return;
  }

  // allocate the data buffer
  
  int nBytes = radarParams.numGates * _params.byteWidth;
  _dataBuf.reserve(nBytes);
  memset(_dataBuf.getPtr(), 0, nBytes);

  if (_params.byteWidth == 1) {

    ui08 *dataOut = (ui08 *) _dataBuf.getPtr();
    ui08 *dataIn = (ui08 *) beam.getData() + index;
    for (int ii = 0; ii < radarParams.numGates; ii++) {
      *dataOut = *dataIn;
      dataIn += nFieldsIn;
      dataOut++;
    }

  } else if (_params.byteWidth == 2) {

    ui16 *dataOut = (ui16 *) _dataBuf.getPtr();
    ui16 *dataIn = (ui16 *) beam.getData() + index;
    for (int ii = 0; ii < radarParams.numGates; ii++) {
      *dataOut = *dataIn;
      dataIn += nFieldsIn;
      dataOut++;
    }

  } else if (_params.byteWidth == 4) {

    fl32 *dataOut = (fl32 *) _dataBuf.getPtr();
    fl32 *dataIn = (fl32 *) beam.getData() + index;
    for (int ii = 0; ii < radarParams.numGates; ii++) {
      *dataOut = *dataIn;
      dataIn += nFieldsIn;
      dataOut++;
    }

  }

}

/////////////////////////////////////////////
// load up mean from 2 input channels

void Field::loadMean(const DsRadarParams &radarParams,
                     const vector<DsFieldParams*> &fp1,
                     const DsRadarBeam &beam1,
                     const vector<DsFieldParams*> &fp2,
                     const DsRadarBeam &beam2)
{

  // find the index of the fields in the input data

  int nFieldsIn1 = fp1.size();
  int index1 = -1;
  for (int ii = 0; ii < nFieldsIn1; ii++) {
    if (_inputName == fp1[ii]->name) {
      index1 = ii;
      break;
    }
  }

  int nFieldsIn2 = fp2.size();
  int index2 = -1;
  for (int ii = 0; ii < nFieldsIn2; ii++) {
    if (_inputName == fp2[ii]->name) {
      index2 = ii;
      break;
    }
  }

  if (index1 < 0 || index2 < 0) {
    // not found, fill with missing
    loadMissing(radarParams);
    return;
  }

  const DsFieldParams *fparams1 = fp1[index1];
  const DsFieldParams *fparams2 = fp2[index2];
  double scale1 = fparams1->scale;
  double scale2 = fparams2->scale;
  double bias1 = fparams1->bias;
  double bias2 = fparams2->bias;
  int missing1 = fparams1->missingDataValue;
  int missing2 = fparams2->missingDataValue;

  // allocate the data buffer
  
  int nBytes = radarParams.numGates * _params.byteWidth;
  _dataBuf.reserve(nBytes);
  memset(_dataBuf.getPtr(), 0, nBytes);

  if (_params.byteWidth == 1) {

    ui08 *dataOut = (ui08 *) _dataBuf.getPtr();
    ui08 *dataIn1 = (ui08 *) beam1.getData() + index1;
    ui08 *dataIn2 = (ui08 *) beam2.getData() + index2;

    for (int ii = 0; ii < radarParams.numGates; ii++) {
      if (*dataIn1 == missing1 || *dataIn2 == missing2) {
        *dataOut = _params.missingDataValue;
      } else {
        double val1 = *dataIn1 * scale1 + bias1;
        double val2 = *dataIn2 * scale2 + bias2;
        double mean = (val1 + val2) / 2.0;
        int imean = (int) floor((mean - _params.bias) / _params.scale + 0.5);
        if (imean < 1) imean = 1;
        if (imean > 255) imean = 255;
        *dataOut = (ui08) imean;
      } // check for missing
      dataIn1 += nFieldsIn1;
      dataIn2 += nFieldsIn2;
      dataOut++;
    } // ii

  } else if (_params.byteWidth == 2) {

    ui16 *dataOut = (ui16 *) _dataBuf.getPtr();
    ui16 *dataIn1 = (ui16 *) beam1.getData() + index1;
    ui16 *dataIn2 = (ui16 *) beam2.getData() + index2;

    for (int ii = 0; ii < radarParams.numGates; ii++) {
      if (*dataIn1 == missing1 || *dataIn2 == missing2) {
        *dataOut = _params.missingDataValue;
      } else {
        double val1 = *dataIn1 * scale1 + bias1;
        double val2 = *dataIn2 * scale2 + bias2;
        double mean = (val1 + val2) / 2.0;
        int imean = (int) floor((mean - _params.bias) / _params.scale + 0.5);
        if (imean < 1) imean = 1;
        if (imean > 65535) imean = 65535;
        *dataOut = (ui16) imean;
      } // check for missing
      dataIn1 += nFieldsIn1;
      dataIn2 += nFieldsIn2;
      dataOut++;
    } // ii

  } else if (_params.byteWidth == 4) {

    fl32 *dataOut = (fl32 *) _dataBuf.getPtr();
    fl32 *dataIn1 = (fl32 *) beam1.getData() + index1;
    fl32 *dataIn2 = (fl32 *) beam2.getData() + index2;
    
    for (int ii = 0; ii < radarParams.numGates; ii++) {
      if (*dataIn1 == (fl32) missing1 || *dataIn2 == (fl32) missing2) {
        *dataOut = _params.missingDataValue;
      } else {
        fl32 mean = (*dataIn1 + *dataIn2) / 2.0;
        *dataOut = mean;
      } // check for missing
      dataIn1 += nFieldsIn1;
      dataIn2 += nFieldsIn2;
      dataOut++;
    } // ii

  } // byteWidth

}

