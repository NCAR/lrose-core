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
// NsslMrdRadxFile.hh
//
// Support for NSSL MRD data format
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2015
//
///////////////////////////////////////////////////////////////

#ifndef NsslMrdRadxFile_HH
#define NsslMrdRadxFile_HH

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
/// FILE IO CLASS FOR NSSL MRD FILE FORMAT
///
/// This subclass of RadxFile handles I/O for NsslMrd files.

class NsslMrdRadxFile : public RadxFile

{
  
public:

  /// Constructor
  
  NsslMrdRadxFile();
  
  /// Destructor
  
  virtual ~NsslMrdRadxFile();
  
  /// clear all data
  
  virtual void clear();

  //////////////////////////////////////////////////////////////
  /// \name File inspection:
  //@{
  
  /// Check if specified file is a NsslMrd file.
  /// Returns true on success, false on failure
  
  virtual bool isSupported(const string &path);
    
  /// Check if the specifed file is a NsslMrd file.
  ///
  /// Returns true on success, false on failure.
  
  bool isNsslMrd(const string &path);
  
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
  
  bool isSwapped() { return _mrdIsSwapped; }

  //@}
  
  //////////////////////////////////////////////////////////////
  /// \name Perform writing:
  //@{
  
  //////////////////////////////////////////////////////////////
  /// Writing in NsslMrd format is not supported.
  /// 
  /// Data will be written in CfRadial instead.
  
  virtual int writeToDir(const RadxVol &vol,
                         const string &dir,
                         bool addDaySubDir,
                         bool addYearSubDir);
  
  //////////////////////////////////////////////////////////////
  /// Writing in NsslMrd format is not supported.
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
  
  /// Print NsslMrd data file in native format.
  ///
  /// Returns 0 on success, -1 on failure.
  ///
  /// Use getErrStr() if error occurs
  
  virtual int printNative(const string &path, ostream &out,
                          bool printRays, bool printData);

  // compute and return output file name
  
  string computeFileName(int volNum,
                         string instrumentName,
                         int year, int month, int day,
                         int hour, int min, int sec);
  
  //@}

protected:
private:

  ////////////////////////////////
  // HEADERS
  
  // main header
  // 45 * 16-bit words
  
  typedef struct mrd_header {
    
    Radx::si16 word_1;
    Radx::si16 word_2;
    Radx::si16 word_3;
    Radx::si16 raw_rot_ang_x10; /* deg. */
    Radx::si16 lat_deg;
    Radx::si16 lat_min;
    Radx::si16 lat_sec_x10;
    Radx::si16 lon_deg;
    Radx::si16 lon_min;
    Radx::si16 lon_sec_x10;
    Radx::si16 altitude; /* meters */
    Radx::si16 roll_x10; /* deg. */
    Radx::si16 heading_x10; /* deg. */
    Radx::si16 drift_x10; /* deg. */
    Radx::si16 pitch_x10; /* deg. */
    Radx::si16 raw_tilt_x10;
    Radx::si16 nyq_vel_x10; /* m/s */
    Radx::si16 julian_date;
    Radx::si16 azimuth_samples;
    Radx::si16 gate_length; /* m */
    Radx::si16 range_delay; /* m */
    Radx::si16 ground_speed_x64; /* m/s */
    Radx::si16 vert_airspeed_x64; /* m/s */
    char flight_number[8];
    char storm_name[12];
    Radx::si16 wind_dir_x10; /* deg. */
    Radx::si16 nav_flag;
    Radx::si16 wind_speed_x10; /* m/s */
    Radx::si16 noise_threshold;
    Radx::si16 corrected_tilt_x10; /* deg. */
    Radx::si16 num_good_gates;
    Radx::si16 gspd_vel_corr_x10; /* m/s */
    Radx::si16 sweep_num;
    Radx::si16 max_gates;
    Radx::si16 tilt_corr_flag;
    Radx::si16 altitude_flag;
    char aircraft_id[2];
    
  } mrd_header_t;
  
  // secondary header for date/time
  // 40 * 32-bit words
  
