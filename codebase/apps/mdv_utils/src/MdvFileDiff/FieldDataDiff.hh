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
 * @file FieldDataDiff.hh
 * @class FieldDataDiff 
 * All Data related Differences between two inputs for one Field
 */
#ifndef FIELD_DATA_DIFF_HH
#define FIELD_DATA_DIFF_HH
#include <string>

class FieldDataDiff
{
  friend class FieldSummaryStats;
public:

  /**
   * empty Constructor needed for map
   */
  FieldDataDiff(void);

  /**
   * Constructor
   * @param[in] name the field name
   */
  FieldDataDiff(const std::string &name);

  /**
   * Destructor
   */
  ~FieldDataDiff(void);

  /**
   * Print out differences
   * @param[in] minDiff if percentage of points that are different < this
   *                    number, do not print out differences
   */
  void print(const double minDiff) const;

  /**
   * Print out any field header differences
   */
  void printFieldHdrDiffDetails(void) const;

  /**
   * @return true if name matches local name
   * @param[in] name
   */
  inline bool match(const std::string name) const
  {
    return name == _name;
  }

  /**
   * Set state indicating only one of the two inputs exists for this
   * field.
   * @param[in] which Name of the input that does exist
   */
  void gotOneField(const std::string &which);

  /**
   * Set state indicating field header differences.
   * @param[in] details  Description of the differences
   */
  void fieldHdrDiff(const std::string &details);

  /**
   * Update state for a grid point where data was the same
   */
  void dataSame(void);

  /**
   * Update state for a grid point where data was different
   * @param[in] diff The magnitude of the difference
   * @param[in] x  Index to point 
   * @param[in] y  Index to point 
   */
  void dataDiff(const double diff, const int x, const int y, const int z);

  /**
   * Update state for a grid point where data was missing in one of two
   */
  void dataOnlyOne(void);

  /**
   * @return true if state shows data differences, above a tolerance
   * @param[in] minPct  Minimum percentage of data points different to
   *                    return true
   */
  bool isDifferent(const double minPct) const;


protected:
private:
  
  std::string _name;             /**< Field name */

  int _ntotal;         /**< Total # of points */
  int _ndiff;          /**< # of Points with data diff */
  int _nOnlyOne;       /**< # of points with missing data in one of two */
  double _minDiff;     /**< Minimum data absolute valued diff */
  double _maxDiff;     /**< Maximum data absolute valued diff */
  double _aveDiff;     /**< Average data absolute valued diff */

  int _xAtMaxDiff;     /**< X location where diff is max */
  int _yAtMaxDiff;     /**< Y location where diff is max */
  int _zAtMaxDiff;     /**< Z location where diff is max */
};

#endif
