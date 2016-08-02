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
 * @file RadxFuzzy2d.hh
 * @brief Fuzzy function of 2 variables
 * @class RadxFuzzy2d
 * @brief Fuzzy function of 2 variables
 *

 * One value exists for each x/y pair in a two dimensional
 * table.  For x and y that land exactly on a table point,
 * The value for that point is simply the value at x,y. If x or y
 * request is not in the table, interpolation to nearest x, y is used
 * to set the value.
 *
 * The mapping must be read from a fixed format ASCII file.
 * The format of the file is:
 *
 * #  any number of comment lines all start with #
 *  The x values space separated
 *  y value   function value at each x
 *  y value   function value at each x
 *  .
 *  .
 *
 * Copied over from libs/rapmath.
 */

#ifndef RADX_FUZZY_2D_H
#define RADX_FUZZY_2D_H

#include <map>
#include <string>
#include <vector>

class RadxFuzzy2d
{
 public:

  /**
   * Empty constructor
   */
  RadxFuzzy2d(void);

  /**
   * Destructor
   */
  virtual ~RadxFuzzy2d(void);

  /**
   * Print the contents of the table.
   */
  void printTable(void) const;
  

  /**
   * Get the value for the given x and y.
   *
   * @param[in] x  x value
   * @param[in] y  y value
   *
   * @return  f(x,y)
   *
   * if x or y is not specifically in the lookup table, interpolation is used
   * to determine a value.
   */
  double apply(const double x, const double y) const;

  /**
   * @return true for good object
   */
  inline bool isOk(void) const {return  _ok;}

protected:
private:

  /**
   * Maximum length in characters of a single token in the file that is read
   * in.
   */
  static const int _maxTokenLen;
  
  /**
   * Vector of all x values in the table.
   */
  std::vector< double > _x;

  /**
   * Vector of all y values in the table.
   */
  std::vector< double > _y;
  
  /**
   * The mapping from x, y, pairs to a value
   */
  std::map< std::pair< double, double >, double > _table;

  bool _ok;  /**< True if object is good */

  /**
   * Read the first non-comment line from the parameter file,
   * which should have x values
   *
   * @param[in,out] fp  FILE pointer to an opened file
   */
  void _readX(FILE *fp);

  /**
   * Parse the next non-comment line from the file, which should have one y
   * value and then output values for all x values
   *
   * @param[in] line  The contents of the line from the file.
   * @param[in] lineLen  Number of characters in the line
   * @param[in] expectedNumTokens Expected number of fields in the line
   * 
   * @return true if the line could be parsed.
   *
   * @note expectedNumTokens = number of x + 1
   * @note This method updates the state if the parse was successful.
   */
  bool _parseLine(const char *line, const int lineLen,
		  const int expectedNumTokens);

  /**
   * @return value for a given x and y
   * @param[in] x
   * @param[in] y
   */
  double _getValue(const double x, const double y) const;

};


#endif
