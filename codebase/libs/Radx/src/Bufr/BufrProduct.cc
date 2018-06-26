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
#include <algorithm>
#include <stdlib.h>

using namespace std;

//////////////
// Constructor

BufrProduct::BufrProduct()
{
  //replicators.reserve(5);
  dataBuffer = NULL;
  reset();
  _debug = false;
  _verbose = false;
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
  _maxBinsAlongTheRadial = 0;
  //totalData = 0;
  if (dataBuffer != NULL)
    free(dataBuffer);
  dataBuffer = NULL;
  sweepData.clear(); // assume that the Rays are copied
  // reset the replicators vector
  while (!replicators.empty()) 
    trashReplicator();
}

void BufrProduct::allocateSpace(unsigned int n) {
  if (dataBuffer == NULL) {
    dataBuffer = (unsigned char *) malloc(n);
    maxData = n;
  }
}

void BufrProduct::haveTheTable(TableMap *tableMap) {
  _bufrTable = tableMap;
}

//size_t BufrProduct::getMaxBinsAlongTheRadial() {
//  return _maxBinsAlongTheRadial;
//}

void BufrProduct::setNBinsAlongTheRadial(size_t nBins) {
  if (sweepData.size() > 0) {
    if ((nBins != sweepData[0].nBinsAlongTheRadial) && _debug) {
      cerr << "Changing the number of bins along the radial from " <<
	sweepData[0].nBinsAlongTheRadial << " to " << nBins << endl;
    // don't resize here, just request everything by sweep number
    }
  }
  nBinsAlongTheRadial = nBins;
  if (nBins > _maxBinsAlongTheRadial)
    _maxBinsAlongTheRadial = nBins;
}

size_t BufrProduct::getNBinsAlongTheRadial(int sweepNumber) {
  return sweepData[sweepNumber].nBinsAlongTheRadial;
}

void BufrProduct::setAntennaElevationDegrees(double value) {
  // if (sweepData.size() > 0) {
  //  if (value!= sweepData[0].antennaElevationDegrees)
  //    throw "Cannot change the antenna elevation.";
  // }
  antennaElevationDegrees = value;
}
double BufrProduct::getAntennaElevationDegrees(int sweepNumber) {
  return sweepData[sweepNumber].antennaElevationDegrees;
}

void BufrProduct::setRangeBinSizeMeters(double value) {
  if (sweepData.size() > 0) {
    if ((value != sweepData[0].rangeBinSizeMeters) && _debug) {
      cerr << "Changing the range bin size from  " <<
	sweepData[0].rangeBinSizeMeters << " to " << value << endl;
    }
  }
  rangeBinSizeMeters = value;
}
double BufrProduct::getRangeBinSizeMeters(int sweepNumber) {
  return sweepData[sweepNumber].rangeBinSizeMeters;
}

void BufrProduct::setRangeBinOffsetMeters(double value) {
  if (sweepData.size() > 0) {
    if (value != sweepData[0].rangeBinOffsetMeters)
      throw "Cannot change the range bin offset.";
  }
  rangeBinOffsetMeters = value;
}
double BufrProduct::getRangeBinOffsetMeters() {
  return rangeBinOffsetMeters;
}

void BufrProduct::setNAzimuths(size_t value) {
  if (sweepData.size() > 0) {
    if (value != sweepData[0].nAzimuths)
      throw "Cannot change the number of azimuths.";
  }
  nAzimuths = value;
}
size_t BufrProduct::getNAzimuths() {
  return nAzimuths;
}

void BufrProduct::setAntennaBeamAzimuthDegrees(double value) {
  //if (sweepData.size() > 0) {
  //  if (value != sweepData[0].antennaBeamAzimuthDegrees)
  //    throw "Cannot change the antenna beam azimuth.";
  //}
  antennaBeamAzimuthDegrees = value;
}
double BufrProduct::getAntennaBeamAzimuthDegrees(int sweepNumber) {
  return sweepData[sweepNumber].antennaBeamAzimuthDegrees;
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
    break;
  case 2:  
    // reset dataBuffer
    nData = 0;
    break;
  case 3:
    break;
  case 4:
    allocateSpace(value); 
    break;
  default:
    throw "Unexpected number of entries in Replicator store";
  }
}

