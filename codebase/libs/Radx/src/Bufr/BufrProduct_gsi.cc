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
#include "Radx/TableMapKey.hh"
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
  descriptorsToDefine.clear();
  // set default values
  rangeBinOffsetMeters = 0;
  //_verbose = true;
}

/////////////
// destructor

BufrProduct_gsi::~BufrProduct_gsi()
{
}


void BufrProduct_gsi::ConstructDescriptor(string &desF,
  string &desX, string &desY, string &value,
  string &des_fieldName, char *line) {

  //char line[2048];
  char f; string x; string y;
  if (value.size() >=6) {
    f = value.at(0);
    x = value.substr(1,2);
    y = value.substr(3,3);
    //x[0] = value.at(1); x[1] = value.at(2);
    //y[0] = value.at(3); y[1] = value.at(4); y[2] = value.at(5);

  }
          
  sprintf(line, "%1s;%2s;%3s; %1c;%2s;%3s; %s",
          desF.c_str(), desX.c_str(), desY.c_str(), f,x.c_str(),y.c_str(),
          des_fieldName.c_str());

}
/*
For Reflectivity:
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

For Velocity:

                                            Descriptor       width(bits)   scale      units  reference  value
+(0 63 000)   BYTCNT --------------------------------------------    16    0          BYTES              0 564
+(0 01 018)   SSTN     RADAR STATION IDENTIFIER (SHORT) ---------    40    0      CCITT IA5              0 KABR
+(0 04 001)   YEAR     YEAR -------------------------------------    12    0           YEAR              0 2017
+(0 04 002)   MNTH     MONTH ------------------------------------     4    0          MONTH              0 5
+(0 04 003)   DAYS     DAY --------------------------------------     6    0            DAY              0 13
+(0 04 004)   HOUR     HOUR -------------------------------------     5    0           HOUR              0 11
+(0 04 005)   MINU     MINUTE -----------------------------------     6    0         MINUTE              0 30
+(0 04 006)   SECO     SECOND -----------------------------------     6    0         SECOND              0 4
+(0 05 002)   CLAT     LATITUDE (COARSE ACCURACY) ---------------    15    2        DEGREES          -9000 45.46
+(0 06 002)   CLON     LONGITUDE (COARSE ACCURACY) --------------    16    2        DEGREES         -18000 -98.41
+(0 07 030)   HSMSL    HEIGHT OF STATION GROUND ABOVE MSL -------    17    1         METERS          -4000 397
+(2 02 126)
+(2 01 118)
+(0 07 032)   HSALG    HEIGHT OF ANTENNA ABOVEGROUND ------------    16    2         METERS              0 20
+(2 01 000)
+(2 02 000)
+(0 02 134)   ANAZ     ANTENNA AZIMUTH ANGLE --------------------    16    2        DEGREES              0 137.51
+(0 02 135)   ANEL     ANTENNA ELEVATION ANGLE ------------------    15    2        DEGREES          -9000 3.45
+(0 33 251)   QCRW     QUALITY MARK FOR WINDSALONG RADIAL LINE --     3    0     CODE TABLE              0 1
+(1 01 000)  repeats 129 << varies from 78 .. 129  << create Accumulator here
 +(0 06 210)   DIST125M DISTANCE (FROM ANTENNATO GATE CENTER) IN UNITS OF 125M     12    0EIGHTHS OF A KM              0 358
 +(0 21 014)   DMVR     DOPPLER MEAN RADIAL VELOCITY ------------    13    1     METERS/SEC          -4096 409.5
 +(0 21 017)   DVSW     DOPPLER VELOCITY SPECTRAL WIDTH ---------     8    1     METERS/SEC              0 25.5
+(0 01 212)   VOID     RADAR VOLUME ID (IN THEFORM DDHHMM) ------    19    0        NUMERIC              0 131122
+(0 01 213)   SCID     RADAR SCAN ID (RANGE 1-21) ---------------     5    0        NUMERIC              0 4
+(2 01 129)
+(0 21 019)   HNQV     HIGH NYQUIST VELOCITY --------------------    10    1     METERS/SEC              0 26.4
+(2 01 000)
+(0 21 197)   VOCP     VOLUME COVERAGE PATTERN ------------------    10    0        NUMERIC              0 32
+(1 02 000)  repeats 3  << varies 
 +(2 06 001)
 +(0 63 255)   BITPAD -------------------------------------------     1    0           NONE              0 0

 *** create sweep at end of file  (sweep is from about 119.51 degrees to 208.51 degrees
 *** accumulate ray data at each repeater  (rays vary in the number of gates)
*/