  typedef struct mrd_header2 {
    
    Radx::si32 year;
    Radx::si32 month;
    Radx::si32 day;
    Radx::si32 hour;
    Radx::si32 minute;
    Radx::si32 second;
    Radx::si32 spares[34];
    
  } mrd_header2_t;

  // file handle
  
  FILE *_file;
  
  // raw input buffer

  size_t _inBufSize;
  RadxBuf _inBuf;

  // data buffer
  
  size_t _dataBufSize;
  RadxBuf _datBuf;
  int _dataLen;
  unsigned char *_dataBuf;
  Radx::ui16 *_dataRec;

  // objects to be set on read

  int _nRaysRead;
  mrd_header_t _hdr;
  mrd_header2_t _hdr2;
  bool _mrdIsSwapped;

  // time limits

  time_t _startTimeSecs, _endTimeSecs;
  double _startNanoSecs, _endNanoSecs;

  // derived values

  static int _volumeNumber;
  int _nGates;
  int _nSamples;

  double _pulseWidthUs;
  double _frequencyHz;
  double _nyquist;
  
  double _gateSpacingKm;
  double _startRangeKm;

  double _latitude;
  double _longitude;
  double _altitudeM;

  // private methods
  
  int _openRead(const string &path);
  int _openWrite(const string &path);
  void _close();

  int _readRec();
  int _loadHeaders();
  void _handleRay();

  void _setVolMetaData(const string &instrumentName);
  void _setRayMetadata(RadxRay &ray);

  void _swap(Radx::si16 *vals, int n);
  void _swap(Radx::ui16 *vals, int n);
  void _swap(mrd_header_t &hdr);
  void _swap(mrd_header2_t &hdr);
  
  double _decodeDbz(int ival);
  double _decodeVel(int ival);
  
  void _print(const mrd_header_t &hdr, ostream &out);
  void _print(const mrd_header2_t &hdr, ostream &out);
  
  void _printRay(bool printData,
                 ostream &out);
  
  void _printFieldData(const string &fieldName,
                       const vector<double> &data,
                       ostream &out);
  
  void _printPacked(ostream &out, int count,
                    double val, double missing);
  
  string _getInstrumentNameFromPath(const string path);

};

#endif

