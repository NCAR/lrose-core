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
// RadxRay.hh
//
// Ray object for Radx data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#ifndef RadxRay_HH
#define RadxRay_HH

#include <string>
#include <vector>
#include <map>
#include <pthread.h>
#include <Radx/Radx.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxMsg.hh>
class RadxSweep;
class RadxGeoref;
class RadxCfactors;
using namespace std;

///////////////////////////////////////////////////////////////
/// RAY CLASS
///
/// This class holds the data and meta-data for a single ray,
/// or beam.
///
/// The field data is held by a vector of RadxField objects
/// on this class.
///
/// This class is used in 2 main ways: (a) to store ray data as
/// it is read by an ingest-type application. The ray is then added
/// to a RadxVol object; (b) to represent the ray data in a RadxVol
/// object, when data is read in for a complete volume at a time.
///
/// In (a) above, the field data is managed on a ray-by-ray basis by
/// the RadxField objects in a ray. See RadxField for more details on
/// this memory management. Once all rays have been added to a
/// RadxVol, the method RadxVol::loadFieldsFromRays() may be called
/// to create contiguous fields for the volume, and the ray field
/// objects are modified so that their data member points into the
/// contiguous field on the volume.  Memory management at that point
/// transfers from the ray fields to the volume fields.

class RadxRay : public RadxRangeGeom {

public:

  /// Constructor.
  
  RadxRay();

  /// Copy constructor.
  
  RadxRay(const RadxRay &rhs);

  /// Destructor.
  
  virtual ~RadxRay();
    
  /// Assignment.
  
  RadxRay& operator=(const RadxRay &rhs);
  
  /// Copy the meta data, but not the fields

  void copyMetaData(const RadxRay &rhs);

  /// copy meta data from sweep
  
  void setMetadataFromSweep(const RadxSweep &sweep);

  /// Delete the field objects, as well as the field vector itself.
  
  void clearFields();

  /// Clear the georeference info by deleting the RadxGeoref object.
  
  void clearGeoref();
  
  /// Clear the correction factors by deleting the cfactors object.
  
  void clearCfactors();
  
  /////////////////////////////////////////////////////////////
  /// \name Set methods:
  //@{

  /// Set the ray number in the volume

  inline void setRayNumber(int val) { _rayNum = val; }

  /// Set the volume number.

  inline void setVolumeNumber(int val) { _volNum = val; }

  /// Set the sweep number.

  inline void setSweepNumber(int val) { _sweepNum = val; }

  /// Set the calibration index.  If this index is 0 or positive, it
  /// references the object which holds the calibration data relevant
  /// to this ray. The calibration object for radar data is an
  /// instance of RadxRcalib. The calibration objects are owned by a
  /// RadxVol object.
  ///
  /// If no calibration is applicable to the ray, this is set to -1.

  inline void setCalibIndex(int val) { _calibIndex = val; }

  /// Set the name of the current scan

  inline void setScanName(const string &val) {
    _scanName = val;
  }

  /// Set the sweep mode which was active when this ray was gathered.

  inline void setSweepMode(Radx::SweepMode_t val) {
    _sweepMode = val;
  }

  /// Set the polarization mode which was active when this ray was
  /// gathered.

  inline void setPolarizationMode(Radx::PolarizationMode_t val) {
    _polarizationMode = val;
  }

  /// Set the PRT mode which was active when this ray was
  /// gathered.

  inline void setPrtMode(Radx::PrtMode_t val) {
    _prtMode = val;
  }

  /// Set the follow mode which was active when this ray was
  /// gathered.

  inline void setFollowMode(Radx::FollowMode_t val) {
    _followMode = val;
  }

  /// Set the time for this ray, in seconds and nanoseconds.

  inline void setTime(time_t secs, double nanoSecs) {
    _timeSecs = secs;
    _nanoSecs = nanoSecs;
  }
  
  /// Set the time for this ray, given time as a double

  inline void setTime(double secs) {
    _timeSecs = (time_t ) secs;
    _nanoSecs = (secs - _timeSecs) * 1.0e9;
  }

  /// Set the time for this ray, given a RadxTime

  inline void setTime(const RadxTime &rayTime) {
    _timeSecs = rayTime.utime();
    _nanoSecs = rayTime.getSubSec() * 1.0e9;
  }
  
  /// Set the azimuth angle for this ray, in degrees.

  inline void setAzimuthDeg(double val) { _az = val; }

  /// Set the elevation angle for this ray, in degrees.

  inline void setElevationDeg(double val) { _elev = val; }

  /// Set the sweep fixed angle for this ray, in degrees. The fixed
  /// angle is the target angle for the sweep that contains this
  /// ray. For RHI scans the fixed angle will be an azimuth angle. For
  /// all other scans this is an elevation angle.