// Put the info in the correct storage location
// and take care of any setup that needs to happen
//  all of the values will be text!  AGH!
bool BufrProduct_gsi::StuffIt(unsigned short des, string fieldName, string &value) {
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
      desF = value;
      break;
    case 11:
      des_x = atoi(value.c_str());
      desX = value;
      break;
    case 12:
      des_y = atoi(value.c_str());
      desY = value;
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
      _bufrTable->AddDescriptorFromBufrFile(des_f, des_x, des_y, des_fieldName,
           des_scale, des_units, des_referenceValue, des_dataWidthBits);
      break;
    case 448:
      // station Id
      break;
    case 34112: // 2;5;64 insert as data field
      // 1;5;0 0;31;1 3;0;3 2;5;64 1;1;0 0;31;1 0;0;30 
      // are grouped together because they define new descriptors
      // and expansions (Table D entries) that need to be 
      // added or defined.
      // 1;5;0 0;31;1 3;0;3 specifies f,x,y
      // 2;5;64 specifies the descriptor name
      // 1;1;0 0;31;1 0;0;30 specifies the expansion descriptors
      // add the descriptor to the tables after each expansion descriptor
      // Follow the format of the BUFR tables:
      // f;x;y f2;x2;y2  for the first element 
      //  ; ;  f3;x3;y3  for subsequent elements
      // 
      des_fieldName = value;
      break;
    case 30: // 0;0;30
      // could accumulate info into a string and then pass string
      // to ReadInternalTableD(const char **table, size=1) ?? <-- try this
      // or write a special function to take individual f,x,y, f2,x2,y2?
      char line[2048];
      ConstructDescriptor(desF, desX, desY, value, des_fieldName, line);
      /*
        char line[2048];
        char f[1]; char x[2]; char y[3];
        if (value.size() >=6) {
          f = value.at(0);
          x[0] = value.at(1); x[1] = value.at(2);
          y[0] = value.at(3); y[1] = value.at(4); y[2] = value.at(5);
        }
          
        sprintf(line, "%1u;%2u;%3u; %1c;%2s;%3s; %s",
                des_f, des_x, des_y, f,x,y, des_fieldName.c_str());
      */
      if (_verbose) printf("queuing new descriptor: %s\n", line);
      // TODO: accumulate lines in a vector until we have all of the
      // expanded descriptors; then add to the table
      
      descriptorsToDefine.push_back(line);
// when we are at the end of the list; or the beginning of a new list
//  add the lines to table D
      desF = "" ;
      desX = "";
      desY = "";
      
      break;
    default:
      unsigned char f, x, y;
      TableMapKey().Decode(des, &f, &x, &y);
      printf("ERROR - don't know what to do with ");
      printf("descriptor %1u;%2u;%3u (%d) value %s\n", f,x,y, des, value.c_str()); 
      ok = false;
  }
  return ok;
}

