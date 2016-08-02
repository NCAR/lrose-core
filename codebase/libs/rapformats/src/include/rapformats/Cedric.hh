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
/*********************************************************************
 * Cedric.hh
 *
 * Reading / writing CEDRIC files
 *
 * NOTE ON UNITS:
 *   In this API, all distance units are in km,
 *   and angle/lat/lon units are in deg.
 *   The set/get methods handle the conversion to/from the units
 *   used internally by the cedric file.
 * 
 *********************************************************************/

#ifndef CEDRIC_HH
#define CEDRIC_HH

#include <rapformats/cedric.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <vector>

class Cedric

{
  
public:

  // constructor
  
  Cedric();
  
  // destructor
  
  virtual ~Cedric();

  // set debugging on
  
  void setDebug(bool state) { _debug = state; }
 
  // clear all data
  
  void clear();
  
  // Read a cedric file, load up this object
  // Returns 0 on success, -1 on failure
  
  int readFromPath(const std::string &path);
  
  // Write file to specified dir
  // Returns 0 on success, -1 on failure
  
  int writeToDir(const std::string &dir,
                 const std::string &appName,
                 const std::string &volLabel = "vol");
  
  // Write file to specified path
  // Returns 0 on success, -1 on failure
  
  int writeToPath(const std::string &path,
                  const std::string &volLabel = "vol");
  
  // get the current path in use for read or write

  std::string getPathInUse() const { return _pathInUse; }
  
  // print
  
  void print(std::ostream &out, bool printData = false);
  void printNative(std::ostream &out, bool printData = false);
  void printMetaData(std::ostream &out);
  void printFieldData(int fieldNum, std::ostream &out) const;

  static void print(CED_file_head_t &hdr, std::ostream &out);
  static void print(CED_vol_head_t &hdr, std::ostream &out);
  static void print(CED_level_head_t &hdr, std::ostream &out);
  
  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  /// set methods
  
  void setId(const std::string &val) {
    _setString(val, _volHdr.id, 8);
  }
  void setProgram(const std::string &val) {
    _setString(val, _volHdr.program, 6);
  }
  void setProject(const std::string &val) {
    _setString(val, _volHdr.project, 4);
  }
  void setScientist(const std::string &val) {
    _setString(val, _volHdr.scientist, 6);
  }
  void setRadar(const std::string &val) {
    _setString(val, _volHdr.radar, 6);
  }
  void setCoordType(const std::string &val);
  void setTape(const std::string &val) {
    _setString(val, _volHdr.tape, 6);
  }

  void setTapeStartTime(time_t utime) {
    _setTime(utime,
             _volHdr.tape_start_year,
             _volHdr.tape_start_month,
             _volHdr.tape_start_day,
             _volHdr.tape_start_hour,
             _volHdr.tape_start_min,
             _volHdr.tape_start_sec);
  }
  
  void setTapeEndTime(time_t utime) {
    _setTime(utime,
             _volHdr.tape_end_year,
             _volHdr.tape_end_month,
             _volHdr.tape_end_day,
             _volHdr.tape_end_hour,
             _volHdr.tape_end_min,
             _volHdr.tape_end_sec);
  }

  void setLatitudeDeg(double latDeg) {
    _setDegMinSec(latDeg, _volHdr.lat_deg, _volHdr.lat_min, _volHdr.lat_sec);
  }
  void setLongitudeDeg(double lonDeg) {
    _setDegMinSec(lonDeg, _volHdr.lon_deg, _volHdr.lon_min, _volHdr.lon_sec);
  }
  void setOriginHtKm(double htKm) { _setKmDouble(htKm, _volHdr.origin_height); }
  
  void setXaxisAngleFromNDeg(double val) {
    _setAngle(val, _volHdr.angle1);
  }

  void setOriginX(double val) { _setDouble(val, _volHdr.origin_x); }
  void setOriginY(double val) { _setDouble(val, _volHdr.origin_y); }
  
