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
 * @file Histo.hh
 * @brief Histogram
 * @class Histo
 * @brief Histogram
 *
 * Note this implementation assumes the lowest bin is a threshold for which
 * everything less than this value is considered 'the same', because it is
 * the clutter threshold.
 */

# ifndef    HISTO_HH
# define    HISTO_HH

#include <vector>

//------------------------------------------------------------------
class Histo
{
public:

  /**
   * Empty constructor
   */
  Histo(void);

  /**
   * Constructor
   *
   * @param[in] clutterThresh  Threshold that defines the lowest bin
   * @param[in] deltaBin  Bin width
   * @param[in] maxBin  Maximum bin value
   */
  Histo(const double clutterThresh, const double deltaBin, const double maxBin);

  /**
   *  Destructor
   */
  virtual ~Histo(void);

  /**
   * Initialize, same result as constructor
   *
   * @param[in] clutterThresh  Threshold that defines the lowest bin
   * @param[in] deltaBin  Bin width
   * @param[in] maxBin  Maximum bin value
   */
  void init(const double clutterThresh, const double deltaBin,
	    const double maxBin);

  /**
   * Add to histograms state
   *
   * @param[in] v  Value to add to state
   */
  void update(const double v);

  /**
   * Add missing data to histograms state.  Assumes one case for which data
   * is missing has occured
   */
  void updateMissing(void);

  /**
   * Compute and return the pct'th percentile data value
   *
   * @param[in] pct  The pcertnile
   * @param[out] v  The value
   *
   * @return true if computed a value, false if not enough values
   *              or the percentile came out above the clutterThreshold
   */
  bool computePercentile(const double pct, double &v) const;

  /**
   * Debug print 
   */
  void print(void) const;

  /**
   * Debug print with an x=input value  prior
   *
   * @param[in] x  The value to print out before debug print
   */
  void print(const double x) const;

protected:
private:  
  
  int _nbin;             /**< number of bins */
  double _deltaBin;      /**< delta between bins */
  double _minBin;        /**< minimum bin */
  double _maxBin;        /**< maximum bin */

  int _nmissing;         /**< number of missing data points */
  int _countBelowMin;    /**< number of points below the minimum bin */
  int _ntotal;           /**< number of points total */

  std::vector<int> _counts;  /**< The bin contents */
};

# endif