  inline void setFixedAngleDeg(double val) { _fixedAngle = val; }

  /// Set the scan rate, in degrees per second, for this ray, if it is
  /// known.

  inline void setTrueScanRateDegPerSec(double val) { _trueScanRate = val; }

  /// Set the scan rate, in degrees per second, for this ray, if it is
  /// known.

  inline void setTargetScanRateDegPerSec(double val) { _targetScanRate = val; }

  /// Set the flag to indicate that this ray is 'indexed'. Indexed
  /// rays have an angle (elevation for RHIs, azimuth for other scans)
  /// which is regularly-spaced with respect to other rays. If
  /// indexing is off, the rays are not regularly spaced.
  
  inline void setIsIndexed(bool val) { _isIndexed = val; }
  
  /// Set the angular resolution for indexed rays, in degrees. See
  /// setIsIndexed().

  inline void setAngleResDeg(double val) { _angleRes = val; }

  /// Set the flag to indicate that the antenna was in transition when
  /// this ray was gathered. 'In transition' means that the antenna
  /// was moving between sweeps. Some users and applications need to
  /// discard transition rays.

  inline void setAntennaTransition(bool val) { _antennaTransition = val; }

  /// Set the number of samples (or hits, or pulses) used for this ray.

  inline void setNSamples(int val) { _nSamples = val; }

  /// Set the pulse width, in microseconds, in use for this ray.

  inline void setPulseWidthUsec(double val) { _pulseWidthUsec = val; }

  /// Set the primary PRT - pulse repetition time - for this ray, in
  /// seconds.

  inline void setPrtSec(double val) { _prtSec = val; }

  /// Set the PRT ratio - pulse repetition time ratio - for this ray.
  /// This allows the computation of the secondary PRT.
  /// Secondary PRT = primary PRT / ratio
  ///
  /// The secondary PRT applies to 2 pulsing modes: (a) staggered PRT
  /// in which the PRT toggles between two values on a pulse-by-pulse
  /// basis, and (b) dual PRT in which the PRT toggles between two
  /// values on a ray-by-ray basis.
  ///
  /// The PRT ratio is always < 1.
  /// Typical values are 2/3, 3/4 and 4/5.

  inline void setPrtRatio(double val) { _prtRatio = val; }

  /// Set the nyquist velocity, in m/s, for this ray, if known.

  inline void setNyquistMps(double val) { _nyquistMps = val; }

  /// Set the maximum unambiguous range, in km, for this ray, if known.

  inline void setUnambigRangeKm(double val) { _unambigRangeKm = val; }

  /// Set the measured transmit power for the H (horizontal)
  /// waveguide, in dBm, if known.
  ///
  /// For single polarization operations, use this value.
  
  inline void setMeasXmitPowerDbmH(double val) { _measXmitPowerDbmH = val; }

  /// Set the measured transmit power for the V (vertical) waveguide,
  /// in dBm, if known.
  
  inline void setMeasXmitPowerDbmV(double val) { _measXmitPowerDbmV = val; }

  /// set the estimated noise value for the H copolar channel, in dBm

  void setEstimatedNoiseDbmHc(double val) { _estimatedNoiseDbmHc = val; }

  /// set the estimated noise value for the V copolar channel, in dBm

  void setEstimatedNoiseDbmVc(double val) { _estimatedNoiseDbmVc = val; }

  /// set the estimated noise value for the H cross-polar channel, in dBm

  void setEstimatedNoiseDbmHx(double val) { _estimatedNoiseDbmHx = val; }

  /// set the estimated noise value for the V cross-polar channel, in dBm

  void setEstimatedNoiseDbmVx(double val) { _estimatedNoiseDbmVx = val; }

  // set sweep and volume flags

  void clearEventFlags();
  void setStartOfSweepFlag(bool state);
  void setEndOfSweepFlag(bool state);
  void setStartOfVolumeFlag(bool state);
  void setEndOfVolumeFlag(bool state);

  // set utility flag

  void setUtilityFlag(bool state);

  // is this a long-range ray?
  
  void setIsLongRange(bool val) { _isLongRange = val; }

  /// if not already set, set unambiguous range computed from the
  /// wavelength, PRT and PRT mode.  Before calling this, make sure
  /// the prt mode is set, along with the relevant prt values.

  void setUnambigRange();

  /// Set georeference info for this ray.
  /// 
  /// This applies to moving platforms only.
  
  void setGeoref(const RadxGeoref &ref);

  /// Set correction factors info for this ray.
  
  void setCfactors(const RadxCfactors &cfac);