void BufrProduct::trashReplicator() {

  switch (replicators.size()) {
  case 1:
    break;
  case 2:  // we know the number of sweeps
    break;
  case 3:
    break;
  case 4:  
    // we are done with a data segment;  
    // combine data segments as needed
    if (dataBuffer != NULL) {
      //combine data segments
      unsigned int size;
      size = nData; 
      compressedData.add(dataBuffer, size);
      nData = 0;
      free(dataBuffer);
      dataBuffer = NULL;
    }
    break;
  default:
    throw "Unexpected number of replicators in store";
  }
  replicators.pop_back();
}

// Record all the information now that we have all the data values
void BufrProduct::createSweep() {
  //double *realData;
  float *realData;
      
      realData = decompressDataFl32();
      if (realData == NULL) {
	throw "ERROR - could not decompress data";
      }
      SweepData newSweep;
      int nTimeStamps = timeStampStack.size();
      if (nTimeStamps < 2) 
        throw "Missing start or end time stamp for sweep.";
      newSweep.endTime = timeStampStack.back();
      // Ok, don't remove the time stamps, just pick the last two
      // values
      // timeStampStack.pop_back();

      newSweep.startTime = timeStampStack.at(nTimeStamps-2); // back(); 
      //timeStampStack.pop_back();
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

bool BufrProduct::StuffIt(unsigned short des, string fieldName, string &value) {
  Radx::addErrStr(_errString, "", "WARNING - BufrFile::StuffIt", true);
  Radx::addErrStr(_errString, "", "  calling base function ", true);
  // cerr <<  _errString.c_str() << endl;
  return true;
}

// Put the info in the correct storage location
// and take care of any setup that needs to happen
bool BufrProduct::StuffIt(unsigned short des, string name, double value) {
  bool ok = true;

  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  if (name.find("byte element") != string::npos) {
    // ugh!  It's not a double value, it's an unsigned char...
    addData((unsigned char) value);
  } else if (name.find("latitude") != string::npos) {
    latitude = value;
    // ******* TEST  *******
    //double latitudeTest;
    //latitudeTest = value;
    //ByteOrder::swap64(&latitudeTest, sizeof(double), true);
    //cerr << "latitude test = " << latitudeTest << endl;
  } else if (name.find("longitude") != string::npos) {
    longitude = value;
  } else if (name.find("height") != string::npos) {
    height = value;
  } else if (name.find("antenna elevation") != string::npos) {
    setAntennaElevationDegrees(value);
  } else if (name.find("number of bins along the radial") != string::npos) {
    setNBinsAlongTheRadial((size_t) value);
  } else if ((name.find("range-bin size") != string::npos) ||
    (name.find("range bin size") != string::npos)) {
    setRangeBinSizeMeters(value);
  } else if ((name.find("range-bin offset") != string::npos) ||
    (name.find("range bin offset") != string::npos)) {
    setRangeBinOffsetMeters(value);
  } else if (name.find("number of azimuths") != string::npos) {
    setNAzimuths((size_t) value);
  } else if (name.find("antenna beam azimuth") != string::npos) {
    setAntennaBeamAzimuthDegrees(value);

  } else if (name.find("year") != string::npos) {
    putYear((int) value);
  } else if (name.find("month") != string::npos) {
    putMonth((int) value);
  } else if (name.find("day") != string::npos) {
    putDay((int) value);
  } else if (name.find("hour") != string::npos) {
    putHour((int) value);
  } else if (name.find("minute") != string::npos) {
    putMinute((int) value);
  } else if (name.find("second") != string::npos) {
    putSecond((int) value);
    // could probably use key to identify these fields...
    // instead of a string search
  } else if (name.find("wmo block") != string::npos) {
    WMOBlockNumber = value;
  } else if (name.find("wmo station") != string::npos) {
    WMOStationNumber = value;
  } else if (name.find("compression method") != string::npos) {
    ; // ignore it
  } else if (name.find("type of station") != string::npos) {
    ; // ignore it
  } else if (name.find("type of product") != string::npos) {
    int code = (int) value;
    // make sure the type of product agrees with the field name
    switch(code) {
    case 0:
      typeOfProduct = "DBZH";
      break;
    case 40:
      typeOfProduct = "VRAD";
      break;
    case 80:
      typeOfProduct = "ZDR";
      break;
    case 90:
      break;
    case 91: 
    case 230:
      typeOfProduct = "TH";
      break;
    case 92:
    case 60:
      typeOfProduct = "WRAD";
      break;	  
    case 243:
      typeOfProduct = "CM";
      break;
    case 240:
      typeOfProduct = "KDP";
      break;
    case 239:
      typeOfProduct = "PHIDP";
      break;
    case 241:
      typeOfProduct = "RHOHV";
      break;
    case 242:
      // TODO: move _fieldName comparison with typeOfProduct into BufrFile
      // ugh! allow for different name for this variable
      // fieldName == "TDR"  ==> use "TDR"
      // fieldName == ""     ==> use "ZDR"
      // fieldName == "ZDR"  ==> use "ZDR"
      if (_fieldName.size() <= 0) {
          typeOfProduct = "ZDR";
      } else {
        string temp = _fieldName;
        std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
        if (temp.compare("ZDR") == 0) {
          typeOfProduct = "ZDR";
	} else {
	  typeOfProduct = "TDR";
	}
      }
      break;
    case 231:
      typeOfProduct = "TV";
      break; 
    default:
      typeOfProduct = "UNKNOWN";
      ok = false;
    } 
    // make sure the field name we are looking for agrees with
    // the code in the data file
    // skip this check if the type of product has code 90; 
    // I don't know what product code 90 means.
    if (code != 90) {
      if (_fieldName.size() > 0) {
        string temp = _fieldName;
        std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
        if (typeOfProduct.compare(temp) != 0) {
	  Radx::addErrStr(_errString, "", "ERROR - BufrFile::StuffIt", true);
	  Radx::addErrStr(_errString, "  Expected Type of Product code for ", 
			  temp.c_str(), true);
	  Radx::addErrInt(_errString, "  Found code ", code, true);
	  throw _errString.c_str();
	}
      } else {
        //  TODO: we'll need to use the typeOfProduct as the field name
	;
      }
    }
  } else {
    ok = false;
  }
  return ok;
}


double *BufrProduct::decompressData() {
  unsigned long nn = nBinsAlongTheRadial * nAzimuths * sizeof(double);

  unsigned char *UnCompDataBuff = (unsigned char *) malloc(nn);
  unsigned long DestBuffSize = nn;
  
  if (uncompress(UnCompDataBuff, &DestBuffSize, 
		 (unsigned char *) compressedData.getPtr(), 
		 compressedData.getLen()) != Z_OK) {
		 //                 nData) != Z_OK) {
    return NULL;
  }
#if __BYTE_ORDER == __BIG_ENDIAN
  unsigned char str[sizeof(double)];
  for(int i = 0; i < nn/sizeof(double); ++i) {
    for (int j = 0; j < sizeof(double); ++j) {
      str[j] = UnCompDataBuff[i*sizeof(double)+j];
    }
    for (int j = 0; j < sizeof(double); ++j) {
      UnCompDataBuff[i*sizeof(double)+j] = str[sizeof(double) - 1 - j];
    }
  }
#endif
  compressedData.clear();
  double *temp;
  temp = (double *)UnCompDataBuff;
  if (_debug) { 
    printf ("--> %g %g %g\n", temp[0], temp[1], temp[2]);
    //printf (" ... %g %g %g <--\n", temp[n-3], temp[n-2], temp[n-1]);
  }
  return (double *)UnCompDataBuff;
}

float *BufrProduct::decompressDataFl32() {
  unsigned long nn = nBinsAlongTheRadial * nAzimuths * sizeof(double);

  unsigned char *UnCompDataBuff = (unsigned char *) malloc(nn);
  unsigned long DestBuffSize = nn;
  
  int result;
  result = uncompress(UnCompDataBuff, &DestBuffSize, 
		 (unsigned char *) compressedData.getPtr(), 
		      compressedData.getLen()); 

  if (result != Z_OK) {
    
    switch(result) {
    case Z_BUF_ERROR:	 	
      throw "The buffer dest was not large enough to hold the uncompressed data.";
      break;

    case Z_MEM_ERROR:	 	
      throw "Insufficient memory.";
      break;

    case Z_DATA_ERROR:	 	
      throw "The compressed data (referenced by source) was corrupted.";
      break;
    default:
      return NULL;
    }
  }

#if __BYTE_ORDER == __BIG_ENDIAN
  unsigned char str[sizeof(double)];
  for(int i = 0; i < nn/sizeof(double); ++i) {
    for (int j = 0; j < sizeof(double); ++j) {
      str[j] = UnCompDataBuff[i*sizeof(double)+j];
    }
    for (int j = 0; j < sizeof(double); ++j) {
      UnCompDataBuff[i*sizeof(double)+j] = str[sizeof(double) - 1 - j];
    }
  }
#endif
  compressedData.clear();
  double *temp;
  temp = (double *)UnCompDataBuff;
  if (_debug) {
    printf ("--> %g %g %g\n", temp[0], temp[1], temp[2]);
    //printf (" ... %g %g %g <--\n", temp[n-3], temp[n-2], temp[n-1]);
  }

  // convert the data to float
  unsigned long n32;
  n32 = nBinsAlongTheRadial * nAzimuths * sizeof(float);
  unsigned char *UnCompDataBuff32 = (unsigned char *) malloc(n32);
  float *temp32;
  temp32 = (float *) UnCompDataBuff32;
  for (unsigned long i=0; i< nBinsAlongTheRadial * nAzimuths; i++) {
    temp32[i] = (float) temp[i];
  }

  if (_debug) {
    printf("after conversion to float ...\n");
    printf ("--> %g %g %g\n", temp32[0], temp32[1], temp32[2]);
    //unsigned long nFloats;
    //nFloats = nBinsAlongTheRadial * nAzimuths;
    //printf (" ... %g %g %g <--\n", temp32[200], 
    //	    temp32[201], temp32[202]);
  }

  free(UnCompDataBuff);
  return temp32; //  (float *)UnCompDataBuff32;
}

// maintain the timestamps in a stack
// start a new timestamp with Year,
// always update the top timestamp with day, hour, etc.
void BufrProduct::putYear(double value) {
  RadxTime *timeStamp = new RadxTime();
  timeStamp->setYear((int) value);
  if (_debug) cerr << "timeStamp " << timeStamp->asString() << endl;
  timeStampStack.push_back(timeStamp);
}

void BufrProduct::putMonth(double value) {
  timeStampStack.back()->setMonth((int) value);
}

void BufrProduct::putDay(double value)  {
  timeStampStack.back()->setDay((int) value);
}
void BufrProduct::putHour(double value)  {
  timeStampStack.back()->setHour((int) value);
}
void BufrProduct::putMinute(double value)  {
  timeStampStack.back()->setMin((int) value);
}
void BufrProduct::putSecond(double value)  {
  timeStampStack.back()->setSec((int) value);
}

void BufrProduct::printSweepData(ostream &out) {
  int i;
  i = 0;
  for (vector<SweepData>::iterator sw = sweepData.begin();
       sw != sweepData.end(); ++sw) {
    out << "sweep: " << i << endl;
    RadxTime *time;
    time = sw->startTime;
    out << "   start time: " << time->asString() << endl;
    time = sw->endTime;
    out << "     end time: " << time->asString() << endl;

    // TimeStamp time = sw->startTime;
    // out << "   start time: " << time.year << "/" << time.month << "/" <<
    //   time.day << " " << time.hour << ":" << time.minute << ":" <<
    //   time.second << endl;
    // time = sw->endTime;
    // out << "     end time: " << time.year << "/" << time.month << "/" <<
    //   time.day << " " << time.hour << ":" << time.minute << ":" <<
    //   time.second << endl;
    out << "     antenna elevation (deg): " << sw->antennaElevationDegrees << endl; 
    out << "     n bins along the radial: " << sw->nBinsAlongTheRadial << endl; 
    out << "          range bin size (m): " << sw->rangeBinSizeMeters << endl; 
    out << "        range bin offset (m): " << sw->rangeBinOffsetMeters << endl; 
    out << "          number of azimuths: " << sw->nAzimuths << endl; 
    out << "  antenna beam azimuth (deg): " << sw->antennaBeamAzimuthDegrees << endl; 
    for (vector<ParameterData>::iterator dd=sw->parameterData.begin();
	 dd != sw->parameterData.end(); ++dd) {
      out << dd->typeOfProduct << endl;
      // cannot print the data values, because they have been moved and deleted.
      //out << dd->data[0] << ";" << dd->data[1] << ";" << dd->data[2] << ";" <<
      //	dd->data[3] << ";" << endl; 
    }
      i+=1;
  }
}