  void setTimeZone(const std::string &val) {
    _setString(val, _volHdr.time_zone, 4);
  }
  void setSequence(const std::string &val) {
    _setString(val, _volHdr.sequence, 6);
  }
  void setSubmitter(const std::string &val) {
    _setString(val, _volHdr.submitter, 6);
  }
  void setDateTimeRun(time_t runTime);
  void setDateRun(const std::string &val) {
    _setString(val, _volHdr.date_run, 8);
  }
  void setTimeRun(const std::string &val) {
    _setString(val, _volHdr.time_run, 8);
  }

  void setTapeEdNumber(int val) { _volHdr.tape_ed_number = val; }
  void setHeaderRecordLength(int val) {
    _volHdr.header_record_length = val;
  }
  
  void setComputer(const std::string &val) {
    _setString(val, _volHdr.computer, 2);
  }

  void setBitsDatum(int val) { _volHdr.bits_datum = val; }
  void setBlockingMode(int val) { _volHdr.blocking_mode = val; }
  void setBlockSize(int val) { _volHdr.block_size = val; }
  void setMissingDataVal(int val) { _volHdr.missing_val = val; }
  
  void setScaleFactor(int val) { _volHdr.scale_factor = val; }
  void setAngleFactor(int val) { _volHdr.angle_factor = val; }
  
  void setSource(const std::string &val) {
    _setString(val, _volHdr.source, 8);
  }

  void setTapeLabel2(const std::string &val) {
    _setString(val, _volHdr.tape_label2, 8);
  }
  void setTapeLabel3(const std::string &val) {
    _setString(val, _volHdr.tape_label3, 8);
  }
  void setTapeLabel4(const std::string &val) {
    _setString(val, _volHdr.tape_label4, 8);
  }
  void setTapeLabel5(const std::string &val) {
    _setString(val, _volHdr.tape_label5, 8);
  }
  void setTapeLabel6(const std::string &val) {
    _setString(val, _volHdr.tape_label6, 8);
  }

  void setRecordsPlane(int val) { _volHdr.records_plane = val; }
  void setRecordsField(int val) { _volHdr.records_field = val; }
  void setRecordsVolume(int val) { _volHdr.records_volume = val; }
  void setTotalRecords(int val) { _volHdr.total_records = val; }
  void setTotRecords(int val) { _volHdr.tot_records = val; }

  void setVolName(const std::string &val) {
    _setString(val, _volHdr.vol_name, 8); 
  }

  void setNumPlanes(int val) { _volHdr.num_planes = val; }

  void setCubicKm(int val) { _volHdr.cubic_km = val / _volHdr.scale_factor; }

  void setTotalPoints(int val) {
    _volHdr.total_points = val / _volHdr.scale_factor;
  }

  void setSamplingDensity(double val) {
    _setDouble(val, _volHdr.sampling_density);
  }

  void setNumPulses(int val) { _volHdr.num_pulses = val; }
  void setVolumeNumber(int val) { _volHdr.volume_number = val; }
  
  void setVolStartTime(time_t utime) {
    _setTime(utime,
             _volHdr.vol_start_year,
             _volHdr.vol_start_month,
             _volHdr.vol_start_day,
             _volHdr.vol_start_hour,
             _volHdr.vol_start_min,
             _volHdr.vol_start_second);
  }
  
  void setVolEndTime(time_t utime) {
    _setTime(utime,
             _volHdr.vol_end_year,
             _volHdr.vol_end_month,
             _volHdr.vol_end_day,
             _volHdr.vol_end_hour,
             _volHdr.vol_end_min,
             _volHdr.vol_end_second);
  }
  
  void setVolumeTimeSec(int val) { _volHdr.volume_time = val; }
  void setIndexNumberTime(int val) { _volHdr.index_number_time = val; }

  void setMinRangeKm(double val) { _setDouble(val, _volHdr.min_range); }
  void setMaxRangeKm(double val) { _setDouble(val, _volHdr.max_range); }

