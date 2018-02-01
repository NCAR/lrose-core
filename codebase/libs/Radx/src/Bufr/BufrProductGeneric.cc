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

#include "BufrProductGeneric.hh"
#include <cstring>
#include <cstdio>
#include <iostream>
// #include <zlib.h>
#include <algorithm>
#include <stdlib.h>

using namespace std;

//////////////
// Constructor

BufrProductGeneric::BufrProductGeneric()
{
  duration = 0;
}

/////////////
// destructor

BufrProductGeneric::~BufrProductGeneric()
{
}


// Put the info in the correct storage location
// and take care of any setup that needs to happen
bool BufrProductGeneric::StuffIt(unsigned short des, string name, double value) {
  bool ok = true;

  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  if ((name.find("pixel value") != string::npos) ||
    (name.find("reflectivite pour la valeur du pixel") != string::npos)) {
    // ugh!  It's not a double value, it's an unsigned char...
    addData((unsigned char) value);
  } else if (name.find("latitude") != string::npos) {
    latitude = value;
  } else if (name.find("longitude") != string::npos) {
    longitude = value;
  } else if (name.find("height") != string::npos) {
    height = value;
  } else if (name.find("antenna elevation") != string::npos) {
    setAntennaElevationDegrees(value);
  } else if (name.find("number of pixels per row") != string::npos) {
    setNBinsAlongTheRadial((size_t) value);
  } else if (name.find("longueur de la porte distance apres integration") != string::npos) {
    setRangeBinSizeMeters(value);
  } else if ((name.find("range-bin offset") != string::npos) ||
    (name.find("range bin offset") != string::npos)) {
    setRangeBinOffsetMeters(value);
  } else if (name.find("number of pixels per column") != string::npos) {
    setNAzimuths((size_t) value);
  } else if (name.find("antenna beam azimuth") != string::npos) {
    setAntennaBeamAzimuthDegrees(value);

  } else if (name.find("year") != string::npos) {
    putYear((int) value);

    // TODO:  there are two of these, one for minutes and another for seconds
    // they have the same text
  } else if (name.find("time period or displacement") != string::npos) {
    duration = duration * 60 + (int) value;
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
  } else if (name.find("type of station") != string::npos) {
    ; // ignore it
  } else {
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
void BufrProductGeneric::storeReplicator(unsigned int value) {
    // do generic data storage into dictionary
  currentAccumulator = new vector<unsigned char>(); // (value);
    // TODO: how to handle repeaters that have multiple descriptors in them???
 
}

void BufrProductGeneric::trashReplicator() {

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
void BufrProductGeneric::createSweep() {
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
  parameterData.typeOfProduct = _fieldName; // "TH"; // TODO: fix this ... typeOfProduct;
  parameterData.data = realData;
  newSweep.parameterData.push_back(parameterData);
  sweepData.push_back(newSweep);
}


void BufrProductGeneric::addData(unsigned char value) {
  //if (value == 255)
  //  printf("%u at nData = %d\n", value, nData);

  //  if (nData < maxData)
  //  dataBuffer[nData++] = value;
  //else 
  //  throw "out of space in dataBuffer";

  //  set 255 to zero, so that both 255 and zero are marked as missing
  if (value == 255)
    value = 0;   
  currentAccumulator->push_back(value);
}


double *BufrProductGeneric::decompressData() {
  /*
  int n;
  n = nBinsAlongTheRadial * nAzimuths * sizeof(double);

  //int i, j;
  //unsigned char str[sizeof(double)];
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
  if (_debug) { 
    printf ("--> %g %g %g\n", temp[0], temp[1], temp[2]);
    //printf (" ... %g %g %g <--\n", temp[n-3], temp[n-2], temp[n-1]);
  }
  
  return (double *)UnCompDataBuff;
  */
  return NULL;
}

float *BufrProductGeneric::decompressDataFl32() {

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


void BufrProductGeneric::printGenericStore() {


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


/*

float *BufrProductGeneric::decompressDataFl32VitVraipol() {

  // convert the data to float
  //
  //  Code    |  V range (m/s)
  //    0     |  V < -59.75 m/s
  //    N     | [-60.25 + 0.5*N; -60.25 + 0.5*(N+1)]
  //   241    |  V >= 60.25 m/s
  //   255    |  missing
  //

  std::vector<unsigned char> *uCharVec;
  uCharVec = genericStore.back();
  std::vector<float> temp32(uCharVec->begin(), uCharVec->end());

  if (_debug) {
    printf("after conversion to float ...\n");
    printf ("--> %g %g %g\n", temp32[0], temp32[1], temp32[2]);
    //unsigned long nFloats;
    //nFloats = nBinsAlongTheRadial * nAzimuths;
    //printf (" ... %g %g %g <--\n", temp32[200], 
    //	    temp32[201], temp32[202]);
  }

  //  for (i=0; i<uCharVec->size(); ++i) {
  //    if (uCharVec[i] == 0) temp32[i] = 
  //  }

  // TODO:  free(UnCompDataBuff);
  return temp32.data();
}
*/

/*
// maintain the timestamps in a stack
// start a new timestamp with Year,
// always update the top timestamp with day, hour, etc.
void BufrProductGeneric::putYear(double value) {
  RadxTime *timeStamp = new RadxTime();
  timeStamp->setYear((int) value);
  if (_debug) cerr << "timeStamp " << timeStamp->asString() << endl;
  timeStampStack.push_back(timeStamp);
}

void BufrProductGeneric::putMonth(double value) {
  timeStampStack.back()->setMonth((int) value);
}

void BufrProductGeneric::putDay(double value)  {
  timeStampStack.back()->setDay((int) value);
}
void BufrProductGeneric::putHour(double value)  {
  timeStampStack.back()->setHour((int) value);
}
void BufrProductGeneric::putMinute(double value)  {
  timeStampStack.back()->setMin((int) value);
}
void BufrProductGeneric::putSecond(double value)  {
  timeStampStack.back()->setSec((int) value);
}

void BufrProductGeneric::printSweepData(ostream &out) {
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
*/
