/**
 * @file RadxMesoCyclone.hh 
 * @class RadxMesoCyclone
 */

#ifndef RadxMesoCyclone_H
#define RadxMesoCyclone_H

#include "Parms.hh"
#include <FiltAlgVirtVol/Algorithm.hh>

class VolumeData;

class RadxMesoCyclone
{
public:

  /**
   * Constructor empty
   */
  RadxMesoCyclone(void);

  /**
   * Constructor
   * @param[in] parms
   * @param[in] cleanExit  Method to call for exiting program
   */
  RadxMesoCyclone(const Parms &parms, void cleanExit(int));

  /**
   * Destructor
   */
  virtual ~RadxMesoCyclone (void);

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
   * Run the RadxMesoCyclone algorithm on a volume
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
