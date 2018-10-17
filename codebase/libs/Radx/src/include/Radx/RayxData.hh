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
/**
 * @file RayxData.hh 
 * @brief RayxData is one ray typically from radar data, one field
 * @class RayxData
 * @brief RayxData is one ray typically from radar data, one field
 */


#ifndef RayxData_H
#define RayxData_H

#include <Radx/Radx.hh>
#include <Radx/RadxFuzzyF.hh>
#include <Radx/RadxFuzzy2d.hh>
#include <string>
#include <vector>

class RadxField;

class RayxData
{
  
public:

  /**
   * An empty ray constructor
   */
  RayxData (void);
  
  /**
   * A ray with specific data, but no values yet
   *
   * @param[in] name  field name
   * @param[in] units data units
   * @param[in] npt   Number of points in the ray
   * @param[in] missing  Missing data value
   * @param[in] az    Azimuth (degrees)
   * @param[in] elev  Elevation (degrees)
   * @param[in] gate_spacing  Km between gates
   * @param[in] start_range  Km to first gate
   *
   * Initializes to missing data value along the ray
   */
  RayxData (const std::string &name, const std::string &units,
            const int npt, const double missing,
            const double az, const double elev,
            const double gate_spacing, const double start_range);

  /**
   * A ray with specific data, and values from a RadxField object
   *
   * @param[in] name  field name
   * @param[in] units data units
   * @param[in] npt   Number of points in the ray
   * @param[in] missing  Missing data value
   * @param[in] az    Azimuth (degrees)
   * @param[in] elev  Elevation (degrees)
   * @param[in] gate_spacing  Km between gates
   * @param[in] start_range  Km to first gate
   * @param[in] data  Data array contained within
   *
   */
  RayxData(const std::string &name, const std::string &units,
           const int npt, const double missing, const double az,
           const double elev, const double gate_spacing, 
           const double start_range, const RadxField &data);

  /**
   * Destructor
   */
  virtual ~RayxData(void);
  
  /**
   * Copy data contents from input into local, including missing data value
   */
  bool transferData(const RayxData &r);
  

  /**
   * Change name to input value
   * @param[in] name  Name to change to
   */
  inline void changeName(const string &name) {_name = name;}

  /**
   * Change units to input value
   * @param[in] units  Name to change to
   */
  inline void changeUnits(const std::string &units) { _units = units;}

  /**
   * Change missing data value
   * @param[in] missing New value
   *
   * Changes all data that was missing to the new missing data value
   */
  void changeMissing(const double missing);


  /**
   * Retrieve the data values into an array
   * @param[in] data The array to fill
   * @param[in] npt  The length of the array
   *
   * @return false if npt not equal to local _npt
   */
  bool retrieveData(Radx::fl32 *data, const int npt) const;

  /**
   * Retrieve the data values into an array, where the array can be bigger
   * than the local ray.
   * @param[in] data The array to fill
   * @param[in] npt  The length of the array passed in
   *
   * If npt is larger than local ray, the data array is padded with missing
   *
   * @return false if npt is smaller than local ray
   */
  bool retrieveSubsetData(Radx::fl32 *data, const int npt) const;

  /**
   * Store the local data values from an array into local state
   * @param[in] data The array to use
   * @param[in] npt  The length of the array
   *
   * @note if npt not equal to local _npt, either not all data is stored,
   *       or the surplus positions are set to missing
   */
  void storeData(const Radx::fl32 *data, const int npt);

  /**
   * Set the data value at an index
   * @param[in] index  The index into the data
   * @param[in] value  The data value to set
   */
  void setV(const int index, const double value);

  /**
   * Get data value at an index
   * @param[in] index  The index into the data
   * @param[out] value  The data value to get
   *
   * @return true if data was set into value and is not the missing value
   */
  bool getV(const int index, double &value) const;

  /**
   * Multiply the data at each point by a fixed value
   * @param[in] v  The value
   */
  void multiply(const double v);

  /**
   * Multiply the data at each point by the value from another ray
   * @param[in] inp  The other ray
   * @param[in] missing_ok  True to pass through one value if the other is missing
   */
  void multiply(const RayxData &inp, const bool missing_ok);

