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
// RadxField.hh
//
// Field object for Radx data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#ifndef RadxField_HH
#define RadxField_HH

#include <string>
#include <Radx/Radx.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxPacking.hh>
#include <Radx/RadxBuf.hh>
#include <Radx/RadxMsg.hh>
#include <Radx/RadxRemap.hh>
using namespace std;

//////////////////////////////////////////////////////////////////////
/// CLASS FOR STORING FIELD DATA
/// 
/// This class stored the actual data for radar or lidar fields.
///
/// The data may be stored using the following types:
///
/// \code
///   Radx::fl64: 8-byte floating point
///   Radx::fl32: 4-byte floating point
///   Radx::si32: 4-byte signed scaled integer
///   Radx::si16: 2-byte signed scaled integer
///   Radx::si08: 1-byte signed scaled integer
/// \endcode
///
/// The data itself may either be managed locally by this object, or
/// may point into an array owned by a different field object.  The
/// reason for having both options is to facilitate the storage of
/// field data in both RadxRay and RadxVol objects.
///
/// It is useful to first consider the perspective of the RadxVol
/// object.
///
/// RadxVol is designed to hold the data for a radar volume,
/// comprising one or more sweeps. RadxVol contains a vector of
/// RadxField objects.  Each of these RadxField objects holds the
/// data, contiguously, for the entire radar volume. The memory for
/// the data for these fields is managed by the field objects
/// themselves. When the field objects are deleted, so is the data
/// array they hold.
///
/// RadxVol also contains a vector of RadxRay objects, each of which
/// also contains a vector of RadxField objects. Generally, the field
/// objects contained by the ray objects will not manage the field
/// data. Rather, their _data members will point into the arrays held
/// by the field objects contained by the volume.
///
/// The above method of storing the field data is generally used when
/// the data is already available as a complete volume (or sweep) and
/// is read in that manner. The data 'arrives' as a volume and is
/// stored as such.
///
/// Now, let us consider how to deal with data which arrives a ray at
/// at time. We do not yet have a complete volume - we need to build
/// it as we go. For each ray of data which arrives, we create a new
/// RadxRay object, and add it to the RadxVol volume object. The _data
/// members on the field objects point to memory allocated and managed
/// by the field objects themselves. While these ray objects are being
/// created, no actual data is (yet) held by the volume object.
///
/// Now, once all the rays for a volume have been read in, and stored
/// in fields in RadxRay objects, we can then call the
/// RadxVol::loadFieldsFromRays() method to convert the data held
/// by individual ray fields into contiguous fields held by the
/// volume. That method creates the contiguous fields, copies the data
/// from the ray fields into the contiguous fields and then releases
/// the data arrays held by the ray fields, and instead points to the
/// data held by the volume fields. Once that is complete the
/// management of the memory has passed from the fields in the rays to
/// the fields in the volume.

class RadxField : public RadxRangeGeom, public RadxPacking {

public:

  /// Constructor
  
  RadxField(const string &name = "not-set",
            const string &units = "");
  
  /// Copy constructor
  
  RadxField(const RadxField &rhs);

  /// Destructor

  virtual ~RadxField();
  
  /// Assignment
  
  RadxField& operator=(const RadxField &rhs);

  /// copy metadata, but leave the data array empty

  RadxField &copyMetaData(const RadxField &rhs);
  
  /// \name Set methods:
  //@{

  /// Change field name
  
  void setName(const string &val) { _name = val; }

  /// Change units

  void setUnits(const string &val) { _units = val; }

  /// Set long name.
  
  void setLongName(const string &val) { _longName = val; }

  /// Set standard name.

  void setStandardName(const string &val) { _standardName = val; }

  /// Set legend XML
  /// This is intended for 'discrete' fields - i.e. those which
  /// take on a select set of values - e.g. PID. The legend
  /// allows the user to interpret what the field values mean.

  void setLegendXml(const string &val) { _legendXml = val; }

  /// Set thresholding XML
  /// If the field has been thresholded in some manner, this
  /// XML block should be used to give details on how the
  /// thresholding was done.

