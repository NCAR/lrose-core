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
// RadxVol.hh
//
// RadxVol object
//
// NetCDF data for radar radial data in CF-compliant file
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#ifndef RadxVol_HH
#define RadxVol_HH

#include <string>
#include <vector>
#include <Radx/Radx.hh>
#include <Radx/RadxPlatform.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxTime.hh>
class RadxSweep;
class RadxRay;
class RadxRcalib;
class RadxFile;
class RadxCfactors;
class RadxGeoref;
class PseudoRhi;
using namespace std;

//////////////////////////////////////////////////////////////////////
/// CLASS FOR STORING VOLUME (and SWEEP) data
/// 
/// This is the primary class for storing data from RADAR and LIDAR
/// volumes.
///
/// A volume is made up of a number of sweeps (RadxSweep), each of
/// which contain a number of rays (RadxRay).
///
/// A volume may contain one or more sweeps.
///
/// The information held by the volume object is references in a
/// number of ways. There are 3 primary vectors on a volume, and
/// all are inter-related:
///
/// \code
///  (a) rays
///  (b) sweeps
///  (c) fields 
/// \endcode
///
/// The RAYS are stored as a vector of RadxRay objects. Each RadxRay
/// owns a vector of field objects - see RadxRay for more information
/// on this.
///
/// A SWEEP is formed from a sequence of RAYS. RadxSweep is a
/// light-weight object which merely keeps information about how the
/// rays are grouped into sweeps. The sweep objects do not hold data,
/// they just keep track of the ray indices, and sweep properties.
///
/// As each ray is created, it manages the field data for that ray.
/// When this process is complete, the volume is a collection of
/// rays, which autonomously manage their own data and geometry.
///
/// A call to loadFieldsFromRays() will create fields on the
/// volume, copy the data from the rays into contiguous arrays on
/// these new fields, and then change the ray fields so that they
/// point to the volume fields rather than manage their own data.
/// After this step, the volume fields hold contiguous arrays for the
/// field data. These volume-owned fields manage the field data
/// memory, and the rays now just hold pointers into these main
/// fields.
///
/// NOTE ON CONVERTING DATA TYPES IN FIELDS:
///
/// If you convert the data type for individual fields, rather than
/// calling the methods on the RadxVol class, you must call
/// setRayFieldPointers() before using the field data in the RadxRay
/// objects. The call will ensure that the pointers in the field
/// objects on the rays points correctly to the data in the field
/// objects on the volume.

class RadxVol : public RadxRangeGeom, public RadxPacking {

public:

  /// Constructor
  
  RadxVol();
  
  /// Copy constructor
  
  RadxVol(const RadxVol &rhs);

  /// Copy constructor, one sweep only
  
  RadxVol(const RadxVol &rhs, int sweepNum);

  /// Destructor
  
  virtual ~RadxVol();

  /// Assignment
  
  RadxVol& operator=(const RadxVol &rhs);
  
  /// Copy one sweep only into existing object,
  /// clearing other sweeps as needed
  
  void copy(const RadxVol &rhs, int sweepNum);

  /// copy following meta data only:
  ///   main members, calibs, sweeps as in file
  ///
  /// does not copy sweeps, rays and fields
  
  void copyMeta(const RadxVol &rhs);
  
  //////////////////////////////////////////////////////////////////
  /// \name Set methods - except for platform parameters
  //@{

  /// Set debugging on/off. Off by default.

  void setDebug(bool val);

  /// Set the volume version, if available. Use this for the project name.

  void setVersion(const string &val) { _version = val; }

  /// Set the volume title, if available. Use this for the project name.

  void setTitle(const string &val) { _title = val; }

  /// Set the institution responsible for gathering the data.

  void setInstitution(const string &val) { _institution = val; }

  /// Set a references string. Use this for the flight number, if
  /// appropriate.

  void setReferences(const string &val) { _references = val; }
  
  /// Set source. Use this for the name of the facility generating the
  /// data.

  void setSource(const string &val) { _source = val; }

  /// Set history.
  ///
  /// This string should be appended to as different
  /// operations are performed on the data set, to provide a history
  /// of those operations.

  void setHistory(const string &val) { _history = val; }

  /// Set comment as applicable.

  void setComment(const string &val) { _comment = val; }

  /// Set author as applicable.

  void setAuthor(const string &val) { _author = val; }

  /// Set driver as applicable.

  void setDriver(const string &val) { _driver = val; }

  /// Set created as applicable.

  void setCreated(const string &val) { _created = val; }

  /// Set original file format.

  void setOrigFormat(const string &val) { _origFormat = val; }

  /// Set status XML as applicable. For general-purpose status information.

  void setStatusXml(const string &val) { _statusXml = val; }

  /// Add user-specified global attributes.
  /// This allows a user to add global attributes to a volume, to then
  /// be written to a NetCDF CfRadial file.
  /// For INT and DOUBLE types, 'val' is a string represention of the value
  /// For INT_ARRAY and DOUBLE_ARRAY types,
  ///     'val' is a comma-delimited list of values

  class UserGlobAttr {
  public:
    typedef enum {
      ATTR_STRING,
      ATTR_INT,
      ATTR_DOUBLE,
      ATTR_INT_ARRAY,
      ATTR_DOUBLE_ARRAY
    } attr_type_t;
    string name;
    attr_type_t attrType;
    string val;
    UserGlobAttr() {
      name.clear();
      attrType = ATTR_STRING;
      val.clear();
    }
  };

  void addUserGlobAttr(const string &name,
                       UserGlobAttr::attr_type_t atype,
                       const string &val) {
    UserGlobAttr attr;
    attr.name = name;
    attr.attrType = atype;
    attr.val = val;
    _userGlobAttr.push_back(attr);
  }

  void clearUserGlobAttr() {
    _userGlobAttr.clear();
  }

  /// Set the scan strategy name, if available.

  inline void setScanName(const string &val) { _scanName = val; }

  /// Set the scan strategy id (VCP), if available.

  inline void setScanId(int val) { _scanId = val; }

  /// Set the volume number.
  /// This increments with every volume, and may wrap.

  void setVolumeNumber(int val);
  
