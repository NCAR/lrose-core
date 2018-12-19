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
 * @file AsciiOutput.hh 
 * @brief AsciiOutput
 * @class AsciiOutput
 * @brief AsciiOutput
 */

#ifndef AsciiOutput_H
#define AsciiOutput_H
#include <rapmath/MathUserData.hh>
#include <string>

/*----------------------------------------------------------------*/
class AsciiOutput : public MathUserData
{
public:

  /**
   * @param[in] name  Variable name
   * @param[in] dir  Top path
   * @param[in] t    Time
   *
   * Set up all the member values and create the path  needed
   */
  AsciiOutput(const std::string &name, const std::string &dir, const time_t &t);

  ~AsciiOutput();

  /**
   * @return true if input name = local name
   * @param[in] name
   */
  inline bool nameMatch(const std::string &name) const {return _name == name;}

  /**
   * Remove the path
   */
  void clear(void) const;

  /**
   * Append input string to the file, with a '\n'
   */
  void append(const std::string &s) const;

  /**
   * Append input string to the file, without a '\n'
   */
  void appendNoCr(const std::string &s) const;

  
  void writeLdataInfo();

  inline virtual bool getFloat(double &f) const {return false;}

protected:
private:

  std::string _name;      /**< variable name within the app */
  std::string _dir;       /**< Top path */
  std::string _fileName;  /**< Name of one file yyyymmdd_hhmmss.humidity.ascii*/
  std::string _relPath;   /**< relative path yyyymmdd/<file> */
  std::string _path;      /**< Full path _dir / _relPath */
  time_t _time;           /**< Current time */
};

#endif
 
