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
// Beam - class that holds beam data
//
// $Id: Beam.cc,v 1.5 2016/03/07 01:23:00 dixon Exp $
//
///////////////////////////////////////////////////////////////

#include <cmath>

#include <toolsa/DateTime.hh>
#include "Beam.hh"


InterestMap *Beam::_dbzTextureInterestMap = NULL;
InterestMap *Beam::_dbzSpinInterestMap = NULL;
InterestMap *Beam::_velInterestMap = NULL;
InterestMap *Beam::_widthInterestMap = NULL;
InterestMap *Beam::_velSdevInterestMap = NULL;

//
// Constants
//
const double Beam::DBZ_DIFF_SQ_MAX = 10000.0;
const double Beam::MISSING_DBL     = -9999.0;
const double Beam::M_TO_KM         = 0.001;

//
//   THINK CAREFULLY BEFORE SETTING THESE TO ANYTHING BUT ZERO.
//   This is the value assumed in the input data!!!
//  
const short  Beam::DBZ_BAD  = 0;
const short  Beam::VEL_BAD  = 0;
const short  Beam::SW_BAD  = 0;
const short  Beam::ZDR_BAD  = 0;
const short  Beam::PHI_BAD  = 0;
const short  Beam::RHO_BAD  = 0;
const short  Beam::SNR_BAD = 0;
const short  Beam::PR_BAD  = 0;
const short  Beam::REC_BAD = 0;

int Beam::createInterestMaps(Params *params)
{  
   //
   // Dbz texture
   //
   if (_dbzTextureInterestMap == NULL) {
      vector<InterestMap::ImPoint> pts;
      if (_convertInterestMapToVector(params->_dbzTextureInterestMap,
				      params->dbzTextureInterestMap_n,
				      pts )) {
         return -1;
      }
      _dbzTextureInterestMap =
         new InterestMap( "DbzTexture", pts,
		          params->dbzTextureInterestWeight );
      POSTMSG( DEBUG, "Creating reflectivity texture interest map" );
   }

   //
   // Dbz spin
   //
   if ( _dbzSpinInterestMap == NULL ) {
      vector<InterestMap::ImPoint> pts;
      if (_convertInterestMapToVector( params->_dbzSpinInterestMap,
				       params->dbzSpinInterestMap_n,
				       pts) ) {
         return -1;
      }
      _dbzSpinInterestMap =
         new InterestMap("DbzSpin", pts,
		         params->dbzSpinInterestWeight);

      POSTMSG( DEBUG, "Creating reflectivity spin interest map" );
   }

   //
   // Velocity
   //
   if (_velInterestMap == NULL) {
      vector<InterestMap::ImPoint> pts;
      if (_convertInterestMapToVector(params->_velInterestMap,
				      params->velInterestMap_n,
				      pts)) {
         return -1;
      }

      _velInterestMap =
         new InterestMap("velocity", pts, params->velInterestWeight);
      
      POSTMSG( DEBUG, "Creating velocity interest map" );
   }

   //
   // Spectrum Width
   //
   if (_widthInterestMap == NULL) {
      vector<InterestMap::ImPoint> pts;
      if (_convertInterestMapToVector(params->_widthInterestMap,
				      params->widthInterestMap_n,
				      pts)) {
         return -1;
      }

      _widthInterestMap =
         new InterestMap("spectrum width", pts, params->widthInterestWeight);
      
      POSTMSG( DEBUG, "Creating spectrum width interest map" );
   }

   //
   // Standard deviation of velocity
   //
   if (_velSdevInterestMap == NULL) {
      vector<InterestMap::ImPoint> pts;
      if (_convertInterestMapToVector(params->_velSdevInterestMap,
				      params->velSdevInterestMap_n,
				      pts)) {
         return -1;
      }

      _velSdevInterestMap =
         new InterestMap("velocity sdev", pts, 
                         params->velSdevInterestWeight);
      
      POSTMSG( DEBUG, "Creating standard devation of velocity interest map" );
   }

   return 0;
  
}