  /// Set the volume start time.
  ///
  /// The time is split into two parts, (a) the seconds UTC since Jan
  /// 1 1970, and the fractional seconds converted to nanosecons.

  inline void setStartTime(time_t secs, double nanoSecs) {
    _startTimeSecs = secs;
    _startNanoSecs = nanoSecs;
    if (_startNanoSecs >= 1.0e9) {
      _startTimeSecs += 1;
      _startNanoSecs -= 1.0e9;
    }
  }

  /// Set the volume end time.
  ///
  /// The time is split into two parts, (a) the seconds UTC since Jan
  /// 1 1970, and the fractional seconds converted to nanosecons.

  inline void setEndTime(time_t secs, double nanoSecs) {
    _endTimeSecs = secs;
    _endNanoSecs = nanoSecs;
    if (_endNanoSecs >= 1.0e9) {
      _endTimeSecs += 1;
      _endNanoSecs -= 1.0e9;
    }
  }

  /// Set the correction factors on the volume, if applicable.
  ///
  /// If not set, all corrections will be assumed to be 0.0.

  void setCfactors(RadxCfactors &cfac);
  
  /// set the target scan rate for all rays in a volume

  void setTargetScanRateDegPerSec(double rate);
  
  /// set the target scan rate for all rays in a sweep

  void setTargetScanRateDegPerSec(int sweepNum, double rate);

  /// Compute the max number of gates, by searching through the rays.
  /// Also determines if number of gates vary by ray.
  /// After this call, use the following to get the results:
  ///   size_t getMaxNGates() - see RadxPacking.
  ///   bool nGatesVary() - see RadxPacking

  void computeMaxNGates() const;

  /// compute the max number of gates in a sweep index.
  /// Return max number of gates.
  
  int computeMaxNGatesSweep(const RadxSweep *sweep) const;
  
  /// Set the number of gates.
  ///
  /// If more gates are needed, extend the field data out to a set number of
  /// gates. The data for extra gates are set to missing values.
  ///
  /// If fewer gates are needed, the data is truncated.
  
  void setNGates(size_t nGates);

  /// Set the number of gates on a sweep.
  ///
  /// If more gates are needed, extend the field data out to a set number of
  /// gates. The data for extra gates are set to missing values.
  ///
  /// If fewer gates are needed, the data is truncated.
  
  void setNGatesSweep(RadxSweep *sweep, size_t nGates);

  /// Set to constant number of gates per ray.
  /// 
  /// First we determine the max number of gates, and also check
  /// for a variable number of gates. If the number of gates does
  /// vary, the shorter rays are padded out with missing data.
  
  void setNGatesConstant();

  /////////////////////////////////////////////////////////////////////////
  /// Set the maximum range.
  /// Removes excess gates as needed.
  /// Does nothing if the current max range is less than that specified.
  
  void setMaxRangeKm(double maxRangeKm);
  
  /// set the packing from the rays

  void setPackingFromRays();

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Set methods specific to platform
  //@{

  /// Set the platform, using a filled-out object

  void setPlatform(const RadxPlatform &val) { _platform = val; }

  /// Set the instrument name, if available.

  inline void setInstrumentName(const string &val) {
    _platform.setInstrumentName(val);
  }

  /// Set the site name, if available.

  inline void setSiteName(const string &val) {
    _platform.setSiteName(val);
  }

  /// Set the instrument type. Default is RADAR.

  inline void setInstrumentType(Radx::InstrumentType_t val) {
    _platform.setInstrumentType(val);
  }

  /// Set the platform type. Default is FIXED.

  inline void setPlatformType(Radx::PlatformType_t val) {
    _platform.setPlatformType(val);
  }

  /// Set the primary rotation axis. Default is AXIS_Z.

  inline void setPrimaryAxis(Radx::PrimaryAxis_t val) {
    _platform.setPrimaryAxis(val);
  }

  /// Set the latitude of the platform in degrees.
  ///
  /// Used for non-mobile platforms.

  void setLatitudeDeg(double val);

  /// Set the longitude of the platform in degrees.
  ///
  /// Used for non-mobile platforms.

  void setLongitudeDeg(double val);

  /// Set the altitude of the platform in km.
  ///
  /// Used for non-mobile platforms.

  void setAltitudeKm(double val);

  /// Set the sensor ht above the surface

  void setSensorHtAglM(double val);

  /// set the radar location

  void setLocation(double latitudeDeg,
                   double longitudeDeg,
                   double altitudeKm);

  /// override the radar location
  /// this also sets the location in any georeference objects
  /// attached to the rays

  void overrideLocation(double latitudeDeg,
                        double longitudeDeg,
                        double altitudeKm);

  /// set the radar location from the start ray,
  // if georefs are active

  void setLocationFromStartRay();

  /// override the radar ht AGL
  /// this also sets the location in any georeference objects
  /// attached to the rays
  
  void overrideSensorHtAglM(double val);

  /// Set up the list of frequencies, adding them one at a time.
  /// Normally there is only one frequency, but multiple are supported.

  /// The set methods clear the list first, and then add the value
  /// The add methods do not clear the list first
  
  void setFrequencyHz(double val);
  void setWavelengthM(double val);
  void setWavelengthCm(double val);
  
  void addFrequencyHz(double val);
  void addWavelengthM(double val);
  void addWavelengthCm(double val);
  
  /// Set the RADAR beam width, horizontal, in degrees.

  void setRadarBeamWidthDegH(double val);

  /// Set the RADAR beam width, vertical, in degrees.

  void setRadarBeamWidthDegV(double val);

  /// Set the RADAR antenna gain, horizontal, in dB.

  void setRadarAntennaGainDbH(double val);

  /// Set the RADAR antenna gain, vertical, in dB.

  void setRadarAntennaGainDbV(double val);

  /// Set the RADAR receiver bandwidth, in MHz.

  void setRadarReceiverBandwidthMhz(double val);

  /// Set the instrument type to LIDAR. Default is RADAR.

  void setIsLidar(bool val);

  /// Set the LIDAR constant.

  void setLidarConstant(double val);

  /// Set the LIDAR pulse energy, in Joules.

  void setLidarPulseEnergyJ(double val);

  /// Set the LIDAR peak power, in Watts.

  void setLidarPeakPowerW(double val);

