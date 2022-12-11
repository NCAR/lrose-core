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
 */

# ifndef    HISTO_HH
# define    HISTO_HH

#include <vector>

//------------------------------------------------------------------
class Histo
{
public:

  /**
   * constructor
   */
  Histo();

  /**
   * constructor
   * Set member values to inputs
   * @param[in]  minBin  Center of smallest bin 
   * @param[in] deltaBin Distance between bin centers
   * @param[in]  maxBin  Center of largest bin 
   */
  Histo(const double minBin, const double deltaBin, const double maxBin);

  /**
   *  Dstructor
   */
  virtual ~Histo();

  /**
   * init
   */
  void init(const double minBin, const double deltaBin, const double maxBin);

  /**
   * Add input value to data vector
   * @param[in] v  Value
   */
  void update(const double v);

  /**
   * Add missing data value to histograms state
   * 
   */
  void updateMissing();

  /**
   * Compute the median and return result
   * @param[out] v  Median
   * @return true if successful, false if over half the data
   *              was missing, or no data, or error
   */
  bool computeMedian(double &v) const;

  /**
   * Compute the mode and return result
   * @param[out] v  Mode
   * @return true if successful, false if over half the data
   *              was missing, or no data, or error
   */
  bool computeMode(double &v) const;

  /** 
   * Debug print 
   */
  void print(FILE *out) const;

  /**
   * Print out input value, then debug print
   * @param[in] x  Value
   */
  void print(FILE *out, const double x) const;

protected:
private:  
  
  std::vector<double> _data;  /**< The data */
  int _nmissing;              /**< Number of missing data points*/
  double _minBin;             /**< Histogram min bin center */
  double _deltaBin;           /**< Histogram bin seperation */
  double _maxBin;             /**< Histogram max bin center */
};

# endif
