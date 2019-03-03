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
 * @file ThresholdBiasMapping.hh
 * @brief Mapping from lead seconds to bias and to threshold
 * @class ThresholdBiasMapping
 * @brief Mapping from lead seconds to bias and to threshold
 *
 * Stored to SPDB by gen time, contains XML mappings from lead seconds to
 * bias, and from lead seconds to threshold
 */

# ifndef    ThresholdBiasMapping_hh
# define    ThresholdBiasMapping_hh

#include <string>
#include <vector>
#include <map>

class DsSpdb;

//----------------------------------------------------------------
class ThresholdBiasMapping
{
public:

  /**
   * Empty
   */
  ThresholdBiasMapping(void);

  /**
   * @param[in] spdb  The URL
   */
  ThresholdBiasMapping(const std::string &spdb);

  /**
   * Destructor
   */
  virtual ~ThresholdBiasMapping(void);
  
  /**
   * Store inputs into local state, sets map values at the lead time
   *
   * @param[in] lt  Lead time
   * @param[in] bias  Bias
   * @param[in] w  Threshold
   */
  void store(int lt, double bias, double w);

  /**
   * Create XML for local state and write to SPDB specified by local state
   * for input gen time
   *
   * @param[in] gt  Gen time
   */
  void writeAndClear(const time_t &gt);

  /**
   * Read the newest data whose gen time is less than input, back to a maximum
   *
   * @param[in] gt  Gen time
   * @param[in] maxSecondsBack
   *
   * @return true if successful and state is filled in
   */
  bool readFirstBefore(const time_t &gt, int maxSecondsBack);

  /**
   * Read the newest data whose gen time is greater than input, up to a maximum
   *
   * @param[in] gt  Gen time
   * @param[in] maxSecondsAhead
   *
   * @return true if successful and state is filled in
   */
  bool readFirstAfter(const time_t &gt, int maxSecondsAhead);

  /**
   * Read the data nearest to the gen time before or after, up to maxes
   *
   * @param[in] gt  Gen time
   * @param[in] maxSecondsBack
   * @param[in] maxSecondsAhead if < 0, no looking ahead
   *
   * @return true if successful and state is filled in
   */
  bool readNearest(const time_t &gt, int maxSecondsBack, int maxSecondsAhead);

  /**
   * Read the exact same gen time, but in a previous day, up to some maximum
   * number of days
   *
   * @param[in] gt  Gen time to look multiples of days before
   * @param[in] maxDaysBack  
   * 
   * @return true if successful and state is filled in
   */
  bool readExactPrevious(const time_t &gt, int maxDaysBack);

  /**
   * Read the exact same gen time, in current or previous day,
   * up to some maximum
   *
   * @param[in] gt  Gen time to look multiples of days before
   * @param[in] maxDaysBack
   * 
   * @return true if successful and state is filled in
   */
  bool readExactPreviousOrCurrent(const time_t &gt, int maxDaysBack);

  /**
   * Read the exact gen time.
   *
   * @param[in] gt  Gen time to look for
   * 
   * @return true if successful and state is filled in
   */
  bool readExact(const time_t &gt);

  /**
   * Create and return a string in which the thresholds for each lead time
   * are set using 'setenv'
   *
   * @return the string
   *
   * The returned string is multiple lines of the form:
   *      'setenv THRESH_<lt>  <thresh>'
   */
  std::string setEnvString(void) const;

  /**
   * Retrieve threshold for a lead time
   *
   * @param[in] lt  Lead time
   * @param[out] thresh
   * @return true if a thresh exists for lead time
   */
  bool getThreshold(int lt, double &thresh) const;

  /**
   * Retrieve the interpolated threshold, with interpolation over lead times
   *
   * @param[in] lt  Lead time
   * @param[out] thresh
   * @return true if a threshold was set.
   *
   * If the lead time exactly matches a lead time in the local state, the
   * returned threshold is the threshold for that lead time.
   *
   * If there are lead times smaller and larger than the input lead time,
   * the two closest ones are used, call them lt0 and lt1, the returned
   * threshold = (threshold(lt0)*(lt1-lt) + threshold(lt1)*(lt-lt0))/(lt1-lt0)
   *
   * If the input lead is smaller than the smallest lead in state, the returned
   * threshold is that of the smallest lead time, with a warning.
   *
   * If the input lead is larger than the largest lead in state, the returned
   * threshold is that of the largest lead time, with a warning.
   */
  bool getLeadtimeInterpolatedThreshold(int lt, double &thresh) const;


  /**
   * Retrieve threshold for a lead time, within a partition of lead times
   *
   * @param[in] lt  Lead time
   * @param[out] thresh
   * @return true if a thresh exists for a partition associated with
   * the lead time. 
   *
   * The partitioning depends on the lead time resolution.  If lead
   * resolution is not constant, this method returns false.  Otherwise,
   * the partitions are based on each lead time and the dt between
   * lead times.  If the lead times in the object are lt[0], lt[1], ..
   * lt[n], then the partitions are [lt[0],lt[0]+dt), [lt[1], lt[1]+dt), ...
   * [lt[n], lt[n] + dt)
   */
  bool getPartitionedThreshold(int lt, double &thresh) const;

  /**
   * Retrieve bias for a lead time
   *
   * @param[in] lt  Lead time
   * @param[out] bias
   * @return true if a bias exists for lead time
   */
  bool getBias(int lt, double &bias) const;

  /**
   * Compare the contents of input and local object
   *
   * @param[in] inp  Object to compare with local
   * @param[in] maxdiff  Max difference between thresholds for 'very close'
   * @param[out] comparisons  Results of comparisons line by line
   *
   * @return true if very close to each other, false otherwise, or there are
   *              not the same lead times in the mappings
   */
  bool compare(const ThresholdBiasMapping &inp, const double maxdiff,
	       std::string &comparisons) const;

  /**
   * @return true if all bias values are within tolerance of target
   *
   * @param[in] target
   * @param[in] tolerance
   */
  bool biasGood(double target, double tolerance) const;

  /**
   * @return true if all bias values are within tolerance of target,
   * ignoring specified lead times
   *
   * @param[in] target
   * @param[in] tolerance
   * @param[in] leadSeconds  Lead times to ignore (seconds)
   */
  bool biasGood(double target, double tolerance,
		std::vector<int> leadSeconds) const;

  /**
   * Copy input threshold and bias info into local state
   *
   * @param[in] inp  Object to copy from
   */
  void copyData(const ThresholdBiasMapping &inp);

  /**
   * @return a string that has one line per leadtime, with format
   *         leadtime   threshold   bias
   */
  std::string asciiTable(void) const;

  /**
   * @return a string that has one line per leadtime, with format
   *         yyyy mm dd hh mm ss leadtime   threshold   bias
   *
   * @param[in] t  The time from which to get yyyy mm dd hh mm ss
   */
  std::string asciiTable(const time_t &t) const;

protected:
private:  

  /**
   * SPDB location
   */
  std::string _url;

  /**
   * Lead to threshold mapping
   */
  std::map<int, double> _threshMap;

  /**
   * Lead to Bias mapping
   */
  std::map<int, double> _biasMap;

  /**
   * Private methods
   */
  std::string _toXml(void) const;
  std::string _appendXml(const std::string &pairKey,
			 const std::string &valueKey,
			 int lt, double w) const;
  bool _fromXml(const std::string &xml);
  bool _load(DsSpdb &s);
};

# endif