  /// Set the LIDAR aperture diameter, in cm.

  void setLidarApertureDiamCm(double val);

  /// Set the LIDAR aperture efficiency, in percent.

  void setLidarApertureEfficiency(double val);

  /// Set the LIDAR field of view, in milli-radians.

  void setLidarFieldOfViewMrad(double val);
  
  /// Set the LIDAR beam divergence, in milli-radians.

  void setLidarBeamDivergenceMrad(double val);

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Add methods:
  //@{

  /// Add a ray to the volume.
  ///
  /// The ray must be created with new() before calling this method.
  /// RadxVol takes responsibility for freeing the object.
  
  void addRay(RadxRay *ray);

  /// Add a sweep object.
  ///
  /// The sweep must be created with new() before calling this method.
  /// RadxVol takes responsibility for freeing the sweep object.
  
  void addSweep(RadxSweep *sweep);

  /// Add sweep to 'as-in-file' vector
  /// These are the sweeps as originally in the file before selected
  /// sweeps were requested.
  /// A copy of the object is made, and is managed by this object.

  void addSweepAsInFile(RadxSweep *sweep);
  
  /// Add a calibration to the volume.
  ///
  /// The calibration must be created with new() before calling this method.
  /// RadxVol takes responsibility for freeing the calibration object.
  
  void addCalib(RadxRcalib *calib);
  
  /// Add a field to the volume.
  ///
  /// The field must be created with new() before calling this method.
  /// RadxVol takes responsibility for freeing the field object.

  void addField(RadxField *field);

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Change data representation in volume:
  //@{

  /// After you you have added rays with field data local to each ray,
  /// you may wish to convert those to contiguous fields and
  /// initialize other information on the volume before using it.
  ///
  /// The method loads up contiguous fields in the volume from data in
  /// the rays. The fields in the rays are then set to point to the
  /// contiguous fields.
  ///
  /// nFieldsConstantPerRay:
  ///   only applicable if not all input fields are present in the
  ///   input data.
  /// If true, each ray will have the same number of fields, even if in
  /// the incoming data some rays do not have all of the fields. If
  /// false, a field will only be included in a ray if that field
  /// also exists in the incoming data.
  ///
  /// See also loadRaysFromFields().

  void loadFieldsFromRays(bool nFieldsConstantPerRay = false);

  /// Load up the ray fields from the contiguous fields in the volume.
  /// This is the inverse of loadFieldsFromRays()

  void loadRaysFromFields();
  
  /// Set field data pointers in the rays to point into the
  /// main contiguous fields on the volume.
  ///
  /// If the rays that have been added to the volume do not hold the
  /// data themselves, call this method to set the pointers in the ray
  /// fields so that they refer to the main fields on the volume.
  
  void setRayFieldPointers();

  /// Make a copy of the field with the specified name.
  /// This forms a contiguous field from the ray data.
  /// Returns a pointer to the field on success, NULL on failure.

  RadxField *copyField(const string &fieldName) const;

  /// Rename a field
  /// returns 0 on success, -1 if field does not exist in any ray

  int renameField(const string &oldName, const string &newName);
  
  /// Load volume information from the rays.
  ///
  /// This sets the volume number and the start and end times.
  
  void loadVolumeInfoFromRays();
  
  /// check that the ray times increase, set flag accordingly

  void checkRayTimesIncrease();
  
  /// Check through the rays, and increment the sweep number
  /// if the pol mode changes in the middle of a sweep
  
  void incrementSweepOnPolModeChange();
  
  /// Check through the rays, and increment the sweep number
  /// if the PRT mode changes in the middle of a sweep
  
  void incrementSweepOnPrtModeChange();
  
  /// Load the sweep information from the rays.
  ///
  /// This loops through all of the rays, and determines the sweep
  /// information from them. The resulting information is stored
  /// in the sweeps array on the volume.
  ///
  /// Also sets the start/end of sweep/volume flags
  
  void loadSweepInfoFromRays();
  
  /// Adjust the limits of sweeps, by comparing the measured angles
  /// to the fixed angles.
  ///
  /// Sometimes the transitions from one fixed angle to another are
  /// not accurately described by the scan flags, and as a result rays
  /// are not correctly assigned to the sweeps.
  ///
  /// This routine goes through the volume in ray order, and adjusts
  /// the way rays are associated with each sweep. It does this by
  /// comparing the actual angle with the fixed angle, and minimizes
  /// the angular difference.
  ///
  /// Before calling this routine you need to ensure that the fixed
  /// sweep angle and measured ray angle has been set on the rays.
  
  void adjustSweepLimitsUsingAngles();
  
  /// Adjust surveillance sweep limits based on azimuth.
  ///
  /// Some radars (e.g. DOWs) always change elevation angle at a
  /// fixed theorerical azimuth.
  /// 
  /// This function sets the transitions between sweeps based on a
  /// designated azimuth.
  
  void adjustSurSweepLimitsToFixedAzimuth(double azimuth);
  
  /// compute the geometry limits from rays

  void computeGeomLimitsFromRays(double &minElev,
                                 double &maxElev,
                                 double &minRange,
                                 double &maxRange);
  
  /// Compute the fixed angle for each sweep from the rays.
  /// Also sets the fixed angle on rays and sweeps.
  ///
  /// If useMean is true, computes using the mean
  /// If useMean is false, uses the median
  ///
  /// If force is true, the angles will be computed for all sweeps.
  /// If force is false, the angles will only be computed for
  /// sweeps with a missing fixed angle.

  void computeFixedAnglesFromRays(bool force = true, 
                                  bool useMean = true);

  /// Compute sweep scan rates from ray data - in deg/sec.
  ///
  /// This is done using the angle information on the rays.
  /// Sets the measureScanRate on the sweeps.
  /// Use sweep->getMeasuredScanRateDegPerSec() to retrieve
  /// the scan rates after this call completes.

  void computeSweepScanRatesFromRays();
  
  /// Compute median angle for specified sweep from ray data.
  /// Does not set the angle on the sweep or ray objects.
  ///
  /// Returns computed median angle.
  
  double computeSweepMedianFixedAngle(const RadxSweep *sweep) const;
  