  /**
   * Modify the data at each point to be the min of that value and the
   * value from another ray
   * @param[in] inp  The other ray
   */
  void min(const RayxData &inp);

  /**
   * Modify the data at each point to be the max of that value and the
   * value from another ray
   * @param[in] inp  The other ray
   */
  void max(const RayxData &inp);

  /**
   * Divide the data at each point by a value from another RayxData 
   * 
   * @param[in] w  The values to divide by
   */
  void divide(const RayxData &w);

  /**
   * Divide the data at each point by a constant value
   * 
   * @param[in] w  The value to divide by
   */
  void divide(const double w);

  /**
   * Add data from an input ray point by point to local RayxData
   * @param[in] i  Data values to add
   * @param[in] missing_ok  True to pass through one value if the other is missing
   */
  void inc(const RayxData &i, const bool missing_ok);

  /**
   * Increment each data value by one fixed value
   * @param[in] v  The fixed value
   */
  void inc(const double v);

  /**
   * Subtract input data from local ray point by point to local RayxData
   * @param[in] i  Data values to add
   * @param[in] missing_ok  True to pass through one value if the other is missing
   */
  void dec(const RayxData &i, const bool missing_ok);

  /**
   * Decrement each data value by one fixed value
   * @param[in] v  The fixed value
   */
  void dec(const double v);

  /**
   * at each point replace with absolute value
   * @param[in] v  The fixed value
   */
  void abs(void);

  /**
   * Increment one data value by one fixed value
   * @param[in] i  index at which to increment
   * @param[in] v  The fixed value
   */
  void incAtPoint(const int i, const double v);

  /**
   * Subtract data from an input ray point by point to local RayxData
   * @param[in] i  Data values to subtract
   */
  void subtract(const RayxData &i);

  /**
   * At each point linearly remap the data
   * @param[in] scale 
   * @param[in] offset
   *
   * Result = data*scale + offset
   */
  void remap(const double scale, const double offset);

  /**
   * @return data at i minus data from other at i.
   * @param[in] other  The Other ray
   * @param[in] i  Index
   */
  double differenceAtPoint(const RayxData &other, const int i) const;

  /**
   * Set the data values at each point to one fixed value
   * @param[in] v  The fixed value
   */
  void setAllToValue(const double v);

  /**
   * @return the name 
   */
  inline const std::string &getName() const {return _name;}

  /**
   * @return true if name matches input
   * @param[in] n
   */
  inline bool namesMatch(const std::string &n) const {return _name == n;}

  /**
   * @return the units
   */
  inline const std::string &getUnits() const {return _units;}

  /**
   * @return the number of points
   */
  inline int getNpoints() const {return _npt;}

  /**
   * @return the missing data value
   */
  inline double getMissing() const {return _missing;}

  /**
   * @return azimuth degrees
   */
  inline double getAzimuth() const {return _az;}

  /**
   * @return true if input azimuth and elevation match local values
   * @param[in] az
   * @param[in] elev
   */
  inline bool match(const double az, const double elev) const
  {
    return az == _az && _elev == elev;
  }

  /**
   * @return true if input beam location and resolution are constant
   * @param[in] x0  
   * @param[in] dx
   */
  bool matchBeam(const double x0, const double dx) const;