  void setThresholdingXml(const string &val) { _thresholdingXml = val; }

  /// Set the comment
  /// The comment is used to document specific and non-standard
  /// aspects of a field

  void setComment(const string &val) { _comment = val; }

  /// Set sampling ratio.
  ///
  /// Sometimes the number of samples used to compute different
  /// fields varies. This ratio is the number of samples used
  /// to compute this field divided by the number of samples
  /// specified in the RadxRay object.
  /// By default it is 1.0, which will apply to most cases.

  void setSamplingRatio(double val) { _samplingRatio = val; }

  /// Set folding behavior on.
  /// If the field folding, then when the value exceeds foldLimitUpper,
  /// it wraps to the lower value, and vice versa. This occurs for
  /// radial velocity, and phidp.

  void setFieldFolds(double foldLimitLower, double foldLimitUpper) {
    _fieldFolds = true;
    _foldLimitLower = foldLimitLower;
    _foldLimitUpper = foldLimitUpper;
    _foldRange = _foldLimitUpper - foldLimitLower;
  }

  /// Is this a discrete field? In other words it takes on
  /// discrete values, such as particle ID
  
  void setIsDiscrete(bool val = true) { _isDiscrete = val; }

  //////////////////////////////////////////////////////////////////
  /// Set thresholding field name.
  ///
  /// This is used to indicated if this field has been censored
  /// based on a different field.
  ///
  /// The thresholding operation itself is not handled by this oject,
  /// but must be performed by external code.
  ///
  /// If the threshold field name is empty, no thresholding is active.
  
  void setThresholdFieldName(const string &val) { _thresholdFieldName = val; }

  //////////////////////////////////////////////////////////////////
  /// Set thresholding value.
  ///
  /// See setThresholdFieldName()
  
  void setThresholdValue(double val) { _thresholdValue = val; }

  //@}
  
  /// \name Set missing data value to that specified
  //@{

  /// Set missing value for fl64 data.
  ///
  /// Only changes the missing value, and any points which have
  /// missing values. The remainder of the data is unchanged.

  void setMissingFl64(Radx::fl64 missingVal);
  
  /// Set missing value for fl32 data.
  ///
  /// Only changes the missing value, and any points which have
  /// missing values. The remainder of the data is unchanged.

  void setMissingFl32(Radx::fl32 missingVal);
  
  /// Set missing value for si32 data.
  ///
  /// Only changes the missing value, and any points which have
  /// missing values. The remainder of the data is unchanged.

  void setMissingSi32(Radx::si32 missingVal);
  
  /// Set missing value for si16 data.
  ///
  /// Only changes the missing value, and any points which have
  /// missing values. The remainder of the data is unchanged.

  void setMissingSi16(Radx::si16 missingVal);
  
  /// Set missing value for si08 data.
  ///
  /// Only changes the missing value, and any points which have
  /// missing values. The remainder of the data is unchanged.

  void setMissingSi08(Radx::si08 missingVal);
  
  //@}
  
  /// \name Set data types:
  //@{

  /// Set data type to 64-bit floating point.
  /// Missing data value is also set.
  /// Clears existing data.
  
  void setTypeFl64(Radx::fl64 missingValue);

  /// Set data type to 32-bit floating point.
  /// Missing data value is also set.
  /// Clears existing data.
  
  void setTypeFl32(Radx::fl32 missingValue);
  
  /// Set data type to 32-bit integer.
  /// Missing data value, scale and offset are also set.
  /// Clears existing data.

  void setTypeSi32(Radx::si32 missingValue,
                   double scale,
                   double offset);

  /// Set data type to 16-bit integer.
  /// Missing data value, scale and offset are also set.
  /// Clears existing data.

  void setTypeSi16(Radx::si16 missingValue,
                   double scale,
                   double offset);

  /// Set data type to 08-bit integer.
  /// Missing data value, scale and offset are also set.
  /// Clears existing data.

  void setTypeSi08(Radx::si08 missingValue,
                   double scale,
                   double offset);

  //@}
  
  /// \name Add ray of data
  //@{

  /// Add nGates of 64-bit floating point data to the field
  
