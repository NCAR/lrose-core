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
// $Id: ArcBeamMsg.hh,v 1.3 2016/03/06 23:53:40 dixon Exp $
//
// HiQ Message class
/////////////////////////////////////////////////////////

#ifndef ArcBeamMsg_hh
#define ArcBeamMsg_hh

#include <math.h>
#include <iostream>

#include <dataport/port_types.h>
#include <rapformats/ds_radar.h>

#include "HiqMsg.hh"
#include "ProductConstants.hh"

using namespace std;


class ArcBeamMsg : public HiqMsg
{

public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    DATA_SIMPLEPP = 0,
    DATA_POLYPP,
    DATA_DUALPP,
    DATA_POL1,
    DATA_POL2,
    DATA_POL3,
    DATA_SIMPLE16,
    DATA_DOW,
    DATA_FULLPOL1,
    DATA_FULLPOLP,
    DATA_MAXPOL,
    DATA_HVSIMUL,
    DATA_SHRTPUL,
    DATA_SMHVSIM,
    DATA_STOKES,   // This type and following are new
    DATA_FFT,
    PIRAQ_ABPDATA,
    PIRAQ_ABPDATA_STAGGER_PRT
  } data_format_t;


  ////////////////////
  // Public methods //
  ////////////////////

  ArcBeamMsg(const bool debug = false);
  ArcBeamMsg(const ArcBeamMsg &rhs);
  ~ArcBeamMsg();

  bool init(const char *buffer);

  void print(ostream &stream) const;
  void printSummary(ostream &stream) const;


  ////////////////////
  // Access methods //
  ////////////////////

  // Set methods

  virtual void setAzimuth(const double azimuth)
  {
    _msgHeader.az = azimuth;
  }

  virtual void setElevation(const double elevation)
  {
    _msgHeader.el = elevation;
  }

  // Get methods

  virtual si16 *getAbp() const
  {
    return _abp;
  }

  virtual double getAntennaGain() const
  {
    return _msgHeader.antenna_gain;
  }

  virtual double getAzimuth() const
  {
    return _msgHeader.az;
  }

  virtual int getBeamNum() const
  {
    return _msgHeader.beam_num;
  }
  
  virtual data_format_t getDataFormat() const
  {
    return (data_format_t)_msgHeader.data_format;
  }

  virtual double getDataSysSat() const
  {
    return _msgHeader.data_sys_sat;
  }

  virtual time_t getDataTime() const
  {
    return _msgHeader.secs;
  }

  virtual double getElevation() const
  {
    return _msgHeader.el;
  }

  virtual double getFrequency() const
  {
    return _msgHeader.frequency;
  }

  virtual double getGateSpacing() const   // m
  {
    return _msgHeader.rcvr_pulse_width * HALF_LIGHT_SPEED;
  }

  virtual int getHits(void) const
  {
    return _msgHeader.hits;
  }

  virtual double getHorizBeamWidth() const
  {
    return _msgHeader.h_beam_width;
  }
  
  virtual double getHorizXmitPower() const
  {
    //return _logToLinear(_msgHeader.h_xmit_power);
    return _msgHeader.h_measured_xmit_power;
  }

  virtual msg_type_t getMsgType() const
  {
    return ARC_BEAM_MSG;
  }

  virtual double getNoisePower() const
  {
    return _msgHeader.noise_power;
  }

  virtual int getNumGates() const
  {
    return _msgHeader.gates;
  }

  virtual double getPeakPower() const
  {
//    double peak_power_log = _msgHeader.gate0_mag + _msgHeader.xmit_coupler;

//    return _logToLinear(peak_power_log);
    return _msgHeader.h_xmit_power;
  }

  virtual double getPhaseOffset() const
  {
    return _msgHeader.phase_offset;
  }

  virtual int getPolarization() const
  {
    string polarization = (char *)_msgHeader.polarization;
    
    if (polarization == "H")
      return DS_POLARIZATION_HORIZ_TYPE;
    if (polarization == "V")
      return DS_POLARIZATION_VERT_TYPE;
    if (polarization == "RC")
      return DS_POLARIZATION_RIGHT_CIRC_TYPE;
    if (polarization == "LC")
      return DS_POLARIZATION_LEFT_CIRC_TYPE;
    
    // Default to horizontal since there isn't an unknown type

    return DS_POLARIZATION_HORIZ_TYPE;
  }
  
  virtual double getPrf(void) const
  {
    return 1.0 / _msgHeader.prt[0];
  }

  virtual double getPrt(void) const
  {
    return _msgHeader.prt[0];
  }

  virtual double getPrt2(void) const
  {
    return _msgHeader.prt[1];
  }

  virtual double getRadarAltitude() const
  {
    return _msgHeader.radar_altitude;
  }

  virtual double getRadarConstant() const
  {
    return _msgHeader.rconst;
  }
  
  virtual double getRadarLatitude() const
  {
    return _msgHeader.radar_latitude;
  }

  virtual double getRadarLongitude() const
  {
    return _msgHeader.radar_longitude;
  }

  virtual void setRadarLatitude(double  lat)
  {
    _msgHeader.radar_latitude = lat;
  }

  virtual void setRadarLongitude(double lon)
  {
    _msgHeader.radar_longitude = lon;
  }

  virtual string getRadarName() const
  {
    return (char *)_msgHeader.radar_name;
  }

  virtual double getRcvrPulseWidth(void) const
  {
    return _msgHeader.rcvr_pulse_width;
  }

  virtual double getReceiverGain() const
  {
    return _msgHeader.receiver_gain;
  }

  virtual int getSamplesPerBeam(void) const
  {
    return _msgHeader.hits;
  }

  virtual int getScanType(void) const
  {
    return _msgHeader.scan_type;
  }

  virtual double getStartRange() const
  {
    return _msgHeader.meters_to_first_gate;
  }

  virtual double getTargetElevation(void) const
  {
    return _msgHeader.el;
  }

  virtual int getTiltNumber(void) const
  {
    return _msgHeader.scan_num;
  }

  virtual double getUnambiguousRange(void) const
  {
    return _msgHeader.prt[0] * 299.79 * 500.0;
  }

  virtual double getVertBeamWidth() const
  {
    return _msgHeader.v_beam_width;
  }
  
  virtual double getVertNoisePower() const
  {
    return _msgHeader.v_noise_power;
  }

  virtual double getVertRcvrGain() const
  {
    return _msgHeader.v_receiver_gain;
  }

  virtual double getVertXmitPower() const
  {
    return _msgHeader.v_xmit_power;
  }

  virtual int getVolumeNumber(void) const
  {
    return _msgHeader.vol_num;
  }

  virtual double getWavelength() const
  {
    return ProductConstants::C / 10000.0 / _msgHeader.frequency;
  }

  virtual double getXmitPulsewidth() const
  {
    return _msgHeader.xmit_pulse_width;
  }