Beam::Beam( Params *params, float elev)
{
  _params       = params;
  _az           = elev;

  _snrThresh    = 0.0;

  _nReflGates   = 0;
  _nVelGates    = 0;
  _nSwGates     = 0;

  _rangeToFirstReflGate = 0;
  _reflGateWidth        = 0;
  _rangeToFirstVelGate  = 0;
  _velGateWidth         = 0;
  _rangeToFirstSwGate  = 0;
  _swGateWidth         = 0;

  _dbz        = NULL;
  _vel        = NULL;
  _width      = NULL;
  _zdr        = NULL;
  _phi        = NULL;
  _rho        = NULL;
  _snr        = NULL;
  _pr         = NULL;
  _rec        = NULL;

  _recDbzDiffSq = NULL;
  _recDbzSpinChange = NULL;
  _recVarsReady = false;

  _saveZdr = false;
  _savePhi = false;
  _saveRho = false;
  for( int ii = 0; ii < params->momentFieldDefs_n; ii++ ) {
    switch( params->_momentFieldDefs[ii].outputField ) {
    case Params::ZDR :
      _saveZdr = true;
      break;
    case Params::PHI :
      _savePhi = true;
      break;
    case Params::RHO :
      _saveRho = true;
      break;
    }
  }

  for( int ii = 0; ii < params->derivedFieldDefs_n; ii++ ) {
    switch( params->_derivedFieldDefs[ii].outputField ) {
    case Params::SNR :
      _snrScale = params->_derivedFieldDefs[ii].scale;
      _snrBias  = params->_derivedFieldDefs[ii].bias;
      break;
      
    case Params::PR :
      _prScale = params->_derivedFieldDefs[ii].scale;
      _prBias  = params->_derivedFieldDefs[ii].bias;
      break;
      
    case Params::REC :
      createInterestMaps(_params);
      _recScale       = params->_derivedFieldDefs[ii].scale;
      _recBias        = params->_derivedFieldDefs[ii].bias;
    }
  }

}


Beam::~Beam()
{
  if(_dbz)
    delete [] _dbz;
  if(_vel)
    delete [] _vel;
  if(_width)
    delete [] _width;
  if(_phi)
    delete [] _phi;
  if(_zdr)
    delete [] _zdr;
  if(_rho)
    delete [] _rho;
  if(_snr)
    delete [] _snr;
  if(_pr)
    delete [] _pr;
  if(_rec)
    delete [] _rec;
  if(_recDbzDiffSq)
    delete [] _recDbzDiffSq;
  if(_recDbzSpinChange)
    delete [] _recDbzSpinChange; 
}

void Beam::mergeDbz(Beam *prevBeam)
{  
  _nReflGates = prevBeam->getNReflGates();
  _dbzScale = prevBeam->getDbzScale();
  _dbzBias = prevBeam->getDbzBias();
  _rangeToFirstReflGate = prevBeam->getRangeToFirstReflGate();
  _reflGateWidth = prevBeam->getReflGateWidth();

  delete [] _dbz;
  _dbz = new unsigned char [_nReflGates];
  memcpy(_dbz, prevBeam->getDbz(), _nReflGates);
}

void Beam::addData(ReadGemtronik::VolumeEnum_t  dataType, int nbins, 
		   float datamin, float datascale, float startrange, 
		   float rangestep, float snr_thresh, unsigned char *data)
{
  _snrThresh = snr_thresh;

  if(dataType == ReadGemtronik::REFLECTIVITY)
  {
    _nReflGates = nbins;
    _dbzScale = datascale;
    _dbzBias = datamin;
    _rangeToFirstReflGate = startrange;
    _reflGateWidth = rangestep;

    _storeData( data, _nReflGates, &_dbz);

  } else if(dataType == ReadGemtronik::VELOCITY)
  {
    _nVelGates = nbins;
    _velScale = datascale;
    _velBias = datamin;
    _rangeToFirstVelGate  = startrange;
    _velGateWidth = rangestep;

    _storeData( data, _nVelGates, &_vel);

  } else if(dataType == ReadGemtronik::SPECTRUMWIDTH)
  {

    _nSwGates = nbins;
    _swScale = datascale;
    _swBias = datamin;
    _rangeToFirstSwGate  = startrange;
    _swGateWidth = rangestep;

    _storeData( data, _nSwGates, &_width);

  }

}