// Put the info in the correct storage location
// and take care of any setup that needs to happen
//  all of the values will be text!  AGH!
// ok, we are going to get data like this ...
//  date, time, lat, long, height, az, elev
//  repeat x ... so, this is ray data for a fixed az, elev, date and time
//     distance from antenna to gate
//     doppler mean radial velocity
bool BufrProduct_gsi::StuffIt(unsigned short des, string fieldName, double value) {
  bool ok = true;

  switch (des) {
    case 274:

      // TODO: record the name of the radar  KABR
      break;
    case 646: // antenna azimuth   0;02;134
      //antennaBeamAzimuthDegrees = 360; // we need to use 360 so that when
      // azimuths are calculated, in BufrRadxFile, the azimuth steps come out
      // to be 1 degree which is what the velocity data files have; 
      // value;
      antennaBeamAzimuthDegrees = value;
      break;
    case 647: // antenna elevation 0;02;135
      antennaElevationDegrees = value;
      break;
    case 1025:
      // year
      putYear((int) value);
      break;
    case 1026:
      // month
      putMonth((int) value);
      break;
    case 1027:
      // day
      putDay((int) value);
      break;
    case 1028:
      // hour
      putHour((int) value);
      break;
    case 1029:
      // minute
      putMinute((int) value);
      break;
    case 1030:
      // second
      putSecond((int) value);
      break;
    case 1282:
      // latitude
      latitude = value;
      break;
    case 1538:
      // longitude
      longitude = value;
      break;
      //case 1025:
      // radar volume id (DDHHMM)
      // break;
      // radar scan id (range 1 - 21)
    case 1746: 
      // Distance from antenna to gate center in units of 125M
      distanceFromAntennaUnitsOf125M.push_back((float) value);
      if (distanceFromAntennaUnitsOf125M.size() > 0) {
        float diff;
        diff = (value - distanceFromAntennaUnitsOf125M.back()) * 125.0;
        if (diff != rangeBinSizeMeters) 
          cout << "Warning: gate size changed from " << rangeBinSizeMeters 
               << " to " << diff << endl;
        rangeBinSizeMeters = diff;
      }
      break;
    case 3264:
    case 3267:
    case 3268:
    case 3269:
    case 3265:
    case 450: 
      // ignore these; mostly temperatures
      break;

    case 5377:
      horizontalReflectivityDb.push_back((float) value);
      break;
    case 5390: 
      // Doppler Mean Radial Velocity
      dopplerMeanRadialVelocity.push_back((float) value);
      break;
    case 5393:
      // Doppler Velocity Spectral Width 
      dopplerVelocitySpectralWidth.push_back((float) value);
      break;
    case 16383:
      // 0;63;255  end of the descriptors
      _addRayToSweep();
      distanceFromAntennaUnitsOf125M.clear(); // resize(0);
      dopplerMeanRadialVelocity.clear(); // resize(0);
      dopplerVelocitySpectralWidth.clear(); // resize(0);
      horizontalReflectivityDb.clear(); // resize(0);
      break;
    default:
      unsigned char f, x, y;
      TableMapKey().Decode(des, &f, &x, &y);
      printf("ERROR - don't know what to do with ");
      printf("descriptor %1u;%2u;%3u (%d) value %g\n", f,x,y, des, value);
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
  
  /*
    // do generic data storage into dictionary
  // NULL => create
  // not NULL:
  //   empty ==> reuse
  //   full  ==> create
  bool okToCreate = false;
  if (currentAccumulator == NULL) okToCreate = true;
  else if (currentAccumulator->size() > 0) okToCreate = true;
  else okToCreate = false;

  if (okToCreate) {
    currentAccumulator = new vector<unsigned char>();
  }
    // to handle repeaters that have multiple descriptors in them ...
  // just stripe the data in the vector
  */
}

void BufrProduct_gsi::trashReplicator() {
  /*
  if (_verbose) {
    printf("currentAccumulator: \n"); 
    for (std::vector<unsigned char>::const_iterator i = currentAccumulator->begin(); 
       i != currentAccumulator->end(); ++i)
    printf(" %x ", *i);
    printf("\n");
  }
  
  // do generic data storage into dictionary
  if (currentAccumulator->size() > 0) {
    genericStore.push_back(currentAccumulator);
  }
  */
  /*
  int n;
  n = genericStore.size();
  if (n >= 6) {
    if (_verbose) {
      printGenericStore();
    }
    //createSweep();
  }
  */

  if (descriptorsToDefine.size() > 0) {
    if (_debug) {
      printf("adding new table D descriptor ");
      for (std::vector<string>::const_iterator line = descriptorsToDefine.begin();
           line != descriptorsToDefine.end(); line++) { 
          cout << *line << endl; 
      }
    }
    _bufrTable->AddToTableD(descriptorsToDefine);
    descriptorsToDefine.clear();
  }
  
}

// Record all the information now that we have all the data values
void BufrProduct_gsi::createSweep() {
}


/*
There will be multiple rays (typically four) with the same start and end time.
  typedef struct {
    RadxTime *startTime;               << default = epoch
    RadxTime *endTime;                 << default = epoch + 1 sec.
    double    antennaElevationDegrees; << default = 0
    size_t    nBinsAlongTheRadial;  << this will vary
    double    rangeBinSizeMeters;   << default = 1
    double    rangeBinOffsetMeters; << default = 0
    size_t    nAzimuths;            << default = 360 to get downstream calc to 1 deg
    double    antennaBeamAzimuthDegrees;
    vector<ParameterData> parameterData;
    vector<ParameterDataFl64> parameterDataFl64;  << not used

    } SweepData;  << in this case, better known as ray data
*/
// add data in current accumulator to the sweep
void BufrProduct_gsi::_addRayToSweep() {

  //double *realData;
  float *realData;
       
  _isVelocity = false;
  _isReflectivity = false;
  if (dopplerMeanRadialVelocity.size() > 0) {
    _isVelocity = true;
  } else if (horizontalReflectivityDb.size() > 0) {
    _isReflectivity = true;
  } else {
    printf("We have no data for GSI\n");
    return;
  }
  
  // decompressDataFl32 uses dataForDecompress pointer
  realData = decompressDataFl32();
  if (realData == NULL) {
    throw "ERROR - could not decompress data";
  }

  SweepData newRay;  // ok, in this case, SweepData is really info about a ray  
  // there will only be one time stamp and a duration
  int nTimeStamps = timeStampStack.size();
  if (nTimeStamps < 1) {
    printf("WARNING - Missing start time stamp for sweep. Setting to default.\n");
    newRay.startTime = new RadxTime();
    RadxTime *endTime = new RadxTime();
    *endTime += 1;
    newRay.endTime = endTime;
  } else {
    // grab the first time stamp, because the second one
    // is for the last calibration

    newRay.startTime = timeStampStack.back();

    // convert RadxTime to time_t
    RadxTime *endTime = new RadxTime();
    endTime->copy(*(timeStampStack.back()));
    // add the duration
    *endTime += 1; // TODO: I don't know what the duration is //  duration;
    // convert back to RadxTime object
    newRay.endTime = endTime; 
    timeStampStack.pop_back();  // remove the time stamp
  }

  cerr << newRay.endTime->getW3cStr() << endl;
  cerr << newRay.startTime->getW3cStr() << endl;
  cerr << "------------" << endl;

  if (_debug) {
    RadxTime *time = newRay.startTime;
    cerr << "startTime " << time->asString() << endl; 
    time = newRay.endTime;
    cerr << "endTime " << time->asString() << endl; 
    cout << "azimuth " << antennaBeamAzimuthDegrees << endl;
  }
  if (_very_verbose) {
    cout << "i   vel, dist to gate center" << endl;
    for (size_t i = 0; i < dopplerMeanRadialVelocity.size(); i++) {
      printf(" %zu: %g, %g\n", i, dopplerMeanRadialVelocity[i],
             distanceFromAntennaUnitsOf125M[i]);
    }
  }
  newRay.antennaElevationDegrees = antennaElevationDegrees;
  if (distanceFromAntennaUnitsOf125M.size() > 0) 
    rangeBinSizeMeters = distanceFromAntennaUnitsOf125M[0] * 125.0;
  else 
    rangeBinSizeMeters = 0.0;
  newRay.rangeBinSizeMeters = rangeBinSizeMeters;
  newRay.rangeBinOffsetMeters = rangeBinOffsetMeters;
  newRay.nAzimuths = 360;
  newRay.antennaBeamAzimuthDegrees = antennaBeamAzimuthDegrees;
  ParameterData parameterData;
  if (_isReflectivity) {
    parameterData.typeOfProduct = "HREF"; // _fieldName; 
    newRay.nBinsAlongTheRadial = horizontalReflectivityDb.size();
  }
  if (_isVelocity) {
    parameterData.typeOfProduct = "VR";
    newRay.nBinsAlongTheRadial = dopplerMeanRadialVelocity.size();
  }
  parameterData.data = realData;
  // if there were multiple data fields, we'd add them here, or stripe the data
  newRay.parameterData.push_back(parameterData);
  sweepData.push_back(newRay);
  nAzimuths = 1; //360; // so that in the end, we will have 1 degree steps in azimuth // sweepData.size();
}


void BufrProduct_gsi::addData(unsigned char value) {
  
  //  set 255 to zero, so that both 255 and zero are marked as missing
  //if (value == 255)
  //  value = 0;   
  //currentAccumulator->push_back(value);
  
}


double *BufrProduct_gsi::decompressData() {
  return NULL;
}

float *BufrProduct_gsi::decompressDataFl32() {
  
  float *temp32 = NULL;
  int n = 0;
  if (_isVelocity) {
    n = dopplerMeanRadialVelocity.size();
    // if there are data ...
    if (n > 0) {
      // copy the data
      printf("before memcpy ...\n");
       printf ("--> %g %g %g\n", dopplerMeanRadialVelocity[0],
               dopplerMeanRadialVelocity[1], dopplerMeanRadialVelocity[2]);
      temp32 = (float *) malloc(n*sizeof(float));
      memcpy(temp32, &dopplerMeanRadialVelocity[0], n*sizeof(float));
    }
  }
  if (_isReflectivity) {
    n = horizontalReflectivityDb.size();
    // if there are data ...
    if (n > 0) {
      // copy the data
      temp32 = (float *) malloc(n*sizeof(float));
      memcpy(temp32, &horizontalReflectivityDb[0], n*sizeof(float));
    }
  }

  if ((temp32 != NULL) && (_debug)) {
     printf("after conversion to float ...\n");
     printf ("--> %g %g %g\n", temp32[0], temp32[1], temp32[2]);
  }

  return temp32;
  
  //return NULL;
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