private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const int HEADER_SIZE;

  static const int PIRAQ_CLOCK_FREQ;
  static const double HALF_LIGHT_SPEED;
  
  // These constants must be defined here because they are used
  // in the structure definition.

#define ARC_DESC_LEN 4
#define ARC_NUM_PRT 4
#define ARC_MAX_SEGMENTS 8
#define ARC_NUM_CLUTTER_REGIONS 4
#define ARC_MAX_GPS_DATUM 8
#define ARC_MAX_RADAR_NAME 16
#define ARC_MAX_CHANNEL_NAME 16
#define ARC_MAX_PROJECT_NAME 16
#define ARC_MAX_OPERATOR_NAME 12
#define ARC_MAX_SITE_NAME 12
#define ARC_POLARIZATION_STR_LEN 4
#define ARC_NUM_TEST_PULSE_RINGS 2
#define ARC_SZ_COMMENT 64
#define ARC_TRANSFORM_NUMX 2
#define ARC_TRANSFORM_NUMY 2
#define ARC_TRANSFORM_NUMZ 2
#define ARC_NUM_STOKES 4
#define ARC_NUM_SPARES 3


  ///////////////////
  // Private types //
  ///////////////////

  // Message header.  The fields appear with these exact sizes at the
  // beginning of each message.  Note that if any of these types change,
  // the associated byte swapping call for that field in the init() method
  // must also be changed.  If a change is made that violates word boundaries,
  // the code will need to be changed.

  typedef struct
  {
    ui32 record_len;              // Total length of record
    ui08 description[ARC_DESC_LEN];
                                  // == 'DWLX'
    ui32 channel;                 // e.g., RapidDOW range 0-5
    ui32 rev;                     // Format revision number
    ui32 one;                     // Always set to 1 (endian flag)
    ui32 byte_offset_to_data;
    ui32 data_format;
    ui32 type_of_compression;
    ui64 pulse_num;               // Number of transmitted pulses since Jan
                                  //   2007.  It is assumed that the first pulse
                                  //   (pulse_num  = 0) falls exactly at the
                                  //   midnight Jan 1, 1970 epoch.  To get unix
                                  //   time, multiply by the PRT.  The PRT is a
                                  //   rational number a/b.  More specifically
                                  //   N/Fc where Fc is the counter clock
                                  //   (PIRAQ_CLOCK_FREQ), and N is the divider
                                  //   number.  So you can get unix time without
                                  //   roundoff error by:
                                  //     secs = pulse_num * N / Fc.
                                  //   The nanosecs field is derived without
                                  //   roundoff error by:
                                  //     100 * (pulse_num * N % Fc).
    ui64 beam_num;                // Number of beams since Jan 1, 1970.  The
                                  //   first beam (beam_num = 0) was completed
                                  //   exactly at the epoch.
                                  //   beam_num = pulse_num / _hits.
    ui32 gates;
    ui32 start_gate;
    ui32 hits;
    ui32 ctrl_flags;              // Equivalent to _packetFlag below?
    ui32 bytes_per_gate;
    fl32 rcvr_pulse_width;
    fl32 prt[ARC_NUM_PRT];
    fl32 meters_to_first_gate;
    ui32 num_segments;            // Number of segments in beam
    fl32 gate_spacing_meters[ARC_MAX_SEGMENTS];
    ui32 gates_in_segment[ARC_MAX_SEGMENTS];
                                  // Number of gates in this segment
    ui32 clutter_start[ARC_NUM_CLUTTER_REGIONS];
                                  // Start gate of clutter filtered region
    ui32 clutter_end[ARC_NUM_CLUTTER_REGIONS];
                                  // End gate of clutter filtered region
    ui32 clutter_type[ARC_NUM_CLUTTER_REGIONS];
                                  // Type of clutter filtering applied
    ui32 secs;                    // Unix standard time -- seconds since 1/1/1970
                                  //   = _pulseNum * N / PIRAQ_CLOCK_FREQ
    ui32 nanoseconds;             // Within this second
    fl32 az;                      // Azimuth: referenced to 9550 MHz.  Possibly
                                  //   modified to be relative to true North.
    fl32 az_off_ref;              // Azimuth offset off reference
    fl32 el;                      // Elevation: referenced to 9550 MHz.
    fl32 el_off_ref;              // Elevation offset off reference
    fl32 radar_longitude;
    fl32 radar_latitude;
    fl32 radar_altitude;          // Units???
    ui08 gps_datum[ARC_MAX_GPS_DATUM];
                                  // e.g. "NAD27"
    ui32 ts_start_gate;           // Starting time series gate, set to 0 for none
    ui32 ts_end_gate;             // Ending time series gate, sto to 0 for none
    fl32 ew_velocity;
    fl32 ns_velocity;
    fl32 vert_velocity;
    fl32 fxd_angle;               // In degrees instead of counts
    fl32 true_scan_rate;          // degrees/second
    ui32 scan_type;
    ui32 scan_num;
    ui32 vol_num;
    ui32 transition;
    fl32 h_xmit_power;
    fl32 yaw;
    fl32 pitch;
    fl32 roll;
    fl32 track;
    fl32 gate0_mag;               // Magnetron sample amplitude
    fl32 dacv;
    ui32 packet_flag;
    ui32 year;                    // e.g. 2003
    ui32 julian_day;
    ui08 radar_name[ARC_MAX_RADAR_NAME];
    ui08 channel_name[ARC_MAX_CHANNEL_NAME];
    ui08 project_name[ARC_MAX_PROJECT_NAME];
    ui08 operator_name[ARC_MAX_OPERATOR_NAME];
    ui08 site_name[ARC_MAX_SITE_NAME];
    ui08 polarization[ARC_POLARIZATION_STR_LEN];
    fl32 test_pulse_pwr;
    fl32 test_pulse_frq;
    fl32 frequency;                // Radar transmit frequency
    fl32 stalo_freq;               // Radar local oscillator frequency
    fl32 noise_figure;
    fl32 noise_power;
    fl32 receiver_gain;
    fl32 e_plane_angle;            // Offsets from normal pointing angle
    fl32 h_plane_angle;
    fl32 data_sys_sat;
    fl32 antenna_gain;
    fl32 h_beam_width;
    fl32 v_beam_width;
    fl32 xmit_pulse_width;
    fl32 rconst;
    fl32 phase_offset;
    fl32 zdr_fudge_factor;
    fl32 mismatch_loss;
    fl32 rcvr_const;
    fl32 test_pulse_rngs_km[ARC_NUM_TEST_PULSE_RINGS];
    fl32 antenna_rotation_angle;  // S-Pol 2nd frequency antenna may be 30
                                  //   degrees off vertical
    ui08 comment[ARC_SZ_COMMENT];
    fl32 i_norm;                  // Normalization for timeseries
    fl32 q_norm;
    fl32 i_compand;               // Companding (compression) parameters
    fl32 q_compand;
    fl32 transform_matrix[ARC_TRANSFORM_NUMX][ARC_TRANSFORM_NUMY][ARC_TRANSFORM_NUMZ];
    fl32 stokes[ARC_NUM_STOKES];
    fl32 v_xmit_power;
    fl32 v_receiver_gain;
    fl32 v_antenna_gain;
    fl32 v_noise_power;
    fl32 h_measured_xmit_power;
    fl32 v_measured_xmit_power;
    ui32 angle_source;            // See angle source definitions
    ui32 timing_mode;
    fl32 tp_width;                // Trigger width as in DOS code but expressed
                                  //   in seconds
    fl32 tp_delay;                // Trigger delay as in DOS code but expressed
                                  //   in seconds
    fl32 delay;                   // Delay as in DOS code but expressed in
                                  //   seconds
    fl32 pll_freq;                // Frequency command to L0 expressed in Hz
    fl32 pll_alpha;               // AFC time constant 0 < pll_alpha < 1.
                                  //   Averaging time ~= 1.0/(PRT*(1.0 - pll_alpha))
    fl32 afc_lo;                  // AFC parameters: typical _afcLo = 95e6 Hz
    fl32 afc_hi;                  //                 typical _afcHi = 120e6 Hz
    fl32 afc_gain;                //                 typical _afcGain = 3.0
    fl32 vel_sign;                // +/-1 to reverse sense of velocities, if
                                  //   necessary. Value depends on radar
                                  //   characteristics.
    fl32 spare[ARC_NUM_SPARES];
    fl32 xmit_coupler;            // Calibrates G0 vs measured xmit power.
                                  //   Changes asame sign as power reading.
  } _msgHeader_t;
  
  
  /////////////////////
  // Private members //
  /////////////////////

  // Header received for this message

  _msgHeader_t _msgHeader;
  
  // The beam data as it appears in the message

  si16 *_abp;
  int _abpSize;
  

  /////////////////////
  // Private methods //
  /////////////////////

  static double _logToLinear(const double value)
  {
    return pow(10.0, 0.1 * value);
  }
  
  static void _printString(ostream &stream, const string &label,
			   const ui08 *value, const int num_chars);
  

};

#endif