  /// Estimate nyquist per sweep from velocity field
  ///
  /// If nyquist values are missing, we can estimate the nyquist
  /// finding the max absolute velocity in each sweep.
  
  void estimateSweepNyquistFromVel(const string &velFieldName);
  
  /// Set the sweep scan modes from ray angles
  ///
  /// Deduce the antenna scan mode from the ray angles in each
  /// sweep, and set the scan mode on the rays and sweep objects.
  
  void setSweepScanModeFromRayAngles();

  /// Set the modes on the rays, by copying from the sweeps.
  ///
  /// If the sweep mode, prf mode etc have been set correctly on the 
  /// sweep objects, you can call this method to copy the modes over to
  /// the ray objects.
  
  void loadModesFromSweepsToRays();

  /// Set the fixed angle on the rays, by copying from the sweeps.
  ///
  /// If the fixed angles have been set correctly on the sweep
  /// objects, you can call this method to copy the modes over to the
  /// ray objects.
  
  void loadFixedAnglesFromSweepsToRays();

  /// Load the ray metadata from sweep information.
  ///
  /// This loops through all of the sweeps, setting the
  /// sweep-related info on the rays
  
  void loadMetadataFromSweepsToRays();
  
  /// load the calbration index on the rays, using the pulse width to
  /// determine which calibration is relevant.
  ///
  /// This method checks the pulse witdth on each ray, and compares
  /// these with the pulse width for each calibration object. It then
  /// sets the calibration index to point to the calibration data with
  /// the pulse width closest to that on the ray.
  
  void loadCalibIndexOnRays();

  /// Constrain the data by specifying fixedAngle limits.
  ///
  /// This operation will remove unwanted rays from the data set,
  /// remap the field arrays for the remaining rays and set the field
  /// pointers in the rays to the remapped fields on the volume.
  ///
  /// If strictChecking is TRUE, we only get rays within the specified limits.
  /// If strictChecking is FALSE, we are guaranteed to get at least 1 sweep.
  ///
  /// Returns 0 on success, -1 on failure
  
  int constrainByFixedAngle(double minFixedAngleDeg,
                            double maxFixedAngleDeg,
                            bool strictChecking = false);
  
  /// Constrain the data by specifying sweep number limits.
  ///
  /// This operation will remove unwanted rays from the data set,
  /// remap the field arrays for the remaining rays and set the field
  /// pointers in the rays to the remapped fields on the volume.
  ///
  /// If strictChecking is TRUE, we only get rays within the specified limits.
  /// If strictChecking is FALSE, we are guaranteed to get at least 1 sweep.
  ///
  /// Returns 0 on success, -1 on failure
  
  int constrainBySweepNum(int minSweepNum,
                          int maxSweepNum,
                          bool strictChecking = false);

  /// remove rays with all missing data
  
  void removeRaysWithDataAllMissing();

  /////////////////////////////////////////////////////////////////
  /// Ensure ray times are monotonically increasing by
  /// interpolating the times if there are duplicates

  void interpRayTimes();

  /////////////////////////////////////////////////////////////////
  /// Sort rays by time
  
  void sortRaysByTime();

  /////////////////////////////////////////////////////////////////
  // Sort sweeps by fixed angle, reordering the rays accordingly

  void sortSweepsByFixedAngle();
  
  /////////////////////////////////////////////////////////////////
  /// Sort rays by number

  void setRayNumbersInOrder();
  void sortRaysByNumber();

  /////////////////////////////////////////////////////////////////
  /// Sort sweep rays by azimuth

  void sortSweepRaysByAzimuth();

  /////////////////////////////////////////////////////////////////
  /// Remap all fields and rays onto the specified geometry.
  ///
  /// This leaves the memory managed by the rays.
  /// Call loadFieldsFromRays() if you need the field data
  /// to be managed by the volume.
  ///
  /// If interp is true, uses interpolation if appropriate.
  /// Otherwise uses nearest neighbor.
  
  virtual void remapRangeGeom(double startRangeKm,
                              double gateSpacingKm,
                              bool interp = false);

  /// Remap data in all rays to the predominant range geometry.
  ///
  /// A search is made through the rays, to identify which is the
  /// predominant range geometry.  All rays which do not match this
  /// are then remapped to this predominant geometry.
  ///
  /// This leaves the memory managed by the rays.
  /// Call loadFieldsFromRays() if you need the field data
  /// to be managed by the volume.
  
  void remapToPredomGeom();
  
  /// Remap data in all rays to the finest range geometry.
  ///
  /// A search is made through the rays, to identify which has the
  /// finest gate spacing.  All rays which do not match this
  /// are then remapped to this predominant geometry.
  ///
  /// This leaves the memory managed by the rays.
  /// Call loadFieldsFromRays() if you need the field data
  /// to be managed by the volume.
  
  void remapToFinestGeom();

  /// Remove rays which do not match the predominant range geometry.
  ///
  /// A search is made through the rays, to identify which is the
  /// predominant range geometry.  All rays which do not match this
  /// are then removed from the volume.
  ///
  /// This leaves the memory managed by the rays.
  /// Call loadFieldsFromRays() if you need the field data
  /// to be managed by the volume.
  
  void filterOnPredomGeom();

  /// Copy the range geom from the fields to the rays, provided
  /// the fields have consistent in geometry
  
  void copyRangeGeomFromFieldsToRays();

  ////////////////////////////////////////////////////
  /// Copy the range geom to the fields from the rays
  
  void copyRangeGeomFromRaysToFields();
  
  /// remove rays with utility flag set
  
  void removeFlaggedRays();

  /// filter based on ray vectors
  /// Keep the good rays, remove the bad rays
  
  void removeBadRays(vector<RadxRay *> &goodRays,
                     vector<RadxRay *> &badRays);
  
  /// Clear antenna transition flag on all rays

  void clearTransitionFlagOnAllRays();

  /// Remove rays with the antenna transition flag set.
  
  void removeTransitionRays();

  /// Remove rays with transitions, with the specified margin.
  ///
  /// Sometimes the transition flag is turned on too early in
  /// a transition, on not turned off quickly enough after a transition.
  /// If you set this to a number greater than 0, that number of rays
  /// will be included at each end of the transition, i.e. the
  /// transition will effectively be shorter at each end by this
  /// number of rays
  
