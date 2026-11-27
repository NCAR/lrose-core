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
 * @file FrequencyCount.hh
 * @brief Histograms of the number of scans with data >= threshold 
 *        (normalized to [0,1]) as a function of percentage of data
 * @class FrequencyCount
 * @brief Histograms of the number of scans with data >= threshold 
 *        (normalized to [0,1]) as a function of percentage of data
 */

# ifndef    FREQUENCY_COUNT_HH
# define    FREQUENCY_COUNT_HH

#include <vector>
#include <string>

//------------------------------------------------------------------
class FrequencyCount
{
public:

  /**
   * Constructor
   * @param[in] nbins  Number of bins in the histogram (range is 0 to 1)
   * @param[in] nscan  Total number of scans
   *
   * The i'th bin is at i/nscan
   */
  FrequencyCount(const int nbins, const int nscan);

  /**
   *  Destructor
   */
  virtual ~FrequencyCount(void);

  /**
   * Update with a count of the number of times data is >= threshold at a pixel
   * @param[in] count  The number
   *
   * The bin count/nscan is incremented by 1
   */
  void update(const double count);

  /**
   * Append a line with the normalized results to a file
   *
   * @param[in] fp  The file
   *
   * _counts[0]/_totalCount  _counts[1]/_totalCount    ...
   *
   */
  void append(FILE *fp);

  /**
   * Append a string with the normalized results to a vector
   *
   * @param[in,out] lines
   *
   * _counts[0]/_totalCount  _counts[1]/_totalCount    ...
   *
   */
  void appendString(std::vector<std::string> &lines);

protected:
private:  
  
  double _nscan;  /**< Total number of scans */
  double _nbin;   /**< Number of bins */

  double _totalCount;  /**< Total count for normalization */
  std::vector<double> _counts;  /**< The counts */
  std::vector<double> _frequency;  /**< The "x axis" of the data, not used */

};

# endif
