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
/////////////////////////////////////////////////////////////
// BufrProduct.cc
//
// BUFR Product 
//
// Brenda Javornik, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2018
//
///////////////////////////////////////////////////////////////

#include "BufrProduct_gsi.hh"
#include <cstring>
#include <cstdio>
#include <iostream>
// #include <zlib.h>
#include <algorithm>
#include <stdlib.h>

using namespace std;

//////////////
// Constructor

BufrProduct_gsi::BufrProduct_gsi()
{
  duration = 0;
}

/////////////
// destructor

BufrProduct_gsi::~BufrProduct_gsi()
{
}

/*
                                  Descriptor       width(bits)   scale      units  reference  value
+(1 03 000)  repeats 15
 +(0 00 001)   Table A:  entry ----------------------------------    24    0 CCITT IA5 0 254
 +(0 00 002)   Table A:  data category description, line 1 ------   256    0 CCITT IA5 0 QKSWND QUIKSCAT SCATTEROMETER
 +(0 00 003)   Table A:  data category description, line 2 ------   256    0 CCITT IA5 0 DATA (REPROCESSED WIND SPEED)
+(1 01 000)  repeats 79
 +(0 00 010)   F descriptor to be added or defined --------------     8    0 CCITT IA5 0 0
 +(0 00 011)   X descriptor to be added or defined --------------    16    0 CCITT IA5 0 12
 +(0 00 012)   Y descriptor to be added or defined --------------    24    0 CCITT IA5 0 192
 +(0 00 013)   Element name, line 1 -----------------------------   256    0 CCITT IA5 0 TOB TEMPERATURE OBSERVATION
 +(0 00 014)   Element name, line 2 -----------------------------   256    0 CCITT IA5 0
 +(0 00 015)   Units name ---------------------------------------   192    0 CCITT IA5 0 DEG C
 +(0 00 016)   Units scale sign ---------------------------------     8    0 CCITT IA5 0 +
 +(0 00 017)   Units scale --------------------------------------    24    0 CCITT IA5 0 1
 +(0 00 018)   Units reference sign -----------------------------     8    0 CCITT IA5 0 -
 +(0 00 019)   Units reference value ----------------------------    80    0 CCITT IA5 0 2732
 +(0 00 020)   Element data width -------------------------------    24    0 CCITT IA5 0 14
+(1 05 000)  repeats 0
 +(3 00 003)
 +(2 05 064)
 +(1 01 000)  repeats 319294248
 +(0 31 001)   Delayed descriptor replication factor ------------     8    0   Numeric 0 undefined value
 +(0 00 030)   Descriptor defining sequence ---------------------    48    0 CCITT IA5 0 undefined value
*/

// Put the info in the correct storage location
// and take care of any setup that needs to happen
// TODO: all of the values will be text!  AGH!
bool BufrProduct_gsi::StuffIt(unsigned short des, string name, string &value) {
  bool ok = true;

  switch (des) {
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    case 10:
      des_f = atoi(value.c_str());
      break;
    case 11:
      des_x = atoi(value.c_str());
      break;
    case 12:
      des_y = atoi(value.c_str());
      break;
    case 13:
      des_fieldName = value;
      break;
    case 14:
      des_fieldName.append(value);
      break;
    case 15:
      des_units = value;
      break;
    case 16:
      if (value.find('+') != string::npos)
        des_scale = 1;
      else des_scale = -1;
      break;
    case 17:
      des_scale *= atoi(value.c_str());
      break;
    case 18:
      if (value.find('+') != string::npos)
        des_referenceValue = 1;
      else des_referenceValue = -1;
      break;
    case 19:
      des_referenceValue *= atoi(value.c_str());
      break;
    case 20:
      des_dataWidthBits = atoi(value.c_str());
      _bufrTable->AddDescriptorFromBufrFile(des_f, des_x, des_y, des_fieldName, des_scale,
                                         des_units, des_referenceValue, des_dataWidthBits);
      break;
    default:
      ok = false;
  }
  return ok;
}


// OK, we are assuming a particular order to the replicators.
// The order is:
// - header repeats
// - number of sweeps
// - number of parameters
// - number of data sections
// - number of byte elements in data section 
void BufrProduct_gsi::storeReplicator(unsigned int value) {
    // do generic data storage into dictionary
  currentAccumulator = new vector<unsigned char>(); // (value);
    // TODO: how to handle repeaters that have multiple descriptors in them???
 
}