  /// compute elevation and azimuth from the georeference
  /// data and correction factors.
  ///
  /// If the georeference is null, no action is taken.
  /// If the correction factors is null, no corrections are
  /// applied, and setGeorefApplied() is called.
  ///
  /// If force is true, the georefs are always applied.
  /// If force is false, the georefs are applied only if
  /// they have not been applied previously,
  /// i.e. if _georefApplied = false.
  
  void applyGeoref(Radx::PrimaryAxis_t axis, bool force = true);

  /// set the flag to indicate that georef has been applied

  void setGeorefApplied(bool state);
  
  /// count the number of rays in which each georef element
  /// is not missing
  
  void incrementGeorefNotMissingCount(RadxGeoref &count);

  //@}

  /////////////////////////////////////////////////////////////
  /// \name Add data fields, by type:
  //@{

  /// Create a 64-bit float RadxField object and add it to the ray.
  ///
  /// The missing data value is specified.
  ///
  /// If isLocal is true, the RadxField object will make a copy
  /// of the data and manage it locally. If it is false, the RadxField
  /// object will store the 'data' pointer as it is passed in, and
  /// memory management for the data will be handled by the object
  /// which allocated the data array.
  ///
  /// nGates must match any existing fields.
  ///
  /// Returns a pointer to the RadxField object created.
  
  RadxField *addField(const string &name,
                      const string &units,
                      size_t nGates,
                      Radx::fl64 missingValue,
                      const Radx::fl64 *data,
                      bool isLocal);
  
  /// Create a 32-bit float RadxField object and add it to the ray.
  ///
  /// The missing data value is specified.
  ///
  /// If isLocal is true, the RadxField object will make a copy
  /// of the data and manage it locally. If it is false, the RadxField
  /// object will store the 'data' pointer as it is passed in, and
  /// memory management for the data will be handled by the object
  /// which allocated the data array.
  ///
  /// nGates must match any existing fields.
  ///
  /// Returns a pointer to the RadxField object created.
  
  RadxField *addField(const string &name,
                      const string &units,
                      size_t nGates,
                      Radx::fl32 missingValue,
                      const Radx::fl32 *data,
                      bool isLocal);
  
  /// Create a 32-bit scaled integer RadxField object and add it to
  /// the ray.
  ///
  /// The scale and offset for the integer data is specified, as is
  /// the missing data value.
  ///
  /// If isLocal is true, the RadxField object will make a copy
  /// of the data and manage it locally. If it is false, the RadxField
  /// object will store the 'data' pointer as it is passed in, and
  /// memory management for the data will be handled by the object
  /// which allocated the data array.
  ///
  /// nGates must match any existing fields.
  ///
  /// Returns a pointer to the RadxField object created.
  
  RadxField *addField(const string &name,
                      const string &units,
                      size_t nGates,
                      Radx::si32 missingValue,
                      const Radx::si32 *data,
                      double scale,
                      double offset,
                      bool isLocal);
  
  /// Create a 16-bit scaled integer RadxField object and add it to
  /// the ray.
  ///
  /// The scale and offset for the integer data is specified, as is
  /// the missing data value.
  ///
  /// If isLocal is true, the RadxField object will make a copy
  /// of the data and manage it locally. If it is false, the RadxField
  /// object will store the 'data' pointer as it is passed in, and
  /// memory management for the data will be handled by the object
  /// which allocated the data array.
  ///
  /// nGates must match any existing fields.
  ///
  /// Returns a pointer to the RadxField object created.
  
  RadxField *addField(const string &name,
                      const string &units,
                      size_t nGates,
                      Radx::si16 missingValue,
                      const Radx::si16 *data,
                      double scale,
                      double offset,
                      bool isLocal);
  
  /// Create an 8-bit scaled integer RadxField object and add it to the
  /// ray.
  ///
  /// The scale and offset for the integer data is specified, as is
  /// the missing data value.
  ///
  /// If isLocal is true, the RadxField object will make a copy
  /// of the data and manage it locally. If it is false, the RadxField
  /// object will store the 'data' pointer as it is passed in, and
  /// memory management for the data will be handled by the object
  /// which allocated the data array.
  ///
  /// nGates must match any existing fields.
  ///
  /// Returns a pointer to the RadxField object created.
  
  RadxField *addField(const string &name,
                      const string &units,
                      size_t nGates,
                      Radx::si08 missingValue,
                      const Radx::si08 *data,
                      double scale,
                      double offset,
                      bool isLocal);
  
  /// Add a previously-created field to the ray. The field must have
  /// been dynamically allocted using new(). Memory management for
  /// this field passes to the ray, which will free the field object
  /// using delete().
  ///
  /// If addToFront is true, the field is added at the beginning of the
  /// field list. Otherwise it is added at the end.
  
