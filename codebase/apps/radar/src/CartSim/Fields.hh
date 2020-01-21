// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
/**
 * @file  Fields.hh
 * @brief  static methods to set various values associated with output fields
 * @class  Fields
 * @brief  static methods to set various values associated with output fields
 */

# ifndef    FIELDS_H
# define    FIELDS_H

#include "Params.hh"
#include <string>

//----------------------------------------------------------------
class Fields
{
public:

  /**
   * @return type associated with name
   * @param[in] name
   */
  static Params::Field_t fieldType(const std::string &name);

  /**
   * @return name associated with t ype
   * @param[in] f  Type
   */
  static std::string fieldName(const Params::Field_t f);

  /**
   * @return units associated with t ype
   * @param[in] f  Type
   */
  static std::string fieldUnits(const Params::Field_t f);

  /**
   * @return missing data value associated with t ype
   * @param[in] f  Type
   */
  static double fieldMissingValue(const Params::Field_t f);

protected:
private:  

};

# endif 
