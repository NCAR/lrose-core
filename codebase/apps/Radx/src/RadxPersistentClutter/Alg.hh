/**
 * @file Alg.hh 
 * @class Alg
 */

#ifndef Alg_H
#define Alg_H

#include "Parms.hh"
#include <radar/RadxApp.hh>

class Alg
{
public:

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

  void printOperators(void) const;

  /**
   * @return reference to the parameters
   */
  inline const Parms &getParms(void) const {return _parms;}

protected:
private:

  bool _ok;            /**< True if object well formed */
  Parms _parms;   /**< Parameters */
  RadxApp *_alg;     /**< Algorithm pointer, pointer due to threading */

};

#endif