  void addField(RadxField *field,
                bool addToFront = false);

  //@}

  /////////////////////////////////////////////////////////////
  /// \name Data and range geometry changes
  //@{

  /// set the fields so that they manage their data locally
  /// rather than pointing to another object
  
  void setDataLocal();

  /// override set geometry for constant gate spacing
  
  virtual void setRangeGeom(double startRangeKm,
                            double gateSpacingKm);
  
  /// Remap data onto new gate geometry.
  /// If no mapping is possible at a gate, the value at
  /// the gate is set to missing.
  ///
  /// If interp is true, use interpolation if appropriate.
  /// Otherwise use nearest neighbor.
  
  virtual void remapRangeGeom(double newStartRangeKm,
                              double newGateSpacingKm,
                              bool interp = false);

  /// Remap data onto new gate geometry using remap object.
  /// If no mapping is possible at a gate, the value at
  /// the gate is set to missing.
  ///
  /// If interp is true, use interpolation if appropriate.
  /// Otherwise use nearest neighbor.
  
  virtual void remapRangeGeom(RadxRemap &remap,
                              bool interp = false);

  /// Remap field data onto new gate geometry using finest
  /// resolution in any of the fields.
  /// If all fields are the same, there is no change.
  ///
  /// If interp is true, use interpolation if appropriate.
  /// Otherwise use nearest neighbor.
  
  void remapRangeGeomToFinest(bool interp = false);

  /// Copy the range geom from the fields, if
  /// the fields are consistent in geometry
  
  void copyRangeGeomFromFields();

  /////////////////////////////////////////////////
  /// Copy the range geom to the fields
  
  void copyRangeGeomToFields();
  
  /// Set the number of gates.
  ///
  /// If more gates are needed, extend the field data out to a set number of
  /// gates. The data for extra gates are set to missing values.
  ///
  /// If fewer gates are needed, the data is truncated.
  
  void setNGates(size_t nGates);

  /////////////////////////////////////////////////////////////////////////
  /// Set the maximum range.
  /// Removes excess gates as needed.
  /// Does nothing if the current max range is less than that specified.
  
  void setMaxRangeKm(double maxRangeKm);

  /// Set to constant number of gates on all fields.
  /// 
  /// First we determine the max number of gates.
  /// If the number of gates does between fields,
  /// the shorter fields are padded out with missing data.
  
  void setNGatesConstant();

  /// Set all fields at a specified gate to missing
  
  void setGateToMissing(size_t gateNum);

  /// Set all fields at specified gates to missing
  
  void setGatesToMissing(size_t startGate, size_t endGate);

  /// Set all fields within specified range limits to missing
  
  void setRangeIntervalToMissing(double startRangeKm, double endRangeKm);

  //@}

  /////////////////////////////////////////////////////////////
  /// \name Convert data types on all fields:
  //@{

  ////////////////////////////////////////////
  // data type conversions

  /// Convert all fields to 64-bit floats.
  
  void convertToFl64();

  /// Convert all fields to 32-bit floats.
  
  void convertToFl32();

  /// Convert all fields to scaled 32-bit signed integers,
  /// using the specified scale and offset.
  /// See RadxField for correct use of the scale and offset.
  
  void convertToSi32(double scale, double offset);

  /// Convert all fields to scaled 32-bit signed integers,
  /// dynamically computing the scale and offset.
  
  void convertToSi32();

  /// Convert all fields to scaled 16-bit signed integers,
  /// using the specified scale and offset.
  /// See RadxField for correct use of the scale and offset.

  void convertToSi16(double scale, double offset);

  /// Convert all fields to scaled 16-bit signed integers,
  /// dynamically computing the scale and offset.
  
  void convertToSi16();

  /// Convert all fields to scaled 8-bit signed integers,
  /// using the specified scale and offset.
  /// See RadxField for correct use of the scale and offset.
  
  void convertToSi08(double scale, double offset);
  
  /// Convert all fields to scaled 8-bit signed integers,
  /// dynamically computing the scale and offset.
  
  void convertToSi08();

  /// convert to specified type
  ///
  /// If the data type is an integer type, dynamic scaling
  /// is used - i.e. the min and max value is computed and
  /// the scale and offset are set to values which maximize the
  /// dynamic range.
  ///
  /// If targetType is Radx::ASIS, no conversion is performed.
  
  void convertToType(Radx::DataType_t targetType);

  /// convert to specified type
  ///
  /// If the data type is an integer type, the specified
  /// scale and offset are used.
  ///
  /// If targetType is Radx::ASIS, no conversion is performed.
  
