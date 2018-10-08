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
// Adapted from code written by Mike Dixon for Lirp2Dsr
// 
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2005
//
// $Id: Beam.cc,v 1.32 2018/02/16 20:45:38 jcraig Exp $
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <cmath>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include <dataport/bigend.h>
#include "Beam.hh"
#include "SweepData.hh"
using namespace std;


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
const double Beam::SPEED_OF_LIGHT  = 299792458;

//
//   These constants are assumed in the ridds data stream.
//   They should not be modified unless the ridds format
//   changes.
//
const double Beam::DBZ_SCALE        = 0.5;
const double Beam::DBZ_BIAS         = -33.0;
const double Beam::VEL_SCALE        = 0.5;
const double Beam::VEL_BIAS         = -64.5;

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


Beam::Beam( Params *params, RIDDS_data_hdr* nexradData,
            time_t beamTime, SweepData::scan_t scanType, SweepData* prevSweep )
{
  _params       = params;
  _prevSweep    = prevSweep;
  _beamComplete = false;
  _mergeDone    = false;
  _recDone      = false;
  _recVarsReady = false;
  _time         = (double) beamTime;
  _el           = (nexradData->elevation/8.0)*(180.0/4096.0);
  _az           = (nexradData->azimuth/8.0)*(180.0/4096.0);
  _unambRange   = ((float) nexradData->unamb_range_x10) / 10;
  _prf          = (float)(SPEED_OF_LIGHT / (2 * _unambRange * 1000));
  _nVelocity    = ((float) nexradData->nyquist_vel) / 100;
  _horizNoise   = -999.0;
  _vertNoise    = -999.0;

  _dbz          = NULL;
  _vel          = NULL;
  _width        = NULL;
  _zdr          = NULL;
  _phi          = NULL;
  _rho          = NULL;
  _snr          = NULL;
  _pr           = NULL;

  _recDbzDiffSq     = NULL;
  _recDbzSpinChange = NULL;
  _rec              = NULL;

  _calcSnr = false;
  _calcPr = false;
  _calcRec = false;

  _dbzScale = DBZ_SCALE;
  _dbzBias = DBZ_BIAS;
  _velScale = VEL_SCALE;
  _velBias = VEL_BIAS;
  _swScale = VEL_SCALE;
  _swBias = VEL_BIAS;

  _nReflGates           = 0;
  _rangeToFirstReflGate = 0;
  _reflGateWidth        = 0;
  _nVelGates            = 0;
  _rangeToFirstVelGate  = 0;
  _velGateWidth         = 0;

  //
  // Set up scales and biases for derived variables
  //
  for( int ii = 0; ii < params->derivedFieldDefs_n; ii++ ) {
    switch( params->_derivedFieldDefs[ii].outputField ) {
    case Params::SNR :
      _calcSnr = true;
      _snrScale = params->_derivedFieldDefs[ii].scale;
      _snrBias  = params->_derivedFieldDefs[ii].bias;
      break;
      
    case Params::PR :
      _calcPr = true;
      _prScale = params->_derivedFieldDefs[ii].scale;
      _prBias  = params->_derivedFieldDefs[ii].bias;
      break;
      
    case Params::REC :
      if(scanType == SweepData::BOTH || params->combineSweeps)
      _calcRec = true;
      _recScale       = params->_derivedFieldDefs[ii].scale;
      _recBias        = params->_derivedFieldDefs[ii].bias;
    }
  }


  //
  // Set the data 
  //
  switch( scanType ) {
  case SweepData::REFL_ONLY :
    _nReflGates           = (int) nexradData->ref_num_gates;
    _rangeToFirstReflGate = (float) nexradData->ref_gate1;
    _reflGateWidth        = (float) nexradData->ref_gate_width;
    
    _storeData( ((ui08*)nexradData + nexradData->ref_ptr), _nReflGates, &_dbz);
    
    if( !_params->combineSweeps ) {
      _beamComplete = true;
    }

    break;

  case SweepData::VEL_ONLY:
    _nVelGates       = (int) nexradData->vel_num_gates;
    _rangeToFirstVelGate = (float) nexradData->vel_gate1;
    _velGateWidth        = (float) nexradData->vel_gate_width;

    _storeData( ((ui08*)nexradData + nexradData->vel_ptr), _nVelGates, &_vel);
    _storeData( ((ui08*)nexradData + nexradData->sw_ptr), _nVelGates, &_width);

    _beamComplete = true;
    if( _params->combineSweeps)
    {
      if(_prevSweep->getNBeams() > 0 )
	_mergeDbz();
      else
	_beamComplete = false;
    }

    break;
    
  case SweepData::BOTH:
    _nVelGates            = (int) nexradData->vel_num_gates;
    _nReflGates           = (int) nexradData->ref_num_gates;
    _rangeToFirstReflGate = (float) nexradData->ref_gate1;
    _reflGateWidth        = (float) nexradData->ref_gate_width;
    _rangeToFirstVelGate  = (float) nexradData->vel_gate1;
    _velGateWidth         = (float) nexradData->vel_gate_width;

    _storeData( ((ui08*)nexradData + nexradData->ref_ptr), _nReflGates, &_dbz);
    
    _storeData( ((ui08*)nexradData + nexradData->vel_ptr), _nVelGates, &_vel);
    
    _storeData( ((ui08*)nexradData + nexradData->sw_ptr), _nVelGates, &_width);

    if(_calcSnr)
      _computeSnr();
    if(_calcPr)
      _computePR();

    _beamComplete = true;
    break;
    
  case SweepData::NONE:
    POSTMSG( ERROR, "Could not determine sweep type" );
    break;
  }

  //
  // Setup interest maps if needed
  //   Note that the interest maps are static on the class,
  //   so the are actually only created once
  //
  if (_calcRec) {
    createInterestMaps(_params);
  }

}