  /**
   * Mask the local data into two values, those that are 1 or two point
   * outliers, and those that are not.
   * @param[in] outlierThresh  Difference value that defines an outlier
   * @param[in] maskOutputValue  The value to set at points where outliers are
   * @param[in] nonMaskOutputValue The value to set at points where not outlier.
   * @param[in] up  True if outliers are larger than surroundings,
   *                false if outliers can be either larger or smaller.
   *
   * The nth bin is declared to be a speckle point if its value 
   * is different than both its second nearest neighbors by a threshold
   * value outlierThresh. 
   *
   * If up=true, the bin must exceed its second nearest neighbor values,
   * in other words:
   * 
   * if   _data[n] > outlierThresh + _data[n-2] 
   * and  _data[n] > outlierThresh + _data[n+2] 
   *
   * If up=false, the bin can either be larger or smaller than its second
   * nearest neighbor values, in other words:
   * 
   * if   _data[n] > outlierThresh + _data[n-2] 
   * and  _data[n] > outlierThresh + _data[n+2] 
   * or
   * if   _data[n] < outlierThresh + _data[n-2] 
   * and  _data[n] < outlierThresh + _data[n+2] 
   *
   * The algorithm detects runs of one and two point outliers. If up=false,
   * runs of two point outliers must have the same sign, i.e. be either both
   * bigger or both smaller than the surrounding points
   * 
   */
  void speckleMask(const double outlierThresh, const double maskOutputValue,
		   const double nonMaskOutputValue, 
		   const bool up);

  /**
   * Mask the local data into two values, those that are within tolerance
   * of a value, and those that aren't, with two output mask values
   * @param[in] maskValue  The value to look for
   * @param[in] tolerance  The allowed diff between maskValue and local values
   * @param[in] maskOutputValue  The value to set at points where within
   *                             tolerance of maskValue
   * @param[in] nonMaskOutputValue The value to set at points where not within
   *                             tolerance of maskValue
   */
  void maskWhenEqual(const double maskValue, const double tolerance,
		     const double maskOutputValue,
		     const double nonMaskOutputValue);

  /**
   * Mask local data when values are < threshold
   * @param[in] threshold
   * @param[in] replacement value to replace with
   * @param[in] replaceWithMissing true to replace with the misssing value
   */
  void maskWhenLessThan(double threshold, double replacement, 
			bool replaceWithMissing);

  /**
   * Mask local data when values are <= threshold
   * @param[in] threshold
   * @param[in] replacement value to replace with
   * @param[in] replaceWithMissing true to replace with the misssing value
   */
  void maskWhenLessThanOrEqual(double threshold, double replacement, 
			       bool replaceWithMissing);

  /**
   * Mask local data when values are > threshold
   * @param[in] threshold
   * @param[in] replacement value to replace with
   * @param[in] replaceWithMissing true to replace with the misssing value
   */
  void maskWhenGreaterThan(double threshold, double replacement, 
			   bool replaceWithMissing);

  /**
   * Mask local data when values are >= threshold
   * @param[in] threshold
   * @param[in] replacement value to replace with
   * @param[in] replaceWithMissing true to replace with the misssing value
   */
  void maskWhenGreaterThanOrEqual(double threshold, double replacement, 
				  bool replaceWithMissing);

  /**
   * Mask local data when values are missing
   * @param[in] replacement value to replace with when missing
   */
  void maskWhenMissing(double replacement);


  /**
   * Mask the local data where an input mask does not have a value.
   *
   * @param[in] mask       The data to use as  masking
   * @param[in] maskValue  The value to look for
   * @param[in] nonMaskOutputValue The value to set at points where mask not 
   *                               equal to maskValue
   */
  void maskRestrict(const RayxData &mask, const double maskValue,
		    const double nonMaskOutputValue);

  /**
   * Mask the local data where an input mask does not have a value,
   * setting data there to the missing value.
   *
   * @param[in] mask       The data to use as  masking
   * @param[in] maskValue  The value to look for
   */
  void maskToMissing(const RayxData &mask, const double maskValue);

  /**
   * Where mask data = maskvalue, set data to datavalue
   * @param[in] mask  
   * @param[in] maskValue  
   * @param[in] dataValue
   */
  void maskFilter(const RayxData &mask, double maskValue, double dataValue);

  /**
   * Where mask data < maskvalue, set data to datavalue
   * @param[in] mask  
   * @param[in] maskValue  
   * @param[in] dataValue
   */
  void maskFilterLessThan(const RayxData &mask, double maskValue,
			  double dataValue);

  /**
   * Where mask data = missing, set data to datavalue
   * @param[in] mask  
   * @param[in] dataValue
   */
  void maskMissingFilter(const RayxData &mask, double dataValue);