  void convertToType(Radx::DataType_t targetType,
                     double scale,
                     double offset);

  /// Apply a linear transformation to the data values in a field.
  /// Transforms x to y as follows:
  ///   y = x * scale + offset
  /// After operation, field type is unchanged.
  /// Nothing is done if field does not exist.
  
  void applyLinearTransform(const string &name,
                            double scale, double offset);

  /// Converts field type, and optionally changes the
  /// names.
  ///
  /// If the data type is an integer type, dynamic scaling
  /// is used - i.e. the min and max value is computed and
  /// the scale and offset are set to values which maximize the
  /// dynamic range.
  ///
  /// If targetType is Radx::ASIS, no conversion is performed.
  ///
  /// If a string is empty, the value on the field will
  /// be left unchanged.
  
  void convertField(const string &name,
                    Radx::DataType_t type,
                    const string &newName,
                    const string &units,
                    const string &standardName,
                    const string &longName);

  /// Converts field type, and optionally changes the
  /// names.
  ///
  /// If the data type is an integer type, the specified
  /// scale an offset are used.
  ///
  /// If targetType is Radx::ASIS, no conversion is performed.
  ///
  /// If a string is empty, the value on the field will
  /// be left unchanged.
  
  void convertField(const string &name,
                    Radx::DataType_t type,
                    double scale,
                    double offset,
                    const string &newName,
                    const string &units,
                    const string &standardName,
                    const string &longName);

  /// Load the field name map.
  /// Generally this will be done automatically by the ray object.
  /// However, if you rename fields which have previously been added,
  /// you will need to call this method to update the field name map.
  ///
  /// If more than one field shares a name, the extra fields will be
  /// renamed by adding _copy to the name.
  
  void loadFieldNameMap();

  //@}

  /////////////////////////////////////////////////////////////
  /// \name Remove or free data
  //@{

  /// Remove a field, given its name.
  /// Removed field is deleted in memory.
  /// Returns 0 on success, -1 on failure

  int removeField(const string &name);
  
  /// Replace a field, using its name to find the field
  /// to be replaced.
  /// Returns 0 on success, -1 on failure.
  /// On success, replaced field is deleted from memory.

  int replaceField(RadxField *field);
  
  /// Trim the fields to only include those specified in
  /// the wanted list
  
  void trimToWantedFields(const vector<string> &wantedNames);
  
  /// Reorder the fields, by name, removing any extra fields.

  void reorderFieldsByName(const vector<string> &names);
  
  /// Rename a field
  /// returns 0 on success, -1 if field does not exist

  int renameField(const string &oldName, const string &newName);
  
  /// Check that the fields match those in the template, both
  /// in name and order. Rearrange, delete and add fields as
  /// appropriate.
  
  void makeFieldsMatchTemplate(const vector<const RadxField *> &tplate);
  
  //@}

  /////////////////////////////////////////////////////////////
  /// \name Get methods:
  //@{
  
  /// Get the ray number in the volume for this ray.

  inline int getRayNumber() const { return _rayNum; }

  /// Get the volume number for this ray.

  inline int getVolumeNumber() const { return _volNum; }

  /// Get the sweep number for this ray.

  inline int getSweepNumber() const { return _sweepNum; }

  /// Get the calibration index for this ray. See setCalibIndex().

  inline int getCalibIndex() const { return _calibIndex; }
  
  /// Get the scan name

  inline const string &getScanName() const { return _scanName; }

  /// Get the sweep mode for this ray.

  inline Radx::SweepMode_t getSweepMode() const { return _sweepMode; }

  /// Get the polarization mode for this ray.

  inline Radx::PolarizationMode_t getPolarizationMode() const {
    return _polarizationMode;
  }

  /// Get the PRT mode for this ray.

  inline Radx::PrtMode_t getPrtMode() const { return _prtMode; }

  /// Get the follow mode for this ray.

  inline Radx::FollowMode_t getFollowMode() const { return _followMode; }

  /// Get the time for this ray, in seconds UTC since Jan 1 1970.
  ///
  /// Combine with getNanoSecs() for high-precision time.
  ///
  /// \code
  /// double time = (time in secs) + (nano seconds) / 1.0e9.
  /// \endcode
  
  inline time_t getTimeSecs() const { return _timeSecs; }

  /// Get the time for this ray in nanoseconds.
  ///
  /// Combine with getTimeSecs() for high-precision time.
  ///
  /// \code
  /// double time = (time in secs) + (nano seconds) / 1.0e9.
  /// \endcode

  inline double getNanoSecs() const { return _nanoSecs; }

