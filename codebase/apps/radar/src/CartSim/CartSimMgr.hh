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
 * @file CartSimMgr.hh
 * @brief The manager layer for the CartSim algorithm.  Handles
 *        setting up of threads and running one volume per thread
 * @class CartSimMgr
 * @brief The manager layer for the CartSim algorithm.  Handles
 *        setting up of threads and running one volume per thread
 *        
 */

# ifndef    CARTSIM_MGR_H
# define    CARTSIM_MGR_H

#include "CartSim.hh"
#include "DataHandler.hh"
#include <toolsa/TaThreadDoubleQue.hh>

//----------------------------------------------------------------
class CartSimMgr
{
public:

  /**
   * Default constructor
   *
   * @param[in] p  The parameters to put into state
   *
   * Prepare the class for a call to the run() method
   */
  CartSimMgr(const Params &p);
  
  /**
   *  Destructor
   */
  virtual ~CartSimMgr(void);

  /**
   * Run the app for all times
   * @return 1 for bad, 0 for good
   */
  int run(void);

  /**
   * Compute method for threading a volume
   *
   * @param[in] info  Pointer to Info passed in and out of algorithm
   */
  static void compute(void *info);

  /**
   * Lock (typically for read/write from/to disk)
   */
  void lock(void);
  /**
   * UnLock (typically for read/write from/to disk)
   */
  void unlock(void);

  /**
   * @return a cloned copy of the data handler
   */
  DataHandler *cloneData(void) const;

protected:
private:  

  /**
   * @class CartSimhreads
   * @brief implement clone() method to have a TaThreadDoubleQue to use
   *        for threading.
   */
  class CartSimThreads : public TaThreadDoubleQue
  {
  public:
    /**
     * Trivial constructor
     */
    inline CartSimThreads(void) : TaThreadDoubleQue() {}
    /**
     * Trivial destructor
     */
    inline virtual ~CartSimThreads(void) {}

    /**
     * @return pointer to a TaThread created by the method
     * @param[in] index  Index value that might be used
     */
    TaThread *clone(int index);
  };



  /**
   * The Algorithm parameters, kept as internal state
   */
  Params _parms;

  /**
   * The Algorithm object
   */
  CartSim _alg;

  /**
   * The data input/output handling object (pointer as its a derived class)
   */
  DataHandler *_data;

  /**
   * Threading object
   */
  CartSimThreads _thread;

  /**
   * Seconds between processing steps
   */
  int _step_seconds;

  /**
   * Seconds total for entire simulation
   */
  int _max_seconds;
};

# endif 
