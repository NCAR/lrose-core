/**
 * @file ParmEnhance.hh 
 * @brief Parms for Enhance filter
 * @class ParmEnhance
 * @brief Parms for Enhance filter
 */

#ifndef PARM_ENHANCE_H
#define PARM_ENHANCE_H

#include <rapmath/FuzzyF.hh>
#include "Window.hh"
#include <string>

//------------------------------------------------------------------
class ParmEnhance
{
public:
  /**
   * Constructor
   */
  inline ParmEnhance(void) {}

  /**
   * Destructor
   */
  inline virtual ~ParmEnhance(void) {}

  bool _allow_missing_side;  /**< True to allow missing values in template
			      * side*/
  std::string _out_orient;  /**< Name of output orientation data */
  Window _window;           /**< The window for the enhancement alg */
  FuzzyF _enhanceF;          /**< Fuzzy function for enhancement (interest
			     * mapped to interest) */
  int _numThread;          /**< Number of threads */

protected:
private:

};

#endif