Beam::Beam( Params *params, RIDDS_data_31_hdr* nexradData,
            time_t beamTime, SweepData::scan_t scanType, SweepData* prevSweep )
{
  _params       = params;
  _prevSweep    = prevSweep;
  _beamComplete = false;
  _mergeDone    = false;
  _recDone      = false;
  _recVarsReady = false;
  _time         = (double) beamTime;

  //
  // Get pointers to each header section

  //RIDDS_volume_31_hdr *volData = (RIDDS_volume_31_hdr *)((ui08*)nexradData + nexradData->volume_ptr);
  
  //RIDDS_elev_31_hdr *elevData = (RIDDS_elev_31_hdr *)((ui08*)nexradData + nexradData->elevation_ptr);
  
  RIDDS_radial_31_hdr *radialData = (RIDDS_radial_31_hdr *)((ui08*)nexradData + nexradData->radial_ptr);


  _el           = nexradData->elevation;
  _az           = nexradData->azimuth;
  _unambRange   = ((float) radialData->unamb_range_x10) / 10;
  _prf          = (float)(SPEED_OF_LIGHT / (2 * _unambRange * 1000));
  _nVelocity    = ((float) radialData->nyquist_vel) / 100;
  _horizNoise   = radialData->horiz_noise;
  _vertNoise    = radialData->vert_noise;

  _dbz          = NULL;
  _vel          = NULL;
  _width        = NULL;
  _zdr          = NULL;
  _phi          = NULL;
  _rho          = NULL;
  _snr          = NULL;
  _pr           = NULL;

  _recDbzDiffSq     = NULL;
  _recDbzSpinChange = NULL;
  _rec              = NULL;

  _nReflGates           = 0;
  _rangeToFirstReflGate = 0;
  _reflGateWidth        = 0;
  _nVelGates            = 0;
  _rangeToFirstVelGate  = 0;
  _velGateWidth         = 0;

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
    case Params::DZ :
    case Params::VE :
    case Params::SW :
    case Params::CMAP :
    case Params::BMAP :
      break;
    }
  }

  //
  // Set up scales and biases for derived variables
  //
  _dbzScale = 0;
  _dbzBias  = 0;
  _velScale = 0;
  _velBias  = 0;
  _swScale  = 0;
  _swBias   = 0;
  _zdrScale = 0;
  _zdrBias  = 0;
  _phiScale = 0;
  _phiBias  = 0;
  _rhoScale = 0;
  _rhoBias  = 0;

  _calcSnr = false;
  _calcPr  = false;
  _calcRec = false;
  for( int ii = 0; ii < params->derivedFieldDefs_n; ii++ ) {
    switch( params->_derivedFieldDefs[ii].outputField ) {
    case Params::SNR :
      _calcSnr = true;
      _snrScale = params->_derivedFieldDefs[ii].scale;
      _snrBias  = params->_derivedFieldDefs[ii].bias;
      break;
      
    case Params::PR :
      _calcPr = true;
      _prScale = params->_derivedFieldDefs[ii].scale;
      _prBias  = params->_derivedFieldDefs[ii].bias;
      break;
      
    case Params::REC :
      if(scanType == SweepData::BOTH || params->combineSweeps)
      _calcRec = true;
      _recScale       = params->_derivedFieldDefs[ii].scale;
      _recBias        = params->_derivedFieldDefs[ii].bias;
    }
  }

  RIDDS_data_31 *reflData;
  RIDDS_data_31 *velData;
  RIDDS_data_31 *swData;

  bool print = false;

  if(_params->verbose)
    print = true;
  if(_el > 359.5)
    print = true;

  //
  // Set the data 
  //
  switch( scanType ) {
  case SweepData::REFL_ONLY :

    reflData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->ref_ptr);

    _dbzScale = 1.0 / reflData->scale;
    _dbzBias = (reflData->offset / reflData->scale) * -1.0;

    _nReflGates           = (int) reflData->num_gates;
    _rangeToFirstReflGate = (float) reflData->gate1;
    _reflGateWidth        = (float) reflData->gate_width;
    
    if(reflData->data_size == 8)
      _storeData( ((ui08*)reflData + sizeof(RIDDS_data_31)), _nReflGates, &_dbz);
    else
      POSTMSG( ERROR, "REFL data size is not ui08." );

    if(_saveZdr && nexradData->zdr_ptr != 0) {
      RIDDS_data_31 *zdrData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->zdr_ptr);
      _zdrScale = 1.0 / zdrData->scale;
      _zdrBias = (zdrData->offset / zdrData->scale) * -1.0;
      if(zdrData->data_size == 8)
	_storeData( ((ui08*)zdrData + sizeof(RIDDS_data_31)), _nReflGates, zdrData->num_gates, &_zdr);
      else
	POSTMSG( ERROR, "ZDR data size is not ui08." );
    }
    if(_savePhi && nexradData->phi_ptr != 0) {
      RIDDS_data_31 *phiData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->phi_ptr);
      _phiScale = 1.0 / phiData->scale;
      _phiBias = (32768.0 - phiData->offset) / phiData->scale;
      if(phiData->data_size == 8)
	POSTMSG( ERROR, "ZDR data size is not ui16." );
      else
	_storeData( (ui16*)((ui08*)phiData + sizeof(RIDDS_data_31)), _nReflGates, phiData->num_gates, &_phi);
    }

    if(_saveRho && nexradData->rho_ptr != 0) {
      RIDDS_data_31 *rhoData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->rho_ptr);
      _rhoScale = 1.0 / rhoData->scale;
      _rhoBias = (rhoData->offset / rhoData->scale) * -1.0;
      if(rhoData->data_size == 8)
	_storeData( ((ui08*)rhoData + sizeof(RIDDS_data_31)), _nReflGates, rhoData->num_gates, &_rho);
      else
	POSTMSG( ERROR, "RHO data size is not ui08." );
    }

    if( !_params->combineSweeps ) {
      _beamComplete = true;
    }

    break;

  case SweepData::VEL_ONLY:
    if( ((RIDDS_volume_31_hdr *)((ui08*)nexradData + nexradData->vel_ptr))->block_name[0] == 'S' &&
	nexradData->sw_ptr == 0 && nexradData->ref_ptr > 0) {
	nexradData->sw_ptr = nexradData->vel_ptr;
	nexradData->vel_ptr = nexradData->ref_ptr;
	nexradData->ref_ptr = 0;
    }

    velData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->vel_ptr);

    _velScale = 1.0 / velData->scale;
    _velBias = (velData->offset / velData->scale) * -1.0;

    _nVelGates       = (int) velData->num_gates;
    _rangeToFirstVelGate = (float) velData->gate1;
    _velGateWidth    = (float) velData->gate_width;
    if(velData->data_size == 8)
      _storeData( ((ui08*)velData + sizeof(RIDDS_data_31)), _nVelGates, &_vel);
    else
      POSTMSG( ERROR, "VEL data size is not ui08." );

    if(nexradData->sw_ptr != 0) {
      swData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->sw_ptr);
      _swScale = 1.0 / swData->scale;
      _swBias = (swData->offset / swData->scale) * -1.0;
      if(swData->data_size == 8)
	_storeData( ((ui08*)swData + sizeof(RIDDS_data_31)), _nVelGates, &_width);
      else
	POSTMSG( ERROR, "SW data size is not ui08." );

      if(swData->gate1 != _rangeToFirstVelGate) {
	POSTMSG( ERROR, "SW gate1 does not equal Vel gate1." );
      }
      if(swData->gate_width != _velGateWidth) {
	POSTMSG( ERROR, "SW gate width does not equal Vel gate width." );
      }

    }

    if(_saveZdr && nexradData->zdr_ptr != 0) {
      RIDDS_data_31 *zdrData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->zdr_ptr);
      _zdrScale = 1.0 / zdrData->scale;
      _zdrBias = (zdrData->offset / zdrData->scale) * -1.0;
      if(zdrData->data_size == 8)
	_storeData( ((ui08*)zdrData + sizeof(RIDDS_data_31)), _nVelGates, zdrData->num_gates, &_zdr);
      else
	POSTMSG( ERROR, "ZDR data size is not ui08." );
    }

    if(_savePhi && nexradData->phi_ptr != 0) {
      RIDDS_data_31 *phiData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->phi_ptr);
      _phiScale = 1.0 / phiData->scale;
      _phiBias = (phiData->offset / phiData->scale) * -1.0;
      if(phiData->data_size == 8)
	POSTMSG( ERROR, "PHI data size is not ui16." );
      else
	_storeData( (ui16*)((ui08*)phiData + sizeof(RIDDS_data_31)), _nVelGates, phiData->num_gates, &_phi);
    }

    if(_saveRho && nexradData->rho_ptr != 0) {
      RIDDS_data_31 *rhoData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->rho_ptr);
      _rhoScale = 1.0 / rhoData->scale;
      _rhoBias = (rhoData->offset / rhoData->scale) * -1.0;
      if(rhoData->data_size == 8)
	_storeData( ((ui08*)rhoData + sizeof(RIDDS_data_31)), _nVelGates, rhoData->num_gates, &_rho);
      else
	POSTMSG( ERROR, "RHO data size is not ui08." );
    }

    _beamComplete = true;
    if( _params->combineSweeps)
    {
      if(_prevSweep->getNBeams() > 0 ) {
	_mergeDbz();
	if(print) {
	  POSTMSG( DEBUG, "Beam Merged with previous sweep");
	}
      } else {
	_beamComplete = false;
      }
    }

    break;
    
  case SweepData::BOTH:

    reflData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->ref_ptr);
    velData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->vel_ptr);

    _velScale = 1.0 / velData->scale;
    _velBias = (velData->offset / velData->scale) * -1.0;

    _nVelGates            = (int) velData->num_gates;
    _rangeToFirstVelGate  = (float) velData->gate1;
    _velGateWidth         = (float) velData->gate_width;

    if(velData->data_size == 8)
      _storeData( ((ui08*)velData + sizeof(RIDDS_data_31)), _nVelGates, &_vel);
    else
      POSTMSG( ERROR, "VEL data size is not ui08." );

    if(nexradData->sw_ptr != 0) {
      swData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->sw_ptr);
      if(swData->num_gates != _nVelGates) {
	POSTMSG( ERROR, "SW num_gates does not equal Vel num_gates." );
      }
      _swScale = 1.0 / swData->scale;
      _swBias = (swData->offset / swData->scale) * -1.0;
      if(swData->data_size == 8)
	_storeData( ((ui08*)swData + sizeof(RIDDS_data_31)), _nVelGates, &_width);
      else
	POSTMSG( ERROR, "SW data size is not ui08." );
    }

    if(_params->combineSweeps && _prevSweep != NULL && _prevSweep->getNBeams() > 0 && _prevSweep->getScanType() == SweepData::REFL_ONLY) {
      _mergeDbz();
      if(print)
	POSTMSG( DEBUG, "Beam Merged with previous DBZ");
    } else {
      _dbzScale = 1.0 / reflData->scale;
      _dbzBias = (reflData->offset / reflData->scale) * -1.0;
      _nReflGates           = (int) reflData->num_gates;
      _rangeToFirstReflGate = (float) reflData->gate1;
      _reflGateWidth        = (float) reflData->gate_width;
      if(reflData->data_size == 8)
	_storeData( ((ui08*)reflData + sizeof(RIDDS_data_31)), _nReflGates, &_dbz);
      else
	POSTMSG( ERROR, "REFL data size is not ui08." );

    }

    if(_saveZdr && nexradData->zdr_ptr != 0) {
      RIDDS_data_31 *zdrData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->zdr_ptr);
      _zdrScale = 1.0 / zdrData->scale;
      _zdrBias = (zdrData->offset / zdrData->scale) * -1.0;
      if(zdrData->data_size == 8)
	_storeData( ((ui08*)zdrData + sizeof(RIDDS_data_31)), _nVelGates, zdrData->num_gates, &_zdr);
      else
	POSTMSG( ERROR, "ZDR data size is not ui08." );
    }

    if(_savePhi && nexradData->phi_ptr != 0) {
      RIDDS_data_31 *phiData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->phi_ptr);
      _phiScale = 1.0 / phiData->scale;
      _phiBias = (phiData->offset / phiData->scale) * -1.0;
      if(phiData->data_size == 8)
	POSTMSG( ERROR, "PHI data size is not ui16." );
      else
	_storeData( (ui16*)((ui08*)phiData + sizeof(RIDDS_data_31)), _nVelGates, phiData->num_gates, &_phi);
    }

    if(_saveRho && nexradData->rho_ptr != 0) {
      RIDDS_data_31 *rhoData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->rho_ptr);
      _rhoScale = 1.0 / rhoData->scale;
      _rhoBias = (rhoData->offset / rhoData->scale) * -1.0;
      if(rhoData->data_size == 8)
	_storeData( ((ui08*)rhoData + sizeof(RIDDS_data_31)), _nVelGates, rhoData->num_gates, &_rho);
      else
	POSTMSG( ERROR, "RHO data size is not ui08." );
    }

    if(_calcSnr)
      _computeSnr();
    if(_calcPr)
      _computePR();

    _beamComplete = true;
    break;
    
  case SweepData::NONE:
    POSTMSG( ERROR, "Could not determine sweep type" );
    break;
  }

  //
  // Setup interest maps if needed
  //   Note that the interest maps are static on the class,
  //   so the are actually only created once
  //
  if (_calcRec) {
    createInterestMaps(_params);
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

const ui08* Beam::getRec()
{
  if(_recDone)
    return _rec;
  return NULL;
}

void Beam::computeRec( const deque<Beam *> beams,
		       int midBeamIndex )
{
   if( !_beamComplete || !_calcRec || _width == NULL ||
       _reflGateWidth == 0 || _velGateWidth == 0) {
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

void Beam::_storeData(const ui08* nexradData, int numGatesIn, ui08** data) 
{
  *data = new ui08 [numGatesIn];

  //
  // Loop through all the values in our input data buffer
  //
  for( int i = 0; i < numGatesIn; i++ ) {
    
    ui08 value = nexradData[i];
    
    //
    // 0: below SNR, 1: ambiguous range
    //
    if ( value != 0  &&  value != 1 ) {
      (*data)[i] = value;
    } else {
      (*data)[i] = 0;
    }
    
  }  
}

void Beam::_storeData(const ui16* nexradData, int numGatesIn, si16** data) 
{
  *data = new si16 [numGatesIn];

  //
  // Loop through all the values in our input data buffer
  //
  for( int i = 0; i < numGatesIn; i++ ) {
    
    ui16 value = nexradData[i];
    BE_from_array_16(&value, 2);

    si16 sval;
    if (value < 2) {
      sval = -999.99;
    } else {
      int ival = value - 32768;
      sval = ival;
    }

    (*data)[i] = sval;
    
  }  
}

void Beam::_storeData(const ui08* nexradData, int numGatesOut, int numGatesIn, ui08** data) 
{
  *data = new ui08 [numGatesOut];

  //
  // Store only the numGatesOut from the input data buffer
  // or if input data is smaller, store missing after input buffer ends
  //
  int numGates = numGatesOut;
  if(numGatesIn < numGatesOut)
    numGates = numGatesIn;

  for( int i = 0; i < numGates; i++ ) {
    
    ui08 value = nexradData[i];
    
    //
    // 0: below SNR, 1: ambiguous range
    //
    if ( value != 0  &&  value != 1 ) {
      (*data)[i] = value;
    } else {
      (*data)[i] = 0;
    }
    
  }

  if(numGates < numGatesOut)
    for( int i = numGates; i < numGatesOut; i++ )
      (*data)[i] = 0;

}

void Beam::_storeData(const ui16* nexradData, int numGatesOut, int numGatesIn, si16** data) 
{
  *data = new si16 [numGatesOut];

  //
  // Store only the numGatesOut from the input data buffer
  // or if input data is smaller, store missing after input buffer ends
  //
  int numGates = numGatesOut;
  if(numGatesIn < numGatesOut)
    numGates = numGatesIn;

  for( int i = 0; i < numGates; i++ ) {
    
    ui16 value = nexradData[i];
    BE_from_array_16(&value, 2);

    si16 sval;
    if (value < 2) {
      sval = -999.99;
    } else {
      int ival = value - 32768;
      sval = ival;
    }

    (*data)[i] = sval;
    
  }

  if(numGates < numGatesOut)
    for( int i = numGates; i < numGatesOut; i++ )
      (*data)[i] = -999.99;

}

void Beam::_storeData(const si16* nexradData, int numGatesOut, int numGatesIn, si16** data) 
{
  *data = new si16 [numGatesOut];

  //
  // Store only the numGatesOut from the input data buffer
  // or if input data is smaller, store missing after input buffer ends
  //
  int numGates = numGatesOut;
  if(numGatesIn < numGatesOut)
    numGates = numGatesIn;

  for( int i = 0; i < numGates; i++ ) {
    
    si16 value = nexradData[i];

    (*data)[i] = value;
    
  }

  if(numGates < numGatesOut)
    for( int i = numGates; i < numGatesOut; i++ )
      (*data)[i] = -999.99;

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

void Beam::_computePR() 
{
  if(_pr)
    delete [] _pr;

  _pr = new ui08 [_nReflGates];

  int unambigRngOffset = 
    (int) ( _unambRange / ( _reflGateWidth * M_TO_KM ) + 0.5 );
  
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


void Beam::_mergeDbz() 
{
  //
  // Get the closest beam
  //
  Beam *dbzBeam = _prevSweep->getClosestBeam( _az, _el, _time );
  
  if( !dbzBeam || !dbzBeam->getDbz() ) {
    POSTMSG( DEBUG, "Failed to Merge Beam. elevation: %f azimuth: %f", _el, _az);   
    _nReflGates     = 0;
    _reflGateWidth  = 0;
    _rangeToFirstReflGate = 0;
    _beamComplete = false;
    return;
  }
  
  //
  // Get the data from the previous sweep
  //
  _dbzScale       = dbzBeam->getDbzScale();
  _dbzBias        = dbzBeam->getDbzBias();
  _nReflGates     = dbzBeam->getNReflGates();
  _reflGateWidth  = dbzBeam->getReflGateWidth();
  _rangeToFirstReflGate = dbzBeam->getRangeToFirstReflGate();

  //
  // Copy the data into our local arrays   
  //
  if(_dbz)
    delete [] _dbz;

  const ui08  *dbzIn = dbzBeam->getDbz();
  _dbz = new ui08 [_nReflGates];
  memcpy( (void *) _dbz, (void *) dbzIn, _nReflGates * sizeof(ui08) );

  if(_saveZdr && !_zdr && dbzBeam->getZdr())
  {
    _zdrScale         = dbzBeam->getZdrScale();
    _zdrBias          = dbzBeam->getZdrBias();
    const ui08 *zdrIn = dbzBeam->getZdr();
    _storeData(zdrIn, _nVelGates, _nReflGates, &_zdr);
  }
  if(_savePhi && !_phi && dbzBeam->getPhi())
  {
    _phiScale         = dbzBeam->getPhiScale();
    _phiBias          = dbzBeam->getPhiBias();
    const si16 *phiIn = dbzBeam->getPhi();
    _storeData(phiIn, _nVelGates, _nReflGates, &_phi);
  }
  if(_saveRho && !_rho && dbzBeam->getRho())
  {
    _rhoScale         = dbzBeam->getRhoScale();
    _rhoBias          = dbzBeam->getRhoBias();
    const ui08 *rhoIn = dbzBeam->getRho();
    _rho = new ui08 [_nVelGates];
    _storeData(rhoIn, _nVelGates, _nReflGates, &_rho);
  }
  
  if(_calcSnr)
    _computeSnr();
  if(_calcPr)
    _computePR();

  _mergeDone = true;
  _beamComplete = true;
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
   float midEl  = _el;
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

      //
      // Check elevation difference with middle beam
      //   Make sure we are operating on the same tilt
      //
      float el = bb->getElevation();
      if (fabs(el - midEl) > _params->elevationTolerance) {
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

      //
      // Check elevation differences with middle beam
      //
      float el = bb->getElevation();
      if (fabs(el - midEl) > 2*_params->elevationTolerance) {
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


void Beam::_printBeam() 
{
   for( int i = 0; i < _nVelGates; i++ ) {
      fprintf( stderr, "ve[%d] = %d\n", 
               i, _vel[i] );
   }
   for( int i = 0; i < _nReflGates; i++ ) {
      fprintf( stderr, "dz[%d] = %d\n", 
               i, _dbz[i]);
   }
}









