/**
 * @file RadxModelQc.hh 
 * @class RadxModelQc
 */

#ifndef RadxModelQc_H
#define RadxModelQc_H

#include "Parms.hh"
#include <FiltAlgVirtVol/Algorithm.hh>

class VolumeData;
class RayData;
class RadxApp;

class RadxModelQc
{
public:

  RadxModelQc(void);

  /**
   * Constructor
   * @param[in] parms
   * @param[in] cleanExit  Method to call for exiting program
   */
  RadxModelQc(const Parms &parms, void cleanExit(int));

  /**
   * Destructor
   */
  virtual ~RadxModelQc (void);

  /**
   * True if object is well formed
   */
  inline bool ok(void) const {return _ok;}

  void printOperators(void) const;

  /**
   * @return reference to the parameters
   */
  inline const Parms &getParms(void) const {return _parms;}

  /**
   * @return a pointer to the Algorithm object
   */
  inline const RadxApp *getAlgorithm(void) const {return _alg;}
  
  /**
   * @return a pointer to the Algorithm object
   */
  inline RadxApp *getAlgorithm(void) {return _alg;}
  
  /**
   * Run the RadxModelQc algorithm on a volume
   * @param[in] inputs  Input data
   * @param[out] outputs Output data
   * @return true for success, false for error 
   */
  bool run(RayData *inputs);

  bool write(RayData *data);

protected:
private:

  bool _ok;            /**< True if object well formed */
  Parms _parms;   /**< Parameters */
  RadxApp *_alg;     /**< Algorithm pointer, pointer due to threading */
};

#endif
