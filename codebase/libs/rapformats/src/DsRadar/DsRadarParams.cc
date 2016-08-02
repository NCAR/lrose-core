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
////////////////////////////////////////////////////////////////////////////////
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// April 1998
//
////////////////////////////////////////////////////////////////////////////////


#include <dataport/bigend.h>
#include <toolsa/str.h>
#include <rapformats/DsRadarParams.hh>
#include <iostream>
using namespace std;

DsRadarParams::DsRadarParams()
{
  radarId          = 0;
  radarType        = DS_RADAR_GROUND_TYPE;
  numFields        = 0;
  numGates         = 0;
  samplesPerBeam   = 0;
  scanType         = 0;
  scanMode         = DS_RADAR_SURVEILLANCE_MODE;
  followMode       = DS_RADAR_FOLLOW_MODE_NONE;
  polarization     = DS_POLARIZATION_HORIZ_TYPE;
  prfMode          = DS_RADAR_PRF_MODE_FIXED;

  radarConstant    = 0.0;
  altitude         = 0.0;
  latitude         = 0.0;
  longitude        = 0.0;
  gateSpacing      = 0.0;
  startRange       = 0.0;
  horizBeamWidth   = 0.0;
  vertBeamWidth    = 0.0;
  pulseWidth       = 0.0;
  pulseRepFreq     = 0.0;
  prt              = 0.0;
  prt2             = 0.0;
  wavelength       = 0.0;
  xmitPeakPower    = 0.0;
  receiverMds      = 0.0;
  receiverGain     = 0.0;
  antennaGain      = 0.0;
  systemGain       = 0.0;
  unambigVelocity  = 0.0;
  unambigRange     = 0.0;

  measXmitPowerDbmH = 0.0;
  measXmitPowerDbmV = 0.0;

}

DsRadarParams&
DsRadarParams::operator=( const DsRadarParams &inputParams ) 
{
  copy( inputParams );
  return( *this );
}

void
DsRadarParams::copy( const DsRadarParams &inputParams )
{
  if (this == &inputParams) {
    return;
  }
  radarId          = inputParams.radarId;
  radarType        = inputParams.radarType;
  numFields        = inputParams.numFields;
  numGates         = inputParams.numGates;
  samplesPerBeam   = inputParams.samplesPerBeam;
  scanType         = inputParams.scanType;
  scanMode         = inputParams.scanMode;
  followMode       = inputParams.followMode;
  polarization     = inputParams.polarization;
  prfMode          = inputParams.prfMode;

  radarConstant    = inputParams.radarConstant;
  altitude         = inputParams.altitude;
  latitude         = inputParams.latitude;
  longitude        = inputParams.longitude;
  gateSpacing      = inputParams.gateSpacing;
  startRange       = inputParams.startRange;
  horizBeamWidth   = inputParams.horizBeamWidth;
  vertBeamWidth    = inputParams.vertBeamWidth;
  pulseWidth       = inputParams.pulseWidth;
  pulseRepFreq     = inputParams.pulseRepFreq;
  prt              = inputParams.prt;
  prt2             = inputParams.prt2;
  wavelength       = inputParams.wavelength;
  xmitPeakPower    = inputParams.xmitPeakPower;
  receiverMds      = inputParams.receiverMds;
  receiverGain     = inputParams.receiverGain;
  antennaGain      = inputParams.antennaGain;
  systemGain       = inputParams.systemGain;
  unambigVelocity  = inputParams.unambigVelocity;
  unambigRange     = inputParams.unambigRange;

  measXmitPowerDbmH = inputParams.measXmitPowerDbmH;
  measXmitPowerDbmV = inputParams.measXmitPowerDbmV;

  radarName        = inputParams.radarName;
  scanTypeName     = inputParams.scanTypeName;

}

