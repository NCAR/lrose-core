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
 * @file Data1d.hh 
 * @brief A number and a name.
 * @class Data1d
 * @brief A number and a name.
 */

#ifndef DATA_1D_H
#define DATA_1D_H
#include <string>
#include <cstdio>

//------------------------------------------------------------------
class Data1d
{
public:

  /** 
   * Constructor
   * Name is set to a bad value 
   */
  Data1d(void);

  /**
   * Constructor
   * Named data, no data
   * @param[in] name  The name to use
   */
  Data1d(const std::string &name);

  /**
   * Constructor
   * Named data, with the data
   * @param[in] name  The name to use
   * @param[in] value  The value to use
   */
  Data1d(const std::string &name, const double value);

  /**
   * Destructor
   */
  virtual ~Data1d(void);

  /**
   * Print out the name and value to stdout
   */
  void print_1d(void) const;

  /**
   * Write as xml string
   * @return XML string with content equivalant to state
   */
  std::string write_xml(void) const;

  /**
   * Set state from xml string
   * @param[in] xml  A string that should be parseable as local state
   * @note goes with write_xml() method
   */
  bool set_from_xml(const std::string &xml);

  /**
   * @return String that indicates 'bad' content
   */
  inline static std::string bad_1d_name(void) {return "Missing";}

  /**
   * @return the name
   */
  inline std::string get_1d_name(void) const {return _name;}

  /**
   * Return the value
   * @param[out] v  The value to return
   * @return false if the object has name = bad_1d_name(), and if so no value
   * is set.
   */
  bool get_1d_value(double &v) const;

  /**
   * Set the name to input
   * @param[in] n  Name
   */
  void set_1d_name(const std::string &n);

  /**
   * Set local value to input
   * @param[in] v  The value
   * @return false if object has name = bad_1d_name()
   */
  bool set_1d_value(const double v);

  /**
   * Increment local value by input value.
   * @param[in] d  Value to increment local object with
   *
   * @return false if either input object or local object has
   *         name = bad_1d_name(), and in that case don't do anything
   */
  bool add_1d_value(const Data1d &d);

  /**
   * Increment local value by input value.
   * @param[in] v  Value to increment local object with
   * 
   * @return false if local object has name = bad_1d_name(), and in that case
   *         don't do anything
   */
  bool inc_1d_value(const double v);

  /**
   * Multiply local value by input value.
   * @param[in] d  Value to multiply local object by
   * 
   * @return false if either input object or local object has
   *         name = bad_1d_name(), and in that case don't do anything
   */
  bool multiply_1d_value(const Data1d &d);

  /**
   * Multiply local value by input value.
   * @param[in] v  Value to multiply local object by
   * 
   * @return false if local object has name = bad_1d_name(), and in that case
   *         don't do anything
   */
  bool prod_1d_value(const double v);


  /**
   * @return true if the name is the bad_1d_name()
   */
  inline bool is_bad(void) const {return _name == bad_1d_name();}

protected:
private:

  std::string _name;    /**< Name */
  double _value;   /**< Value */
};

#endif
