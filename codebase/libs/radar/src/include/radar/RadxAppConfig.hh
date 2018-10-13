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
 * @file RadxAppConfig.hh 
 * @brief Handling of multiple inputs
 * @class RadxAppConfig
 * @brief Handling of multiple inputs
 *
 * Each input is a 'Group', with one primary input and any number of secondary
 * inputs.
 * 
 */

#ifndef RADX_APP_CONFIG_H
#define RADX_APP_CONFIG_H
#include <radar/RadxAppParams.hh>
#include <vector>
#include <string>

//------------------------------------------------------------------
class RadxAppConfig  : public RadxAppParams
{
public:
  /**
   * Constructor empty.
   */
  RadxAppConfig(void);

  /**
   * Constructor..sets base class to input
   * @param[in] p  Base class values to use
   */
  RadxAppConfig(const RadxAppParams &p);

  /**
   * Destructor
   */
  virtual ~RadxAppConfig(void);

  /**
   * @return true if input field names are in either the primary or one of
   * the secondary groups
   *
   * @param[in] inputs  The field names
   */
  bool inputsAccountedFor(const std::vector<std::string> &inputs) const;

  /**
   * @class Group
   * @brief  Specifiation for on input source
   */
  class Group
  {
  public:
    string dir;        /**< path for data */
    int index;         /**< numerical value to refer to this elsewhere */

    /** 
     * Ignored for primary, for secondary used to search for best time match to
     * primary (seconds)
     */
    double fileTimeOffset; 

    /** 
     * Ignored for primary, for secondary used to search for best time match to
     * primary (seconds)
     */
    double fileTimeTolerance;

    /** 
     * Ignored for primary, for secondary used to search for best matching ray
     * in primary (degrees)
     */
    double rayElevTolerance;

    /** 
     * Ignored for primary, for secondary used to search for best matching ray
     * in primary (degrees)
     */
    double rayAzTolerance;

    /** 
     * Ignored for primary, for secondary used to search for best matching ray
     * in primary (seconds)
     */
    double rayTimeTolerance;

    bool isClimo;  /**< If true fixed file is used no matter what */
    string climoFile; /**< Fixed file to use if isClimo = true */

    /**
     * The field names for inputs in a group
     */
    std::vector<std::string> names;
  };
  
  Group _primaryGroup;   /**< The main input */
  std::vector<Group> _secondaryGroups;  /**< any number of secondary inputs */

protected:
private:

};

#endif