  /// Get the time for this ray, as a double,
  /// in seconds UTC since Jan 1 1970.
  ///
  
  inline double getTimeDouble() const { 
    return ((double) _timeSecs + _nanoSecs / 1.0e9);
  }

  /// Get the time for this ray as RadxTime
  
  inline RadxTime getRadxTime() const {
    return RadxTime(_timeSecs, _nanoSecs / 1.0e9);
  }

  /// Get the azimuth angle for this ray, in degrees.

  inline double getAzimuthDeg() const { return _az; }

  /// Get the elevation angle for this ray, in degrees.

  inline double getElevationDeg() const { return _elev; }

  /// Get the fixed angle for this ray, in degrees.
  ///
  /// See setFixedAngleDeg().

  inline double getFixedAngleDeg() const { return _fixedAngle; }

  /// Get the true scan rate for this ray, in degrees per second.

  inline double getTrueScanRateDegPerSec() const { return _trueScanRate; }

  /// Get the target scan rate for this ray, in degrees per second.

  inline double getTargetScanRateDegPerSec() const { return _targetScanRate; }

  /// Get the flag which indicates whether this ray is indexed.
  ///
  /// Seet setIsIndexed().

  inline bool getIsIndexed() const { return _isIndexed; }

  /// Get the angular resolution for this ray, if indexed, in degrees.
  ///
  /// See getIsIndexed().

  inline double getAngleResDeg() const { return _angleRes; }

  /// Get the antenna transition flag for this ray.
  ///
  /// See setAntennaTransition().

  inline bool getAntennaTransition() const { return _antennaTransition; }

  /// Get the number of samples (or hits, or pulses) for this ray.

  inline int getNSamples() const { return _nSamples; }

  /// Get the pulse width for this ray, in microseconds.

  inline double getPulseWidthUsec() const { return _pulseWidthUsec; }

  /// Get the primary PRT for this ray, in seconds.
  ///
  /// See setPrtSec().

  inline double getPrtSec() const { return _prtSec; }

  /// Get the PRT ratio for this ray.
  ///
  /// See setPrtRatio().

  inline double getPrtRatio() const { return _prtRatio; }

  /// Get the nyquist velocity for this ray, in meters per second.

  inline double getNyquistMps() const { return _nyquistMps; }

  /// Get the unambiguous range for this ray, in km.

  inline double getUnambigRangeKm() const { return _unambigRangeKm; }
  
  /// Get the measured transmit power for the H (horizontal)
  /// waveguide, in dBm.
  ///
  /// For single polarization operations, use this value.
  
  inline double getMeasXmitPowerDbmH() const { return _measXmitPowerDbmH; }

  /// Get the measured transmit power for the V (vertical)
  /// waveguide, in dBm.

  inline double getMeasXmitPowerDbmV() const { return _measXmitPowerDbmV; }

  /// get the estimated noise value for the H copolar channel, in dBm

  double getEstimatedNoiseDbmHc() const { return _estimatedNoiseDbmHc; }

  /// set the estimated noise value for the V copolar channel, in dBm

  double getEstimatedNoiseDbmVc() const { return _estimatedNoiseDbmVc; }

  /// set the estimated noise value for the H cross-polar channel, in dBm

  double getEstimatedNoiseDbmHx() const { return _estimatedNoiseDbmHx; }

  /// set the estimated noise value for the V cross-polar channel, in dBm

  double getEstimatedNoiseDbmVx() const { return _estimatedNoiseDbmVx; }

  // get sweep and volume flags
  
  bool getEventFlagsSet() const { return _eventFlagsSet; }
  bool getStartOfSweepFlag() const { return _startOfSweepFlag; }
  bool getEndOfSweepFlag() const { return _endOfSweepFlag; }
  bool getStartOfVolumeFlag() const { return _startOfVolumeFlag; }
  bool getEndOfVolumeFlag() const { return _endOfVolumeFlag; }

  /// get utility flag
  
  bool getUtilityFlag() const { return _utilityFlag; }

  /// is this a long-range ray?
  
  bool getIsLongRange() const { return _isLongRange; }

  /// Get the georeference info for moving platforms.
  /// Returns NULL if georeference information is not available.

  inline const RadxGeoref *getGeoreference() const { return _georef; }
  inline RadxGeoref *getGeoreference() { return _georef; }
  inline bool getGeorefApplied() const { return _georefApplied; }

  /// Get the correction factors
  /// Returns NULL if correction factors information is not available.

  inline const RadxCfactors *getCfactors() const { return _cfactors; }
  inline RadxCfactors *getCfactors() { return _cfactors; }

  /// get the number of gates
  
  inline size_t getNGates() const { return _nGates; }
  