  void addDataFl64(size_t nGates, const Radx::fl64 *data);

  /// Add nGates of 32-bit floating point data to the field
  
  void addDataFl32(size_t nGates, const Radx::fl32 *data);

  /// Add nGates of 32-bit integers to the field
  
  void addDataSi32(size_t nGates, const Radx::si32 *data);

  /// Add nGates of 16-bit integers to the field
  
  void addDataSi16(size_t nGates, const Radx::si16 *data);

  /// Add nGates of 08-bit integers to the field
  
  void addDataSi08(size_t nGates, const Radx::si08 *data);

  // Add nGates of missing data
  
  void addDataMissing(size_t nGates);

  /// Set the number of gates.
  ///
  /// If more gates are needed, extend the field data out to a set number of
  /// gates. The data for extra gates are set to missing values.
  ///
  /// If fewer gates are needed, the data is truncated.
  
  void setNGates(size_t nGates);

  /// Set value at a specified gate to missing
  
  void setGateToMissing(size_t gateNum);

  /// Set value at specified gates to missing
  
  void setGatesToMissing(size_t startGate, size_t endGate);

  /// Set values within specified range limits to missing
  
  void setRangeIntervalToMissing(double startRangeKm, double endRangeKm);

  //@}
  
  /// \name Set data on the field:
  //@{

  /// set fl64 data for nGates
  ///
  /// If isLocal is false, the data pointer will be stored
  /// and the data memory is ownded by the calling object.
  ///
  /// If isLocal is true, the data will be copied to the
  /// local buffer.

  void setDataFl64(size_t nGates,
                   const Radx::fl64 *data,
                   bool isLocal);
  
  /// set fl64 data for a vector of rays
  ///
  /// The data is copied to the local buffer, and managed locally.
  
  void setDataFl64(const vector<size_t> &rayNGates,
                   const Radx::fl64 *data);
  
  /// set fl32 data for nGates
  ///
  /// If isLocal is false, the data pointer will be stored
  /// and the data memory is ownded by the calling object.
  ///
  /// If isLocal is true, the data will be copied to the
  /// local buffer.

  void setDataFl32(size_t nGates,
                   const Radx::fl32 *data,
                   bool isLocal);
  
  /// set fl32 data for a vector of rays
  ///
  /// The data is copied to the local buffer, and managed locally.
  
  void setDataFl32(const vector<size_t> &rayNGates,
                   const Radx::fl32 *data);
  
  /// set si32 data for nGates
  ///
  /// If isLocal is false, the data pointer will be stored
  /// and the data memory is ownded by the calling object.
  ///
  /// If isLocal is true, the data will be copied to the
  /// local buffer.

  void setDataSi32(size_t nGates,
                   const Radx::si32 *data,
                   bool isLocal);
  
  /// set si32 data for a vector of rays
  ///
  /// The data is copied to the local buffer, and managed locally.
  
  void setDataSi32(const vector<size_t> &rayNGates,
                   const Radx::si32 *data);
  
  /// set si16 data for nGates
  ///
  /// If isLocal is false, the data pointer will be stored
  /// and the data memory is ownded by the calling object.
  ///
  /// If isLocal is true, the data will be copied to the
  /// local buffer.

  void setDataSi16(size_t nGates,
                   const Radx::si16 *data,
                   bool isLocal);
  
  /// set si16 data for a vector of rays
  ///
  /// The data is copied to the local buffer, and managed locally.
  
  void setDataSi16(const vector<size_t> &rayNGates,
                   const Radx::si16 *data);
  
  /// set si08 data for nGates
  ///
  /// If isLocal is false, the data pointer will be stored
  /// and the data memory is ownded by the calling object.
  ///
  /// If isLocal is true, the data will be copied to the
  /// local buffer.

  void setDataSi08(size_t nGates,
                   const Radx::si08 *data,
                   bool isLocal);
  
  /// set si08 data for a vector of rays
  ///
  /// The data is copied to the local buffer, and managed locally.
  
  void setDataSi08(const vector<size_t> &rayNGates,
                   const Radx::si08 *data);