void
DsRadarParams::copy( const DsRadarParams_t &inputParams )
{
  radarId          = inputParams.radar_id;
  radarType        = inputParams.radar_type;
  numFields        = inputParams.nfields;
  numGates         = inputParams.ngates;
  samplesPerBeam   = inputParams.samples_per_beam;
  scanType         = inputParams.scan_type;
  scanMode         = inputParams.scan_mode;
  followMode       = inputParams.follow_mode;
  polarization     = inputParams.polarization;
  prfMode          = inputParams.prf_mode;

  radarConstant    = inputParams.radar_constant;
  altitude         = inputParams.altitude;
  latitude         = inputParams.latitude;
  longitude        = inputParams.longitude;
  gateSpacing      = inputParams.gate_spacing;
  startRange       = inputParams.start_range;
  horizBeamWidth   = inputParams.horiz_beam_width;
  vertBeamWidth    = inputParams.vert_beam_width;
  pulseWidth       = inputParams.pulse_width;
  pulseRepFreq     = inputParams.prf;
  prt              = inputParams.prt;
  prt2             = inputParams.prt2;
  wavelength       = inputParams.wavelength;
  xmitPeakPower    = inputParams.xmit_peak_pwr;
  receiverMds      = inputParams.receiver_mds;
  receiverGain     = inputParams.receiver_gain;
  antennaGain      = inputParams.antenna_gain;
  systemGain       = inputParams.system_gain;
  unambigVelocity  = inputParams.unambig_vel;
  unambigRange     = inputParams.unambig_range;
  measXmitPowerDbmH= inputParams.measXmitPowerDbmH;
  measXmitPowerDbmV= inputParams.measXmitPowerDbmV;
  radarName        = inputParams.radar_name;
  scanTypeName     = inputParams.scan_type_name;
}

bool
DsRadarParams::operator==( const DsRadarParams& otherParams ) const
{
  if ( radarId          != otherParams.radarId )         return false;
  if ( radarType        != otherParams.radarType )       return false;
  if ( numFields        != otherParams.numFields )       return false;
  if ( numGates         != otherParams.numGates )        return false;
  if ( samplesPerBeam   != otherParams.samplesPerBeam )  return false;
  if ( scanType         != otherParams.scanType )        return false;
  if ( scanMode         != otherParams.scanMode )        return false;
  if ( followMode       != otherParams.followMode )      return false;
  if ( polarization     != otherParams.polarization )    return false;
  if ( prfMode          != otherParams.prfMode )         return false;

  if ( radarConstant    != otherParams.radarConstant )   return false;
  if ( altitude         != otherParams.altitude )        return false;
  if ( latitude         != otherParams.latitude )        return false;
  if ( longitude        != otherParams.longitude )       return false;
  if ( gateSpacing      != otherParams.gateSpacing )     return false;
  if ( startRange       != otherParams.startRange )      return false;
  if ( horizBeamWidth   != otherParams.horizBeamWidth )  return false;
  if ( vertBeamWidth    != otherParams.vertBeamWidth )   return false;
  if ( pulseWidth       != otherParams.pulseWidth )      return false;
  if ( pulseRepFreq     != otherParams.pulseRepFreq )    return false;
  if ( prt              != otherParams.prt )             return false;
  if ( prt2             != otherParams.prt2 )            return false;
  if ( wavelength       != otherParams.wavelength )      return false;
  if ( xmitPeakPower    != otherParams.xmitPeakPower )   return false;
  if ( receiverMds      != otherParams.receiverMds )     return false;
  if ( receiverGain     != otherParams.receiverGain )    return false;
  if ( antennaGain      != otherParams.antennaGain )     return false;
  if ( systemGain       != otherParams.systemGain )      return false;
  if ( unambigVelocity  != otherParams.unambigVelocity ) return false;
  if ( unambigRange     != otherParams.unambigRange )    return false;
  if ( radarName        != otherParams.radarName )       return false;
  if ( scanTypeName     != otherParams.scanTypeName )    return false;

  return( true );
}

