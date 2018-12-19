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
////////////////////////////////////////////////////////////////////////
// Particle ID
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2008
//
///////////////////////////////////////////////////////////////////////
//
// NcarParticleId reads dual pol moments in a DsRadar FMQ,
// and computes particle ID
//
///////////////////////////////////////////////////////////////////////

#include <iostream>
#include <algorithm>
#include <functional>
#include <cerrno>
#include <cstring>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaFile.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <radar/FilterUtils.hh>
#include <radar/DpolFilter.hh>
#include <radar/NcarParticleId.hh>
#include <radar/BeamHeight.hh>

using namespace std;

double NcarParticleId::_missingDouble = -9999.0;
const double NcarParticleId::pseudoEarthDiamKm = 17066.0;

// Constructor

NcarParticleId::NcarParticleId()

{

  _debug = false;
  _verbose = false;

  _startRangeKm = 0.0;
  _gateSpacingKm = 0.0;

  // create particle types

  _cl = new Particle("cl", "Cloud", CLOUD);
  _drz = new Particle("drz", "Drizzle", DRIZZLE);
  _lr = new Particle("lr", "Light_Rain", LIGHT_RAIN);
  _mr = new Particle("mr", "Moderate_Rain", MODERATE_RAIN);
  _hr = new Particle("hr", "Heavy_Rain", HEAVY_RAIN);
  _ha = new Particle("ha", "Hail", HAIL);
  _rh = new Particle("rh", "Rain_Hail_Mixture", RAIN_HAIL_MIXTURE);
  _gsh = new Particle("gsh", "Graupel_Small_Hail", GRAUPEL_SMALL_HAIL);
  _grr = new Particle("grr", "Graupel_Rain", GRAUPEL_RAIN);
  _ds = new Particle("ds", "Dry_Snow", DRY_SNOW);
  _ws = new Particle("ws", "Wet_Snow", WET_SNOW);
  _ic = new Particle("ic", "Ice_Crystals", ICE_CRYSTALS);
  _iic = new Particle("iic", "Irreg_Ice_Crystals", IRREG_ICE_CRYSTALS);
  _sld = new Particle("sld", "Supercooled_Liquid_Droplets", SUPERCOOLED_DROPS);
  _bgs = new Particle("bgs", "Flying_Insects", FLYING_INSECTS);
  _trip2 = new Particle("trip2", "Second_trip", SECOND_TRIP);
  _gcl = new Particle("gcl", "Ground_Clutter", GROUND_CLUTTER);
  _chaff = new Particle("chaff", "chaff", CHAFF);
  _misc = new Particle("misc", "miscellaneous", MISC);

  // add to vector
  
  _particleList.push_back(_cl);
  _particleList.push_back(_drz);
  _particleList.push_back(_lr);
  _particleList.push_back(_mr);
  _particleList.push_back(_hr);
  _particleList.push_back(_ha);
  _particleList.push_back(_rh);
  _particleList.push_back(_gsh);
  _particleList.push_back(_grr);
  _particleList.push_back(_ds);
  _particleList.push_back(_ws);
  _particleList.push_back(_ic);
  _particleList.push_back(_iic);
  _particleList.push_back(_sld);
  _particleList.push_back(_bgs);
  _particleList.push_back(_trip2);
  _particleList.push_back(_gcl);
  _particleList.push_back(_chaff);
  _particleList.push_back(_misc);

  // default weights
  
  _tmpWt = 20.0;
  _zhWt = 20.0;
  _zdrWt = 20.0;
  _kdpWt = 10.0;
  _ldrWt = 10.0;
  _rhvWt = 10.0;
  _sdzdrWt = 10.0;
  _sphiWt = 10.0;
  
  // default temp

  _tmpMinHtMeters = 0;
  _tmpMaxHtMeters = 0;
  _tmpBottomC = 0;
  _tmpTopC = 0;

  _snrThreshold = 3.0;
  _snrUpperThreshold = 9999.0; // no thresholding by default

  // median filter

  _applyMedianFilterToDbz = false;
  _dbzMedianFilterLen = 5;
  
  _applyMedianFilterToZdr = false;
  _zdrMedianFilterLen = 5;
  
  _applyMedianFilterToLdr = false;
  _ldrMedianFilterLen = 5;
  
  _applyMedianFilterToRhohv = false;
  _rhohvMedianFilterLen = 5;
  
  _applyMedianFilterToPid = false;
  _pidMedianFilterLen = 7;

  _replaceMissingLdr = false;
  _missingLdrReplacementValue = 0.0;

  // constraints

  _wavelengthCm = 10.0;

  // number of gates for standard deviation in range

  _ngatesSdev = 9;

  // temperature profile - init time

  _prevProfileDataTime = 0;

  // melting layer

  _mlInit();

}

/////////////////////////////////////////////////////////
// destructor

NcarParticleId::~NcarParticleId()

{

  for (int ii = 0; ii < (int) _particleList.size(); ii++) {
    delete _particleList[ii];
  }
  _particleList.clear();

  if (_mlDbzInterest) {
    delete _mlDbzInterest;
  }
  if (_mlZdrInterest) {
    delete _mlZdrInterest;
  }
  if (_mlRhohvInterest) {
    delete _mlRhohvInterest;
  }
  if (_mlTempInterest) {
    delete _mlTempInterest;
  }

  clear();

}

/////////////////////////////////////////////////////////
// clean up

void NcarParticleId::clear()

{

  _tempProfile.clear();
  _tempHtLookup_.free();

}

////////////////////////////////////////////
// Set processing options from params object

int NcarParticleId::setFromParams(const NcarPidParams &params)
{
  
  _params = params;

  setSnrThresholdDb(_params.PID_snr_threshold);
  setSnrUpperThresholdDb(_params.PID_snr_upper_threshold);

  if (_params.PID_apply_median_filter_to_DBZ) {
    setApplyMedianFilterToDbz(_params.PID_DBZ_median_filter_len);
  }
  if (_params.PID_apply_median_filter_to_ZDR) {
    setApplyMedianFilterToZdr(_params.PID_ZDR_median_filter_len);
  }

  if (_params.PID_apply_median_filter_to_LDR) {
    setApplyMedianFilterToLdr(_params.PID_LDR_median_filter_len);
  }
  if (_params.PID_replace_missing_LDR) {
    setReplaceMissingLdr(_params.PID_LDR_replacement_value);
  }

  if (_params.PID_apply_median_filter_to_RHOHV) {
    setApplyMedianFilterToRhohv(_params.PID_RHOHV_median_filter_len);
  }

  setNgatesSdev(_params.PID_ngates_for_sdev);
  setMinValidInterest(_params.PID_min_valid_interest);
  
  if (_params.PID_locate_melting_layer) {
    setComputeMeltingLayer(true);
  }
  
  if (readThresholdsFromFile(_params.PID_thresholds_file_path)) {
    cerr << "ERROR - NcarParticleId::setFromParams" << endl;
    cerr << "  Cannot read in pid thresholds from file: "
         << _params.PID_thresholds_file_path << endl;
    return -1;
  }

  return 0;
  
}