  //@}
  
  /// \name Convert data type:
  //@{

  /////////////////////////////////////////////
  // data type convertions
  
  /// Convert field data to 64-bit floating point.
  
  void convertToFl64();

  /// Convert field data to 32-bit floating point.
  
  void convertToFl32();

  /// Convert field data to 32-bit scaled signed integers,
  /// specifying the scale and offset.

  void convertToSi32(double scale, double offset);

  /// Convert field data to 32-bit scaled signed integers,
  /// dynamically computing scale and offset.

  void convertToSi32();

  /// Convert field data to 16-bit scaled signed integers,
  /// specifying the scale and offset.

  void convertToSi16(double scale, double offset);

  /// Convert field data to 16-bit scaled signed integers,
  /// dynamically computing scale and offset.

  void convertToSi16();

  /// Convert field data to 8-bit scaled signed integers,
  /// specifying the scale and offset.

  void convertToSi08(double scale, double offset);
  
  /// Convert field data to 8-bit scaled signed integers,
  /// dynamically computing scale and offset.

  void convertToSi08();

  /// convert to specified type
  /// If the data type is an integer type, dynamic scaling
  /// is used - i.e. the min and max value is computed and
  /// the scale and offset are set to values which maximize the
  /// dynamic range.
  /// If targetType is Radx::ASIS, no conversion is performed.
  
  void convertToType(Radx::DataType_t targetType);
  
  /// convert to specified type
  /// For integer types, the specified scale and offset are
  /// used.
  /// If targetType is Radx::ASIS, no conversion is performed.

  void convertToType(Radx::DataType_t targetType,
                     double scale,
                     double offset);
  
  /// Converts field type, and optionally changes the
  /// names.
  ///
  /// If the data type is an integer type, dynamic scaling
  /// is used - i.e. the min and max value is computed and
  /// the scale and offset are set to values which maximize the
  /// dynamic range.
  ///
  /// If dataType is Radx::ASIS, no type conversion is performed.
  ///
  /// If a string argument has zero length, the value on the
  /// field will be left unchanged.
  
  void convert(Radx::DataType_t dtype,
               const string &name,
               const string &units,
               const string &standardName,
               const string &longName);

  /// Converts field type, and optionally changes the
  /// names.
  ///
  /// If the data type is an integer type, the specified
  /// scale and offset are used.
  ///
  /// If dataType is Radx::ASIS, no type conversion is performed.
  ///
  /// If a string argument has zero length, the value on the
  /// field will be left unchanged.
  
  void convert(Radx::DataType_t dtype,
               double scale,
               double offset,
               const string &name,
               const string &units,
               const string &standardName,
               const string &longName);
  
  //@}
  
  /// \name Data management:
  //@{

  /// Clear the data array in the object.
  
  void clearData();
  
  /// Set the object so that the data is locally managed.
  ///
  /// If the data points to a remote object, allocate 
  /// a local data array and copy the remote data into it.
  
  void setDataLocal();

  /// Set data on the object to point to data managed by a different
  /// field object.
  ///
  /// Therefore the data is not managed by this object.
  
  void setDataRemote(const RadxField &other,
                     const void *data,
                     size_t nGates);

  /// Determine memory management.
  /// If dataIsLocal() is true, data is owned by this object.
  /// If dataIsLocal() is false, data points to memory in another object.

  bool dataIsLocal() const { return _dataIsLocal; }
  
  //@}
  
  /// \name Remapping:
  //@{

  /////////////////////////////////////////////////
  /// Remap data for a single ray onto new range
  /// geometry using lookup table passed in.
  ///
  /// If interp is true, use interpolation if appropriate.
  /// Otherwise use nearest neighbor.
  
  void remapRayGeom(const RadxRemap &remap,
                    bool interp = false);
  
  ///////////////////////////////////////////////////////////////////
  /// Remap the data on this field, using fewer rays.
  ///
  /// Only the data between minRayIndex and maxRayIndex, inclusive,
  /// is preserved. All other data is removed.

  virtual void remapRays(int minRayIndex,
                         int maxRayIndex);