void
DsRadarParams::print(FILE *out) const
{
   
  fprintf(out, "RADAR PARAMS\n");
   
  fprintf(out, "  radar id:  %d\n", radarId);
  fprintf(out, "  radar type:  %s\n", radarType2Str(radarType).c_str());
  fprintf(out, "  number of fields:  %d\n", numFields);
  fprintf(out, "  number of gates:  %d\n", numGates);
  fprintf(out, "  samples per beam:  %d\n", samplesPerBeam);
  fprintf(out, "  scan type:  %d\n", scanType);
  fprintf(out, "  scan mode:  %s\n", scanMode2Str(scanMode).c_str());
  fprintf(out, "  follow mode:  %s\n", followMode2Str(followMode).c_str());
  fprintf(out, "  polarization:  %s\n", polType2Str(polarization).c_str());
  fprintf(out, "  prfMode:  %s\n", prfMode2Str(prfMode).c_str());
   
  fprintf(out, "  radar constant:  %f\n", radarConstant);
  fprintf(out, "  altitude (km):  %f\n", altitude);
  fprintf(out, "  latitude (deg):  %f\n", latitude);
  fprintf(out, "  longitude (deg):  %f\n", longitude);
  fprintf(out, "  gate spacing (km):  %f\n", gateSpacing);
  fprintf(out, "  start range (km):  %f\n", startRange);
  fprintf(out, "  horizontal beam width (deg):  %f\n", horizBeamWidth);
  fprintf(out, "  vertical beam width (deg):  %f\n", vertBeamWidth);
  fprintf(out, "  pulse width (us):  %f\n", pulseWidth);
  fprintf(out, "  pulse repetition frequency (/s):  %f\n", pulseRepFreq);
  fprintf(out, "  pulse repetition time (s):  %f\n", prt);
  fprintf(out, "  prt2 - staggered/dual PRT (s):  %f\n", prt2);
  fprintf(out, "  wavelength (cm):  %f\n", wavelength);
  fprintf(out, "  transmit peak power (W):  %f\n", xmitPeakPower);
  fprintf(out, "  receiver minimum detectable signal: (dBM) %f\n", 
	  receiverMds);
  fprintf(out, "  receiver gain (dB):  %f\n", receiverGain);
  fprintf(out, "  antenna gain (dB):  %f\n", antennaGain);
  fprintf(out, "  system gain (dB):  %f\n", systemGain);
  fprintf(out, "  unambig velocity (m/s):  %f\n", unambigVelocity);
  fprintf(out, "  unabig range (km):  %f\n", unambigRange);
   
  fprintf(out, "  measXmitPowerDbmH (dBm):  %f\n", measXmitPowerDbmH);
  fprintf(out, "  measXmitPowerDbmV (dBm):  %f\n", measXmitPowerDbmV);

  fprintf(out, "  radar name:  %s\n", radarName.c_str());
  fprintf(out, "  scan type name:  %s\n", scanTypeName.c_str());

  fprintf(out, "\n");
   
}

void
DsRadarParams::print(ostream &out)  const
{
   
  out << "RADAR PARAMS" << endl;
  out << "------------" << endl;
   
  out << "  radar id:  " << radarId << endl;
  out << "  radar type:  " << radarType2Str(radarType) << endl;
  out << "  number of fields:  " << numFields << endl;
  out << "  number of gates:  " << numGates << endl;
  out << "  samples per beam:  " << samplesPerBeam << endl;
  out << "  scan type:  " << scanType << endl;
  out << "  scan mode:  " << scanMode2Str(scanMode) << endl;
  out << "  follow mode:  " << followMode2Str(followMode) << endl;
  out << "  polarization:  " << polType2Str(polarization) << endl;
  out << "  prfMode:  " << prfMode2Str(prfMode) << endl;

  out << "  radar constant:  " << radarConstant << endl;
  out << "  altitude (km):  " << altitude << endl;
  out << "  latitude (deg):  " << latitude << endl;
  out << "  longitude (deg):  " << longitude << endl;
  out << "  gate spacing (km):  " << gateSpacing << endl;
  out << "  start range (km):  " << startRange << endl;
  out << "  horizontal beam width (deg):  " << horizBeamWidth << endl;
  out << "  vertical beam width (deg):  " << vertBeamWidth << endl;
  out << "  pulse width (us):  " << pulseWidth << endl;
  out << "  pulse repetition frequency (/s):  " << pulseRepFreq << endl;
  out << "  pulse repetition time (s):  " << prt << endl;
  out << "  prt2 - staggered/dual PRT (s):  " << prt2 << endl;
  out << "  wavelength (cm):  " << wavelength << endl;
  out << "  transmit peak power (W):  " << xmitPeakPower << endl;
  out << "  receiver minimum detectable signal (dBM):  " << receiverMds << endl;
  out << "  receiver gain (dB):  " << receiverGain << endl;
  out << "  antenna gain (dB):  " << antennaGain << endl;
  out << "  system gain (dB):  " << systemGain << endl;
  out << "  unambig velocity (m/s):  " << unambigVelocity << endl;
  out << "  unabig range (km):  " << unambigRange << endl;
   
  out << "  measXmitPowerDbmH (dBm): " << measXmitPowerDbmH << endl;
  out << "  measXmitPowerDbmV (dBm): " << measXmitPowerDbmV << endl;

  out << "  radar name:  " << radarName << endl;
  out << "  scan type name:  " << scanTypeName << endl;

  out << endl;
   
}

