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
// TdwrRadxFile.hh
//
// Support for TDWR data format
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2013
//
///////////////////////////////////////////////////////////////

#ifndef TdwrRadxFile_HH
#define TdwrRadxFile_HH

#include <cstring>
#include <string>
#include <vector>
#include <cmath>

#include <Radx/Radx.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxBuf.hh>

class RadxVol;
class RadxRay;
using namespace std;

///////////////////////////////////////////////////////////////
/// FILE IO CLASS FOR TDWR FILE FORMAT
///
/// This subclass of RadxFile handles I/O for Tdwr files.

class TdwrRadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  TdwrRadxFile();
  
  /// Destructor
  
  virtual ~TdwrRadxFile();
  
  /// clear all data
  
  virtual void clear();

  //////////////////////////////////////////////////////////////
  /// \name File inspection:
  //@{
  
  /// Check if specified file is a Tdwr file.
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if the specifed file is a Tdwr file.
  ///
  /// Returns true on success, false on failure.
  
  bool isTdwr(const string &path);
  
  /// Check if this file needs to be byte-swapped on this host.
  ///
  /// Returns 0 on success, -1 on failure
  ///
  /// Use isSwapped() to get result after making this call.
  
  int checkIsSwapped(const string &path);
  
  /// Get the result of checkIsSwapped().
  ///
  /// If true, file is byte-swapped with respect to the native 
  /// architecture, and must be swapped on read.
  
  bool isSwapped() { return _tdwrIsSwapped; }

  //@}
  
  //////////////////////////////////////////////////////////////
  /// \name Perform writing:
  //@{
  
  //////////////////////////////////////////////////////////////
  /// Writing in Tdwr format is not supported.
  /// 
  /// Data will be written in CfRadial instead.
  
  virtual int writeToDir(const RadxVol &vol,
                         const string &dir,
                         bool addDaySubDir,
                         bool addYearSubDir);
  
  //////////////////////////////////////////////////////////////
  /// Writing in Tdwr format is not supported.
  /// 
  /// Data will be written in CfRadial instead.
  
  virtual int writeToPath(const RadxVol &vol,
                          const string &path);

  //@}

  //////////////////////////////////////////////////////////////
  /// \name Perform the read:
  //@{
  
  /// Read in data file from specified path,
  /// load up volume object.
  /// Returns 0 on success, -1 on failure
  /// Use getErrStr() if error occurs
  
  virtual int readFromPath(const string &path,
                           RadxVol &vol);

  //@}

  ////////////////////////
  /// \name Printing:
  //@{
  
  /// Print summary after read.
  
  virtual void print(ostream &out) const;
  
  /// Print Tdwr data file in native format.
  ///
  /// Returns 0 on success, -1 on failure.
  ///
  /// Use getErrStr() if error occurs
  
  virtual int printNative(const string &path, ostream &out,
                          bool printRays, bool printData);

  //@}