void Beam::_storeData(unsigned char *dataIn, int numGates, unsigned char **dataOut) 
{
  if( *dataOut )
    delete [] *dataOut;
  *dataOut = new unsigned char [numGates];

  //
  // Loop through all the values in our input data buffer
  //
  for( int i = 0; i < numGates; i++ ) {
    
    unsigned char value = dataIn[i];
    
    //
    //
    if ( value != 0 ) {
      (*dataOut)[i] = value;
    } else {
      (*dataOut)[i] = 0;
    }
    
  }  
}

float Beam::computeMinDbz0()
{
  float minSnrPc = 999.99;
  for (int igate = 0; igate < _nReflGates; igate++) {
    if(_dbz[igate] != DBZ_BAD)
    {
      float range = ((igate * _reflGateWidth) + _rangeToFirstReflGate) / 1000.0;
      float rcorrect = 20 * log10(range);
      float snrPc = (_dbz[igate] * _dbzScale ) + _dbzBias - rcorrect - _snrThresh;
      if(snrPc < minSnrPc)
	minSnrPc = snrPc;
    }
  }
  return minSnrPc;
}

void Beam::computeRec( const deque<Beam *> beams,
		       int midBeamIndex )
{
   if( _width == NULL || _reflGateWidth == 0 || _velGateWidth == 0) {
      return;
   }
   if(_rec)
     delete [] _rec;
   _rec                     = new ui08 [_nVelGates];
   double *recDbzTexture    = new double[_nVelGates];
   double *recDbzSpin       = new double[_nVelGates];
   double *recVel           = new double[_nVelGates];
   double *recVelSdev       = new double[_nVelGates];
   double *recWidth         = new double[_nVelGates];

   for (int igate = 0; igate< _nVelGates; igate++) {
     recDbzTexture[igate] = MISSING_DBL;
     recDbzSpin[igate] = MISSING_DBL;
     recVelSdev[igate] = MISSING_DBL;
     recVel[igate] = MISSING_DBL;
     recWidth[igate] = MISSING_DBL;
   }

   // Determine how many dopler bins per reflectivity bins
   int num_dop_per_refl = (int)(_reflGateWidth / _velGateWidth);
   // Veryify it is a even multiple
   if(num_dop_per_refl * _velGateWidth != _reflGateWidth)
     cerr << "Reflectivity spacing is not an even multiple of doppler bin spacing\n";

   //
   // Set limits for REC computations by checking the surrounding beams
   //
   int minBeamIndex, maxBeamIndex;
   _computeRecBeamLimits(beams, midBeamIndex, minBeamIndex, maxBeamIndex);
  
   //
   // Make sure all beams have computed the rec variables
   //
   for (int ii = minBeamIndex; ii <= maxBeamIndex; ii++) {
      beams[ii]->_computeRecVars();
   }

   //
   // Compute number of gates in kernel, making sure there is an odd number
   //
   int nGatesKernel = (int)
      (_params->recKernelRangeLen / (_velGateWidth*M_TO_KM) + 0.5);
   if ((nGatesKernel % 2) == 0) {
      nGatesKernel++;
   }
   int nGatesHalf = nGatesKernel / 2;
  
   //
   // Fields based on dbz, vel and width
   //
   for (int igate = nGatesHalf; igate < _nVelGates - nGatesHalf; igate++) {

      //
      // Compute sums etc. for stats over the kernel space
      //
      double nTexture     = 0.0;
      double sumDbzDiffSq = 0.0;

      double nSpinChange = 0.0;
      double nSpinTotal = 0.0;

      double nVel = 0.0;
      double sumVel = 0.0;
      double sumVelSq = 0.0;
    
      for (int ibeam = minBeamIndex; ibeam <= maxBeamIndex; ibeam++) {
      
         const Beam *beam = beams[ibeam];
         const double *dbzDiffSq = beam->_recDbzDiffSq;
         const double *dbzSpinChange = beam->_recDbzSpinChange;
         const ui08 *vel = beam->_vel;
      
         for (int jj = igate - nGatesHalf; jj <= igate + nGatesHalf; jj++) {

	   int jj_refl = jj / num_dop_per_refl;
            //
	    // dbz texture
            //
	    double dds = dbzDiffSq[jj_refl];
	    if (dds != MISSING_DBL) {
               sumDbzDiffSq += dds;
	       nTexture++;
	    }

            //
	    // spin
            //
	    double dsc = dbzSpinChange[jj_refl];

	    if (dsc != MISSING_DBL) {
               nSpinChange += dsc;
	       nSpinTotal++;
	    }

            //
	    // vel
            //
	    if (vel[jj] != VEL_BAD) {
	      double vv = vel[jj] * _velScale + _velBias;
	      sumVel += vv;
	      sumVelSq += (vv * vv);
	      nVel++;
	    } 
            
         } // jj
         
      } // ibeam

      //
      // texture
      //
      if (nTexture > 0)
	recDbzTexture[igate] = sumDbzDiffSq / nTexture;

      //
      // spin
      //
      if (nSpinTotal > 0)
	recDbzSpin[igate] = (nSpinChange / nSpinTotal) * 100.0;

      //
      // vel
      //
      if (nVel > 0) {
         double meanVel = sumVel / nVel;
         if (nVel > 2) {
            double term1 = sumVelSq / nVel;
	    double term2 = meanVel * meanVel;
	    if (term1 >= term2) {
               recVelSdev[igate] = sqrt(term1 - term2);
	    }
         }
      }
      if(beams[midBeamIndex]->_vel[igate] != VEL_BAD)
	recVel[igate] = beams[midBeamIndex]->_vel[igate] * _velScale + _velBias;

      //
      // width
      //
      if(beams[midBeamIndex]->_width[igate] != SW_BAD)
	recWidth[igate] = beams[midBeamIndex]->_width[igate] * _swScale + _swBias;
      
   } // igate

   //
   // compute rec clutter field
   //
   for (int igate = 0; igate < _nVelGates; igate++) {
     
     int igate_refl = igate / num_dop_per_refl;
     if( _dbz[igate_refl] == DBZ_BAD ) {
       _rec[igate] = REC_BAD;
       continue;
     }

      double sumInterest = 0.0;
      double sumWeights  = 0.0;
    
      _dbzTextureInterestMap->accumInterest(recDbzTexture[igate], sumInterest, 
                                             sumWeights);
      _dbzSpinInterestMap->accumInterest(recDbzSpin[igate], sumInterest, 
                                          sumWeights);
      _velInterestMap->accumInterest(recVel[igate], sumInterest, sumWeights);
      _widthInterestMap->accumInterest(recWidth[igate], sumInterest, sumWeights);
      _velSdevInterestMap->accumInterest(recVelSdev[igate], sumInterest, 
                                          sumWeights);
    
      if( sumWeights != 0 ) {

	double recVal    = sumInterest / sumWeights;
	double recScaled = ( recVal - _recBias ) / _recScale;
	_rec[igate] = (ui08) (recScaled + 0.5);

	/*
	if( (-.5 < recScaled) && (recScaled < 255.0) ) {
	  _rec[igate] = (ui08) (recScaled + 0.5);
	}
	else {
	  fprintf( stderr, "WARNING: REC value outside ncbyte range.  %f -> %f -> %d\n", recVal,
		   recScaled, (ui08) (recScaled + 0.5));
	  _rec[igate] = PR_BAD;
	}
	*/

      } else {
	_rec[igate] = REC_BAD;
      }
      

   } // igate

   _recDone = true;

   delete [] recDbzTexture;
   delete [] recDbzSpin;
   delete [] recVel;
   delete [] recVelSdev;
   delete [] recWidth;

}