  void setNumGatesBeam(int val) { _volHdr.num_gates_beam = val; }
  void setGateSpacingKm(double val) { _setKmDouble(val, _volHdr.gate_spacing); }
  void setMinGates(int val) { _volHdr.min_gates = val; }
  void setMaxGates(int val) { _volHdr.max_gates = val; }
  void setIndexNumberRange(int val) { _volHdr.index_number_range = val; }
  void setMinAzimuthDeg(double val) { _setAngle(val, _volHdr.min_azimuth); }
  void setMaxAzimuthDeg(double val) { _setAngle(val, _volHdr.max_azimuth); }
  void setNumBeamsPlane(int val) { _volHdr.num_beams_plane = val; }
  void setAveAngleDeg(double val) { _setAngle(val, _volHdr.ave_angle); }
  void setMinBeamsPlane(int val) { _volHdr.min_beams_plane = val; }
  void setMaxBeamsPlane(int val) { _volHdr.max_beams_plane = val; }
  void setNumStepsBeam(int val) { _volHdr.num_steps_beam = val; }
  void setIndexNumberAzimuth(int val) { _volHdr.index_number_azimuth = val; }

  void setPlaneType(const std::string &val) {
    _setString(val, _volHdr.plane_type, 2);
  }

  void setMinElevDeg(double val) { _setAngle(val, _volHdr.min_elev); }
  void setMaxElevDeg(double val) { _setAngle(val, _volHdr.max_elev); }
  void setNumElevs(int val) { _volHdr.num_elevs = val; }
  void setAveDeltaElevDeg(double val) {
    _setAngle(val, _volHdr.ave_delta_elev);
  }
  void setAveElevDeg(double val) { _setAngle(val, _volHdr.ave_elev); }
  void setDirection(int val) { _volHdr.direction = val; }
  void setBaselineAngleDeg(double val) {
    _setAngle(val, _volHdr.baseline_angle);
  }
  void setIndexNumberCoplane(int val) { _volHdr.index_number_coplane = val; }
  
  void setMinX(double val) { _setDouble(val, _volHdr.min_x); }
  void setMaxX(double val) { _setDouble(val, _volHdr.max_x); }
  void setNx(int val) { _volHdr.nx = val; }
  void setDx(double val) { _volHdr.dx = (int) floor(val * 1000.0 + 0.5); }
  void setFastAxis(int val) { _volHdr.fast_axis = val; }
  
  void setMinY(double val) { _setDouble(val, _volHdr.min_y); }
  void setMaxY(double val) { _setDouble(val, _volHdr.max_y); }
  void setNy(int val) { _volHdr.ny = val; }
  void setDy(double val) { _volHdr.dy = (int) floor(val * 1000.0 + 0.5); }
  void setMidAxis(int val) { _volHdr.mid_axis = val; }
  
  void setMinZ(double val) { _setZDouble(val, _volHdr.min_z); }
  void setMaxZ(double val) { _setZDouble(val, _volHdr.max_z); }
  void setNz(int val) { _volHdr.nz = val; }
  void setDz(double val) { _setDzDouble(val, _volHdr.dz); }
  void setSlowAxis(int val) { _volHdr.slow_axis = val; }

  void setPointsPlane(int val) { _volHdr.points_plane = val; }
  
  void setNumLandmarks(int val) { _volHdr.num_landmarks = val; }
  void setNumRadars(int val) { _volHdr.num_radars = val; }
  
  void setNyquistVel(double val) { _setDouble(val, _volHdr.nyquist_vel); }
  void setRadarConst(double val) { _setDouble(val, _volHdr.radar_const); }
  
  // add a vertical level

  void addVlevel(int index, double zval,
                 int nfields,
                 int nx, int ny,
                 double nyquist);

  // add a field given floats
  // returns 0 on success, -1 on failure

  int addField(const std::string &name, const fl32 *data, fl32 missingVal);

  // add a landmark
  // returns 0 on success, -1 on failure

  int addLandmark(const std::string &name,
                  double xposKm, double yposKm, double zposKm);

  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  /// get methods


  int getVolumeNum() const { return _fileHdr.vol_index[0]; }
  std::string getVolumeLabel() const {
    return _getString(_fileHdr.vol_label[0], 56);
  }

