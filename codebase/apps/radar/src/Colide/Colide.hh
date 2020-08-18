/**
 * @file Colide.hh 
 * @brief  Algorithm object
 * @file Colide.hh 
 * @class  Algorithm object
 */

#ifndef COLIDE_H
#define COLIDE_H

#include "Parms.hh"
#include <FiltAlgVirtVol/Algorithm.hh>

class VolumeData;

class Colide
{
public:

  /**
   * Empty 
   */
  Colide(void);

  /**
   * Constructor
   * @param[in] parms  Parameters
   * @param[in] cleanExit  Method to call for exiting program
   */
  Colide(const Parms &parms, void cleanExit(int));

  /**
   * Destructor
   */
  virtual ~Colide (void);

  /**
   * True if object is well formed
   */
  inline bool ok(void) const {return _ok;}

  /**
   * @return reference to the parameters
   */
  inline const Parms &getParms(void) const {return _parms;}

  /**
   * @return a pointer to the Algorithm object
   */
  inline const Algorithm *getAlgorithm(void) const {return _alg;}
  
  /**
   * @return a pointer to the Algorithm object
   */
  inline Algorithm *getAlgorithm(void) {return _alg;}
  

  /**
   * Run the Colide algorithm on a volume
   * @param[in] inputs  Input data
   * @param[out] outputs Output data
   * @return true for success, false for error 
   */
  bool run(VolumeData *inputs);

protected:
private:

  bool _ok;            /**< True if object well formed */
  Parms _parms;        /**< Parameters */
  Algorithm *_alg;     /**< Algorithm pointer, pointer due to threading */
};

#endif
