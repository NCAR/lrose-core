/**
 * @file Alg.hh 
 * @class Alg
 */

#ifndef Alg_H
#define Alg_H

#include "Parms.hh"
#include "Volume.hh"
#include "RadxTimeMedian.hh"
#include <radar/RadxApp.hh>

class Alg
{
public:

  /**
   * Empty
   */
  Alg(void);

  /**
   * Constructor
   * @param[in] parms
   * @param[in] cleanExit  Method to call for exiting program
   */
  Alg(const Parms &parms, void cleanExit(int));

  /**
   * Destructor
   */
  virtual ~Alg (void);

  /**
   * True if object is well formed
   */
  inline bool ok(void) const {return _ok;}

  /**
   * Print all possible operators
   */
  void printOperators(void) const;

  /**
   * @return reference to the parameters
   */
  inline const Parms &getParms(void) const {return _parms;}

  /**
   * Process a volume
   * @param[in,out]  volume  The data
   */
  void processVolume(Volume *volume);

  /**
   * Process the last volume
   * @param[in,out]  volume  The data
   */
void processLast(Volume *volume);

protected:
private:

  bool _ok;          /**< True if object well formed */
  Parms _parms;      /**< Parameters */
  RadxApp *_alg;     /**< Algorithm pointer, pointer due to threading */
};

#endif