void BufrProduct_gsi::trashReplicator() {

  if (_verbose) {
    printf("currentAccumulator: \n"); 
    for (std::vector<unsigned char>::const_iterator i = currentAccumulator->begin(); 
       i != currentAccumulator->end(); ++i)
    printf(" %x ", *i);
    printf("\n");
  }
    //std::cout << *i << ' ';
  //}
  // do generic data storage into dictionary
  genericStore.push_back(currentAccumulator);
  int n;
  //  int nDataValues;
  //  nDataValues = currentAccumulator->size();
  n = genericStore.size();
  if (n >= 6) {
    if (_verbose) {
      printGenericStore();
    }
    createSweep();
  }
  
}

// Record all the information now that we have all the data values
void BufrProduct_gsi::createSweep() {
  //double *realData;
  float *realData;
      
  realData = decompressDataFl32();
  if (realData == NULL) {
    throw "ERROR - could not decompress data";
  }
  SweepData newSweep;
  // there will only be one time stamp and a duration
  int nTimeStamps = timeStampStack.size();
  if (nTimeStamps < 2) 
    throw "Missing start time stamp for sweep.";
  // grab the first time stamp, because the second one
  // is for the last calibration
  timeStampStack.pop_back();
  newSweep.startTime = timeStampStack.back();
  // convert RadxTime to time_t
  RadxTime *endTime = new RadxTime();
  endTime->copy(*(timeStampStack.back()));
  // add the duration
  *endTime += duration;
  // convert back to RadxTime object
  newSweep.endTime = endTime; 

  cerr << endTime->getW3cStr() << endl;
  cerr << newSweep.startTime->getW3cStr() << endl;
  cerr << "------------" << endl;

  timeStampStack.pop_back();

  if (_debug) {
    RadxTime *time = newSweep.startTime;
    cerr << "startTime " << time->asString() << endl; 
    time = newSweep.endTime;
    cerr << "endTime " << time->asString() << endl; 
  }
  newSweep.antennaElevationDegrees = antennaElevationDegrees;
  newSweep.nBinsAlongTheRadial = nBinsAlongTheRadial;
  newSweep.rangeBinSizeMeters = rangeBinSizeMeters;
  newSweep.rangeBinOffsetMeters = rangeBinOffsetMeters;
  newSweep.nAzimuths = nAzimuths;
  newSweep.antennaBeamAzimuthDegrees = antennaBeamAzimuthDegrees;
  ParameterData parameterData;
  parameterData.typeOfProduct = _fieldName; 
  parameterData.data = realData;
  newSweep.parameterData.push_back(parameterData);
  sweepData.push_back(newSweep);
}


void BufrProduct_gsi::addData(unsigned char value) {
  //  set 255 to zero, so that both 255 and zero are marked as missing
  if (value == 255)
    value = 0;   
  currentAccumulator->push_back(value);
}


double *BufrProduct_gsi::decompressData() {
  return NULL;
}

float *BufrProduct_gsi::decompressDataFl32() {

  // convert the data to float

  std::vector<unsigned char> *uCharVec;
  uCharVec = genericStore.back();
  int n = uCharVec->size();
  float *temp32 = (float *) malloc(n*sizeof(float));
  for (int i = 0; i < n; ++i)
    temp32[i] = (float) uCharVec->at(i);

  if (_debug) {
    printf("after conversion to float ...\n");
    printf ("--> %g %g %g\n", temp32[0], temp32[1], temp32[2]);
    //unsigned long nFloats;
    //nFloats = nBinsAlongTheRadial * nAzimuths;
    //printf (" ... %g %g %g <--\n", temp32[200], 
    //	    temp32[201], temp32[202]);
  }

  return temp32;
}


void BufrProduct_gsi::printGenericStore() {


  //vector< vector<unsigned char> *> genericStore;  
  // 

  for (vector< vector<unsigned char> *>::iterator s = genericStore.begin();
       s != genericStore.end(); ++s) {
    vector<unsigned char> *store;
    store = *s;
    int ncolumns = 256;
    if (store->size() == 240) ncolumns = 3;
    printf(" Store ...\n");
    int count = 0;
    for (vector<unsigned char>::iterator i = store->begin();
       i != store->end(); ++i) {
      printf("%g ", (float) *i);
      count += 1;
      if ((count % ncolumns) == 0) printf("\n");
    }
    printf("\n\n");
  }

}