  void removeTransitionRays(int nRaysMargin);

  /// Check transitions in surveillance mode, ensuring that the
  /// pointing angle error is within the  given margin and that
  /// the ray belongs to the correct sweep.
  
  void optimizeSurveillanceTransitions(double maxFixedAngleErrorDeg);

  /// Trim surveillance sweeps to 360 deg
  /// Remove extra rays in each surveillance sweep
  
  void trimSurveillanceSweepsTo360Deg();

  /// Remove sweeps with fewer that the given number of rays
  
  void removeSweepsWithTooFewRays(size_t minNRays);

  /// Reorder the sweeps into ascending angle order
  ///
  /// If the sweeps are reordered, this means that the rays times
  /// will no longer be monotonically increasing

  void reorderSweepsAscendingAngle();

  /// Reorder the sweeps as in file into ascending angle order
  
  void reorderSweepsAsInFileAscendingAngle();

  /// Apply a time offset, in seconds to all rays in the volume
  /// This applies to the rays currently in the volume, not to
  /// any future reads.
  /// The offset is ADDED to the ray times.
  
  void applyTimeOffsetSecs(double offsetSecs);

  /// Apply an azimuth offset to all rays in the volume
  /// This applies to the rays currently in the volume, not to
  /// any future reads
  
  void applyAzimuthOffset(double offset);

  /// Apply an elevation offset to all rays in the volume
  /// This applies to the rays currently in the volume, not to
  /// any future reads
  
  void applyElevationOffset(double offset);

  /// Set the fixed angle for a sweep.
  /// Also sets the fixed angle for the rays in the sweep
  
  void setFixedAngleDeg(int sweepNum, double fixedAngle);

  /// combine rays from sweeps with common fixed angle and
  /// gate geometry, but with different fields
  
  void combineSweepsAtSameFixedAngleAndGeom(bool keepLongRange = false);

  /// Make fields uniform in the volume.
  /// This ensures that all rays in the volume have the same fields
  /// and that they are in the same order in each ray.
  /// If fields a missing from a ray, a suitable field is added
  /// containing missing data.

  void makeFieldsUniform();
  
  /// Make fields uniform for each sweep.
  /// This ensures that all rays in a sweep have the same fields.
  /// and that they are in the same order in each ray.
  /// If fields a missing from a ray, a suitable field is added
  /// containing missing data.
  
  void makeFieldsUniformPerSweep();
  
  /// Reorder the fields, by name, removing any extra fields.
  
  void reorderFieldsByName(const vector<string> &names);
  
  /// Remove a field, given its name.
  /// Returns 0 on success, -1 on failure

  int removeField(const string &name);
  
  /// apply the georeference corrections for moving platforms
  /// to compute the earth-relative azimuth and elevation
  ///
  /// If force is true, the georefs are always applied.
  /// If force is false, the georefs are applied only if
  /// they have not been applied previously.

  void applyGeorefs(bool force = true);

  /// count the number of rays in which each georef element
  /// is not missing
  
  void countGeorefsNotMissing(RadxGeoref &count) const;

  /// Load up pseudo RHIs, by analyzing the rays in the volume.
  /// Only relevant for surveillance and sector ppi-type volumes.
  /// Returns 0 on success.
  /// Returns -1 on error - i.e. if not ppi-type scan.
  /// After success, you can call getPseudoRhis(),
  ///                and clearPseudoRhis().

  int loadPseudoRhis();

  /// get vector of pseudo RHIs, after calling loadPseudoRhis()

  const vector<PseudoRhi *> &getPseudoRhis() const { return _pseudoRhis; }

  /// clear vector of pseudo RHIs

  void clearPseudoRhis();

  // Load up a 2D field fl32 array from a vector of rays
  // The ray data for the specified field will be converted to fl32.
  // This is a static method, does not use any vol members.
  // Returns 0 on success, -1 on failure
  
  static int load2DFieldFromRays(const vector<RadxRay *> &rays,
                                 const string &fieldName,
                                 RadxArray2D<Radx::fl32> &array,
                                 Radx::fl32 missingValue = -9999.0);

  // Load up a 2D field si32 array from a vector of rays
  // The ray data for the specified field will be converted to si32.
  // This is a static method, does not use any vol members.
  // Returns 0 on success, -1 on failure
  
  static int load2DFieldFromRays(const vector<RadxRay *> &rays,
                                 const string &fieldName,
                                 RadxArray2D<Radx::si32> &array,
                                 Radx::si32 missingValue = -9999.0);

  // Load up ray fields from 2D fl32 field array.
  // This is a static method, does not use any vol members.
  // Returns 0 on success, -1 on failure
  
  static int loadRaysFrom2DField(const RadxArray2D<Radx::fl32> &array,
                                 const vector<RadxRay *> &rays,
                                 const string &fieldName,
                                 const string &units,
                                 Radx::fl32 missingValue);

  // Load up ray fields from 2D si32 field array.
  // This is a static method, does not use any vol members.
  // Returns 0 on success, -1 on failure
  
  static int loadRaysFrom2DField(const RadxArray2D<Radx::si32> &array,
                                 const vector<RadxRay *> &rays,
                                 const string &fieldName,
                                 const string &units,
                                 Radx::si32 missingValue);

  
  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Convert the data type for all fields
  //@{

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

  /// Convert all fields to the specified data type.
  
  void convertToType(Radx::DataType_t targetType);

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
  /// For integer types, the specified scale and offset
  /// are used.
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
  
  /// compute field stats for rays currently in the volume
  ///
  /// Pass in a stats method type, and a vector of fields
  ///
  /// The requested stats on computed for each field,
  /// and on a point-by-point basis.
  ///
  /// If the geometry is not constant, remap to the predominant geom.
  ///
  /// maxFractionMissing indicates the maximum fraction of the input data field
  /// that can be missing for valid statistics. Should be between 0 and 1.
  /// 
  /// Returns NULL if no rays are present in the volume.
  /// Otherwise, returns ray containing results.
  
  RadxRay *computeFieldStats(RadxField::StatsMethod_t method,
                             double maxFractionMissing = 0.25);

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Get methods - except for platform parameters
  //@{