  /// get the maximum range for this ray, in km
  
  inline double getMaxRangeKm() const
    { return getGateRangeKm(_nGates) + (getGateSpacingKm() / 2.0); }
  
  /// Get number of fields on this ray.

  inline size_t getNFields() const { return (int) _fields.size(); }

  /// Get vector of field pointers for this ray.
  
  inline const vector<RadxField *> &getFields() const { return _fields; }
  inline vector<RadxField *> getFields() { return _fields; }

  /// get map of field names

  typedef map<string, int, less<string> > FieldNameMap;
  typedef pair<string, int> FieldNameMapPair;
  typedef FieldNameMap::iterator FieldNameMapIt;
  typedef FieldNameMap::const_iterator FieldNameMapConstIt;

  inline const FieldNameMap &getFieldNameMap() const { return _fieldNameMap; }

  /// Get pointer to a particular field, based on the position in the
  /// field vector.
  ///
  /// Returns NULL on failure.

  /// const version

  inline const RadxField *getField(int index) const {
    if (index < (int) _fields.size()) {
      return _fields[index];
    } else {
      return NULL;
    }
  }

  /// non-const version

  inline RadxField *getField(int index) {
    if (index < (int) _fields.size()) {
      return _fields[index];
    } else {
      return NULL;
    }
  }

  /// Get pointer to a particular field, based on the name.
  ///
  /// Returns NULL on failure.
  
  /// const version
  
  inline const RadxField *getField(const string &name) const {
    FieldNameMapConstIt it = _fieldNameMap.find(name);
    if (it != _fieldNameMap.end()) {
      return _fields[it->second];
    } else {
      return NULL;
    }
  }

  /// non-const version
  
  inline RadxField *getField(const string &name) {
    FieldNameMapConstIt it = _fieldNameMap.find(name);
    if (it != _fieldNameMap.end()) {
      return _fields[it->second];
    } else {
      return NULL;
    }
  }

  /// Check if the data at all gates in all fields is missing?
  /// Returns true if all missing, false otherwise.
  
  bool checkDataAllMissing() const;
  
  //@}

  ///////////////////////////////////////////////
  /// \name Printing:
  //@{

  /// Print ray metadata.
  
  void print(ostream &out) const;
 
  /// Print one-line summary
  
  void printSummary(ostream &out) const;
 
  /// Print with field metadata.
  
  void printWithFieldMeta(ostream &out) const;

  /// print with all metadata and field data.

  void printWithFieldData(ostream &out) const;

  /// print the field name map
  
  void printFieldNameMap(ostream &out) const;
  
  //@}

  ///////////////////////////////////////////////
  /// \name Memory management:
  /// This class uses the notion of clients to decide when it should be deleted.w
  /// If removeClient() returns 0, the object should be deleted.
  //@{

  /// add a client - i.e. an object using this ray
  /// returns the number of clients using the ray
  
  int addClient() const; 
  
  /// client object no longer needs this ray
  /// returns the number of clients using the ray
  
  int removeClient() const;
  
  // set number of clients to zero

  int removeAllClients() const;

  /// delete this ray if no longer used by any client

  static void deleteIfUnused(const RadxRay *ray);
  
  //@}

  /// \name Serialization:
  //@{

  // serialize into a RadxMsg
  
  void serialize(RadxMsg &msg);
  
  // deserialize from a RadxMsg
  // return 0 on success, -1 on failure

  int deserialize(const RadxMsg &msg);

  //@}
  
protected:
private:

  // data
  
  int _rayNum;     // optional ray number in the volume
  int _volNum;
  int _sweepNum;
  int _calibIndex; // which calibration is relevant to this ray

  string _scanName;
  Radx::SweepMode_t _sweepMode;
  Radx::PolarizationMode_t _polarizationMode;
  Radx::PrtMode_t _prtMode;
  Radx::FollowMode_t _followMode;

  time_t _timeSecs;
  double _nanoSecs;
  
  double _az;
  double _elev;
  double _fixedAngle;      // deg
  double _targetScanRate;  // deg/s
  double _trueScanRate;  // deg/s
  
  bool _isIndexed;   // ray is indexed
  double _angleRes;  // angular resolution if indexed (deg)
  
  bool _antennaTransition; // antenna is in transition

  int _nSamples;       // number of pulse samples in dwell
  double _pulseWidthUsec; // pulse width in microsecs
  double _prtSec;         // PRT in secs
  double _prtRatio;       /* PRT ratio, for staggered or dual PRT, < 1
                           * generally 2/3, 3/4 or 4/5 */
  double _nyquistMps;     // nyquist velocity (m/s)
  double _unambigRangeKm; // unambiguous range (km)