  /**
   * Modify data whenever mask data < threshold, set to dataValue or missing
   * @param[in] mask  The mask data
   * @param[in] thresh  The mask threshold
   * @param[in] dataValue  replacement data value
   * @param[in] replaceWithMissing True to replace with data missing value
   *                               when condition is satified
   */
  void modifyWhenMaskLessThan(const RayxData &mask, double thresh,
			      double dataValue, bool replaceWithMissing);

  /**
   * Modify data whenever mask data <= threshold, set to dataValue or missing
   * @param[in] mask  The mask data
   * @param[in] thresh  The mask threshold
   * @param[in] dataValue  replacement data value
   * @param[in] replaceWithMissing True to replace with data missing value
   *                               when condition is satified
   */
  void modifyWhenMaskLessThanOrEqual(const RayxData &mask, double thresh,
				     double dataValue,
				     bool replaceWithMissing);

  /**
   * Modify data whenever mask data > threshold, set to dataValue or missing
   * @param[in] mask  The mask data
   * @param[in] thresh  The mask threshold
   * @param[in] dataValue  replacement data value
   * @param[in] replaceWithMissing True to replace with data missing value
   *                               when condition is satified
   */
  void modifyWhenMaskGreaterThan(const RayxData &mask, double thresh,
				 double dataValue,
				 bool replaceWithMissing);

  /**
   * Modify data whenever mask data >= threshold, set to dataValue or missing
   * @param[in] mask  The mask data
   * @param[in] thresh  The mask threshold
   * @param[in] dataValue  replacement data value
   * @param[in] replaceWithMissing True to replace with data missing value
   *                               when condition is satified
   */
  void modifyWhenMaskGreaterThanOrEqual(const RayxData &mask, double thresh,
					double dataValue,
					bool replaceWithMissing);

  /**
   * Modify data whenever mask data is missing, set to dataValue or missing
   * @param[in] mask  The mask data
   * @param[in] dataValue  replacement data value
   * @param[in] replaceWithMissing True to replace with data missing value
   *                               when condition is satified
   */
  void modifyWhenMaskMissing(const RayxData &mask, double dataValue,
			     bool replaceWithMissing);

  /**
   * Apply a fuzzy function to remap the data at each point
   * @param[in] f  Fuzzy function
   *
   * At each point i  d[i] = f(d[i])
   */
  void fuzzyRemap(const RadxFuzzyF &f);

  /**
   * Apply a 2 dimensional fuzzy function to remap the data at each point
   * The local data is 'x', the input data is 'y' in the fuzzy remap
   *
   * @param[in] f  Fuzzy function of two variables
   * @param[in] y  Second dataset in the 2nd dimension
   *
   * At each point i  x[i] = f(x[i], y[i])
   */
  void fuzzy2dRemap(const RadxFuzzy2d &f, const RayxData &y);

  /**
   * Convert from db to linear units
   */
  void db2linear(void);

  /**
   * Convert from linear to db units
   */
  void linear2db(void);

  /**
   * Remap input ray using a gaussian function of two variables, with the other
   * variable stored in input ray y.
   *
   * At each ray point set output value to:
   *  1.0 - exp(-scale*(x*xfactor + y*yfactor)), where x and y may or may not be absolute
   * values of input based on the the absX and absY settings
   *
   * @param[in] y  The second variable
   * @param[in] xfactor  For the equation
   * @param[in] yfactor  For the equation
   * @param[in] absX  If true absolute value of x goes into the equation
   * @param[in] absY  If true absolute value of y goes into the equation
   * @param[in] scale For the equation
   */
  void gaussian2dRemap(const RayxData &y, const double xfactor, const double yfactor,
		       const bool absX, const bool absY, const double scale);

  /**
   * At each ray point, if value = V, set output value to:
   *   exp(-scale*(V/topv - lowv/topv)^2).
   * If invert = true set the output value to
   *   1 - exp(-scale*(V/topv - lowv/topv)^2).
   *
   * @param[in] scale For the equation
   * @param[in] topv
   * @param[in] lowv
   * @param[in] invert  True to subtract result from 1
   */
  void qscale(double scale, double topv, double lowv, bool invert);