  std::string getId() const { return _getString(_volHdr.id, 8); }
  std::string getProgram() const { return _getString(_volHdr.program, 6); }
  std::string getProject() const { return _getString(_volHdr.project, 4); }
  std::string getScientist() const { return _getString(_volHdr.scientist, 6); }
  std::string getRadar() const { return _getString(_volHdr.radar, 6); }
  std::string getCoordType() const { return _getString(_volHdr.scan_mode, 4); }
  std::string getTape() const { return _getString(_volHdr.tape, 6); }

  time_t getTapeStartTime() const {
    return _getTime(_volHdr.tape_start_year,
                    _volHdr.tape_start_month,
                    _volHdr.tape_start_day,
                    _volHdr.tape_start_hour,
                    _volHdr.tape_start_min,
                    _volHdr.tape_start_sec);
  }
  
  time_t getTapeEndTime() const {
    return _getTime(_volHdr.tape_end_year,
                    _volHdr.tape_end_month,
                    _volHdr.tape_end_day,
                    _volHdr.tape_end_hour,
                    _volHdr.tape_end_min,
                    _volHdr.tape_end_sec);
  }

  double getLatitudeDeg() const {
    return _getDecDegrees(_volHdr.lat_deg, _volHdr.lat_min, _volHdr.lat_sec);
  }
  double getLongitudeDeg() const {
    return _getDecDegrees(_volHdr.lon_deg, _volHdr.lon_min, _volHdr.lon_sec);
  }
  double getOriginHtKm() const { return _getKmDouble(_volHdr.origin_height); }
  double getOriginHtM() const { return _volHdr.origin_height; }
  
  double getXaxisAngleFromNDeg() const {
    return _getAngle(_volHdr.angle1);
  }

  double getOriginX() const { return _getDouble(_volHdr.origin_x); }
  double getOriginY() const { return _getDouble(_volHdr.origin_y); }
  
  std::string getTimeZone() const { return _getString(_volHdr.time_zone, 4); }
  std::string getSequence() const { return _getString(_volHdr.sequence, 6); }
  std::string getSubmitter() const { return _getString(_volHdr.submitter, 6); }
  std::string getDateRun() const { return _getString(_volHdr.date_run, 8); }
  std::string getTimeRun() const { return _getString(_volHdr.time_run, 8); }

  int getTapeEdNumber() const { return _volHdr.tape_ed_number; }
  int getHeaderRecordLength() const { return _volHdr.header_record_length; }
  
  std::string getComputer() const { return _getString(_volHdr.computer, 2); }

  int getBitsDatum() const { return _volHdr.bits_datum; }
  int getBlockingMode() const { return _volHdr.blocking_mode; }
  int getBlockSize() const { return _volHdr.block_size; }
  int getMissingDataVal() const { return _volHdr.missing_val; }
  
  double getScaleFactor() const { return (double) _volHdr.scale_factor; }
  double getAngleFactor() const { return (double) _volHdr.angle_factor; }
  
  std::string getSource() const { return _getString(_volHdr.source, 8); }

  std::string getTapeLabel2() const {
    return _getString(_volHdr.tape_label2, 8);
  }
  std::string getTapeLabel3() const {
    return _getString(_volHdr.tape_label3, 8);
  }
  std::string getTapeLabel4() const {
    return _getString(_volHdr.tape_label4, 8);
  }
  std::string getTapeLabel5() const {
    return _getString(_volHdr.tape_label5, 8);
  }
  std::string getTapeLabel6() const {
    return _getString(_volHdr.tape_label6, 8);
  }

  int getRecordsPlane() const { return _volHdr.records_plane; }
  int getRecordsField() const { return _volHdr.records_field; }
  int getRecordsVolume() const { return _volHdr.records_volume; }
  int getTotalRecords() const { return _volHdr.total_records; }
  int getTotRecords() const { return _volHdr.tot_records; }

  std::string getVolName() const { return _getString(_volHdr.vol_name, 8); }

  int getNumPlanes() const { return _volHdr.num_planes; }

  double getCubicKm() const { return _getDouble(_volHdr.cubic_km); }
  int getTotalPoints() const {
    return _volHdr.total_points * _volHdr.scale_factor;
  }
  double getSamplingDensity() const {
    return _getDouble(_volHdr.sampling_density);
  }

  int getNumPulses() const { return _volHdr.num_pulses; }
  int getVolumeNumber() const { return _volHdr.volume_number; }
  