  //@}
  
  /// \name Utility methods:
  //@{

  /// Compute min and max data values in the field.
  ///
  /// use getMinValue() and getMaxValue() to retrieve the results.
  ///
  /// Returns -1 if no valid data found, 0 otherwise

  int computeMinAndMax() const;

  /// Apply a linear transformation to the data values.
  /// Transforms x to y as follows:
  ///   y = x * scale + offset
  /// After operation, leaves type unchanged.

  void applyLinearTransform(double scale, double offset);

  /// Transorm from db to linear units
  /// Note - will convert to fl32

  void transformDbToLinear();

  /// Transorm from linear to db units
  /// Note - will convert to fl32
  
  void transformLinearToDb();

  /// Check if the data at all gates is missing?
  /// Returns true if all missing, false otherwise.
  
  bool checkDataAllMissing() const;
  
  /// Compute the number of gates without missing data.
  ///
  /// i.e. all gates beyond this number have missing data.
  
  int computeNGatesNonMissing(size_t rayNum) const;
  
  //@}
  
  /// \name Get methods:
  //@{

  /// Get field name.
  
  const string &getName() const { return _name; }

  /// Get long name for field.
  
  const string &getLongName() const { return _longName; }

  /// Get standard name for field.
  
  const string &getStandardName() const { return _standardName; }

  /// Get units for field.
  
  const string &getUnits() const { return _units; }
  
  /// Get the legend XML.
  /// This is intended for 'discrete' fields - i.e. those which
  /// take on a select set of values - e.g. PID. The legend
  /// allows the user to interpret what the field values mean.

  const string &getLegendXml() const { return _legendXml; }

  /// Get thresholding XML
  /// If the field has been thresholded in some manner, this
  /// XML block is used to give details on how the
  /// thresholding was done.

  const string &getThresholdingXml() const { return _thresholdingXml; }

  /// Get comment
  /// The comment is used to document specific and non-standard
  /// aspects of a field

  const string &getComment() const { return _comment; }

  /// Get number of rays represented in field data.
  
  size_t getNRays() const { return _rayStartIndex.size(); }

  /// Get number of points in field
  
  size_t getNPoints() const { return _nPoints; }
  
  /// Get number of bytes in field = (nPoints * byteWidth).
  
  inline size_t getNBytes() const { return _nPoints * _byteWidth; }

  /// Get the data type used to store data in this field.

  Radx::DataType_t getDataType() const { return _dataType; }

  /// Get the byte width of the data type used in this field.

  int getByteWidth() const { return _byteWidth; }

  /// Get scale - used for scaled integers.
  /// \code
  ///   float_value = (integer_value * scale) + offset.
  /// \endcode

  double getScale() const { return _scale; }

  /// Get offset - used for scaled integers.
  /// \code
  ///   float_value = (integer_value * scale) + offset.
  /// \endcode
  
  double getOffset() const { return _offset; }

  /// Get minumum value - see computeMinAndMax().

  double getMinValue() const { return _minVal; }

  /// Get maximum value - see computeMinAndMax().

  double getMaxValue() const { return _maxVal; }

  /// Get sampling ratio.
  ///
  /// Sometimes the number of samples used to compute different
  /// fields varies. This ratio is the number of samples used
  /// to compute this field divided by the number of samples
  /// specified in the RadxRay object.
  /// By default it is 1.0, which will apply to most cases.

  double getSamplingRatio() const { return _samplingRatio; }

  /// Get folding behavior on.
  /// If the field folding, then when the value exceeds foldLimitUpper,
  /// it wraps to the lower value, and vice versa. This occurs for
  /// radial velocity, and phidp.

  bool getFieldFolds() const { return _fieldFolds; }
  double getFoldLimitLower() const { return _foldLimitLower; }
  double getFoldLimitUpper() const { return _foldLimitUpper; }
  double getFoldRange() const { return _foldRange; }
  
  /// Is this a discrete field? In other words it takes on
  /// discrete values, such as particle ID
  
  bool getIsDiscrete() const { return _isDiscrete; }

  /// Get missing value for 64-bit floating point data.