  /// Get version. May be used for project name.

  inline const string &getVersion() const { return _version; }

  /// Get title. May be used for project name.

  inline const string &getTitle() const { return _title; }

  /// Get institution responsible for gathering the data.

  inline const string &getInstitution() const { return _institution; }

  /// Get references. May be used for flight number.

  inline const string &getReferences() const { return _references; }

  /// Get source. Should be used for generating facility.

  inline const string &getSource() const { return _source; }

  /// Get history.

  inline const string &getHistory() const { return _history; }

  /// Get comment.

  inline const string &getComment() const { return _comment; }

  /// Get author.

  inline const string &getAuthor() const { return _author; }

  /// Get driver.

  inline const string &getDriver() const { return _driver; }

  /// Get created.

  inline const string &getCreated() const { return _created; }

  /// Get original file format string.

  inline const string &getOrigFormat() const { return _origFormat; }

  /// Get status XML. For general-purpose status information.

  inline const string &getStatusXml() const { return _statusXml; }

  /// get user-specified global attributes

  const vector<UserGlobAttr> &getUserGlobAttr() const { return _userGlobAttr; }

  /// Get scan name.

  inline const string &getScanName() const { return _scanName; }

  /// Get scan id (VCP).

  inline int getScanId() const { return _scanId; }

  /// Get volume number.

  inline int getVolumeNumber() const { return _volNum; }

  /// get start end end time as RadxTime

  inline RadxTime getStartRadxTime() const {
    return RadxTime(_startTimeSecs, _startNanoSecs / 1.0e9);
  }
  inline RadxTime getEndRadxTime() const {
    return RadxTime(_endTimeSecs, _endNanoSecs / 1.0e9);
  }

  /// Get start time in seconds.
  /// Combine with getNanoSecs() for high-precision time.
  ///
  /// \code
  /// double time = (time in secs) + (nano seconds) / 1.0e9.
  /// \endcode

  inline double getStartNanoSecs() const { return _startNanoSecs; } 

  /// Get nano-seconds for start time.

  inline time_t getStartTimeSecs() const { return _startTimeSecs; }

  /// Get end time in seconds.
  /// Combine with getNanoSecs() for high-precision time.
  ///
  /// \code
  /// double time = (time in secs) + (nano seconds) / 1.0e9.
  /// \endcode

  inline time_t getEndTimeSecs() const { return _endTimeSecs; }

  /// Get nano-seconds for end time.

  inline double getEndNanoSecs() const { return _endNanoSecs; }

  /// get flag to indicate that the ray times are in order -
  /// i.e. they are increasing

  inline bool getRayTimesIncrease() const { return _rayTimesIncrease; }

  /// Get number of sweeps in volume.

  size_t getNSweeps() const { return _sweeps.size(); }

  /// get the predominant sweep mode
  /// i.e. the most common sweep mode in the volume
  /// this makes use of the scan flags set on the rays
  /// if the scan flags are not accurately set, use
  /// getPredomSweepModeFromAngles() instead.

  Radx::SweepMode_t getPredomSweepMode() const;

  /// get the predominant sweep mode from checking the angles
  
  Radx::SweepMode_t getPredomSweepModeFromAngles() const;

  /// get the predominant range geometry

  void getPredomRayGeom(double &startRangeKm, double &gateSpacingKm) const;

  /// get the finest resolution range geometry

  void getFinestRayGeom(double &startRangeKm, double &gateSpacingKm) const;

  /// Get number of rays in volume.
  
  inline size_t getNRays() const { return _rays.size(); }

  /// Get number of fields in volume.

  inline size_t getNFields() const { return _fields.size(); }
  
  /// Get the list of unique field names, compiled by
  /// searching through all rays.
  ///
  /// The order of the field names found is preserved

  vector<string> getUniqueFieldNameList() const;

  /// convert all fields to same data type
  /// widening as required
  
  void widenFieldsToUniformType();

  /// set all fields to same data type

  void setFieldsToUniformType(Radx::DataType_t dataType);

  /// Get field, based on index.
  ///
  /// Returns NULL on failure.

  inline RadxField *getField(int index) const {
    if (index < (int) _fields.size()) {
      return _fields[index];
    } else {
      return NULL;
    }
  }

  /// Get field on the volume, based on name.
  /// Returns NULL on failure.