  time_t getVolStartTime() const {
    return _getTime(_volHdr.vol_start_year,
                    _volHdr.vol_start_month,
                    _volHdr.vol_start_day,
                    _volHdr.vol_start_hour,
                    _volHdr.vol_start_min,
                    _volHdr.vol_start_second);
  }
  
  time_t getVolEndTime() const {
    return _getTime(_volHdr.vol_end_year,
                    _volHdr.vol_end_month,
                    _volHdr.vol_end_day,
                    _volHdr.vol_end_hour,
                    _volHdr.vol_end_min,
                    _volHdr.vol_end_second);
  }
  
  int getVolumeTimeSec() const { return _volHdr.volume_time; }
  int getIndexNumberTime() const { return _volHdr.index_number_time; }

  double getMinRangeKm() const { return _getDouble(_volHdr.min_range); }
  double getMaxRangeKm() const { return _getDouble(_volHdr.max_range); }

  int getNumGatesBeam() const { return _volHdr.num_gates_beam; }
  double getGateSpacingKm() const { return _getKmDouble(_volHdr.gate_spacing); }
  int getGateSpacingM() const { return _volHdr.gate_spacing; }
  int getMinGates() const { return _volHdr.min_gates; }
  int getMaxGates() const { return _volHdr.max_gates; }
  int getIndexNumberRange() const { return _volHdr.index_number_range; }
  double getMinAzimuthDeg() const { return _getAngle(_volHdr.min_azimuth); }
  double getMaxAzimuthDeg() const { return _getAngle(_volHdr.max_azimuth); }
  int getNumBeamsPlane() const { return _volHdr.num_beams_plane; }
  double getAveAngleDeg() const { return _getAngle(_volHdr.ave_angle); }
  int getMinBeamsPlane() const { return _volHdr.min_beams_plane; }
  int getMaxBeamsPlane() const { return _volHdr.max_beams_plane; }
  int getNumStepsBeam() const { return _volHdr.num_steps_beam; }
  int getIndexNumberAzimuth() const { return _volHdr.index_number_azimuth; }

  std::string getPlaneType() const { 
    return _getString(_volHdr.plane_type, 2);
  }

  double getMinElevDeg() const { return _getAngle(_volHdr.min_elev); }
  double getMaxElevDeg() const { return _getAngle(_volHdr.max_elev); }
  int getNumElevs() const { return _volHdr.num_elevs; }
  double getAveDeltaElevDeg() const {
    return _getAngle(_volHdr.ave_delta_elev);
  }
  double getAveElevDeg() const { return _getAngle(_volHdr.ave_elev); }
  int getDirection() const { return _volHdr.direction; }
  double getBaselineAngleDeg() const {
    return _getAngle(_volHdr.baseline_angle);
  }
  int getIndexNumberCoplane() const { return _volHdr.index_number_coplane; }
  
  double getMinX() const { return _getDouble(_volHdr.min_x); }
  double getMaxX() const { return _getDouble(_volHdr.max_x); }
  int getNx() const { return _volHdr.nx; }
  double getDx() const { return _volHdr.dx / 1000.0; }
  int getFastAxis() const { return _volHdr.fast_axis; }
  
  double getMinY() const { return _getDouble(_volHdr.min_y); }
  double getMaxY() const { return _getDouble(_volHdr.max_y); }
  int getNy() const { return _volHdr.ny; }
  double getDy() const { return _volHdr.dy / 1000.0; }
  int getMidAxis() const { return _volHdr.mid_axis; }
  
  double getMinZ() const { return _getZDouble(_volHdr.min_z); }
  double getMaxZ() const { return _getZDouble(_volHdr.max_z); }
  int getNz() const { return _volHdr.nz; }
  double getDz() const { return _getDzDouble(_volHdr.dz); }
  double getZ(size_t iz) const {
    if (iz < _zArray.size()) return _zArray[iz];
    return 0.0;
  }
  int getSlowAxis() const { return _volHdr.slow_axis; }
  
  int getNumFields() const { return _volHdr.num_fields; }
  std::string getFieldName(int fieldNum) const;

