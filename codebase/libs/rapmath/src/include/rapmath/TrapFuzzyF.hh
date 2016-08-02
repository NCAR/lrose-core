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
 * @file TrapFuzzyF.hh
 * @brief Trapezoidal shaped fuzzy (linear) mapping 
 * @class TrapFuzzyF
 * @brief Trapezoidal shaped fuzzy (linear) mapping 
 *
 * The trapezoid has 4 parameters  (a,b,c,d) and the function is
 * f(x) = max(min((x-a)/(b-a), 1, (d-x)/(d-c)), 0)
 *
 * It must be true that a < b < c < d
 */

# ifndef    TRAP_FUZZYF_HH
# define    TRAP_FUZZYF_HH

#include <iostream>
#include <string>
#include <vector>

//------------------------------------------------------------------
class TrapFuzzyF
{
public:

  /**
   * Empty constructor
   */
  TrapFuzzyF(void);

  /**
   * default constructor 
   * @param[in] a  argument to function
   * @param[in] b  argument to function
   * @param[in] c  argument to function
   * @param[in] d  argument to function
   *
   * The fuzzy function then maps x values to y space.
   *
   */
  TrapFuzzyF(double a, double b, double c, double d);

  /**
   *  destructor
   */
  virtual ~TrapFuzzyF(void);

  /**
   * @return true if object is well formed with content
   */
  inline bool ok(void) const {return _ok;}

  /**
   * @eturn true if input identical to local
   * @param[in] f
   */
  bool operator==(const TrapFuzzyF &f) const;

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
   */
  double apply(double x) const;

  /**
   * Return xml representation
   * @param[in] name name to give this trapezoid fuzzy function
   * @return a string with xml
   *
   * @note this is a way to store and the later retrieve content
   */
  std::string xmlContent(const std::string &name) const;

  /**
   * Parse input string as a trapezoidal fuzzy function with a particular name,
   * set internal state accordingly
   *
   * @return true if input string was parsed as a trapezoid fuzzy function with
   * input name
   *
   * @note the string is expected to have content consistent with the
   * output of xml_content(name)
   *
   * @note this is a way to retreive a previously stored function.
   *
   * @param[in] s string with the XML
   * @param[in] name the name expected for the fuzzy function
   */
  bool readXml(const std::string &s, const std::string &name);

protected:
private:  

  bool _ok;  /**< True if object is well formed with content */

  double _a;   /**< param */
  double _b;   /**< param */
  double _c;   /**< param */
  double _d;   /**< param */
  
};

# endif
