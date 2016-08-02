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
 * @file FindSimple.hh 
 * @brief Mimics matlab 'find', logical test on one input
 * @class FindSimple
 * @brief Mimics matlab 'find', logical tests on one input
 * 
 */

#ifndef FIND_SIMPLE_H
#define FIND_SIMPLE_H
#include <string>
#include <FiltAlg/Comb.hh>

//------------------------------------------------------------------
class FindSimple 
{
public:

  /**
   * @enum Compare_t
   * @brief The logical comparisions that are supported
   */
  typedef enum
  {
    GT = 0,  /**< Greater than */
    GE = 1,  /**< Greater than or equal */
    EQ = 2,  /**< Equal */
    LE = 3,  /**< Less than or equal */
    LT = 4,  /**< Less than */
  } Compare_t;

  /**
   * Empty constructor
   */
  FindSimple(void);

//   /**
//    * Constructor from a parsed string
//    *
//    * @param[in] s  String to parse  "A <= 1.0"
//    */
//   FindSimple(const std::string &s);

  /**
   * Constructor with tokens
   * @param[in] name   Name of data
   * @param[in] comp   String with comparison 
   * @param[in] value  String with a numerical value
   */
  FindSimple(const std::string &name, const std::string &comp, 
	     const std::string &value);


  /**
   * Destructor
   */
  virtual ~FindSimple(void);

  /**
   * @return true if well formed (parsed string success, non-empty)
   */
  inline bool ok(void) const {return _ok;}

  /**
   * Debug print to stdout
   */
  void print(void) const;

  /**
   * @return true if input named things are found in 'comb' input
   * @param[in] comb  Input to check against
   */
  bool isConsistent(const Comb &comb) const;

  /**
   * Set pointer to the data using input Comb object
   *
   * @param[in] comb   Object with pointers to Data set
   *
   * @return true if successful
   */
  bool setPointer(const Comb &comb);

  /**
   * Retrieve data value at point
   * 
   * @param[in] ipt  Index into the grids
   * @param[in] vlevel   Vlevel to match
   * @param[in] vlevel_tolerance   Allowed error in vlevel
   * @param[out] value
   *
   * @return true if value at ipt was not missing and is returned
   */
  bool getValue(const int ipt, 
		const double vlevel,
		const double vlevel_tolerance,
		double &value) const;

  /**
   * @return true if condition is satisfied at the input point
   * @param[in] ipt  Index into the grids
   * @param[in] vlevel   Vlevel to match
   * @param[in] vlevel_tolerance   Allowed error in vlevel
   */
  bool satisfiesCondition(const int ipt,
			  const double vlevel,
			  const double vlevel_tolerance) const;


  /**
   * @return the string for a particular comparision enum
   * @param[in] c  Compare type to make a string for
   */
  static std::string comparisonString(const Compare_t &c);


protected:
private:


  bool _ok;           /**< True for well formed */

  std::string _data_name;  /**< Name of data to test */
  Compare_t _test;    /**< The comparison to make */
  double _value;      /**< Value to compare to */
  const Data * _data; /**< Data pointer, filled in as we go */
};

#endif