  // look up field number from the name
  // return -1 if not found

  int getFieldNum(const std::string &name) const;
  
  double getFieldScaleFactor(int fieldNum) const;

  int getPointsPlane() const { return _volHdr.points_plane; }
  int getNumLandmarks() const { return _volHdr.num_landmarks; }
  int getNumRadars() const { return _volHdr.num_radars; }

  double getNyquistVel() const { return _getDouble(_volHdr.nyquist_vel); }
  double getRadarConst() const { return _getDouble(_volHdr.radar_const); }
  
  std::string getLandmarkName(int landmarkNum) const;
  double getLandmarkXposKm(int fieldNum) const;
  double getLandmarkYposKm(int fieldNum) const;
  double getLandmarkZposKm(int fieldNum) const;

  // get field data in place, as si16
  // memory remains owned by this object

  const si16 *getFieldData(int fieldNum) const;

  // get field data as fl32 
  // memory is allocated
  // caller must delete[] the returned array
  
  fl32 *getFieldData(int fieldNum, fl32 missingVal) const;

private:

  // private data

  bool _debug;
  std::string _pathInUse;

  CED_file_head_t _fileHdr;
  CED_vol_head_t _volHdr;
  std::vector<CED_level_head_t> _levelHdrs;
  std::vector<double> _zArray;
  double _zScale;
  double _dzScale;

  bool _needsSwapOnRead;
  
  si16 *_fieldData[CED_MAX_FIELDS];

  // private methods

  void _setZScale();

  int _readLevel(FILE *in, int levelNum);

  inline double _getDouble(int ival) const {
    return (double) ival / (double) _volHdr.scale_factor;
  }
  
  inline void _setDouble(double dval, si16 &ival) {
    ival = (si16) floor(dval * (double) _volHdr.scale_factor + 0.5);
  }

  inline double _getZDouble(int ival) const {
    return (double) ival / _zScale;
  }
  
  inline void _setZDouble(double dval, si16 &ival) {
    ival = (si16) floor(dval * _zScale + 0.5);
  }

  inline void _setDzDouble(double dval, si16 &ival) {
    ival = (si16) floor(dval * _dzScale + 0.5);
  }

  inline double _getDzDouble(int ival) const {
    return (double) ival / _dzScale;
  }
  
  inline void _setKmDouble(double dval, si16 &ival) {
    ival = (si16) floor(dval * 1000.0 + 0.5);
  }

  inline double _getKmDouble(int ival) const {
    return (double) ival / 1000.0;
  }
  
  inline double _getAngle(int ival) const {
    return (double) ival / (double) _volHdr.angle_factor;
  }

  inline void _setAngle(double dval, si16 &ival) {
    ival = (si16) floor(dval * (double) _volHdr.angle_factor + 0.5);
  }

  static std::string _getString(const char *text, int maxLen);
  static void _setString(const std::string &str, char *text, size_t maxLen);

  time_t _getTime(si16 year, si16 month, si16 day,
                  si16 hour, si16 min, si16 sec) const;
  
  void _setTime(time_t dtime,
                si16 &year, si16 &month, si16 &day,
                si16 &hour, si16 &min, si16 &sec);

  double _getDecDegrees(si16 ideg, si16 imin, si16 isec) const;

  void _setDegMinSec(double degrees, si16 &ideg, si16 &imin, si16 &isec);
  
  void _setFieldName(int fieldNum, const std::string &name);
  void _setFieldData(int fieldNum, const si16 *data, int scale_factor);
  void _setFieldData(int fieldNum, const fl32 *data, fl32 missingVal);
  
  void _setLandmarkName(int landmarkNum, const std::string &name);
  void _setLandmarkXpos(int fieldNum, double val);
  void _setLandmarkYpos(int fieldNum, double val);
  void _setLandmarkZpos(int fieldNum, double val);

  static void _printPacked(std::ostream &out, int count, fl32 val, fl32 missingFl32);

  void _swapWords(void *array, int nBytes);
  void _swap(CED_file_head_t &hdr);
  void _swap(CED_vol_head_t &hdr);
  void _swap(CED_level_head_t &hdr);

};

#endif