/***********************************************************************
 *  c  Descriptor for NSSL/MRD disc file format airborne Doppler Data
 *  
 *  
 *  c  One sweep consists of up to 400 radials of data with each radial
 *  c  containing up to 1024 gates.
 *  c  Radial Velocity and Reflectivity data are stored in a "compressed
 *  c  sequential" format such that only the first N_Gates of data are stored.
 *  c  Hence the disc records are variable length
 *  c  End of sweeps (usually when the antenna passes through
 *  c  the vertical) are denoted by sweep marks (Ihead(41) = -1)
 *  
 *  c  Each beam consists of two physical records.
 *  c  The first record consists of a short header (45 integer*2 words) followed
 *  c  by a second header (40 integer*4 words).
 *  c  The data record then follows which consists of N_Gates of data.  The data
 *  c  is thresholded prior to writing the file by marching inward from the 
 *  c  end of the beam and finding the first good gate, calling that point 
 *  c  N_Gates.  Data is thresholded on board the aircraft by either returned
 *  c  power (dBm) or by spectral width.  This
 *  c  methodology seems to work well for an airborne radar since the beam usually
 *  c  spends most of its time pointing into outer space or below the surface.
 *  
 *  c  The format of the disc header record is as follows:
 *  c   Ihead(1) = unused
 *  c   Ihead(2) = unused
 *  c   Ihead(3) = unused
 *  c   Ihead(4) = Raw Rotation Angle [deg*10] from straight up
 *  c              i.e., not roll corrected, relative to the fuselage)
 *  c   Ihead(5) = Latitude [deg]
 *  c   Ihead(6) = Latitude [min]
 *  c   Ihead(7) = Latitude [sec*10.0]
 *  c   Ihead(8) = Longitude [deg]
 *  c   Ihead(9) = Longitude [min]
 *  c   Ihead(10) = Longitude [sec*10.0]
 *  c   Ihead(11) = Altitude [m] defined at load time as AGL (i.e., radar
 *  c               altitude) or MSL (i.e., pressure altitude appropriate
 *  c               for work over highly variable terrain)
 *  c   Ihead(12) = Roll [deg*10.0]
 *  c   Ihead(13) = Head [deg*10.0]
 *  c   Ihead(14) = Drift [deg*10.0]
 *  c   Ihead(15) = Pitch [deg*10.0]
 *  c   Ihead(16) = Raw "Tilt" angle [deg*10] fore/aft pointing angle relative
 *  c               to a plane normal to the longitudinal axis of the aircraft
 *  c   Ihead(17) = Nyquist velocity [m/s*10]
 *  c   Ihead(18) = Julian date
 *  c   Ihead(19) = # of azimuth samples
 *  c   Ihead(20) = Gate length [m]
 *  c   Ihead(21) = Range delay [m]
 *  c   Ihead(22) = Ground speed [m/s*64.0]
 *  c   Ihead(23) = Vertical airspeed [m/s*64.0]
 *  c   Ihead(24-27) = Flight number (8 ASCII characters)
 *  c   Ihead(28-33) = Storm name (12 ASCII characters)
 *  c   Ihead(34) = Wind dir [deg*10]
 *  c   Ihead(35) = flag for navigation system used to derive position and winds
 *  c               1=INE1; 2=INE2; 3=GPS
 *  c   Ihead(36) = Wind speed [m/s*10]
 *  c   Ihead(37) = Threshold [dBm] used to identify "noise" -999=spectral width
 *  c   Ihead(38) = "corrected" tilt angle for pitch, roll, and drift
 *  c   Ihead(39) = # of good gates 
 *  c   Ihead(40) = radial velocity correction for the ray due to ground speed
 *  c   Ihead(41) = Sweep Number
 *  c   Ihead(42) = Maximum number of gates
 *  c   Ihead(43) = Tilt correction flag (0=no; 1=yes) if data has been changed
 *  c   Ihead(44) = flag for altitude 0 for RA, 1 for PA
 *  c   Ihead(45) = Not Used
 *  
 *  c   The second header currently uses only the first 6 words:
 *  c     Iheadr(1) = year (00 - 99)
 *  c     Iheadr(2) = month (00 - 12)
 *  c     Iheadr(3) = day (00 - 31)
 *  c     Iheadr(4) = hour (00 - 23)
 *  c     Iheadr(5) = minute (00 - 59)
 *  c     Iheadr(6) = second (00 - 59)
 *  c
 *  c   The velocity and reflectivity data are scaled for each gate as
 *  c   a single 16 bit integer (to save disc space) as follows:
 *  c
 *  c                                 bit #
 *  c                             1111110000000000
 *  c                             5432109876543210
 *  c                             zzzzzzvvvvvvvvvv
 *  c    where:
 *  c          z: scaled reflectivity [6 bits] 0 - 63 dBZ
 *  c          v: velocity data [10 bits] +-68 m/s 0.15 m/s resolution
 *  c       If signal <noise then word is =-1
 *  
 *  
 *  Character*8 Flid
 *  Character*16 Project
 *  
 *  Dimension Hed(20), Vel(1024), Ref(1024), Itime(6)
 *  
 *  Integer*2 Ihead(45), Ibufr(1024), Ival
 *  Integer*4 Iheadr(40)
 *  
 *  c  First read the headers
 *  
 *  1 Read (75,End=1000,Err=999,Iostat=Ierr) (Ihead(i),i=1,45),
 *  #     (Iheadr(i),i=1,40)
 *  
 *  If (Ihead(41) .eq. -1) Go To 1    !   psuedo end of sweep mark
 *  
 *  N_Gates = Ihead(39)
 *  Max_Gates = Ihead(42)
 *  
 *  If (N_Gates .gt. 1024 .or. Max_Gates .gt. 1024 .or. N_Gates
 *  #    .gt. Max_Gates) Then
 *  Write (60,'("Probem with n_gates or max_gates",2i14)')
 *  #           N_Gates, Max_Gates
 *  Stop
 *  End If
 *  
 *  Itime(1) = Iheadr(1)   ! year without 1900
 *  Itime(2) = Iheadr(2)   ! month
 *  Itime(3) = Iheadr(3)   ! day
 *  Itime(4) = Iheadr(4)   ! hour
 *  Itime(5) = Iheadr(5)   ! minute
 *  Itime(6) = Iheadr(6)   ! second
 *  
 *  c  Next read the data record which is of variable length and could be 
 *  c  as large as 1024 16 bit integer words
 *  
 *  c  Note that only the first N_Gates out of Max_Gates are actually stored,
 *  c  since thresholding is performed prior to writing the file.
 *  
 *  Read (75,End=1000,Err=999,Iostat=Ierr) (Ibufr(i),i=1,N_Gates)
 *  
 *  c  Load the header information needed by the calling routine
 *  
 *  c  Latitude
 *  
 *  Hed(2) = Float(Ihead(5)) + Float(Ihead(6))/60.0 +
 *  #         Float(Ihead(7))/36000.0
 *  
 *  c  Longitude
 *  
 *  Hed(3) = Float(Ihead(8)) + Float(Ihead(9))/60.0 +
 *  #         Float(Ihead(10))/36000.0
 *  
 *  c  Gate Length
 *  
 *  Hed(4) = Float(Ihead(20))/1000.0
 *  
 *  c  Number of good gates in the ray
 *  
 *  Hed(5) = Float(Ihead(39))
 *  
 *  c  Sweep Number
 *  
 *  Hed(7) = Float(Ihead(41))
 *  
 *  c  Altitude (m)
 *  
 *  Hed(10) = Float(Ihead(11))
 *  
 *  c  Range delay (km) "RDEL"
 *  
 *  Hed(11) = Float(Ihead(21))/1000.0
 *  
 *  c  Which altitude flag
 *  
 *  Hed(12) = Float(Ihead(44))  ! =0 for RA, = 1 for PA
 *  
 *  c  Project and flight identifiers
 *  
 *  Write (Flid,'(4a2)') (Ihead(kk),kk=24,27)
 *  Write (Project,'(6a2,4H- - )') (Ihead(kk),kk=28,33)
 *  
 *  c  Nyquist interval (m/s)
 *  
 *  Xniq = Float(Ihead(17)) / 10.0
 *  
 *  c  Load up the data
 *  
 *  Do i = 1, Max_Gates
 *  If (i .le. N_Gates) Then
 *  Ival = Ibufr(i)
 *  Else
 *  Ival = -1
 *  End If
 *  
 *  If (Ival .eq. -1) Then
 *  Vel(i) = -999.0
 *  Ref(i) = -999.0
 *  Else
 *  Ivel = Ibits(Ival,0,10)
 *  Vel(i) = Float(Ivel - 511) / 7.5
 *  Ref(i) = Ibits (Ival,10,6)
 *  End If
 *  End Do
 *  
 *  c  Correct the ray for bad antenna behavior and compute angles
 *  c  relative to the track
 *  
 *  Call Correct_Ray (Vel, Ref, Ihead, Tilt, Rot, CompAz, Azm_RC)
 *  
 *  c  Azimuth roll corrected
 *  
 *  Hed(6) = Azm_RC
 *  
 *  c  Rotation angle from zenith
 *  
 *  Hed(8) = Rot
 *  
 *  c  Track relative tilt angle (fore/aft direction)
 *  
 *  Hed(9) = Tilt
 *  
 *  c  Compass azimuth from North
 *  
 *  Hed(1) = CompAz
 *  
 *  Return
 *  
 *  999 Write (60,'("Error:",i5)') Ierr
 *  Return
 *  
 *  1000 Write (60,'(/" End of file on input disc file",i5)') Ierr
 *  Return
 *  
 *  End
 *  
 *  --
 *  
 *  David P. Jorgensen
 *
 *  Research Meteorologist and Chief, NOAA/National Severe Storms Lab/Warning R&D Division
 *  e-mail: David.P.Jorgensen@noaa.gov  http://www.nssl.noaa.gov
 *  Phone: (405) 325-6270; fax: (405) 325-6780
 *
 *  Mailing Address: 
 *  NOAA/NSSL/WRDD
 *  The National Weather Center
 *  120 David L. Boren Blvd.
 *  Norman, OK 73072-7323
 *  
 ********************************************************************/
