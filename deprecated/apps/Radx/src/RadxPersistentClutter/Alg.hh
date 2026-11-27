/**
 * @file Alg.hh 
 * @brief Parameters and RadxApp used to run the algorithm
 * @class Alg
 * @brief Parameters and RadxApp used to run the algorithm
 */

#ifndef Alg_H
#define Alg_H

#include "Parms.hh"
#include <radar/RadxApp.hh>

class Volume;

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
   * Print all the binary and unary operators supported by this app
   * (not all are implemented to do things)
   */
  void printOperators(void) const;

  /**
   * @return reference to the parameters
   */
  inline const Parms &getParms(void) const {return _parms;}

  /**
   * Run the RadxPersistentClutter algorithm on a volume
   *
   * @param[in,out] volume  Data with inputs, modified to include outputs
   * @return true for success, false for error 
   */
  bool run(Volume *volume);

  /**
   * Write a volume to the main output URL specified in the parameters
   *
   * @param[in] volume 
   * @return true for success, false for error 
   */
  bool write(Volume *volume);

  /**
   * Write a volume to a URL
   *
   * @param[in] volume 
   * @param[in] url
   * @return true for success, false for error 
   */
  bool write(Volume *volume, const std::string &url);

protected:
private:

  bool _ok;       /**< True if object well formed */
  Parms _parms;   /**< Parameters */
  RadxApp *_alg;  /**< Algorithm pointer, pointer due to threading */

};

#endif
