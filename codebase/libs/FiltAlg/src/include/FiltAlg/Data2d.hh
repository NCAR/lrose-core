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
 * @file Data2d.hh 
 * @brief Data2d  A vector of numbers, with one name
 * @class Data2d
 * @brief Data2d  A vector of numbers, with one name
 * 
 * It represents results of calculations in a radar volume that
 * produce one number per vertical level.
 */

#ifndef DATA_2D_H
#define DATA_2D_H

#include <string>
#include <vector>
#include <cstdio>

//------------------------------------------------------------------
class Data2d
{
public:

  /**
   * Constructor
   * No name, no data
   */
  Data2d(void);

  /**
   * Constructor
   * Named data, with no data
   * @param[in] name  Name to use
   */
  Data2d(const std::string &name);

  /**
   *  Destructor
   */
  virtual ~Data2d(void);

  /**
   * Print out all the values to standard out
   */
  void print_2d(void) const;

  /** 
   * Add one value to state, append
   * @param[in] value  The value to append
   */
  void add(const double value);

  /**
   * Write state to string as XML
   * @return the string with XML equivalent to internal state
   */
  std::string write_xml(void) const;

  /**
   * Set state from an input xml string
   * @param[in] xml  String that should have the state in it.
   *
   * @note goes with write_xml()
   */
  bool set_from_xml(const std::string &xml);

  /**
   * @return  A fixed name indicating 'bad' 
   */
  inline static std::string bad_2d_name(void) {return "Missing";}

  /**
   * @return The name of the data
   */
  inline std::string get_name(void) const {return _name;}

protected:
private:

  std::string _name;         /**< The name */
  std::vector<double> _v;    /**< The values */
};

#endif