void
DsRadarParams::decode( DsRadarParams_t *rparams_msg )
{

  DsRadarParams_t rparams = *rparams_msg;
  BE_to_DsRadarParams(&rparams);

  radarId          = rparams.radar_id;
  radarType        = rparams.radar_type;
  numFields        = rparams.nfields;
  numGates         = rparams.ngates;
  samplesPerBeam   = rparams.samples_per_beam;
  scanType         = rparams.scan_type;
  scanMode         = rparams.scan_mode;
  followMode       = rparams.follow_mode;
  polarization     = rparams.polarization;
  prfMode          = rparams.prf_mode;

  radarConstant    = rparams.radar_constant;
  altitude         = rparams.altitude;      
  latitude         = rparams.latitude;      
  longitude        = rparams.longitude;     
  gateSpacing      = rparams.gate_spacing;  
  startRange       = rparams.start_range;   
  horizBeamWidth   = rparams.horiz_beam_width;
  vertBeamWidth    = rparams.vert_beam_width; 
  pulseWidth       = rparams.pulse_width;   
  pulseRepFreq     = rparams.prf;
  prt              = rparams.prt;
  prt2             = rparams.prt2;
  wavelength       = rparams.wavelength;    
  xmitPeakPower    = rparams.xmit_peak_pwr; 
  receiverMds      = rparams.receiver_mds;  
  receiverGain     = rparams.receiver_gain; 
  antennaGain      = rparams.antenna_gain;  
  systemGain       = rparams.system_gain;   
  unambigVelocity  = rparams.unambig_vel;   
  unambigRange     = rparams.unambig_range; 

  measXmitPowerDbmH= rparams.measXmitPowerDbmH;
  measXmitPowerDbmV= rparams.measXmitPowerDbmV;

  radarName        = rparams.radar_name;
  scanTypeName     = rparams.scan_type_name;

}

void
DsRadarParams::encode( DsRadarParams_t *rparams_msg ) const
{

  loadStruct(rparams_msg);
  BE_from_DsRadarParams(rparams_msg);

}

void
DsRadarParams::loadStruct( DsRadarParams_t *rparams_msg ) const
{

  memset( rparams_msg, 0, sizeof(DsRadarParams_t) );
  
  rparams_msg->radar_id           = radarId;
  rparams_msg->radar_type         = radarType;
  rparams_msg->nfields            = numFields;
  rparams_msg->ngates             = numGates;
  rparams_msg->samples_per_beam   = samplesPerBeam;
  rparams_msg->scan_type          = scanType;
  rparams_msg->scan_mode          = scanMode;
  rparams_msg->follow_mode        = followMode;
  rparams_msg->polarization       = polarization;
  rparams_msg->prf_mode           = prfMode;
  
  rparams_msg->radar_constant     = radarConstant;
  rparams_msg->altitude           = altitude;
  rparams_msg->latitude           = latitude;
  rparams_msg->longitude          = longitude;
  rparams_msg->gate_spacing       = gateSpacing;
  rparams_msg->start_range        = startRange;
  rparams_msg->horiz_beam_width   = horizBeamWidth;
  rparams_msg->vert_beam_width    = vertBeamWidth;
  rparams_msg->pulse_width        = pulseWidth;
  rparams_msg->prf                = pulseRepFreq;
  rparams_msg->prt                = prt;
  rparams_msg->prt2               = prt2;
  rparams_msg->wavelength         = wavelength;
  rparams_msg->xmit_peak_pwr      = xmitPeakPower;
  rparams_msg->receiver_mds       = receiverMds;
  rparams_msg->receiver_gain      = receiverGain;
  rparams_msg->antenna_gain       = antennaGain;
  rparams_msg->system_gain        = systemGain;
  rparams_msg->unambig_vel        = unambigVelocity;
  rparams_msg->unambig_range      = unambigRange;

  rparams_msg->measXmitPowerDbmH  = measXmitPowerDbmH;
  rparams_msg->measXmitPowerDbmV  = measXmitPowerDbmV;
  
  STRncopy( rparams_msg->radar_name, radarName.c_str(), DS_LABEL_LEN );
  STRncopy( rparams_msg->scan_type_name, scanTypeName.c_str(), DS_LABEL_LEN );
  
}

