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
 * @file FuzzyF.hh
 * @brief fuzzy (linear) mapping from one variable to another.
 * @class FuzzyF
 * @brief fuzzy (linear) mapping from one variable to another.
 *
 * This is a simple implementation of the FuzzyFunction-like virtual class
 * but it is not a derived class, because this class supports a public copy
 * constructor and FuzzyFunction does not
 */

# ifndef    FUZZYF_HH
# define    FUZZYF_HH

#include <iostream>
#include <string>
#include <vector>

//------------------------------------------------------------------
class FuzzyF
{
public:

  /**
   * Empty constructor
   */
  FuzzyF(void);

  /**
   * default constructor from pairs of x,y points in the mapping.
   * @param[in] f  f[i].first = ith x value for the fuzzy function
   *               f[i].second= ith y value for the fuzzy function
   *
   * The fuzzy function then maps x values to y space.
   *
   * @note the x values should be strictly decreasing or strictly
   * increasing (i.e. consistent).
   */
  FuzzyF(const std::vector<std::pair<double,double> > &f);

  /**
   * default constructor from two vectors of x,y points in the mapping.
   * @param[in] x  x values for the fuzzy function
   * @param[in] y  y values for the fuzzy function
   *
   * The fuzzy function then maps x values to y space.
   *
   * @note the x values should be strictly decreasing or strictly
   * increasing (i.e. consistent).
   */
  FuzzyF(const std::vector<double> &x, const std::vector<double> &y);

  /**
   * default constructor from two double arrays of x,y points in the mapping.
   * @param[in] n  Number of x,y values
   * @param[in] x  x values for the fuzzy function
   * @param[in] y  y values for the fuzzy function
   *
   * The fuzzy function then maps x values to y space.
   *
   * @note the x values should be strictly decreasing or strictly
   * increasing (i.e. consistent).
   */
  FuzzyF(const int n, const double *x, const double *y);

  /**
   * Special fuzzy function, at x, y=tan(angle)*x, from x0 to x1, 0 otherwise.
   *
   * @param[in] angle   The angle to take tangent of
   * @param[in] x0  Lower range of x
   * @param[in] x1  Upper range of x
   */
  FuzzyF(double angle, double x0, double x1);

  /**
   * Contructor which reads from a parameter file in a specified format
   * @param[in] parmfile  Name of file to read, assumed to be a
   *                      ParamsFuzzyFuntion input file
   */
  FuzzyF(const std::string &paramfile);

  /**
   *  destructor
   */
  virtual ~FuzzyF(void);

  /**
   * @return true if object is well formed with content
   */
  inline bool ok(void) const {return _ok;}

  /**
   * @eturn true if input identical to local
   * @param[in] f
   */
  bool operator==(const FuzzyF &f) const;

  /**
   * Debug print to stdout
   */
  void print(void) const;

  /**
   * Print to stream
   */
  void print(std::ostream &stream) const;

  /**
   * Apply the fuzzy function to input value x.
   * @param[in] x  an x value
   *
   * @return fuzzy value y corresponding to input value x, or if the
   * function is not set, return bad().
   *
   * @note If x corresponds to one of the constructor pair x values,
   * return the corresponding y value. Otherwise interpolate between
   * x values defined by the constructor.
   *
   * @note if x < smallest x in constructor, returns y corresponsing
   * to smallest x in constructor
   *
   * @note if x > largest x in constructor, returns y corresponsing
   * to largest x in constructor
   */
  double apply(const double x) const;

  /**
   * Return maximum fuzzy x value
   * @return largest x value 
   */
  double maxX(void) const;

  /**
   * Return minimum fuzzy x value
   * @return smallest x value
   */
  double minX(void) const;

  /**
   * Get range of x values in the mapping
   * @return false if no mapping is set
   * @param[out] x0  Lower x
   * @param[out] x1  Upper x
   */
  bool xRange(double &x0, double &x1) const;

  /**
   * Get range of y values in the mapping
   * @return false if no mapping is set
   * @param[out] y0  Lower x
   * @param[out] y1  Upper x
   */
  bool yRange(double &y0, double &y1) const;

  /**
   * @return true if can slice through function at y and intersect the map
   * If true return max x where there is such an intersection
   *
   * @param[in] y  Y value to slice at
   * @param[out] max  Maximum x at y (when return is true)
   */
   bool maxXAtGivenY(double y, double &max) const;

  /**
   * @return true if can slice through function at y and intersect the map
   * If true return min x where there is such an intersection
   *
   * @param[in] y  Y value to slice at
   * @param[out] min  Minimum x at y (when return is true)
   */
   bool minXAtGivenY(double y, double &max) const;

  /**
   * @return number of x,y pairs of points
   */
  inline int num(void) const {return static_cast<int>(_f.size());}

  /**
   * @return i'th x value
   * @param[in] i
   */
  inline double ithX(const int i) const {return _f[i].first;}

  /**
   * @return i'th x value
   * @param[in] i
   */
  inline double ithY(const int i) const {return _f[i].second;}

  inline std::string title(void) const {return _title;}
  inline std::string xUnits(void) const {return _xUnits;}
  inline std::string yUnits(void) const {return _yUnits;}
  inline std::string mapping(void) const {return _xUnits + "--->" + _yUnits;}

  /**
   * Return xml representation of the fuzzy function
   * @param[in] name name to give this fuzzy function
   * @return a string with xml
   *
   * @note this is a way to store and the later retrieve a fuzzy function.
   */
  std::string xmlContent(const std::string &name) const;

  /**
   * Parse input string as a fuzzy function with a particular name,
   * set internal state accordingly
   *
   * @return true if input string was parsed as a fuzzy function with
   * input name
   *
   * @note the string is expected to have content consistent with the
   * output of xml_content(name)
   *
   * @note this is a way to retreive a previously stored fuzzy function.
   *
   * @param[in] s string with the XML
   * @param[in] name the name expected for the fuzzy function
   */
  bool readXml(const std::string &s, const std::string &name);

  /**
   * Multiply each x value by input scale factor
   * This is units conversion
   *
   * @param[in] scale
   */
  void rescaleXValues(const double scale);

  /**
   * @return all the fuzzy function x values in the mapping (map from x to y)
   */
  std::vector<double> xValues(void) const;

  /**
   * @return all the fuzzy function y values in the mapping (map from x to y)
   */
  std::vector<double> yValues(void) const;

  bool writeValues(const std::string &outputFile) const;

  /**
   * @return a fixed 'bad' fuzzy value
   */
  static double bad(void);



protected:
private:  

  /**
   * Internal representation of the fuzzy function with x=first, y=second
   * y = f(x)
   */
  std::vector<std::pair<double,double> > _f;
  
  std::string _xUnits;  /**< Units for the X axis, empty is ok */
  std::string _yUnits;  /**< Units for the Y axis, empty is ok */
  std::string _title;  /**< Name for the function itself, empty is ok */

  bool _ok;  /**< True if object is well formed with content */

  void _checkContent(void);
};

# endif
