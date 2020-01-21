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
 * @file  CartesianSpeck.hh
 * @brief 'Speck' of outlier data with cartesian parameters
 * @class  CartesianSpeck
 * @brief 'Speck' of outlier data with cartesian parameters
 */

# ifndef    CARTESIAN_SPECK_H
# define    CARTESIAN_SPECK_H

#include "Params.hh"
#include "Thing.hh"
#include "OutlierData.hh"
#include "Xyz.hh"
class Data;

//----------------------------------------------------------------
class CartesianSpeck : public Thing
{
public:

  /**
   * Default constructor
   */
  CartesianSpeck(const Params &P, const Params::CartesianSpeck_t &mP);
  
  /**
   * Destructor
   */
  virtual ~CartesianSpeck(void);


protected:

  virtual void _addToData(const Xyz &loc, Data &data) const;

private:  

  Xyz _center;  /**< Center of speck, meters */
  Xyz _len;     /**< Length of speck, meters */
  OutlierData _outlier; /**< Outlier values to use in speck */
};

# endif 