void Beam::_computeRecBeamLimits(const deque<Beam *> beams,
				 int midBeamIndex,
				 int &minBeamIndex,
				 int &maxBeamIndex) const
{
   //
   // Initialize beam limits
   //
   minBeamIndex = midBeamIndex;
   maxBeamIndex = midBeamIndex;
  
   //
   // Get information about current beam
   //
   float prevAz = _az;

   //
   // Compute the "radius" of the azimuth kernel
   //
   float azRadius = _params->recKernelAzimuthWidth / 2.0;

   //
   // Look "behind" current beam
   //
   for (int ii = midBeamIndex - 1; ii >= 0; ii--) {

      //
      // Check number of gates
      //
      const Beam *bb = beams[ii];
      if (bb->getNVelGates() != _nVelGates) {
         break;
      }

      //
      // Check azimuth differences with previous azimuth
      //   Don't include beams that cover a gap
      //
      float az = bb->getAzimuth();
      if (fabs(_computeAzDiff(az, prevAz)) > azRadius*_params->azimuthTolerance) {
         break;
      }

      minBeamIndex = ii;
      prevAz = az;
      
   }
  
   //
   // Look "forward" from current beam
   //
   prevAz = _az;
   for (int ii = midBeamIndex + 1; ii < (int) beams.size(); ii++) {

      //
      // Check number of gates
      //
      const Beam *bb = beams[ii];
      if (bb->getNVelGates() != _nVelGates) {
         break;
      }

      //
      // Check azimuth differences with previous beam
      //
      float az = bb->getAzimuth();
      if (fabs(_computeAzDiff(az, prevAz)) > 2*_params->azimuthTolerance) {
         break;
      }

      maxBeamIndex = ii;
      prevAz = az;
   }

   //if(_params->verbose)
   //POSTMSG( DEBUG, "Rec limits for elevation = %f and azimuth = %f: "
   //      "minBeamIndex = %d, maxBeamIndex = %d", _el, _az,
   //      minBeamIndex, maxBeamIndex );
    
}