  /**
   * At each ray point, if value = V, set output value to:
   *   exp(-scale*(V))
   * If invert = true set the output value to
   *   1 - exp(-scale*(V))
   *
   * @param[in] scale For the equation
   * @param[in] invert  True to subtract result from 1
   */
  void qscale1(double scale, bool invert);

  /**
   * Set data to 1.0/data
   */
  void invert(void);

  /**
   * Set data to sqrt
   */
  void squareRoot(void);

  /**
   * Set data to log10 of data
   */
  void logBase10(void);

  /**
   * FIR filter edge extension techniques
   */
  typedef enum
  {
    FIR_EDGE_CLOSEST,
    FIR_EDGE_MIRROR,
    FIR_EDGE_MEAN,
    FIR_EDGE_INTERP
  } FirFilter_t;

  /**
   * Apply FIR (finite impulse response) filter to a beam
   * @param[in] coeff The filter coefficients, with output index
   *                  assumed at the center coefficient point, which implies
   *                  an odd number of coefficients. Warning given if even.
   * @param[in] type  When the range of values extends outside the range of
   *                  valid data, one of several techniques is used to extend
   *                  the data so the filter can be applied.
   * @param[out] quality  A measure of quality for each point in the beam
   *
   * If there are any problems, no filtering is done.
   */
  void FIRfilter(const std::vector<double> coeff, FirFilter_t type,
		 RayxData &quality);
		 
  /**
   * Constrain data to a range of gates, set to missing outside that range
   * 
   * @param[in] minGateIndex  Minimum gate index with valid data
   * @param[in] maxGateIndex  Maximum gate index with valid data
   */
  void constrain(int minGateIndex, int maxGateIndex);

  void variance(double npt, double maxPctMissing);

  /**
   * Set debugging on
   */
  void setDebug(bool state) { _debug = state; }

protected:
private:

  std::string _name;     /**< Field name */
  std::string _units;    /**< Units */
  int _npt;              /**< Number of points */
  double _missing;       /**< Missing data value */
  std::vector<double> _data;  /**< Storage for the data */

  double _az;           /**< Azimuth of the ray (degrees)*/
  double _elev;         /**< Elevation of ray (degrees) */
  double _gate_spacing; /**< distance between gates (km) */
  double _start_range;  /**< distance to first gate (km) */
  bool _debug;          /**< Debugging flag */

  /**
   * Set output at index to either input or local value if non missing.
   * and missing_ok = true, otherwise set output at index to missing
   */
  void _passthrough(const RayxData &inp, const int i, const bool missing_ok);

  bool _isOnePointOutlier(int i, double outlierThresh) const;
  bool _isTwoPointOutlier(int i, double outlierThresh) const;
  double _FIRquality(int centerCoeff, const vector<double> &tmpData,
		     const vector<double> &gapFilledData,
		     int tindex);
  void _fillGaps(std::vector<double> &data) const;
  void _interp(double d0, double d1, int i0, int i1,
	       std::vector<double> &iData) const;
  std::vector<double> _extendData(int i0, int i1, int centerCoeff, int nCoeff,
				  FirFilter_t type, bool allbad0,
				  double m0, double int0, bool allbad1,
				  double m1, double int1) const;
  double _extend(int mirrorIndex, int interpIndex, int boundaryDataIndex,
		 int otherAveIndex, FirFilter_t type, double m,
		 double intercept, bool allbad) const;
  double _sumProduct(const std::vector<double> &coeff, double sumCoeff,
		     const std::vector<double> &data, int i0) const;
  int _firstValidIndex(void) const;
  int _lastValidIndex(void) const;
  bool _linearRegression(int i0, int i1, int npt, bool up,
			 double &slope, double &intercept) const;
  double _applyFIR(int j, int i0, int i1, int centerCoeff, 
		   FirFilter_t type,
		   const std::vector<double> &tmpData,
		   const std::vector<double> &gapFilledData,
		   const std::vector<double> &coeff, double sumCoeff);
  double _mean(int i0, int i1) const;
};

#endif