protected:
private:

  // constants

  static const int BEGINNING_OF_VOL_SCAN = 16384; /* bit 14 - new volume*/
  static const int END_OF_VOL_SCAN = 32768; /* bit 15 - end of volume*/
  static const int START_OF_NEW_ELEVATION = 4194304; /* bit 22 - new elevation */
  static const int SCAN_STRATEGY = 255;	/* bits 0 - 7 scan strategy */
  static const int SECTOR_SCAN = 256; /* bit 8 - sector scan */
  static const int LOW_PRF = 1; /* bit 0 - low prf scan */
  static const int DUMMY_RECORD = 4; /* bit 2 - dummy record */
  static const int SCAN_RESTART = 1; /* bit 0 - elevation scan restart -
                                        record will contain no base data */
  static const int VOL_RESTART = 2; /* bit 1 - a volume scan restart - 
                                       record will contain no base data */
  static const int INCOMP_ELEV = 4; /* bit 2 - incomplete elevation -
                                       record will contain no base data */
  static const int INCOMP_VOL = 8; /* bit 3 - incomplete volume - record
                                      will contain no base data */
  static const int SECTOR_BLANK = 32768;

  static const int SCAN_MONITOR = 1;
  static const int SCAN_HAZARDOUS = 2;
  static const int SCAN_CLEAR_AIR = 3;
  static const int SCAN_CLUTTER_COL = 4;
  static const int SCAN_CLUTTER_EDIT = 5;
  static const int SCAN_PPI = 6;
  static const int SCAN_RHI = 7;

  static const int REC_LEN = 6144;
  static const int TDWR_VEL_SCALE = 25;
  static const int TDWR_VEL_BIAS = -8000;

  static const int SNR_SCALE = 50;
  static const int SNR_BIAS = 0;
  static const int DBZ_SCALE = 50;
  static const int DBZ_BIAS = -3000;
  static const int VEL_SCALE = 31;   /* original 16 bit data has a scale */
  static const int VEL_BIAS = -4000; /* 25 with the bias of -80 */
  static const int SW_SCALE = 25;
  static const int SW_BIAS = 0;
  
  static const int NORMAL_PRF_RESOLUTION = 150;
  static const int LOW_PRF_RESOLUTION = 300;
  static const int RANGE_TO_FIRST_GATE = 450;
  static const int BEAM_WIDTH = 55;
  static const int PULSE_WIDTH = 1100;
  static const int FREQUENCY = 5600;
  static const int LOW_PRF_GATES = 600;
  static const int NORMAL_PRF_GATES = 1988;

  static const int NORMAL_PRF_BASE_DATA = 0x2B00;
  static const int LOW_PRF_BASE_DATA = 0x2B01;
  static const int LLWAS_SENSOR = 0x2C00;
  static const int LLWASIII_DATA = 0x2C01;
  static const int LLWASII_DATA = 0x2C02;
  static const int LLWASII_MAPPING = 0x4206;

  static const int CAF = 8;
  static const int CTF = 4;
  static const int CVF = 2;
  static const int CCV = 1;
  static const int CV =  64;

  // struct definitions

  typedef struct {

    Radx::ui16 message_id;
    Radx::ui16 message_length;

  } message_hdr_t;
  
  typedef struct {
    
    Radx::ui16 volume_count;

    Radx::ui16 volume_flag; /* the first 8 bits denote the
                             * scan strategy. Bit 14 -start new
                             * volume, bit 15 - volume end
                             */
    
    Radx::ui16 power_trans; /* peak transmitter power */

    Radx::ui16 playback_flag; /* flags for: start of playback,
                               * start of live and a dummy record
                               * indicater 
                               */ 

    Radx::ui32 scan_info_flag; /* flags to specify low prf,
                                * gust front, MB surface, low
                                * elevation, wind shift, precip 
                                * and resolution, MB aloft, 
                                * sector, velocity dealiasing,
                                * spike removal, obscuration 
                                * flagging, 8 bits reserved.
                                * flags for clutter map number,
                                * start of elevation, end of elev
                                * and contains the scan number 
                                */
    
    Radx::fl32 current_elevation;
    Radx::fl32 angular_scan_rate;
    Radx::ui16 pri; /* Pulse Repetion Interval */
    Radx::ui16 dwell_flag; /* pulses per dwell, solar 
                            * indicator, and dwell ID
                            */
    Radx::ui16 final_range_sample; /* specifies the last range 
                                    * sample that contains a valid 
                                    * radar return
                                    */
    Radx::ui16 rng_samples_per_dwell;	
    Radx::fl32 azimuth;
    Radx::fl32 total_noise_power; /* total noise power for each dwell
                                   * compensated for the effects
                                   * of solar flux
                                   */
    Radx::ui32 timestamp;
    
    Radx::ui16 base_data_type; /* indicates unconditioned, edited
                                * and fully conditioned data types
                                */
    
    Radx::ui16 vol_elev_status_flag; /* indicates incomplete and 
                                      * restarted volumes and elevations
                                      */
    Radx::ui16 integer_azimuth;
    Radx::ui16 load_shed_final_sample;
    
  } data_hdr_t;

  typedef struct {

    Radx::ui08 dbz;
    Radx::ui08 snr;
    Radx::ui16 uvel;
    Radx::ui08 width;
    Radx::ui08 caf;
    Radx::ui16 dvel;

  } normal_data_t;
  
  // file handle
  
  FILE *_file;

  // printing controls

  bool _doPrint;
  bool _printRays;
  bool _printData;
  
  // data buffer

  unsigned char _dataBuf[REC_LEN];

  // objects to be set on read
  
  bool _tdwrIsSwapped;
  message_hdr_t _mhdr;
  data_hdr_t _dhdr;
  
  // time limits
  
  time_t _startTimeSecs, _endTimeSecs;
  double _startNanoSecs, _endNanoSecs;
  
  // derived values
  
  int _volumeNumber;
  int _nGatesData;
  int _nGatesDwell;
  int _lastGate;
  int _nSamples;
  int _iAz;

  double _pulseWidthUs;
  double _wavelengthM;
  double _prf;
  double _prtSecs;
  double _nyquist;
  double _peakPowerW;
  double _scanRate;

  double _gateSpacingKm;
  double _startRangeKm;
  
  double _latitude;
  double _longitude;
  double _altitudeM;
  double _frequency;
  
  // private methods
  
  int _openRead(const string &path);
  void _close();
  
  bool _isTdwr(const message_hdr_t &mhdr);
  
  int _performRead(ostream &out);

  int _handleRay(ostream &out);

  int _setVolMetaData(const string &path);
  int _setRayData(RadxRay &ray);

  void _swap(message_hdr_t &hdr);
  void _swap(data_hdr_t &hdr);

  void _print(message_hdr_t &hdr,
              ostream &out);
  
  void _print(data_hdr_t &hdr,
              ostream &out);

};

#endif
