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
// SweepData - class that contains and manages data
//            specific to a single tilt or sweep
//
// Jason Craig, RAP, NCAR, P.O.Box 3000, Boulder, CO, 
//   80307-3000, USA
//
// $Id: SweepData.cc,v 1.12 2016/03/07 01:23:01 dixon Exp $
//
/////////////////////////////////////////////////////////

#include <cmath>

#include <toolsa/MsgLog.hh>
#include "Beam.hh"
#include "SweepData.hh"

const double SweepData::SPEED_OF_LIGHT  = 299792458;

SweepData::SweepData( Params *tdrpParams )
{
  _params = tdrpParams;

  _sur = false;
  _dop = false;
  _sw = false;

  //
  // Set data members that won't change over lifetime of
  // object
  //    
  _maxBeamQueueSize   = _params->recKernelAzimuthWidth;
  _midBeamIndex       = _maxBeamQueueSize / 2;
  _azTolerance        = _params->azimuthTolerance;
  _elevTolerance      = _params->elevationTolerance;
  _timeTolerance      = _params->timeTolerance;
  
  //
  // See if user requested to calc snr, pr, or rec
  //
  _calcSnr = false;
  _calcPr = false;
  _calcRec = false;
  _merged = false;

  //
  // Initialize data members to zero
  //
  _scantime = 0;
  _sur_prf = -999.0;
  _sur_antspeed = -999;
  _dop_prf = -999.0;
  _dop_antspeed = -999;
  
  for( int ii = 0; ii < _params->derivedFieldDefs_n; ii++ ) {
    switch( _params->_derivedFieldDefs[ii].outputField ) {
    case Params::SNR :
      _calcSnr = true;
      break;
      
    case Params::PR :
      _calcPr = true;
      break;
      
    case Params::REC :
      _calcRec = true;
    }
  }
   
  clear();
}

SweepData::~SweepData() 
{
  clear();
}

void SweepData::clear() 
{
  _beamQueue.clear();

  vector< Beam* >::iterator it;
  for( it = _beams.begin(); it != _beams.end(); it++ ) {
    delete (*it);
  }
  _beams.erase( _beams.begin(), _beams.end() );
  
  _sur = false;
  _dop = false;
  _sw = false;

}

void SweepData::finishSweep()
{
  computeRec();
  computeMinDbz0();
}

void SweepData::mergeDbz(SweepData *prevSweep)
{
  if(_long_pulse != 0 || prevSweep->getLongPulse() != 1 || prevSweep->getElevation() != _fixedAngle)
    return;
  _sur = true;
  _sur_pulsecount = prevSweep->getSurPulseCount();
  _sur_prf = prevSweep->getSurPrf();
  _unambiguousRange = prevSweep->getUnambiguousRange();

  for(int b = 0; b < _beams.size(); b++)
  {
    Beam *beam = prevSweep->getClosestBeam(_beams[b]->getAzimuth());
    _beams[b]->mergeDbz(beam);
  }
  computeMinDbz0();
  _merged = true;
}

void SweepData::setInfo(int elevNum, float elevation, const char *date, const char *time, int nrays,
			float bWidth, float wLen)
{
  time_t scantime = date2utime(date, time);

  if(_scantime == 0 || _scantime != scantime)
  {
    clear();
    _vcp = 21;
    _scantime = scantime;
    _beamWidth = bWidth;
    _waveLen = wLen;
    _elevIndex = elevNum;
    _fixedAngle = elevation;
    _nbeams = nrays;
  } else if(elevNum != _elevIndex || elevation != _fixedAngle || _nbeams != nrays)
  {
    printf( "Error SweepData info does not match for %s %s\n", date, time );
  }

}

// Convert date and time string to a time_t unix time
time_t SweepData::date2utime(const char *date, const char *time)
{
  struct tm ct;
    
  int ret = sscanf(date, "%4d-%02d-%02d", &ct.tm_year, &ct.tm_mon, &ct.tm_mday);
  int ret2 = sscanf(time, "%02d:%02d:%02d", &ct.tm_hour, &ct.tm_min, &ct.tm_sec);
  if(ret != 3 && ret2 != 3){
    cerr << "Error: Invalid date: " << date << " " << time 
	 << "format must be YYYY-MM-DD HH:MM:SS" << endl;
    ct.tm_year = 0;
    ct.tm_mon  = 0;
    ct.tm_mday = 0;
    ct.tm_hour = 0;
    ct.tm_min  = 0;
    ct.tm_sec  = 0;
  }
  
  ct.tm_year -= 1900;
  ct.tm_mon -= 1;
  
  time_t utime = timegm(&ct);
  return utime;
}