  // transmit power is generally uncorrected for gains etc

  double _measXmitPowerDbmH; // measured transmit power in H channel
  double _measXmitPowerDbmV; // measured transmit power in V channel

  // noise estimated from the data

  double _estimatedNoiseDbmHc; // H co-polar noise estimate
  double _estimatedNoiseDbmVc; // V co-polar noise estimate
  double _estimatedNoiseDbmHx; // H cross-polar noise estimate
  double _estimatedNoiseDbmVx; // V cross-polar noise estimate

  // sweep and volume event flags
  
  bool _eventFlagsSet;
  bool _startOfSweepFlag;
  bool _endOfSweepFlag;
  bool _startOfVolumeFlag;
  bool _endOfVolumeFlag;

  // general purpose utility flag for identification during processing
  // this is not stored out when CfRadial files are written

  bool _utilityFlag;

  // is this a long-range ray?

  bool _isLongRange;

  // georeference if available

  RadxGeoref *_georef;
  bool _georefApplied;

  // correction factors if applicable

  RadxCfactors *_cfactors;

  // number of gates

  size_t _nGates;

  // data fields
  
  vector<RadxField *> _fields;

  // map of data field names

  FieldNameMap _fieldNameMap;

  // keeping track of clients using this object

  mutable int _nClients;
  mutable pthread_mutex_t _nClientsMutex;

  // methods
  
  void _init();
  RadxRay & _copy(const RadxRay &rhs);
  
  string _addToFieldNameMap(const string &name, int index);

  /////////////////////////////////////////////////
  // serialization
  /////////////////////////////////////////////////
  
  static const int _metaStringsPartId = 1;
  static const int _metaNumbersPartId = 2;
  static const int _georefPartId = 3;
  static const int _cfactorsPartId = 4;
  static const int _fieldPartId = 5;
  
  // struct for metadata numbers in messages
  // strings not included - they are passed as XML
  
  typedef struct {
    
    Radx::fl64 startRangeKm;
    Radx::fl64 gateSpacingKm;

    Radx::si64 timeSecs;
    Radx::fl64 nanoSecs;
    
    Radx::fl64 az;
    Radx::fl64 elev;
    Radx::fl64 fixedAngle;
    Radx::fl64 targetScanRate;
    Radx::fl64 trueScanRate;
    Radx::fl64 angleRes;

    Radx::fl64 pulseWidthUsec;
    Radx::fl64 prtSec;
    Radx::fl64 prtRatio;

    Radx::fl64 nyquistMps;
    Radx::fl64 unambigRangeKm;

    Radx::fl64 measXmitPowerDbmH;
    Radx::fl64 measXmitPowerDbmV;

    Radx::fl64 estimatedNoiseDbmHc;
    Radx::fl64 estimatedNoiseDbmVc;
    Radx::fl64 estimatedNoiseDbmHx;
    Radx::fl64 estimatedNoiseDbmVx;

    Radx::si64 nGates;

    Radx::fl64 spareFl64[10];
  
    Radx::si32 rayNum;
    Radx::si32 volNum;
    Radx::si32 sweepNum;
    Radx::si32 calibIndex;

    Radx::si32 sweepMode;
    Radx::si32 polarizationMode;
    Radx::si32 prtMode;
    Radx::si32 followMode;

    Radx::si32 isIndexed;
    Radx::si32 antennaTransition;
    Radx::si32 nSamples;

    Radx::si32 eventFlagsSet;
    Radx::si32 startOfSweepFlag;
    Radx::si32 endOfSweepFlag;
    Radx::si32 startOfVolumeFlag;
    Radx::si32 endOfVolumeFlag;
    Radx::si32 utilityFlag;

    Radx::si32 isLongRange;
    Radx::si32 georefApplied;

    Radx::si32 spareSi32[13];

  } msgMetaNumbers_t;

  msgMetaNumbers_t _metaNumbers;
  
  /// convert metadata to XML
  
  void _loadMetaStringsToXml(string &xml, int level = 0) const;
  
  /// set metadata from XML
  /// returns 0 on success, -1 on failure
  
  int _setMetaStringsFromXml(const char *xml, 
                             size_t bufLen);

  /// load meta numbers to message struct
  
  void _loadMetaNumbersToMsg();
  
  /// set the meta number data from the message struct
  /// returns 0 on success, -1 on failure
  
  int _setMetaNumbersFromMsg(const msgMetaNumbers_t *metaNumbers,
                             size_t bufLen,
                             bool swap);
  
  /// swap meta numbers
  
  static void _swapMetaNumbers(msgMetaNumbers_t &msgMetaNumbers);
          
};

#endif

