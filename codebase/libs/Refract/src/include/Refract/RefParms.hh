/**
 * @file RefParms.hh 
 * @brief Parameters, derived from RefParams.
 * @class RefParms
 * @brief Parameters, derived from RefParams.
 *
 * The derived class has a few methods, plus two pure virtual methods
 * that will implemented by the app.
 */

#ifndef ALGORITHM_PARMS_H
#define ALGORITHM_PARMS_H

#include <Refract/RefParams.hh>
#include <vector>
#include <string>

class DsTrigger;
class RefractInput;

//------------------------------------------------------------------
class RefParms  : public RefParams
{
public:
  /**
   * Constructor empty.
   */
  RefParms(void);

  /**
   * Constructor..sets base class to input
   *
   * @param[in] p  Base class values to use
   */
  RefParms(const RefParams &p);

  /**
   * Constructor, reads in params from file to set base class
   * @param[in] fname  File
   */
  RefParms(const std::string &fname);

  /**
   * Destructor
   */
  virtual ~RefParms(void);

  /**
   * @return true if object well formed
   */
  inline bool isOk(void) const {return _ok;}

  /**
   * Set object to be well formed
   */
  inline void setOk(void) {_ok = true;}

  /**
   * Create and return a pointer to DsTrigger that is owned by caller, using
   * local params to set it up
   * @param[out] trigger
   * @return true if successful
   */
  bool initTrigger(DsTrigger **trigger) const;

  /**
   * Create and return pointer to RefractInput using local values
   * @return  Pointer
   */
  RefractInput *initInput(void) const;

  /**
   * Initialize using inputs
   * @param[in] argc
   * @param[in] argv
   * @return true for success
   */
  static bool parmInit(int argc, char **argv);

  /**
   * Finish parameter setting actions
   * @param[in] p  parameters
   */
  static void parmFinish(const RefParams &p);

  /**
   * @return param loading flag
   */
   static bool isParmLoad(void);

  /**
   * @return param print flag
   */
  static bool isParmPrint(void);

  /**
   * @return param path
   */
  static std::string parmPath(void);

  /**
   * @return true if -print_params
   */
  static bool isPrintMode(void);

  /**
   * @return true if -print_params and -params
   */
  static bool isPrintAndLoadMode(void);
    

protected:
private:

  bool _ok;      /**< True if object well formed */


};

#endif