void Beam::_computeRecVars() 
{
   //
   // If we've already computed the variables we need for the
   // REC computation, exit
   //
   if (_recVarsReady) {
      return;
   }

   _recDbzDiffSq     = new double[_nReflGates];
   _recDbzSpinChange = new double[_nReflGates];
   
   //
   // Setup the reflectivity difference squared and the
   // spin change variables
   //
   double spinThresh = _params->recSpinDbzThreshold;

   _recDbzDiffSq[0] = MISSING_DBL;
   _recDbzSpinChange[0] = MISSING_DBL;
   for (int ii = 1; ii < _nReflGates; ii++) {

      if (_dbz[ii] != DBZ_BAD && _dbz[ii-1] != DBZ_BAD) {
         double dbzDiff = (_dbz[ii] * _dbzScale + _dbzBias) - (_dbz[ii-1] * _dbzScale + _dbzBias);

         _recDbzDiffSq[ii] = dbzDiff * dbzDiff;
         if (_recDbzDiffSq[ii] > DBZ_DIFF_SQ_MAX) {
            POSTMSG( ERROR, "Reflectivity difference squared limit exceeded"
                     " dbz[%d] = %f, dbz[%d] = %f, difference = %f, "
                     "diff squared = %f", ii, _dbz[ii] * _dbzScale + _dbzBias,
		     ii-1, _dbz[ii-1] * _dbzScale + _dbzBias,
                     dbzDiff, _recDbzDiffSq[ii] );
         }

         if (dbzDiff >= spinThresh) {
            _recDbzSpinChange[ii] = 1;
         } else {
            _recDbzSpinChange[ii] = 0;
         }
      } else {
	_recDbzDiffSq[ii] = MISSING_DBL;
	_recDbzSpinChange[ii] = MISSING_DBL;
      }
   }

   //
   // The variables we need for the REC computation are ready now
   //
   _recVarsReady = true;

}

