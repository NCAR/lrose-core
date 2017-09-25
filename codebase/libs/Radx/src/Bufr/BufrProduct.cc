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
// Aug 2017
//
///////////////////////////////////////////////////////////////

#include <Radx/BufrProduct.hh>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <zlib.h>

using namespace std;

//////////////
// Constructor

BufrProduct::BufrProduct()
{
  //replicators.reserve(5);
  //dataBuffer.
  reset();
}

/////////////
// destructor

BufrProduct::~BufrProduct()
{
  if (dataBuffer != NULL)
    free(dataBuffer);
}

void BufrProduct::reset() {
  currentState = 0;  // initial state
  nData = 0;
  maxData = 0;
  //totalData = 0;
  //if (dataBuffer != NULL)
  //  free(dataBuffer);
  // dataBuffer = NULL;
  // TODO: reset the replictors vector
}

void BufrProduct::allocateSpace(unsigned int n) {
  if (dataBuffer == NULL) {
    dataBuffer = (unsigned char *) malloc(n);
    maxData = n;
  }
}

void BufrProduct::transitionState(int n) {
  /*
  switch (currentState) {
  case 0:
    currentSweepNumber = 1;
    break;
  case 1:
    break;
  case 2:  //  parameters have been recorded
    // n contains the number of data segments
    // allocate space for data values;
    nDataSegments = n;
    break;
  case 3:  // data segments have been recorded
    // n contains the number of data values
    //??   // TODO: what to do here???  allocate too much memory
         // or combine separate buffers into one using memcpy?
    break;
  case 4:
    // increase the size of the data buffer? or 
    // create an additional one???
    break;
  default:
    throw "BufrProduct in unreckognized state";
  }
  state += 1;
  */
}

// OK, we are assuming a particular order to the replicators.
// The order is:
// - header repeats
// - number of sweeps
// - number of parameters
// - number of data sections
// - number of byte elements in data section 
void BufrProduct::storeReplicator(unsigned int value) {
  replicators.push_back(value);
  // once we have the number of byte elements, allocate space for the
  // ensuing data values.
  switch (replicators.size()) {
  case 1:  // we know the number of sweeps
    //sweepData.
    break;
  case 2:  
    // reset dataBuffer
    //totalData = 0;
    nData = 0;
    // dataBuffer.clear();
    break;
  case 3:
    break;
  case 4:
    allocateSpace(value); // TODO: this should be 65534 
    break;
    //  case 5:   
    //    break;
  default:
    throw "Unexpected number of entries in Replicator store";
  }
}

void BufrProduct::trashReplicator() {

  switch (replicators.size()) {
  case 1:
    break;
  case 2:  // we know the number of sweeps
    //sweepData.
    break;
  case 3:
    break;
  case 4:  
    // we are done with a data segment;  
    // combine data segments as needed
    if (dataBuffer != NULL) {
      //combine data segments
      unsigned int size;
      size = nData; // ?? replicators.back(); 
      compressedData.add(dataBuffer, size);
      nData = 0;
    }
    break;
  default:
    throw "Unexpected number of replicators in store";
  }
  replicators.pop_back();
}

// Record all the information now that we have all the data values
void BufrProduct::createSweep() {
      double *realData;
      realData = decompressData();
      SweepData newSweep;
      newSweep.endTime = timeStampStack.back();
      timeStampStack.pop_back();
      newSweep.startTime = timeStampStack.back();
      timeStampStack.pop_back();

      newSweep.antennaElevationDegrees = antennaElevationDegrees;
      newSweep.nBinsAlongTheRadial = nBinsAlongTheRadial;
      newSweep.rangeBinSizeMeters = rangeBinSizeMeters;
      newSweep.rangeBinOffsetMeters = rangeBinOffsetMeters;
      newSweep.nAzimuths = nAzimuths;
      newSweep.antennaBeamAzimuthDegrees = antennaBeamAzimuthDegrees;
      ParameterData parameterData;
      parameterData.typeOfProduct = typeOfProduct;
      parameterData.data = realData;
      newSweep.parameterData.push_back(parameterData);
      sweepData.push_back(newSweep);
}

void BufrProduct::addData(unsigned char value) {
  //if (value == 255)
  //  printf("%u at nData = %d\n", value, nData);
  if (nData < maxData)
    dataBuffer[nData++] = value;
  else 
    throw "out of space in dataBuffer";
}

double *BufrProduct::decompressData() {
  int n;
  n = nBinsAlongTheRadial * nAzimuths * sizeof(double);

  int i, j;
  unsigned char str[sizeof(double)];
  unsigned char *UnCompDataBuff = (unsigned char *) malloc(n);
  unsigned long DestBuffSize = n;
  
  if (uncompress(UnCompDataBuff, &DestBuffSize, 
		 (unsigned char *) compressedData.getPtr(), 
		 compressedData.getLen()) != Z_OK) {
		 //                 nData) != Z_OK) {
    return NULL;
  }
#if __BYTE_ORDER == __BIG_ENDIAN
  for (i = 0; i < *ndecomp/sizeof(double); ++i) {
    for (j = 0; j < sizeof(double); ++j) {
      str[j] = UnCompDataBuff[i*sizeof(double)+j];
    }
    for (j = 0; j < sizeof(double); ++j) {
      UnCompDataBuff[i*sizeof(double)+j] = str[sizeof(double) - 1 - j];
    }
  }
#endif
  compressedData.clear();
  double *temp;
  temp = (double *)UnCompDataBuff;
  printf ("--> %g %g %g\n", temp[0], temp[1], temp[2]);
  return (double *)UnCompDataBuff;
}

// maintain the timestamps in a stack
// start a new timestamp with Year,
// always update the top timestamp with day, hour, etc.
void BufrProduct::putYear(double value) {
  TimeStamp timeStamp;
  timeStamp.year = (int) value;
  timeStampStack.push_back(timeStamp);
}

void BufrProduct::putMonth(double value) {
  timeStampStack.back().month = (int) value;
}

void BufrProduct::putDay(double value)  {
  timeStampStack.back().day = (int) value;
}
void BufrProduct::putHour(double value)  {
  timeStampStack.back().hour = (int) value;
}
void BufrProduct::putMinute(double value)  {
  timeStampStack.back().minute = (int) value;
}
void BufrProduct::putSecond(double value)  {
  timeStampStack.back().second = (int) value;
}

/*
BufrProduct::setAntennaElevationDegrees(double value) {
  MetaData sweep = metaData.at(currentSweepNumber);
  sweep.antennaElevationDegrees = value;
}

BufrProduct::setNBinsAlongRadial(double value) {
  MetaData sweep = metaData.at(currentSweepNumber);
  sweep.setNBinsAlongRadial = value;
}

BufrProduct::setRangeBinSizeMeters(double value) {
  MetaData sweep = metaData.at(currentSweepNumber);
  sweep.setRangeBinSizeMeters = value;
}

BufrProduct::setRangeBinOffsetMeters(double value) {
  MetaData sweep = metaData.at(currentSweepNumber);
  sweep.setRangeBinOffsetMeters = value;
}

BufrProduct::setNAzimuths(double value) {
  MetaData sweep = metaData.at(currentSweepNumber);
  sweep.setNAzimuths = value;
}

BufrProduct::setAntennaBeamAzimuthDegrees(double value) {
  MetaData sweep = metaData.at(currentSweepNumber);
  sweep.antennaBeamAzimuthDegrees = value;
}
*/
