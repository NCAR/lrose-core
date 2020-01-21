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
 * @file  Info.hh
 * @brief Information needed to run the algorithm using threading
 * @class Info
 * @brief Information needed to run the algorithm using threading
 */

# ifndef   INFO_HH
# define   INFO_HH

#include "CartSim.hh"

class DataHandler;
class CartSimMgr;

//----------------------------------------------------------------
class Info
{
public:

  /**
   * Constructor
   * @param[in] dt  Seconds into the simulation
   * @param[in] alg  Reference to the algorithm
   * @param[in] mgr  Pointer to the manager
   */
  Info(const int dt, const CartSim &alg, CartSimMgr *mgr);

  /**
   * Destructor
   */
  virtual ~Info(void);

  /**
   * Process using local information
   * This is where the bulk of everything is done:
   */
  void process(void);


protected:
private:

  DataHandler *_data;  /**< data handler owned by this object */
  CartSim _alg;        /**< Copy of algorithm, owned by this object */
  CartSimMgr *_mgr;    /**< manager pointer, not owned by this object */

};

# endif 