  Radx::fl64 getMissingFl64() const { return _missingFl64; }

  /// Get missing value for 32-bit floating point data.

  Radx::fl32 getMissingFl32() const { return _missingFl32; }

  /// Get missing value for 32-bit scaled integer data.
  ///
  /// The test for missing data is performed on the data as it is
  /// stored, i.e. BEFORE applying the scale and offset.

  Radx::si32 getMissingSi32() const { return _missingSi32; }

  /// Get missing value for 16-bit scaled integer data.
  ///
  /// The test for missing data is performed on the data as it is
  /// stored, i.e. BEFORE applying the scale and offset.

  Radx::si16 getMissingSi16() const { return _missingSi16; }

  /// Get missing value for 8-bit scaled integer data.
  ///
  /// The test for missing data is performed on the data as it is
  /// stored, i.e. BEFORE applying the scale and offset.

  Radx::si08 getMissingSi08() const { return _missingSi08; }

  /// Get missing data value, for stored data type,
  /// converted to a double.

  double getMissing() const;

  /// Get thresholding field name,
  /// if thresholding using another field is active.
  /// If name is empty, no thresholding is active.
  
  const string &getThresholdFieldName() const { return _thresholdFieldName; }

  /// Get thresholding value,
  /// if thresholding using another field is active.
  /// See getThresholdFieldName().
  
  double getThresholdValue() const { return _thresholdValue; }

  /////////////////////////////////////////////////////////////////
  /// get data pointers

  /// Get data in data array - generic, returned as a void*
  
  const void *getData() const { return _data; }
  
  /// Get pointer to data for specified ray
  /// Also sets the number of gates

  const void *getData(size_t rayNum, size_t &nGates) const;
  void *getData(size_t rayNum, size_t &nGates);

  /// Get pointer to 64-bit floating point data.
  /// Note - this assumes data is stored in this type.
  /// An assert will check this assumption, and exit if false.
  
  const Radx::fl64 *getDataFl64() const;
  Radx::fl64 *getDataFl64();

  /// Get pointer to 32-bit floating point data.
  /// Note - this assumes data is stored in this type.
  /// An assert will check this assumption, and exit if false.
  
  const Radx::fl32 *getDataFl32() const;
  Radx::fl32 *getDataFl32();

  /// Get pointer to 32-bit scaled integer data.
  /// Note - this assumes data is stored in this type.
  /// An assert will check this assumption, and exit if false.
  
  const Radx::si32 *getDataSi32() const;
  Radx::si32 *getDataSi32();

  /// Get pointer to 16-bit scaled integer data.
  /// Note - this assumes data is stored in this type.
  /// An assert will check this assumption, and exit if false.
  
  const Radx::si16 *getDataSi16() const;
  Radx::si16 *getDataSi16();

  /// Get pointer to 8-bit scaled integer data.
  /// Note - this assumes data is stored in this type.
  /// An assert will check this assumption, and exit if false.
  
  const Radx::si08 *getDataSi08() const;
  Radx::si08 *getDataSi08();

  //////////////////////////////////////////////////////
  /// Get the value at a gate, as a double.
  /// The scale and offset is applied as applicable before
  /// the value is returned.
  
  inline double getDoubleValue(size_t ipt) const {
    
    // check indices for validity
    
    if (ipt >= _nPoints) {
      return _missingFl64;
    }
    
    switch (_dataType) {
      case Radx::FL64: {
        double data = *(((double *) _data) + ipt);
        return data;
      }
      case Radx::SI32: {
        Radx::si32 data = *(((Radx::si32 *) _data) + ipt);
        if (data == _missingSi32) {
          return _missingFl64;
        }
        return (double) data * _scale + _offset;
      }
      case Radx::SI16: {
        Radx::si16 data = *(((Radx::si16 *) _data) + ipt);
        if (data == _missingSi16) {
          return _missingFl64;
        }
        return (double) data * _scale + _offset;
      }
      case Radx::SI08: {
        Radx::si08 data = *(((Radx::si08 *) _data) + ipt);
        if (data == _missingSi08) {
          return _missingFl64;
        }
        return (double) data * _scale + _offset;
      }
      case Radx::FL32: 
      default: {
        Radx::fl32 data = *(((Radx::fl32 *) _data) + ipt);
        if (data == _missingFl32) {
          return _missingFl64;
        }
        return data;
      }
    } // switch

  }