/////////////////////////////////////////
// read in thresholds from file
// returns 0 on success, -1 on failure

int NcarParticleId::readThresholdsFromFile(const string &path)
  
{

  clear();

  _thresholdsFilePath = path;

  if (_debug) {
    cerr << "Reading thresholds from file: " << path << endl;
  }

  TaFile inFile;
  FILE *in = inFile.fopen(path.c_str(), "r");
  if (in == NULL) {
    int errNum = errno;
    cerr << "ERROR - NcarParticleId::readThresholdsFromFile" << endl;
    cerr << "  Cannot open PID thresholds file for reading" << endl;
    cerr << "  File path: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  char line[8192];
  bool gotWeights = false;

  while (!feof(in)) {

    if (fgets(line, 8192, in) == NULL) {
      break;
    }

    // ignore comments

    if (line[0] == '#') {
      continue;
    }

    if (strlen(line) < 2) {
      continue;
    }

    if (_verbose) {
      cerr << line;
    }

    // force lower case

    for (int ii = 0; ii < (int) strlen(line); ii++) {
      line[ii] = tolower(line[ii]);
    }
    
    // set IDs
    
    if (strncmp(line, "pid.", 4) == 0) {
      for (int ii = 0; ii < (int) _particleList.size(); ii++) {
	_setId(_particleList[ii], line);
      }
    }

    // set temperature profile

    if (strncmp(line, "tpf", 3) == 0) {
      _parseTempProfileLine(line);
    }

    // set weights

    if (strncmp(line, "wts", 3) == 0) {
      _setWeights(line);
      gotWeights = true;
    }

    // set threshold limits

    if (strncmp(line, "thr.", 4) == 0) {
      for (int ii = 0; ii < (int) _particleList.size(); ii++) {
	_setLimits(_particleList[ii], line);
      }
    }
    
    // set interest maps

    if (strncmp(line, "tbl.zh.", 7) == 0) {
      if (!gotWeights) {
        // no weights yet - error
        break;
      }
      for (int ii = 0; ii < (int) _particleList.size(); ii++) {
	_setInterestMaps(_particleList[ii], line);
      }
    }

  } // while (!feof(in) ...
  
  inFile.fclose();

  if (!gotWeights) {
    cerr << "ERROR - NcarParticleId::readThresholdsFromFile" << endl;
    cerr << "  No Wts specified in file" << endl;
    cerr << "  File path: " << path << endl;
    return -1;
  }

  if (_verbose) {
    cerr << "Read successful" << endl;
    print(cerr);
  }

  return 0;

}

// get temperature at a given height

double NcarParticleId::getTmpC(double htKm)

{

  int htMeters = (int) (htKm * 1000.0 + 0.5);

  if (htMeters <= _tmpMinHtMeters) {
    return _tmpBottomC;
  } else if (htMeters >= _tmpMaxHtMeters) {
    return _tmpTopC;
  }

  int kk = htMeters - _tmpMinHtMeters;
  return _tempHtLookup[kk];

}

/////////////////////////////////////////////////////////
// Initialize the object arrays for later use.
// Do this if you need access to the arrays, but have not yet called
// computePidBeam(), and do not plan to do so.
// For example, you may want to output missing fields that you have
// not computed, but the memory needs to be there.

void NcarParticleId::initializeArrays(int nGates)

{

  // allocate local arrays

  _allocArrays(nGates);

  // set to missing
  
  for (int ii = 0; ii < nGates; ii++) {
    _snr[ii] = _missingDouble;
    _dbz[ii] = _missingDouble;
    _zdr[ii] = _missingDouble;
    _kdp[ii] = _missingDouble;
    _ldr[ii] = _missingDouble;
    _rhohv[ii] = _missingDouble;
    _phidp[ii] = _missingDouble;
    _tempC[ii] = _missingDouble;
    _pid[ii] = _missingDouble;
    _category[ii] = CATEGORY_UNKNOWN;
    _interest[ii] = _missingDouble;
    _pid2[ii] = -1;
    _interest2[ii] = -1;
    _confidence[ii] = _missingDouble;
    _sdzdr[ii] = _missingDouble;
    _sdphidp[ii] = _missingDouble;
    _cflags[ii] = false;
    _mlInterest[ii] = _missingDouble;
  }
  
}

/////////////////////////////////////////////////////////
// compute PID for a beam
//
// Passed in:
// ---------
//   nGates: number of gates
//   cflag: censoring flag, pid only computed where this is false
//   dbz: reflectivity
//   zdr: differential reflectivity
//   kdp: phidp slope
//   ldr: linear depolarization ratio
//   rhohv: correlation coeff
//   phidp: phase difference
//   tempC: temperature at each gate, in deg C
//
// Input fields at a gate should be set to _missingDouble
// if they are not valid for that gate.
//
// Results:
// --------
// Stored in local arrays on this class. Use get() methods to retieve them.

void NcarParticleId::computePidBeam(int nGates,
                                    const double *snr,
                                    const double *dbz,
                                    const double *zdr,
                                    const double *kdp,
                                    const double *ldr,
                                    const double *rhohv,
                                    const double *phidp,
                                    const double *tempC)
  
{

  // allocate local arrays

  _allocArrays(nGates);

  for (int ii = 0; ii < (int) _particleList.size(); ii++) {
    _particleList[ii]->allocGateInterest(nGates);
  }

  // copy input data to local arrays

  memcpy(_snr, snr, nGates * sizeof(double));
  memcpy(_dbz, dbz, nGates * sizeof(double));
  memcpy(_zdr, zdr, nGates * sizeof(double));
  memcpy(_kdp, kdp, nGates * sizeof(double));
  memcpy(_ldr, ldr, nGates * sizeof(double));
  memcpy(_rhohv, rhohv, nGates * sizeof(double));
  memcpy(_phidp, phidp, nGates * sizeof(double));
  memcpy(_tempC, tempC, nGates * sizeof(double));

  // replace missing LDR values with speficied value, if requested

  if (_replaceMissingLdr) {
    for (int ii = 0; ii < nGates; ii++) {
      if (_ldr[ii] == _missingDouble) {
        _ldr[ii] = _missingLdrReplacementValue;
      }
    }
  }

  // replace missing KDP values with 0

  for (int ii = 0; ii < nGates; ii++) {
    if (_kdp[ii] == _missingDouble) {
      _kdp[ii] = 0;
    }
  }

  // censor

  for (int ii = 0; ii < nGates; ii++) {
    double sn = snr[ii];
    if (sn == _missingDouble || sn < _snrThreshold) {
      _cflags[ii] = true;
    } else {
      _cflags[ii] = false;
    }
  }

  for (int ii = 0; ii < nGates; ii++) {
    if (_cflags[ii]) {
      _dbz[ii] = _missingDouble;
      _zdr[ii] = _missingDouble;
      _kdp[ii] = _missingDouble;
      _ldr[ii] = _missingDouble;
      _rhohv[ii] = _missingDouble;
      _phidp[ii] = _missingDouble;
    }
  }

  // compute standard deviations in range
  
  FilterUtils::computeSdevInRange(_zdr, _sdzdr, nGates,
                                  _ngatesSdev, _missingDouble);

  // sdev of phidp is a special case since we
  // need to compute it around the circle
  
  _phidpProc.setRangeGeometry(_startRangeKm, _gateSpacingKm);
  _phidpProc.computePhidpSdev(nGates, _ngatesSdev,
                              _phidp, _missingDouble);
  memcpy(_sdphidp, _phidpProc.getPhidpSdev(),
         nGates * sizeof(double));

  // apply median filter as appropriate
  
  if (_applyMedianFilterToDbz) {
    FilterUtils::applyMedianFilter(_dbz, nGates, _dbzMedianFilterLen);
  }
  if (_applyMedianFilterToZdr) {
    FilterUtils::applyMedianFilter(_zdr, nGates, _zdrMedianFilterLen);
  }
  if (_applyMedianFilterToLdr) {
    FilterUtils::applyMedianFilter(_ldr, nGates, _ldrMedianFilterLen);
  }
  if (_applyMedianFilterToRhohv) {
    FilterUtils::applyMedianFilter(_rhohv, nGates, _rhohvMedianFilterLen);
  }

  // compute PID on all gates

  for (int igate = 0; igate < nGates; igate++) {

    // compute pid

    computePid(_snr[igate], _dbz[igate], 
               _tempC[igate], _zdr[igate], _kdp[igate],
               _ldr[igate], _rhohv[igate], _sdzdr[igate], _sdphidp[igate],
               _pid[igate], _interest[igate], _pid2[igate], _interest2[igate],
               _confidence[igate]);

    // save interest value for each particle type

    for (int ii = 0; ii < (int) _particleList.size(); ii++) {
      _particleList[ii]->gateInterest[igate] =
        _particleList[ii]->meanWeightedInterest;
    }

    // set the category
    
    switch (_pid[igate]) {
      case NcarParticleId::HAIL:
      case NcarParticleId::RAIN_HAIL_MIXTURE:
      case NcarParticleId::GRAUPEL_SMALL_HAIL:
        _category[igate] = CATEGORY_HAIL;
        break;
      case NcarParticleId::GRAUPEL_RAIN:
      case NcarParticleId::WET_SNOW:
        _category[igate] = CATEGORY_MIXED;
        break;
      case NcarParticleId::DRY_SNOW:
      case NcarParticleId::ICE_CRYSTALS:
      case NcarParticleId::IRREG_ICE_CRYSTALS:
        _category[igate] = CATEGORY_ICE;
        break;
      case NcarParticleId::DRIZZLE:
      case NcarParticleId::LIGHT_RAIN:
      case NcarParticleId::MODERATE_RAIN:
      case NcarParticleId::HEAVY_RAIN:
      case NcarParticleId::SUPERCOOLED_DROPS:
      default:
        _category[igate] = CATEGORY_RAIN;
    }

  } // igate

  // apply median filter to pid
  
  if (_applyMedianFilterToPid) {
    FilterUtils::applyMedianFilter(_pid, nGates, _pidMedianFilterLen);
    FilterUtils::applyMedianFilter(_pid2, nGates, _pidMedianFilterLen);
  }

  // compute melting layer
  
  if (_computeMl) {
    _mlCompute();
  }
  
}

/////////////////////////////////////////////////////////
// compute PID for a single gate, and related interest value.
// also compute second most likely pid and related interest value.

void NcarParticleId::computePid(double snr,
                                double dbz,
                                double tempC,
                                double zdr,
                                double kdp,
                                double ldr,
                                double rhohv,
                                double sdzdr,
                                double sdphidp,
                                int &pid,
                                double &interest,
                                int &pid2,
                                double &interest2,
                                double &confidence)

{

  // compute interest for each particle type
  
  for (int ii = 0; ii < (int) _particleList.size(); ii++) {
    _particleList[ii]->computeInterest(dbz, tempC, zdr, kdp, ldr,
                                       rhohv, sdzdr, sdphidp);
  }
  
  // find the particle ID with the max interest
  
  double maxInterest = 0.0;
  int idForMax = 0;

  double maxInterest2 = 0.0;
  int idForMax2 = 0;

  for (int ii = (int) _particleList.size() - 1; ii >= 0; ii--) {
    if (fabs(_ldrWt) < 0.0001 && _particleList[ii] == _trip2) {
      // if no LDR, cannot determine second trip
      continue;
    }
    if (_particleList[ii]->meanWeightedInterest > maxInterest) {
      idForMax2 = idForMax;
      maxInterest2 = maxInterest;
      idForMax = _particleList[ii]->id;
      maxInterest = _particleList[ii]->meanWeightedInterest;
    }
  }

  interest = maxInterest;
  if (interest >= _minValidInterest) {
    pid = idForMax;
  } else {
    pid = 0;
  }

  interest2 = maxInterest2;
  if (interest2 >= _minValidInterest) {
    pid2 = idForMax2;
  } else {
    pid2 = 0;
  }

  confidence = interest - interest2;

  // for high SMR values, override

  if (snr > _snrUpperThreshold) {
    pid2 = pid;
    pid = SATURATED_SNR;
    interest2 = interest;
    interest = 1.0;
  }

}

/////////////////////////
// allocate local arrays

void NcarParticleId::_allocArrays(int nGates)
  
{

  _nGates = nGates;
  _snr = _snr_.alloc(nGates);
  _dbz = _dbz_.alloc(nGates);
  _zdr = _zdr_.alloc(nGates);
  _kdp = _kdp_.alloc(nGates);
  _ldr = _ldr_.alloc(nGates);
  _rhohv = _rhohv_.alloc(nGates);
  _phidp = _phidp_.alloc(nGates);
  _tempC = _tempC_.alloc(nGates);
  _pid = _pid_.alloc(nGates);
  _category = _category_.alloc(nGates);
  _interest = _interest_.alloc(nGates);
  _pid2 = _pid2_.alloc(nGates);
  _interest2 = _interest2_.alloc(nGates);
  _confidence = _confidence_.alloc(nGates);
  _sdzdr = _sdzdr_.alloc(nGates);
  _sdphidp = _sdphidp_.alloc(nGates);
  _cflags = _cflags_.alloc(nGates);

  _mlInterest = _mlInterest_.alloc(nGates);

}

/////////////
// set the ID

int NcarParticleId::_setId(Particle *part, const char *line)
  
{

  string searchStr = "pid.";
  searchStr += part->label;
  
  if (strstr(line, searchStr.c_str()) == NULL) {
    return -1;
  }

  const char *openParens = strchr(line, '(');
  if (openParens == NULL) {
    return -1;
  }

  int val;
  if (sscanf(openParens + 1, "%d", &val) != 1) {
    return -1;
  }

  part->id = val;

  return 0;

}

/////////////////////////////////////////////////////
// set the temperature profile, from thresholds file

int NcarParticleId::_parseTempProfileLine(const char *line)
  
{

  // find the first and last paren

  const char *firstOpenParen = strchr(line, '(');
  const char *lastCloseParen = strrchr(line, ')');

  if (firstOpenParen == NULL || lastCloseParen == NULL) {
    return -1;
  }
  
  string sdata(firstOpenParen, lastCloseParen - firstOpenParen + 1);

  // tokenize the line on '.'

  vector<string> toks;
  TaStr::tokenize(sdata, "()", toks);
  if (toks.size() < 2) {
    return -1;
  }

  // scan in profile data

  vector<TempProfile::PointVal> profile;
  for (int ii = 0; ii < (int) toks.size(); ii++) {
    double ht, tmp;
    if (sscanf(toks[ii].c_str(), "%lg,%lg", &ht, &tmp) == 2) {
      TempProfile::PointVal tmpPt(ht, tmp);
      profile.push_back(tmpPt); 
    }
  }

  _tempProfile.setProfile(profile);

  if (_verbose) {
    cerr << "NcarParticleId::_parseTempProfileLine()" << endl;
    for (size_t ii = 0; ii < profile.size(); ii++) {
      cerr << "==>> press, htKm, tmpC: " 
           << profile[ii].pressHpa << ", "
           << profile[ii].htKm << ", "
           << profile[ii].tmpC << endl;
    }
  }
    
  // compute the temperature height lookup table

  _computeTempHtLookup();

  return 0;

}

/////////////////////////////////////////////////////
// compute temperature/ht lookup 

void NcarParticleId::_computeTempHtLookup()
  
{

  const vector<TempProfile::PointVal> &profile = _tempProfile.getProfile();
  
  _tmpMinHtMeters =
    (int) (profile[0].htKm * 1000.0 + 0.5);
  _tmpMaxHtMeters =
    (int) (profile[profile.size()-1].htKm * 1000.0 + 0.5);

  _tmpBottomC = profile[0].tmpC;
  _tmpTopC = profile[profile.size()-1].tmpC;

  // fill out temp array, every meter

  int nHt = (_tmpMaxHtMeters - _tmpMinHtMeters) + 1;
  _tempHtLookup_.free();
  _tempHtLookup = _tempHtLookup_.alloc(nHt);
  
  for (int ii = 1; ii < (int) profile.size(); ii++) {

    int minHtMeters = (int) (profile[ii-1].htKm * 1000.0 + 0.5);
    double minTmp = profile[ii-1].tmpC;

    int maxHtMeters = (int) (profile[ii].htKm * 1000.0 + 0.5);
    double maxTmp = profile[ii].tmpC;

    double deltaMeters = maxHtMeters - minHtMeters;
    double deltaTmp = maxTmp - minTmp;
    double gradient = deltaTmp / deltaMeters;
    double tmp = minTmp;
    int kk = minHtMeters - _tmpMinHtMeters;
    
    for (int jj = minHtMeters; jj <= maxHtMeters; jj++, kk++, tmp += gradient) {
      if (kk >= 0 && kk < nHt) {
	_tempHtLookup[kk] = tmp;
      }
    }

  }

}

///////////////////////////////////////////////////////////
// Load the temperature profile for the specified time.
// This reads in a new sounding, if available, if the time has changed
// by more that 900 secs. If this fails, the profile from
// the thresholds file is used.

int NcarParticleId::loadTempProfile(time_t dataTime)
  
{

  double secsSincePrev
    = fabs((double) _prevProfileDataTime - (double) dataTime);
  if (secsSincePrev < 300) {
    return 0;
  }
  
  // get profile from spdb sounding if appropriate

  if (_params.PID_use_soundings_from_spdb) {

    if (_getTempProfileFromSpdb(dataTime) == 0) {
      _prevProfileDataTime = dataTime;
      return 0;
    }

  } // if (_params.PID_use_soundings_from_spdb)
  
  if (_debug) {
    cerr << "WARNING - NcarParticleId::loadTempProfile" << endl;
    cerr << "  Cannot retrieve sounding, url: "
         << _params.PID_sounding_spdb_url << endl;
    cerr << "                     dataTime: "
         << DateTime::strm(dataTime) << endl;
    cerr << "  Using thresholds file instead: "
         << _params.PID_thresholds_file_path << endl;
  }
  
  // alternatively get profile from thresholds file
  
  if (_tempProfile.loadFromPidThresholdsFile
      (_params.PID_thresholds_file_path)) {
    cerr << "ERROR - NcarParticleId::loadTempProfile" << endl;
    cerr << "  Cannot retrieve temp profile from file: "
         << _params.PID_thresholds_file_path << endl;
    return -1;
  }

  // compute the temperature height lookup table
  
  _computeTempHtLookup();
  _prevProfileDataTime = dataTime;

  return 0;

}

///////////////////////////////////////////////////////////
// Read the temperature profile for the specified time.
// This reads in a new sounding, if available, if the time has changed
// by more that 300 secs.
// returns 0 on success, -1 on failure
// the thresholds file is used.

int NcarParticleId::_getTempProfileFromSpdb(time_t dataTime)
  
{
  
  _tempProfile.clear();
  
  _tempProfile.setSoundingLocationName
    (_params.PID_sounding_location_name);
  _tempProfile.setSoundingSearchTimeMarginSecs
    (_params.PID_sounding_search_time_margin_secs);
  
  _tempProfile.setCheckPressureRange
    (_params.PID_sounding_check_pressure_range);
  _tempProfile.setSoundingRequiredMinPressureHpa
    (_params.PID_sounding_required_pressure_range_hpa.min_val);
  _tempProfile.setSoundingRequiredMaxPressureHpa
    (_params.PID_sounding_required_pressure_range_hpa.max_val);
  
  _tempProfile.setCheckHeightRange
    (_params.PID_sounding_check_height_range);
  _tempProfile.setSoundingRequiredMinHeightM
    (_params.PID_sounding_required_height_range_m.min_val);
  _tempProfile.setSoundingRequiredMaxHeightM
    (_params.PID_sounding_required_height_range_m.max_val);
  
  _tempProfile.setCheckPressureMonotonicallyDecreasing
    (_params.PID_sounding_check_pressure_monotonically_decreasing);
  
  _tempProfile.setHeightCorrectionKm
    (_params.PID_sounding_height_correction_km);
  
  if (_params.PID_sounding_use_wet_bulb_temp) {
    _tempProfile.setUseWetBulbTemp(true);
  }
  
  if (_debug) {
    _tempProfile.setDebug();
  }
  if (_verbose) {
    _tempProfile.setVerbose();
  }

  time_t soundingTime = 0;
  if (_tempProfile.loadFromSpdb(_params.PID_sounding_spdb_url,
                                dataTime,
                                soundingTime)) {
    cerr << "WARNING - NcarParticleId::_getTempProfileFromSpdb" << endl;
    cerr << "  Cannot get sounding from URL: "
         << _params.PID_sounding_spdb_url << endl;
    cerr << "  dataTime: " << DateTime::strm(dataTime) << endl;
    // failure
    return -1;
  }

  // compute the temperature height lookup table
  
  _computeTempHtLookup();

  if (_debug) {
    cerr << "Retrieved sounding, url: "
         << _params.PID_sounding_spdb_url << endl;
    cerr << "               dataTime: "
         << DateTime::strm(dataTime) << endl;
    cerr << "           soundingTime: "
         << DateTime::strm(soundingTime) << endl;
  }
  if (_verbose) {
    _tempProfile.print(cerr);
  }
  
  // success

  return 0;

}

///////////////////////////////////////////////////////////
// Set the temperature profile.
// This is used to override the temperature profile in the
// thresholds file, for example from a sounding.

void NcarParticleId::setTempProfile(const TempProfile &profile)
  
{


  // store the profile

  _tempProfile = profile;

  // compute the temperature height lookup table

  _computeTempHtLookup();

}

///////////////////////////////
// set the weights

int NcarParticleId::_setWeights(const char *line)
  
{

  // find the first and last paren

  const char *firstOpenParen = strchr(line, '(');
  const char *lastCloseParen = strrchr(line, ')');

  if (firstOpenParen == NULL || lastCloseParen == NULL) {
    return -1;
  }
  
  string sdata(firstOpenParen, lastCloseParen - firstOpenParen + 1);

  // tokenize the line on '()'
  
  vector<string> toks;
  TaStr::tokenize(sdata, "()", toks);
  if (toks.size() < 1) {
    return -1;
  }

  // scan in profile data
  
  for (int ii = 0; ii < (int) toks.size(); ii++) {
    vector<string> subtoks;
    TaStr::tokenize(toks[ii], ",", subtoks);
    if (subtoks.size() >= 2) {
      string name = subtoks[0];
      double wt;
      if (sscanf(subtoks[1].c_str(), "%lg", &wt) == 1) {
	if (name == "tmp") {
	  _tmpWt = wt;
	} else if (name == "zh") {
	  _zhWt = wt;
	} else if (name == "zdr") {
	  _zdrWt = wt;
	} else if (name == "kdp") {
	  _kdpWt = wt;
	} else if (name == "ldr") {
	  _ldrWt = wt;
	} else if (name == "rhv") {
	  _rhvWt = wt;
	} else if (name == "sdzdr") {
	  _sdzdrWt = wt;
	} else if (name == "sphi") {
	  _sphiWt = wt;
	}
      }
    }
  }
  
  for (int ii = 0; ii < (int) _particleList.size(); ii++) {
    _particleList[ii]->createImapManagers(_tmpWt, _zhWt, _zdrWt, _kdpWt, _ldrWt,
                                          _rhvWt, _sdzdrWt, _sphiWt);
  }

  return 0;

}

/////////////////////////////////////////
// set limits

int NcarParticleId::_setLimits(Particle *part, const char *line)
  
{

  // search string is thr."label"
  
  string searchStr = "thr.";
  searchStr += part->label;
  
  if (strstr(line, searchStr.c_str()) == NULL) {
    return -1;
  }

  // tokenize the line on '.'

  vector<string> toks;
  TaStr::tokenize(line, ". ", toks);
  if (toks.size() < 5) {
    return -1;
  }

  // parse the value in parens

  const char *openParens = strchr(line, '(');
  if (openParens == NULL) {
    return -1;
  }

  double val;
  if (sscanf(openParens + 1, "%lg", &val) != 1) {
    return -1;
  }

  string field = toks[2];
  string comparator = toks[3];

  if (field == "zh") {
    if (comparator == "lt" || comparator == "le") {
      part->minZh = val;
    } else if (comparator == "gt" || comparator == "ge") {
      part->maxZh = val;
    }
  } else if (field == "tmp") {
    if (comparator == "lt" || comparator == "le") {
      part->minTmp = val;
    } else if (comparator == "gt" || comparator == "ge") {
      part->maxTmp = val;
    }
  } else if (field == "zdr") {
    if (comparator == "lt" || comparator == "le") {
      part->minZdr = val;
    } else if (comparator == "gt" || comparator == "ge") {
      part->maxZdr = val;
    }
  } else if (field == "ldr") {
    if (comparator == "lt" || comparator == "le") {
      part->minLdr = val;
    } else if (comparator == "gt" || comparator == "ge") {
      part->maxLdr = val;
    }
  } else if (field == "sdzdr") {
    if (comparator == "lt" || comparator == "le") {
      part->minSdZdr = val;
    } else if (comparator == "gt" || comparator == "ge") {
      part->maxSdZdr = val;
    }
  } else if (field == "rhv") {
    if (comparator == "lt" || comparator == "le") {
      part->minRhv = val;
    } else if (comparator == "gt" || comparator == "ge") {
      part->maxRhv = val;
    }
  } else if (field == "kdp") {
    if (comparator == "lt" || comparator == "le") {
      part->minKdp = val;
    } else if (comparator == "gt" || comparator == "ge") {
      part->maxKdp = val;
    }
  }

  return 0;

}

/////////////////////////////////////////
// set interest maps

int NcarParticleId::_setInterestMaps(Particle *part, const char *line)
  
{

  // tokenize the line on '.'
  
  vector<string> toks;
  TaStr::tokenize(line, ". ", toks);
  if (toks.size() < 5) {
    return -1;
  }

  string field = toks[2];
  string particleType = toks[3];

  // check that the particle type is correct
  
  if (particleType != part->label) {
    return -1;
  }

  // find the first and last paren

  const char *firstOpenParen = strchr(line, '(');
  const char *lastCloseParen = strrchr(line, ')');
  
  if (firstOpenParen == NULL || lastCloseParen == NULL) {
    return -1;
  }
  
  string sdata(firstOpenParen, lastCloseParen - firstOpenParen + 1);
  
  // tokenize the line on '()'
  
  TaStr::tokenize(sdata, "()", toks);
  if (toks.size() < 3) {
    return -1;
  }

  // scan in Zh limits

  double minZh, maxZh;
  if (sscanf(toks[0].c_str(), "%lg,%lg", &minZh, &maxZh) != 2) {
    return -1;
  }
  
  // scan in interest map data

  vector<PidInterestMap::ImPoint> map;
  for (int ii = 1; ii < (int) toks.size(); ii++) {
    double xx, yy;
    if (sscanf(toks[ii].c_str(), "%lg,%lg", &xx, &yy) == 2) {
      PidInterestMap::ImPoint pt(xx, yy);
      map.push_back(pt);
    }
  }

  // add the interest map

  part->addInterestmap(field, minZh, maxZh, map);
  
  return 0;

}

//////////////////////////////////////////////
// fill temperature array, based on height

void NcarParticleId::fillTempArray(double radarHtKm,
                                   bool setPseudoRadiusRatio,
                                   double pseudoRadiusRatio,
                                   double elevDeg, int nGates,
				   double startRangeKm, double gateSpacingKm,
                                   double *tempC)
  
{
  
  _startRangeKm = startRangeKm;
  _gateSpacingKm = gateSpacingKm;

  BeamHeight beamHt;
  beamHt.setInstrumentHtKm(radarHtKm);
  if (setPseudoRadiusRatio) {
    beamHt.setPseudoRadiusRatio(pseudoRadiusRatio);
  }
  double rangeKm = startRangeKm;
  for (int ii = 0; ii < nGates; ii++, rangeKm += gateSpacingKm) {
    double htKm = beamHt.computeHtKm(elevDeg, rangeKm);
    tempC[ii] = getTmpC(htKm);
  }

}
    
/////////////////////////////////////////////////////////
// print

void NcarParticleId::print(ostream &out)

{

  out << "==== NcarParticleId object ====" << endl;

  out << "--- Particle type parameters ---" << endl;
  for (int ii = 0; ii < (int) _particleList.size(); ii++) {
    _particleList[ii]->print(out);
  }

  const vector<TempProfile::PointVal> &profile = _tempProfile.getProfile();
  out << "--- Temperature profile ---" << endl;
  for (size_t ii = 0; ii < profile.size(); ii++) {
    profile[ii].print(out);
  }
  
  out << "--- Weights ---" << endl;
  out << "  tmpWt: " << _tmpWt << endl;
  out << "  zhWt: " << _zhWt << endl;
  out << "  zdrWt: " << _zdrWt << endl;
  out << "  kdpWt: " << _kdpWt << endl;
  out << "  ldrWt: " << _ldrWt << endl;
  out << "  rhvWt: " << _rhvWt << endl;
  out << "  sdzdrWt: " << _sdzdrWt << endl;
  out << "  sphiWt: " << _sphiWt << endl;

}

//////////////////////////////////////////////
// Initialze melting layer computations

void NcarParticleId::_mlInit()
  
{

  _computeMl = false;
  _mlDbzInterest = NULL;
  _mlZdrInterest = NULL;
  _mlRhohvInterest = NULL;
  _mlTempInterest = NULL;

  vector<InterestMap::ImPoint> pts;

  // dbz interest map

  pts.clear();
  pts.push_back(InterestMap::ImPoint(25.0, 0.0));
  pts.push_back(InterestMap::ImPoint(30.0, 1.0));
  pts.push_back(InterestMap::ImPoint(47.0, 1.0));
  pts.push_back(InterestMap::ImPoint(52.0, 0.0));
  _mlDbzInterest = new InterestMap("dbz4ml", pts, 1.0);
  _mlDbzInterest->setMissingValue(_missingDouble);

  // zdr interest map

  pts.clear();
  pts.push_back(InterestMap::ImPoint(0.60, 0.0));
  pts.push_back(InterestMap::ImPoint(0.80, 1.0));
  pts.push_back(InterestMap::ImPoint(2.50, 1.0));
  pts.push_back(InterestMap::ImPoint(2.70, 0.0));
  _mlZdrInterest = new InterestMap("zdr4ml", pts, 1.0);
  _mlZdrInterest->setMissingValue(_missingDouble);

  // rhohv interest map

  pts.clear();
  pts.push_back(InterestMap::ImPoint(0.85, 0.0));
  pts.push_back(InterestMap::ImPoint(0.90, 1.0));
  pts.push_back(InterestMap::ImPoint(0.97, 1.0));
  pts.push_back(InterestMap::ImPoint(0.972, 0.0));
  _mlRhohvInterest = new InterestMap("rhohv4ml", pts, 1.0);
  _mlRhohvInterest->setMissingValue(_missingDouble);

  // temp interest map

  pts.clear();
  pts.push_back(InterestMap::ImPoint(-5.0, 0.0));
  pts.push_back(InterestMap::ImPoint(-1.0, 1.0));
  pts.push_back(InterestMap::ImPoint(1.0, 1.0));
  pts.push_back(InterestMap::ImPoint(5.0, 0.0));
  _mlTempInterest = new InterestMap("temp4ml", pts, 1.0);
  _mlTempInterest->setMissingValue(_missingDouble);

}

////////////////////////////////////////////////////////////
// compute Melting Layer
//
// Follows Giangrande et al. - Automatic Designation of the
// Melting Layer with Polarimitric Prototype of WSR-88D Radar.
// AMS JAMC, Vol47, 2008.

void NcarParticleId::_mlCompute()
  
{

  for (int igate = 0; igate < _nGates; igate++) {

    // check for missing values

    _mlInterest[igate] = _missingDouble;
    double dbz = _dbz[igate];
    double zdr = _zdr[igate];
    double rhohv = _rhohv[igate];

    if (dbz == _missingDouble ||
        zdr == _missingDouble ||
        rhohv == _missingDouble) {
      continue;
    }

    // check for PID type

    int pidVal = _pid[igate];
    if (pidVal > 14) {
      // not a cloud particle, so set to 0
      _mlInterest[igate] = 0.0;
      continue;
    }

    // check temperature

    // double temp = _tempC[igate];
    // if (temp < -2 || temp > 2) {
    //   continue;
    // }

    // compute interest

    double sumInterest = 0.0;
    double sumWt = 0.0;

    _mlDbzInterest->accumWeightedInterest(dbz, sumInterest, sumWt, -1);
    _mlZdrInterest->accumWeightedInterest(zdr, sumInterest, sumWt, -1);
    _mlRhohvInterest->accumWeightedInterest(rhohv, sumInterest, sumWt, -1);
    _mlTempInterest->accumWeightedInterest(_tempC[igate], sumInterest, sumWt, -1);

    _mlInterest[igate] = sumInterest / sumWt;

    // double dbzInt = _mlDbzInterest->getInterest(dbz);
    // double dbzWt = _mlDbzInterest->getWeight();

    // double zdrInt = _mlZdrInterest->getInterest(zdr);
    // double zdrWt = _mlZdrInterest->getWeight();

    // double rhohvInt = _mlRhohvInterest->getInterest(rhohv);
    // double rhohvWt = _mlRhohvInterest->getWeight();

    // if (dbz > 30) {
    //   cerr << "11111111: "
    //        << dbz << ", " << dbzInt << ", " << dbzWt << " # "
    //        << zdr << ", " << zdrInt << ", " << zdrWt << " #  "
    //        << rhohv << ", " << rhohvInt << ", " << rhohvWt << " # "
    //        << sumInterest << ", " << sumWt << ", " 
    //        << _mlInterest[igate] << ", " << _missingDouble << endl;
    // }

  } // igate

}
    
//////////////////////////////////////////////////////////////
// Particle interior class

// Constructor

NcarParticleId::Particle::Particle(const string &lbl, const string desc,
                                   int idd)

{

  label = lbl;
  description = desc;
  id = idd;
  
  minZh = -1.0e99;
  maxZh = 1.0e99;
  minTmp = -1.0e99;
  maxTmp = 1.0e99;
  minZdr = -1.0e99;
  maxZdr = 1.0e99;
  minLdr = -1.0e99;
  maxLdr = 1.0e99;
  minSdZdr = -1.0e99;
  maxSdZdr = 1.0e99;
  minRhv = -1.0e99;
  maxRhv = 1.0e99;
  minKdp = -1.0e99;
  maxKdp = 1.0e99;

  sumWeightedInterest = 0.0;
  sumWeights = 0.0;
  meanWeightedInterest = 0.0;

}

/////////////////////////////////////////////////////////
// alloc interest array for gate data

void NcarParticleId::Particle::allocGateInterest(int nGates)
{
  gateInterest = gateInterest_.alloc(nGates);
  memset(gateInterest, 0, nGates * sizeof(double));
}

/////////////////////////////////////////////////////////
// destructor

NcarParticleId::Particle::~Particle()

{

  for (int ii = 0; ii < (int) _imaps.size(); ii++) {
    delete _imaps[ii];
  }
  _imaps.clear();

}

/////////////////////////////////////////////////////////
// create interest map managers

void NcarParticleId::Particle::createImapManagers(double tmpWt,
						  double zhWt,
						  double zdrWt,
						  double kdpWt,
						  double ldrWt,
						  double rhvWt,
						  double sdzdrWt,
						  double sphiWt)

{

  for (int ii = 0; ii < (int) _imaps.size(); ii++) {
    delete _imaps[ii];
  }
  _imaps.clear();

  _imapZh = new PidImapManager(label, description, "zh", zhWt, _missingDouble);
  _imaps.push_back(_imapZh);

  _imapZdr = new PidImapManager(label, description, "zdr", zdrWt, _missingDouble);
  _imaps.push_back(_imapZdr);

  _imapLdr = new PidImapManager(label, description, "ldr", ldrWt, _missingDouble);
  _imaps.push_back(_imapLdr);

  _imapKdp = new PidImapManager(label, description, "kdp", kdpWt, _missingDouble);
  _imaps.push_back(_imapKdp);

  _imapRhohv = new PidImapManager(label, description, "rhv", rhvWt, _missingDouble);
  _imaps.push_back(_imapRhohv);

  _imapTmp = new PidImapManager(label, description, "tmp", tmpWt, _missingDouble);
  _imaps.push_back(_imapTmp);

  _imapSdZdr = new PidImapManager(label, description, "sdzdr", zhWt, _missingDouble);
  _imaps.push_back(_imapSdZdr);

  _imapSdPhidp = new PidImapManager(label, description, "sphi", sphiWt, _missingDouble);
  _imaps.push_back(_imapSdPhidp);

}

/////////////////////////////////////////////////////////
// add interest map

void NcarParticleId::Particle::addInterestmap(const string &field,
					      double minZh,
					      double maxZh,
					      const vector<PidInterestMap::ImPoint> map)
  
{

  if (field == "zh") {
    _imapZh->addInterestMap(minZh, maxZh, map);
  } else if (field == "zdr") {
    _imapZdr->addInterestMap(minZh, maxZh, map);
  } else if (field == "ldr") {
    _imapLdr->addInterestMap(minZh, maxZh, map);
  } else if (field == "kdp") {
    _imapKdp->addInterestMap(minZh, maxZh, map);
  } else if (field == "rhv") {
    _imapRhohv->addInterestMap(minZh, maxZh, map);
  } else if (field == "tmp") {
    _imapTmp->addInterestMap(minZh, maxZh, map);
  } else if (field == "sdzdr") {
    _imapSdZdr->addInterestMap(minZh, maxZh, map);
  } else if (field == "sphi") {
    _imapSdPhidp->addInterestMap(minZh, maxZh, map);
  }

}

/////////////////////////////////////////////////////////
// compute interest

void NcarParticleId::Particle::computeInterest(double dbz,
					       double tempC,
					       double zdr,
					       double kdp,
					       double ldr,
					       double rhohv,
					       double sdzdr,
					       double sdphidp)

{

  // initialize

  sumWeightedInterest = 0.0;
  sumWeights = 0.0;
  meanWeightedInterest = 0.0;

  // check limits

  if (_imapZh->getWeight() > 0) {
    if (dbz == _missingDouble) {
      return;
    } else if (dbz < minZh || dbz > maxZh) {
      return;
    }
  }
  
  if (_imapTmp->getWeight() > 0) {
    if (tempC == _missingDouble) {
      return;
    } else if (tempC < minTmp || tempC > maxTmp) {
      return;
    }
  }

  if (_imapZdr->getWeight() > 0) {
    if (zdr == _missingDouble) {
      return;
    } else if (zdr < minZdr || zdr > maxZdr) {
      return;
    }
  }

  if (_imapLdr->getWeight() > 0) {
    if (ldr < minLdr || ldr > maxLdr) {
      return;
    }
  }

  if (_imapKdp->getWeight() > 0) {
    if (kdp == _missingDouble) {
      return;
    }
    if (kdp < minKdp || kdp > maxKdp) {
      return;
    }
  }

  if (_imapRhohv->getWeight() > 0) {
    if (rhohv == _missingDouble) {
      return;
    }
    if (rhohv < minRhv || rhohv > maxRhv) {
      return;
    }
  }
  
  if (_imapSdZdr->getWeight() > 0) {
    if (sdzdr == _missingDouble) {
      return;
    } else if (sdzdr < minSdZdr || sdzdr > maxSdZdr) {
      return;
    }
  }

  if (_imapSdPhidp->getWeight() > 0) {
    if (sdphidp == _missingDouble) {
      return;
    }
  }
      
  _imapZh->accumWeightedInterest(dbz, dbz, sumWeightedInterest, sumWeights);
  _imapTmp->accumWeightedInterest(dbz, tempC, sumWeightedInterest, sumWeights);
  _imapZdr->accumWeightedInterest(dbz, zdr, sumWeightedInterest, sumWeights);
  _imapLdr->accumWeightedInterest(dbz, ldr, sumWeightedInterest, sumWeights);
  _imapKdp->accumWeightedInterest(dbz, kdp, sumWeightedInterest, sumWeights);
  _imapRhohv->accumWeightedInterest(dbz, rhohv, sumWeightedInterest, sumWeights);
  _imapSdZdr->accumWeightedInterest(dbz, sdzdr, sumWeightedInterest, sumWeights);
  _imapSdPhidp->accumWeightedInterest(dbz, sdphidp, sumWeightedInterest, sumWeights);
  if (sumWeights > 0) {
    meanWeightedInterest = sumWeightedInterest / sumWeights;
  }

}

/////////////////////////////////////////////////////////
// print

void NcarParticleId::Particle::print(ostream &out)

{

  out << "====================>>>> particle <<<<=======================" << endl;
  out << "label: " << label << endl;
  out << "  description: " << description << endl;
  out << "  id: " << id << endl;

  if (minZh != -9999) {
    out << "  minZh: " << minZh << endl;
  }
  if (maxZh != 9999) {
    out << "  maxZh: " << maxZh << endl;
  }
  if (minTmp != -9999) {
    out << "  minTmp: " << minTmp << endl;
  }
  if (maxTmp != 9999) {
    out << "  maxTmp: " << maxTmp << endl;
  }
  if (minZdr != -9999) {
    out << "  minZdr: " << minZdr << endl;
  }
  if (maxZdr != 9999) {
    out << "  maxZdr: " << maxZdr << endl;
  }
  if (minLdr != -9999) {
    out << "  minLdr: " << minLdr << endl;
  }
  if (maxLdr != 9999) {
    out << "  maxLdr: " << maxLdr << endl;
  }
  if (minSdZdr != -9999) {
    out << "  minSdZdr: " << minSdZdr << endl;
  }
  if (maxSdZdr != 9999) {
    out << "  maxSdZdr: " << maxSdZdr << endl;
  }
  if (minRhv != -9999) {
    out << "  minRhv: " << minRhv << endl;
  }
  if (maxRhv != 9999) {
    out << "  maxRhv: " << maxRhv << endl;
  }
  if (minKdp != -9999) {
    out << "  minKdp: " << minKdp << endl;
  }
  if (maxKdp != 9999) {
    out << "  maxKdp: " << maxKdp << endl;
  }

  out << "--- Interest maps -----------------------" << endl;
  _imapZh->print(out);
  _imapZdr->print(out);
  _imapLdr->print(out);
  _imapKdp->print(out);
  _imapRhohv->print(out);
  _imapTmp->print(out);
  _imapSdZdr->print(out);
  _imapSdPhidp->print(out);
  out << "-----------------------------------------" << endl;

  out << "=============================================================" << endl;

}