  RadxField *getField(const string &name) const {
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      if (_fields[ii]->getName() == name) {
        return _fields[ii];
      }
    }
    return NULL;
  }

  /// Get a field from a ray, given the name.
  /// Find the first available field on a suitable ray.
  /// Returns field pointer on success, NULL on failure.
  
  const RadxField *getFieldFromRay(const string &name) const;

  /// Get vector of fields.

  inline const vector<RadxField *> &getFields() const { return _fields; }
  inline vector<RadxField *> &getFields() { return _fields; }

  /// Get sweep by sweep number (not the index).
  /// Returns NULL on failure.

  const RadxSweep *getSweepByNumber(int sweepNum) const;
  RadxSweep *getSweepByNumber(int sweepNum);
  
  /// Get sweep by fixed angle
  /// returns the closest sweep if available
  /// Returns NULL on failure.
  
  const RadxSweep *getSweepByFixedAngle(double requestedAngle) const;
  RadxSweep *getSweepByFixedAngle(double requestedAngle);

  /// Get vector of sweeps.

  const vector<RadxSweep *> &getSweeps() const { return _sweeps; }
  vector<RadxSweep *> &getSweeps() { return _sweeps; }

  /// check if all rays in a sweep are in an antenna transition

  bool checkAllSweepRaysInTransition(const RadxSweep *sweep) const;
  bool checkAllSweepRaysInTransition(int sweepNum) const;
 
  /// Get vector of sweeps as they appeared in original file.

  inline const vector<RadxSweep *> &getSweepsAsInFile() const {
    return _sweepsAsInFile;
  }
  
  /// Get vector of rays.

  inline const vector<RadxRay *> &getRays() const { return _rays; }
  inline vector<RadxRay *> &getRays() { return _rays; }
  
  /// Get vector of radar calibrations.

  inline size_t getNRcalibs() const { return _rcalibs.size(); }
  inline const vector<RadxRcalib *> &getRcalibs() const { return _rcalibs; }
  inline vector<RadxRcalib *> &getRcalibs() { return _rcalibs; }
  
  /// Get pointer to correction factors.
  ///
  /// Returns NULL if there are no corrections available.
  
  inline const RadxCfactors *getCfactors() const { return _cfactors; }
  inline RadxCfactors *getCfactors() { return _cfactors; }
  
  /// Determine whether rays are indexed in angle, and what
  /// the predominant angular resolution is.
  //
  /// Returns true if all rays are indexed, false otherwise.
  
  bool checkForIndexedRays() const;
  
  /// check whether volume is predominantly in RHI mode
  ///
  /// Returns true if RHI, false otherwise

  bool checkIsRhi() const;

  /// check whether a series of rays is predominantly in RHI mode
  ///
  /// Returns true if RHI, false otherwise

  bool checkIsRhi(size_t startRayIndex, size_t endRayIndex);

  /// check if rays are predominantly in
  /// SURVEILLANCE mode i.e. 360's azimuth rotation
  /// Side effect - sets sweep info
  /// Returns true if surveillance, false otherwise

  bool checkIsSurveillance() const;

  /// check if sequence of rays are predominantly in
  /// SURVEILLANCE mode i.e. 360's azimuth rotation
  /// Returns true if surveillance, false otherwise
  
  bool checkIsSurveillance(size_t startRayIndex, size_t endRayIndex) const;

  /// check if rays are predominantly in SECTOR mode.
  /// Returns true if sector, false otherwise

  bool checkIsSector() const;

  /// Compute the azimuth swept out by a sweep
  /// Returns the azimuth covered.
  
  double computeAzCovered(const RadxSweep *sweep) const;

  /// Compute the azimuth swept out by sequence of rays
  /// Returns the azimuth covered.
  
  double computeAzCovered(size_t startRayIndex,
                          size_t endRayIndex) const;

  /// check whether start_range and gate_spacing varies per ray
  /// Returns true if gate geom varies by ray, false otherwise

  bool gateGeomVariesByRay() const;

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Get methods for platform parameters
  //@{

  /// Get platform object

  inline const RadxPlatform &getPlatform() const {
    return _platform;
  }

  /// Get instrument name.

  inline const string &getInstrumentName() const {
    return _platform.getInstrumentName();
  }

  /// Get site name.

  inline const string &getSiteName() const {
    return _platform.getSiteName();
  }

  /// Get instrument type

  inline Radx::InstrumentType_t getInstrumentType() const {
    return _platform.getInstrumentType();
  }

  /// Get platform type

  inline Radx::PlatformType_t getPlatformType() const {
    return _platform.getPlatformType();
  }

  /// Get primary axis

  inline Radx::PrimaryAxis_t getPrimaryAxis() const {
    return _platform.getPrimaryAxis();
  }

  /// Get latitude in degrees. Applies to FIXED platform.

  inline double getLatitudeDeg() const {
    return _platform.getLatitudeDeg();
  }

  /// Get longitude in degrees. Applies to FIXED platform.

  inline double getLongitudeDeg() const {
    return _platform.getLongitudeDeg();
  }

  /// Get altitude in km. Applies to FIXED platform.

  inline double getAltitudeKm() const {
    return _platform.getAltitudeKm();
  }

  /// Get the sensor ht above the surface in meters

  inline double getSensorHtAglM() const {
    return _platform.getSensorHtAglM();
  }

  /// Get vector of frequencies - normally only 1 entry

  inline const vector<double> &getFrequencyHz() const {
    return _platform.getFrequencyHz();
  }
  double getWavelengthM() const;
  double getWavelengthCm() const;

  /// For RADAR, get horizontal beam width, in degrees.

  inline double getRadarBeamWidthDegH() const {
    return _platform.getRadarBeamWidthDegH();
  }

  /// For RADAR, get vertical beam width, in degrees.

  inline double getRadarBeamWidthDegV() const {
    return _platform.getRadarBeamWidthDegV();
  }

  /// For RADAR, get horizontal antenna gain, in dB.

  inline double getRadarAntennaGainDbH() const {
    return _platform.getRadarAntennaGainDbH();
  }

  /// For RADAR, get horizontal antrenna gain, in dB.

  inline double getRadarAntennaGainDbV() const {
    return _platform.getRadarAntennaGainDbV();
  }

  /// For RADAR, get receiver band width, in Mhz.

  inline double getRadarReceiverBandwidthMhz() const {
    return _platform.getRadarReceiverBandwidthMhz();
  }

  /// For LIDAR, get lidar constant.

  inline double getLidarConstant() const {
    return _platform.getLidarConstant();
  }

  /// For LIDAR, get pulse energy, in Joules.

  inline double getLidarPulseEnergyJ() const {
    return _platform.getLidarPulseEnergyJ();
  }

  /// For LIDAR, get peak power, in watts.

  inline double getLidarPeakPowerW() const {
    return _platform.getLidarPeakPowerW();
  }

  /// For LIDAR, get aperture diameter, in cm.

  inline double getLidarApertureDiamCm() const {
    return _platform.getLidarApertureDiamCm();
  }

  /// For LIDAR, get aperture efficiency, in percent.

  inline double getLidarApertureEfficiency() const {
    return _platform.getLidarApertureEfficiency();
  }

  /// For LIDAR, get field of view, in milli-radians.

  inline double getLidarFieldOfViewMrad() const {
    return _platform.getLidarFieldOfViewMrad();
  }

  /// For LIDAR, get beam divergence, in milli-radians.

  inline double getLidarBeamDivergenceMrad() const {
    return _platform.getLidarBeamDivergenceMrad();
  }

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Path used in read/write
  //@{

  void setPathInUse(const string &val) const { _pathInUse = val; }

  /// Get the full file path actually used for reading or writing.
  
  const string &getPathInUse() const { return _pathInUse; }

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Clearing data:
  //@{

  /// Clear all of the data in the object.
  
  void clear();
  
  /// Remove all rays on object.

  void clearRays();

  /// Clear sweep information on object.

  void clearSweeps();

  /// Clear sweep information as they appear in original file.

  void clearSweepsAsInFile();

  /// Clear radar calibration info on object.

  void clearRcalibs();

  /// Clear the field vector, deleting the RadxField objects first.
  
  void clearFields();

  /// Clear the correction factors.
  
  void clearCfactors();
  
  /// Clear fields from rays, but retain the rays with their metadata.
  
  void clearRayFields();
  
  /// Clear the frequency list.
  
  void clearFrequency();

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Printing:
  //@{

  /// Print metadata on volume object.
  
  void print(ostream &out) const;
  
  /// Print ray metadata

  void printWithRayMetaData(ostream &out) const;

  /// Print summary of each ray

  void printRaySummary(ostream &out) const;

  /// Print full metadata, and actual field data.

  void printWithFieldData(ostream &out) const;

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

  // debug state

  bool _debug;

  // class for keeping track of the geometry of the rays and
  // remapping data onto a common geometry

  class RayGeom {
  public:
    RayGeom();
    RayGeom(double start_range,
            double gate_spacing);
    void print(ostream &out) const;
    double startRange;
    double gateSpacing;
    int rayCount;
  };

  // class for combining sweeps with same fixed angle but different fields

  class Combo {
  public:
    size_t target;
    vector<size_t> sources; 
    Combo() {
      target = 0;
    }
    Combo(size_t index) {
      target = index;
    }
  };
  
  /// sorting rays

  class RayPtr {
  public:
    RadxRay *ptr;
    RayPtr(RadxRay *p) : ptr(p) {}
  };

  class SortByRayTime {
  public:
    bool operator()(const RayPtr &lhs, const RayPtr &rhs) const;
  };

  class SortByRayNumber {
  public:
    bool operator()(const RayPtr &lhs, const RayPtr &rhs) const;
  };

  class SortByRayAzimuth {
  public:
    bool operator()(const RayPtr &lhs, const RayPtr &rhs) const;
  };

  /// sorting sweeps

  class SweepPtr {
  public:
    RadxSweep *ptr;
    SweepPtr(RadxSweep *p) : ptr(p) {}
  };

  class SortByFixedAngle {
  public:
    bool operator() (const SweepPtr &lhs, const SweepPtr &rhs) const;
  };

  // meta strings

  string _version;     // from CF
  string _title;       // from CF
  string _institution; // from CF
  string _references;  // from CF
  string _source;      // from CF
  string _history;     // from CF
  string _comment;     // from CF
  string _author;      // person generating the files
  string _driver;      // tool used to generate the file
  string _created;     // when data set was created
  string _origFormat;  // format of file read in
  string _statusXml;   // generic status specific to instrument

  vector<UserGlobAttr> _userGlobAttr;
  
  // volume number

  int _volNum;

  // scan details

  string _scanName;
  int _scanId; // VCP

  // predominant sweep mode for volume, by checking angles
  // surveillance, sector or rhi

  mutable bool _sweepModeFromAnglesChecked;
  mutable Radx::SweepMode_t _predomSweepModeFromAngles;
  
  // times

  time_t _startTimeSecs;
  time_t _endTimeSecs;
  double _startNanoSecs;
  double _endNanoSecs;

  // flag to indicate that the ray times are in order -
  // i.e. they are increasing

  bool _rayTimesIncrease;

  // path in use - for reading/writing
  
  mutable string _pathInUse; ///< path in use

  // platform parameters

  RadxPlatform _platform;

  // sweeps
  
  vector<RadxSweep *> _sweeps;

  // sweeps as they were in the file
  
  vector<RadxSweep *> _sweepsAsInFile;

  // calibrations

  vector<RadxRcalib *> _rcalibs;

  // rays

  vector<RadxRay *> _rays;

  // correction factors

  RadxCfactors *_cfactors;

  // fields
  
  vector<RadxField *> _fields;

  // transitions array used in removeTransitionRays()
  // not required in serialization

  vector<bool> _transitionFlags;
  
  // pseudo RHIs
  // not required in serialization

  vector<PseudoRhi *> _pseudoRhis;

  // searching for angle match between sweeps
  // not required in serialization

  static const int _searchAngleN = 36000;
  static const double _searchAngleRes;
  int _searchMaxWidth;
  vector<const RadxRay *> _searchRays;
  
  // private methods
  
  void _init();
  RadxVol & _copy(const RadxVol &rhs);

  void _adjustSweepLimitsPpi();
  void _adjustSweepLimitsRhi();

  RayGeom _getPredomGeom() const;
  RayGeom _getFinestGeom() const;

  double _computeSweepFractionInTransition(int sweepIndex);
  void _constrainBySweepIndex(vector<int> &sweepIndexes);
  void _checkForIndexedRays(const RadxSweep *sweep) const;
  double _computeRoundedAngleRes(double res) const;
  void _findTransitions(int nRaysMargin);
  void _setPredomSweepModeFromAngles() const;
  void _augmentSweepFields(size_t target, size_t source);

  int _loadPseudoFromRealRhis();
  int _setupAngleSearch(size_t sweepNum);
  int _getSearchAngleIndex(double angle);
  double _getSearchAngle(int index);
  void _populateSearchRays(int start, int end);
  void _populateSearchAcross360(int first, int last);

  void _makeFieldsUniform(size_t startIndex, size_t endIndex);

  int _getTransIndex(const RadxSweep *sweep, double azimuth);

  /////////////////////////////////////////////////
  // serialization
  /////////////////////////////////////////////////
  
  static const int _metaStringsPartId = 1;
  static const int _metaNumbersPartId = 2;
  static const int _platformPartId = 3;
  static const int _sweepPartId = 4;
  static const int _sweepAsInFilePartId = 5;
  static const int _cfactorsPartId = 6;
  static const int _rcalibPartId = 7;
  static const int _rayPartId = 8;
  static const int _fieldPartId = 9;
  
  // struct for metadata numbers in messages
  // strings not included - they are passed as XML
  
  typedef struct {
    
    Radx::si64 startTimeSecs;
    Radx::si64 endTimeSecs;
    Radx::si64 startNanoSecs;
    Radx::si64 endNanoSecs;
    
    Radx::fl64 spareFl64[12];
  
    Radx::si32 volNum;
    Radx::si32 scanId;
    Radx::si32 rayTimesIncrease;

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