///////////////////////////////
// convert enum ints to strings

string DsRadarParams::radarType2Str(int rtype)

{
  switch (rtype) {
    case DS_RADAR_AIRBORNE_FORE_TYPE: {
      return "DS_RADAR_AIRBORNE_FORE_TYPE";
    }
    case DS_RADAR_AIRBORNE_AFT_TYPE: {
      return "DS_RADAR_AIRBORNE_AFT_TYPE";
    }
    case DS_RADAR_AIRBORNE_TAIL_TYPE: {
      return "DS_RADAR_AIRBORNE_TAIL_TYPE";
    }
    case DS_RADAR_AIRBORNE_LOWER_TYPE: {
      return "DS_RADAR_AIRBORNE_LOWER_TYPE";
    }
    case DS_RADAR_AIRBORNE_UPPER_TYPE: {
      return "DS_RADAR_AIRBORNE_UPPER_TYPE";
    }
    case DS_RADAR_SHIPBORNE_TYPE: {
      return "DS_RADAR_SHIPBORNE_TYPE";
    }
    case DS_RADAR_VEHICLE_TYPE: {
      return "DS_RADAR_VEHICLE_TYPE";
    }
    case DS_RADAR_GROUND_TYPE:
    default:{
      return "DS_RADAR_GROUND_TYPE";
    }
  }
}

string DsRadarParams::scanMode2Str(int mode)

{
  switch (mode) {
    case DS_RADAR_CALIBRATION_MODE: {
      return "DS_RADAR_CALIBRATION_MODE";
    }
    case DS_RADAR_SECTOR_MODE: {
      return "DS_RADAR_SECTOR_MODE";
    }
    case DS_RADAR_COPLANE_MODE: {
      return "DS_RADAR_COPLANE_MODE";
    }
    case DS_RADAR_RHI_MODE: {
      return "DS_RADAR_RHI_MODE";
    }
    case DS_RADAR_VERTICAL_POINTING_MODE: {
      return "DS_RADAR_VERTICAL_POINTING_MODE";
    }
    case DS_RADAR_TARGET_MODE: {
      return "DS_RADAR_TARGET_MODE";
    }
    case DS_RADAR_MANUAL_MODE: {
      return "DS_RADAR_MANUAL_MODE";
    }
    case DS_RADAR_IDLE_MODE: {
      return "DS_RADAR_IDLE_MODE";
    }
    case DS_RADAR_SURVEILLANCE_MODE: {
      return "DS_RADAR_SURVEILLANCE_MODE";
    }
    case DS_RADAR_AIRBORNE_MODE: {
      return "DS_RADAR_AIRBORNE_MODE";
    }
    case DS_RADAR_HORIZONTAL_MODE: {
      return "DS_RADAR_HORIZONTAL_MODE";
    }
    case DS_RADAR_SUNSCAN_MODE: {
      return "DS_RADAR_SUNSCAN_MODE";
    }
    case DS_RADAR_POINTING_MODE: {
      return "DS_RADAR_POINTING_MODE";
    }
    case DS_RADAR_FOLLOW_VEHICLE_MODE: {
      return "DS_RADAR_FOLLOW_VEHICLE_MODE";
    }
    case DS_RADAR_EL_SURV_MODE: {
      return "DS_RADAR_EL_SURV_MODE";
    }
    case DS_RADAR_MANPPI_MODE: {
      return "DS_RADAR_MANPPI_MODE";
    }
    case DS_RADAR_MANRHI_MODE: {
      return "DS_RADAR_MANRHI_MODE";
    }
    case DS_RADAR_SUNSCAN_RHI_MODE: {
      return "DS_RADAR_SUNSCAN_RHI_MODE";
    }
    default: {
      return "DS_RADAR_SURVEILLANCE_MODE";
    }
  }
}

