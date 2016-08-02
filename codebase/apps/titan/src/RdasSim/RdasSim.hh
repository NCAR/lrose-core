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
// RdasSim.h
//
// RdasSim object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
///////////////////////////////////////////////////////////////

#ifndef RdasSim_H
#define RdasSim_H

#include "Args.hh"
#include "Params.hh"
#include "RdasBeam.hh"
#include <toolsa/umisc.h>
#include <toolsa/Socket.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRadar.hh>
#include <string>
#include <sys/time.h>

using namespace std;

typedef enum {
  OP_MODE_COMMAND = 100,
  SCAN_MODE_COMMAND = 110,
  MAIN_POWER_COMMAND = 200,
  MAG_POWER_COMMAND = 210,
  SERVO_POWER_COMMAND = 220,
  RADIATE_COMMAND = 230,
  CONTROL_FLAGS_COMMAND = 240,
  N_GATES_COMMAND = 300,
  START_RANGE_COMMAND = 310,
  GATE_SPACING_COMMAND = 320,
  PRF_COMMAND = 330,
  AZ_COMMAND = 400,
  EL_COMMAND = 410,
  EL_ARRAY_COMMAND = 420,
  AZ_RATE_COMMAND = 500,
  EL_RATE_COMMAND = 510,
  MAX_AZ_RATE_COMMAND = 520,
  MAX_EL_RATE_COMMAND = 530,
  CONFIG_COMMAND = 600,
  DAC_COMMAND = 700,
  DATA_DECIMATE_COMMAND = 800
} command_t;

typedef enum {
  OP_OFF = 0,
  OP_STANDBY = 1,
  OP_CALIBRATE = 2,
  OP_RUN = 3
} op_mode_t;

typedef enum {
  SHUTDOWN_MODE_OFF = 0,
  SHUTDOWN_MODE_STANDBY = 1
} shutdown_op_mode_t;

typedef enum {
  SCAN_STOP = 0,
  SCAN_MANUAL = 1,
  SCAN_AUTO_VOL = 2,
  SCAN_AUTO_PPI = 3,
  SCAN_TRACK_SUN = 4
} scan_mode_t;

typedef enum {
  POLARIZATION_HORIZONTAL = 0,
  POLARIZATION_VERTICAL = 1,
  POLARIZATION_CIRCULAR = 2
} polarization_t;

// configuration command

typedef struct {
  si32 version;
  si32 struct_len;
  si32 ngates;
  si32 samples_per_az;
  si32 samples_per_gate;
  si32 polarization_code;
  si32 el_voltage_positive;
  si32 az_voltage_positive;
  si32 control_flags;
  si32 dac0;
  si32 dac1;
  si32 auto_reset_timeout;
  si32 shutdown_op_mode;
  si32 spare_ints[15];
  fl32 start_range;
  fl32 gate_spacing;
  fl32 prf;
  fl32 pulse_width;
  fl32 antenna_el_corr;
  fl32 antenna_az_corr;
  fl32 control_el_corr;
  fl32 control_az_corr;
  fl32 antenna_min_el;
  fl32 antenna_max_el;
  fl32 antenna_el_slew_rate;
  fl32 antenna_az_slew_rate;
  fl32 antenna_max_el_slew_rate;
  fl32 antenna_max_az_slew_rate;
  fl32 calib_slope;
  fl32 calib_offset;
  fl32 elev_tolerance;
  fl32 ppi_az_overlap;
  fl32 spare_floats[18];
} config_t;

class RdasSim {
  
public:

  // constructor

  RdasSim (int argc, char **argv);

  // destructor
  
  ~RdasSim();

  // run 

  int Run();

  // data members

  int OK;

protected:
  
private:

  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  DsMdvx _mdvx;

  int _kbdFd;
  string _kbdCmd;
  bool _inEscapeSeq;
  bool _inKeySeq;

  struct timeval _prevBeamTime;
  struct timezone _timezone;
  
  bool _mainPower;
  bool _systemReady;
  bool _servoPower;
  bool _radiate;
  bool _startVol;

  int _version;
  int _struct_len;

  int _statusFlags;
  int _errorFlags;
  int _dac0;
  int _dac1;
  int _autoResetTimeout;
  int _shutdownOpMode;

  int _nGates;
  int _samplesPerAz;
  int _samplesPerGate;
  polarization_t _polarizationCode;
  bool _elVoltagePositive;
  bool _azVoltagePositive;

  double _startRange;
  double _gateSpacing;
  double _prf;
  double _pulseWidth;
  double _antennaElCorr;
  double _antennaAzCorr;
  double _controlElCorr;
  double _controlAzCorr;
  double _antennaMinEl;
  double _antennaMaxEl;
  double _antennaElSlewRate;
  double _antennaAzSlewRate;
  double _antennaMaxElSlewRate;
  double _antennaMaxAzSlewRate;
  double _calibSlope;
  double _calibOffset;
  double _elevTolerance;
  double _ppiAzOverlap;
  double _analog_status[RDAS_BEAM_NSTATUS];

  vector<double> _elList;

  double _requestedEl;
  double _requestedAz;
  double _actualEl;
  double _actualAz;
  double _azSlewRate;
  double _elSlewRate;

  int _pulseGateStart;
  int _pulseGateEnd;
  int _pulseHeightCount;
  
  op_mode_t _opMode;
  scan_mode_t _scanMode;
    
  double _sumAzPpi;
  int _volElevIndex;
  bool _endOfPpi;

  bool _endOfTiltFlag;
  bool _endOfVolFlag;

  int _bprpRayCount;

  char _statusStr[1024];

  int _readInputFile();
  int _serveClient(Socket &client);
  int _readCommand(Socket &client);
  int _sendBeam(Socket &client);
  int _sendCalib(Socket &client);
  int _sendStatus(Socket &client);
  void _handleCommand(si32 command, si32 len, ui08 *buf);
  void _adjPosn(double adjSecs);
  bool _adjustElAz(double el, double az, double adjSecs);
  bool _adjustElSlewAz(double el, double adjSecs);
  double _slewAz(double adjSecs);

  void _readKbd();
  int _readKbdChar(char &cc);
  int _readKbdSelect(long wait_msecs);

  string _opMode2Str(op_mode_t mode);
  string _scanMode2Str(scan_mode_t mode);
  string _command2Str(command_t command);
  string _polarization2Str(polarization_t polarization);
  string _flags2Str(int flags);

  int _serveBprpClient(Socket &client);
  void _setupBprpSim();
  int _sendBprpBeam(Socket &client);

};

#endif
