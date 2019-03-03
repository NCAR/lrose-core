/**
 * @file RadxModelQc.hh 
 * @brief  The algorithm class
 * @class RadxModelQc
 * @brief  The algorithm class
 */

#ifndef RadxModelQc_H
#define RadxModelQc_H

#include "Parms.hh"

class Volume;
class RadxApp;

class RadxModelQc
{
public:

  /**
   * The algorithm class constructor, empty
   */
  RadxModelQc(void);

  /**
   * Constructor with args
   * @param[in] parms  Parameters
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

  /**
   * Print the operators to stdout
   */
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
   *
   * @param[in,out] volume  Data with inputs, modified to include outputs
   * @return true for success, false for error 
   */
  bool run(Volume *volume);

  /**
   * Write a volume
   *
   * @param[in] volume 
   * @return true for success, false for error 
   */
  bool write(Volume *volume);

protected:
private:

  bool _ok;       /**< True if object well formed */
  Parms _parms;   /**< Parameters */
  RadxApp *_alg;  /**< Algorithm pointer, pointer due to threading */
};

#endif