void Beam::_computeSnr() 
{
  if(_snr)
    delete [] _snr;

  _snr = new ui08 [_nReflGates];
  float snrFactor = _params->snrFactor;
  
  //
  // Calculate snr for all gates for which
  // we have reflectivity
  //
  for( int i = 0; i < _nReflGates; i++ ) {
    
    double radialDistance = (i * _reflGateWidth + _rangeToFirstReflGate) * M_TO_KM;

    
    if( _dbz[i] == DBZ_BAD || radialDistance <= 0 ) {
      _snr[i] = SNR_BAD;
      continue;
    }
    
    double dbzUnscaled = (double)_dbz[i] * _dbzScale + _dbzBias;
    double snrVal    = dbzUnscaled + 20*log10(snrFactor/radialDistance);
    double snrScaled = ( snrVal - _snrBias ) / _snrScale;

    if( .5 > snrScaled) 
      snrScaled = 1;
    
    if(snrScaled > 255)
      snrScaled = 255;

    _snr[i] = (ui08) (snrScaled + 0.5);

  }

}

void Beam::_computePR(float unambRange) 
{
  if(_pr)
    delete [] _pr;

  _pr = new ui08 [_nReflGates];

  int unambigRngOffset = 
    (int) ( unambRange / ( _reflGateWidth * M_TO_KM ) + 0.5 );
  
  ui08 powerRatioDefault = 
    (short) ((_params->powerRatioDefault - _prBias) / _prScale);
  
  for( int i = 0; i < _nReflGates; i++ ) {
    if( _snr[i] == SNR_BAD ) {
      _pr[i]      = PR_BAD;
      continue;
    }
    
    int multBack    = -(i / unambigRngOffset);
    int multForward = (_nReflGates - 1 - i) / unambigRngOffset;
    
    float linSnrSum = 0.0;
    
    for( int mi = multBack; mi <= multForward; mi++ ) {
      
      if( mi == 0 ) 
	continue;
      
      int newIdex = i + mi * unambigRngOffset;
      
      if( _snr[newIdex] == SNR_BAD ) {
	continue;
      }
      
      double snrUnscaled = (double)_snr[newIdex] * _snrScale + _snrBias;
      linSnrSum += pow(10, snrUnscaled / 10.0 );
    }
    
    if( linSnrSum != 0 ) {
      double snrUnscaled = (double)_snr[i] * _snrScale + _snrBias;
      double prVal    = snrUnscaled - 10*log10(linSnrSum);
      double prScaled = ( prVal - _prBias ) / _prScale;

      if( .5 > prScaled) 
	prScaled = 1;
      
      if(prScaled > powerRatioDefault - .5)
	prScaled = powerRatioDefault -1;

      _pr[i] = (ui08) (prScaled + 0.5);
      
    }
    else {
      _pr[i] = powerRatioDefault;
    }
    
  }
}

int Beam::_convertInterestMapToVector(const Params::interestMapPoint_t *map,
				      int nPoints,
				      vector<InterestMap::ImPoint> &pts)
{
   pts.clear();

   double prevVal = -1.0e99;
   for (int ii = 0; ii < nPoints; ii++) {
      if (map[ii].value <= prevVal) {
         POSTMSG( ERROR, "Map values must increase monotonically" );
         return -1;
      }

      InterestMap::ImPoint pt(map[ii].value, map[ii].interest);
      pts.push_back(pt);
   }
  
   return 0;
}

double Beam::_computeAzDiff(double az1, double az2) const
{
   double diff = az1 - az2;

   if (diff > 180.0) {
      diff -= 360.0;
   } else if (diff < -180.0) {
      diff += 360.0;
   }

   return diff;

}