void SweepData::setData(ReadGemtronik::VolumeEnum_t dataType, unsigned char* data,
			unsigned long int dataSize, float *rays,
			int nrays, int nbins, float datamin, float datascale,
			float startrange, float rangestep, const char *date, const char *time,
			int pulsecount, float stagger, int prf, float snr_thresh, int pulse)
{
  _long_pulse = pulse;
  time_t scantime = date2utime(date, time);
  double timeDiff = fabs(_scantime - scantime);
  if(timeDiff > _timeTolerance ) {
    POSTMSG( WARNING, "No scan within time tolerance to merge with" );
    return;
  }

  if(dataSize != nrays * nbins) {
    POSTMSG( ERROR, "DataSize does not equal nrays * nbins. dataSize: %ld, nrays: %d, nbins: %d", 
	     dataSize, nrays, nbins);
    return;
  }

  _stagger = stagger;

  if(dataType == ReadGemtronik::REFLECTIVITY) {
    _sur = true;
    _sur_pulsecount = pulsecount;
    _sur_prf = (float)prf;
    _unambiguousRange = (SPEED_OF_LIGHT / ( 2 * (float)prf) ) / 1000.0;
    _nyquistVelocity = _waveLen*(float)prf/4;
  } 

  if(dataType == ReadGemtronik::VELOCITY)  {
    _dop = true;
    _dop_pulsecount = pulsecount;
    _dop_prf = (float)prf;
  }

  if(dataType == ReadGemtronik::SPECTRUMWIDTH)  {
    _sw = true;
  }

  if(_beams.size() == 0)
    for(int a = 0; a < nrays; a++)
      _beams.push_back( new Beam(_params, rays[a]) );

  for(int a = 0; a < nrays; a++)
    {
      Beam* beam = getClosestBeam(rays[a]);
      if(beam == NULL)
      {
	POSTMSG( DEBUG, "Failed to Merge Beam. azimuth: %f", rays[a]);   
      } else 
	{
	  beam->addData(dataType, nbins, datamin, datascale, 
			startrange, rangestep, snr_thresh, data+(a*nbins));
	}

    }

}

bool SweepData::complete()
{
  if(_sur && _dop && _sw)
    return true;
  return false;
}

void SweepData::computeMinDbz0()
{
  float minSnrPc = 999.99;
  for(int a = 0; a < _beams.size(); a++)
  {
    float snrPc = (_beams[a])->computeMinDbz0();
    if(snrPc < minSnrPc)
      minSnrPc = snrPc;
  }
  _dbz0 = minSnrPc;
}

void SweepData::computeRec()
{
  if(_calcRec)
  {
    for(int a = 0; a < _beams.size(); a++)
    {
      if ( (int) _beamQueue.size() == _maxBeamQueueSize ) {
	_beamQueue.pop_back();
      }
      _beamQueue.push_front( _beams[a] );
      
      if( (int) _beamQueue.size() == _maxBeamQueueSize ) {
	_beamQueue[_midBeamIndex]->computeRec( _beamQueue, _midBeamIndex );
      }
    }

    fillRec();
    _beamQueue.clear();
  }
}

void SweepData::fillRec() 
{
  //
  // If we are not computing the rec, don't do anything
  //
  if( !_calcRec) {
    return;
  }
  
  //
  // Find out if we had overlap
  //
  int n = (int) _beams.size();
  
  float prevAz     = (_beams[0])->getAzimuth();
  float sumOfDiffs = 0.0;
  
  for( int i = 1; i < n; i++ ) {
    float currentAz = (_beams[i])->getAzimuth();
    
    //
    // Perform "special" mod - fmodf is rounding towards
    // zero, rather than flooring
    //
    float quot = floorf( (currentAz-prevAz) / 360.0 );
    float currentDiff = (currentAz-prevAz) - (quot * 360.0);
    
    sumOfDiffs += currentDiff;
    
    prevAz = currentAz;
  }
  
  //fprintf( stderr, "sumOfDiffs = %f\n", sumOfDiffs );
  
  
  //
  // If we did not have overlap in the beam azimuths...
  //
  if( sumOfDiffs <= 359.5 ) {
    
    float firstAzimuth = (_beams[0])->getAzimuth();
    float lastAzimuth  = (_beams[n-1])->getAzimuth();
    
    //
    // If there was not too large of a gap between the first
    // and last beam azimuths...
    //
    if( fabs( lastAzimuth - firstAzimuth) < 1.5 * _beamWidth ) {
      
      //
      // Compute the rec for the azimuths that we were not
      // able to before (because of the lack of azimuth overlap)
      // by wrapping around to the start of the list of beams.
      //   Note: This assumes the beamQueue was not cleared out
      //   after the last beam was done, but before the sweep
      //   was written out.
      //
      int nMissing = 2 * ((int) (_maxBeamQueueSize / 2));
      
      for( int i = 0; i < nMissing; i++ ) {

	if ( (int) _beamQueue.size() == _maxBeamQueueSize ) {
	  _beamQueue.pop_back();
	}
	_beamQueue.push_front( _beams[i] );
	
	if( (int) _beamQueue.size() == _maxBeamQueueSize ) {
	  _beamQueue[_midBeamIndex]->computeRec( _beamQueue, _midBeamIndex );
	}
      }
    }
  }
  
}

Beam* SweepData::getClosestBeam(float currentAz) 
{
  int closestIndex = -1;
  float closestAzimuthDiff = 999.99;
  float azimuthDiff;

  for(int a = 0; a < _beams.size(); a++) {
    azimuthDiff = fabs(currentAz - _beams[a]->getAzimuth());
    if(azimuthDiff < closestAzimuthDiff) {
      closestIndex = a;
      closestAzimuthDiff = azimuthDiff;
    }

  }

  if(closestIndex == -1) {
    POSTMSG( WARNING, "No beams with which to merge" );
    return( NULL );
  }

  if(closestAzimuthDiff > _azTolerance ) {
    POSTMSG( WARNING, "No beams within azimuth tolerance to merge with" );
    return( NULL );
  }
  
   return( _beams[closestIndex] );    
}