  //////////////////////////////////////////////////////
  /// Get the stored value at a gate, as a double.
  /// No scale and offset is applied.
  
  inline double getStoredValue(size_t ipt) const {
    
    // check indices for validity
    
    if (ipt >= _nPoints) {
      return _missingFl64;
    }
    
    switch (_dataType) {
      case Radx::FL64: {
        double data = *(((double *) _data) + ipt);
        return data;
      }
      case Radx::SI32: {
        Radx::si32 data = *(((Radx::si32 *) _data) + ipt);
        if (data == _missingSi32) {
          return _missingFl64;
        }
        return (double) data;
      }
      case Radx::SI16: {
        Radx::si16 data = *(((Radx::si16 *) _data) + ipt);
        if (data == _missingSi16) {
          return _missingFl64;
        }
        return (double) data;
      }
      case Radx::SI08: {
        Radx::si08 data = *(((Radx::si08 *) _data) + ipt);
        if (data == _missingSi08) {
          return _missingFl64;
        }
        return (double) data;
      }
      case Radx::FL32: 
      default: {
        Radx::fl32 data = *(((Radx::fl32 *) _data) + ipt);
        if (data == _missingFl32) {
          return _missingFl64;
        }
        return data;
      }
    } // switch

  }

  //@}
  
  /// \name Computing statistics
  //@{

  /// method for computing stats over number of fields
  /// generally used for stats over a number of rays

  typedef enum {
    
    STATS_METHOD_MEAN = 0, /**< Computing the arithmetic mean of a series */
    STATS_METHOD_MEDIAN = 1, /**< Computing the median of a series */
    STATS_METHOD_MAXIMUM = 2, /**< Computing the maximum of a series */
    STATS_METHOD_MINIMUM = 3, /**< Computing the minumum of a series */
    STATS_METHOD_MIDDLE = 4, /**< Using the middle entry in a series */
    STATS_METHOD_LAST = 5

  } StatsMethod_t;

  /// compute stats from a series of fields
  ///
  /// Pass in a method type, and a vector of fields
  ///
  /// Compute the requested stats on those fields, on a point-by-point basis.
  /// Create a field, fill it with the results, and return it.
  ///
  /// If the number of points in the field is not constant, use the minumum number
  /// of points in the supplied fields.
  ///
  /// maxFractionMissing indicates the maximum fraction of the input data field
  /// that can be missing for valid statistics. Should be between 0 and 1.
  ///
  /// Returns NULL if fieldIn.size() == 0.
  /// Otherwise, returns field containing results.
  
  RadxField *computeStats(RadxField::StatsMethod_t method,
                          const vector<const RadxField *> &fieldsIn,
                          double maxFractionMissing = 0.25);

  /// convert enums to strings

  static string statsMethodToStr(StatsMethod_t method);

  //@}

  /// \name Printing:
  //@{

  /// Print metadata.
  
  void print(ostream &out) const;

  /// Print metadata and data values.
  
  void printWithData(ostream &out) const;

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

  // private data

  string _name;
  string _longName;
  string _standardName;
  string _units;
  string _legendXml;
  string _thresholdingXml;
  string _comment;
  
  Radx::DataType_t _dataType;
  int _byteWidth;

  double _scale;  // for si08, si16 and si32
  double _offset; // for si08, si16 and si32
  
  // Sampling ratio:
  // Some fields may be computed using a different number of samples
  // from others. This ratio is the number of samples used
  // to compute this field divided by the number of samples
  // stored on the RadxRay object.
  // By default it is 1.0, which will apply to most cases.

  double _samplingRatio;

  // folding behavior
  // if the field folding, then when the value exceeds foldLimitUpper,
  // it wraps to the lower value, and vice versa. This occurs for
  // radial velocity, and phidp

