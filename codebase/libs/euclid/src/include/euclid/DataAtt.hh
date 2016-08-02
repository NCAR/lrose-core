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
 * @file DataAtt.hh
 * @brief Data characterization attributes (min,max,ave,median, npt)
 * @class DataAtt
 * @brief Data characterization attributes (min,max,ave,median, npt)
 */
# ifndef    DATA_ATT_H
# define    DATA_ATT_H

#include <string>

class PointList;
class Grid2d;

class DataAtt 
{
 public:

  /**
   * Construct, give all members 'big' values
   */
  DataAtt(void);

  /**
   * Construct, set members to input values
   *
   * @param[in] mind  Minimum data value
   * @param[in] maxd  Maximum data value
   * @param[in] med  Median data value
   * @param[in] aved  Average data value
   * @param[in] nptd  Number of data values
   */
  DataAtt(double mind, double maxd, double med, double aved, double nptd);

  /**
   * Construct setting members to those from a set of points in a grid
   *
   * @param[in] l  The set of points within the Grid2d to use
   * @param[in] data  A data grid
   */
  DataAtt(const PointList &l, const Grid2d &data);

  /**
   * Destructor
   */
  virtual ~DataAtt(void);

  /**
   * Operator==
   *
   * @param[in] v
   */
  bool operator==(const DataAtt &v) const;

  /**
   * Debug print
   */
  void print(void) const;

  /**
   * Debug print
   * @param[in] fp
   */
  void print(FILE *fp) const;

  /**
   * Debug print
   */
  std::string sprint(void) const;

  /**
   * @return true if average > min_ave and max > min_max
   * @param[in] min_ave
   * @param[in] min_max
   */
  bool hasData(double min_ave, double min_max) const;

  /**
   * @return minimum
   */
  inline double getMin(void) const {return _min;}

  /**
   * @return maximum
   */
  inline double getMax(void) const {return _max;}

  /**
   * @return average
   */
  inline double getAve(void) const {return _ave;}

  /**
   * @return median
   */
  inline double getMedian(void) const {return _median;}

  /**
   * @return number of points
   */
  inline double getNpt(void) const {return _npt;}

 private:

  double _min;   /**< minimum data value.*/
  double _max;   /**< maximum data value.*/
  double _ave;   /**< average data value */
  double _median;/**< median data NOT IMPLEMENTED. */
  double _npt;   /**< Number of values */

  void _fillFromList(const PointList &l, const Grid2d &data);
};

# endif