string DsRadarParams::followMode2Str(int mode)

{
  switch (mode) {
    case DS_RADAR_FOLLOW_MODE_UNKNOWN: {
      return "DS_RADAR_FOLLOW_MODE_UNKNOWN";
    }
    case DS_RADAR_FOLLOW_MODE_NONE: {
      return "DS_RADAR_FOLLOW_MODE_NONE";
    }
    case DS_RADAR_FOLLOW_MODE_SUN: {
      return "DS_RADAR_FOLLOW_MODE_SUN";
    }
    case DS_RADAR_FOLLOW_MODE_VEHICLE: {
      return "DS_RADAR_FOLLOW_MODE_VEHICLE";
    }
    case DS_RADAR_FOLLOW_MODE_AIRCRAFT: {
      return "DS_RADAR_FOLLOW_MODE_AIRCRAFT";
    }
    case DS_RADAR_FOLLOW_MODE_TARGET: {
      return "DS_RADAR_FOLLOW_MODE_TARGET";
    }
    case DS_RADAR_FOLLOW_MODE_MANUAL: {
      return "DS_RADAR_FOLLOW_MODE_MANUAL";
    }
    default: {
      return "DS_RADAR_FOLLOW_MODE_UNKNOWN";
    }
  }
}

string DsRadarParams::polType2Str(int ptype)

{
  switch (ptype) {
    case DS_POLARIZATION_HORIZ_TYPE: {
      return "DS_POLARIZATION_HORIZ_TYPE";
    }
    case DS_POLARIZATION_VERT_TYPE: {
      return "DS_POLARIZATION_VERT_TYPE";
    }
    case DS_POLARIZATION_RIGHT_CIRC_TYPE: {
      return "DS_POLARIZATION_RIGHT_CIRC_TYPE";
    }
    case DS_POLARIZATION_ELLIPTICAL_TYPE: {
      return "DS_POLARIZATION_ELLIPTICAL_TYPE";
    }
    case DS_POLARIZATION_LEFT_CIRC_TYPE: {
      return "DS_POLARIZATION_LEFT_CIRC_TYPE";
    }
    case DS_POLARIZATION_DUAL_TYPE: {
      return "DS_POLARIZATION_DUAL_TYPE";
    }
    case DS_POLARIZATION_DUAL_HV_ALT: {
      return "DS_POLARIZATION_DUAL_HV_ALT";
    }
    case DS_POLARIZATION_DUAL_HV_SIM: {
      return "DS_POLARIZATION_DUAL_HV_SIM";
    }
    case DS_POLARIZATION_DUAL_H_XMIT: {
      return "DS_POLARIZATION_DUAL_H_XMIT";
    }
    case DS_POLARIZATION_DUAL_V_XMIT: {
      return "DS_POLARIZATION_DUAL_V_XMIT";
    }
    default: {
      return "DS_POLARIZATION_HORIZ_TYPE";
    }
  }
}

string DsRadarParams::prfMode2Str(int mode)

{
  switch (mode) {
    case DS_RADAR_PRF_MODE_NOT_SET: {
      return "DS_RADAR_PRF_MODE_NOT_SET";
    }
    case DS_RADAR_PRF_MODE_STAGGERED_2_3: {
      return "DS_RADAR_PRF_MODE_STAGGERED_2_3";
    }
    case DS_RADAR_PRF_MODE_STAGGERED_3_4: {
      return "DS_RADAR_PRF_MODE_STAGGERED_3_4";
    }
    case DS_RADAR_PRF_MODE_STAGGERED_4_5: {
      return "DS_RADAR_PRF_MODE_STAGGERED_4_5";
    }
    case DS_RADAR_PRF_MODE_FIXED:
    default: {
      return "DS_RADAR_PRF_MODE_FIXED";
    }
  }
}