  bool _fieldFolds;
  double _foldLimitLower;
  double _foldLimitUpper;
  double _foldRange;

  // Is this a discrete field? In other words it takes on
  // discrete values, such as particle ID
  
  bool _isDiscrete;

  // max and min values in field

  mutable double _minVal;
  mutable double _maxVal;

  // actual missing value in use, by type

  Radx::fl64 _missingFl64;
  Radx::fl32 _missingFl32;
  Radx::si32 _missingSi32;
  Radx::si16 _missingSi16;
  Radx::si08 _missingSi08;

  // data array buffer - all types are stored here

  RadxBuf _buf;
  const void *_data;
  bool _dataIsLocal;   /* If true, _data is _buf.getPtr().
                        * If false, _data points to an array owned
                        * by another object */
  
  // thresholding on another field

  string _thresholdFieldName;
  double _thresholdValue;
  
  // private methods
  
  void _init();
  RadxField & _copy(const RadxField &rhs);
  void _setMissingToDefaults();
  void _remapDataNearest(const RadxRemap &remap);
  void _remapDataInterp(const RadxRemap &remap);
  void _printPacked(ostream &out, int count, double val) const;
  void _printTypeMismatch(const string &methodName,
                          Radx::DataType_t dtype) const;

  double _interpFolded(double val0, double val1,
                       double wt0, double wt1);
  double _getFoldAngle(double val);
  double _getFoldValue(double angle);
  static double _getFoldAngle(double val, double foldLimitLower, double foldRange);
  static double _getFoldValue(double angle, double foldLimitLower, double foldRange);

  void _computeMean(size_t nPoints,
                    const vector<const RadxField *> &fieldsIn,
                    Radx::fl64 *data,
                    double maxFractionMissing);
  
  void _computeMeanFolded(size_t nPoints,
                          double foldLimitLower,
                          double foldRange,
                          const vector<const RadxField *> &fieldsIn,
                          Radx::fl64 *data,
                          double maxFractionMissing);
  
  void _computeMedian(size_t nPoints,
                      const vector<const RadxField *> &fieldsIn,
                      Radx::fl64 *data,
                      double maxFractionMissing);

  void _computeMaximum(size_t nPoints,
                       const vector<const RadxField *> &fieldsIn,
                       Radx::fl64 *data,
                       double maxFractionMissing);

  void _computeMinimum(size_t nPoints,
                       const vector<const RadxField *> &fieldsIn,
                       Radx::fl64 *data,
                       double maxFractionMissing);

  void _computeMiddle(size_t nPoints,
                      const vector<const RadxField *> &fieldsIn,
                      Radx::fl64 *data,
                      double maxFractionMissing);

  int _computeMinValid(int nn,
                       double maxFractionMissing);
  
  /////////////////////////////////////////////////
  // serialization
  /////////////////////////////////////////////////

  static const int _metaStringsPartId = 1;
  static const int _metaNumbersPartId = 2;
  static const int _dataPartId = 3;
  
  // struct for metadata numbers in messages
  // strings not included - they are passed as XML
  
  typedef struct {

    Radx::fl64 startRangeKm;
    Radx::fl64 gateSpacingKm;
    Radx::fl64 scale;
    Radx::fl64 offset;
    Radx::fl64 samplingRatio;
    Radx::fl64 foldLimitLower;
    Radx::fl64 foldLimitUpper;
    Radx::fl64 foldRange;
    Radx::fl64 minVal;
    Radx::fl64 maxVal;
    Radx::fl64 missingFl64;
    Radx::fl64 thresholdValue;
    Radx::fl64 spareFl64[4];
  
    Radx::fl32 missingFl32;
    Radx::fl32 spareFl32[1];
    
    Radx::si32 rangeGeomSet;
    Radx::si32 nGates;
    Radx::si32 dataType;
    Radx::si32 byteWidth;
    Radx::si32 fieldFolds;
    Radx::si32 isDiscrete;
    Radx::si32 missingSi32;
    Radx::si32 missingSi16;
    Radx::si32 missingSi08;
    Radx::si32 spareSi32[5];

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

