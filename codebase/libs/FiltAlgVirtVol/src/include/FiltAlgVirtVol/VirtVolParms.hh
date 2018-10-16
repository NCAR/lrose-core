/**
 * @file VirtVolParms.hh 
 * @brief Parameters, derived from VirtVolParams
 * @class VirtVolParms
 * @brief Parameters, derived from VirtVolParams
 */

#ifndef VIRT_VOL_PARMS_H
#define VIRT_VOL_PARMS_H

#include <FiltAlgVirtVol/VirtVolParams.hh>
#include <FiltAlgVirtVol/UrlSpec.hh>
#include <tdrp/tdrp.h>
#include <vector>
#include <string>

class Algorithm;

//------------------------------------------------------------------
class VirtVolParms  : public VirtVolParams
{
public:
  /**
   * Constructor empty.
   */
  VirtVolParms(void);

  /**
   * Constructor..sets base class to input
   *
   * @param[in] p  Base class values to use
   */
  VirtVolParms(const VirtVolParams &p);

  /**
   * Destructor
   */
  virtual ~VirtVolParms(void);

  /**
   * @return true if object well formed
   */
  inline bool isOk(void) const {return _ok;}

  /**
   * Set base values to input, then initialize derived values
   * @param[in] v  Base class object
   */
  inline void set(const VirtVolParams &v)
  {
    *((VirtVolParams *)this) = v;
    _init();
  }

  /**
   * TDRP print base values
   * @param[in] mode
   */
  void printParams(tdrp_print_mode_t mode = PRINT_LONG);

  /**
   * Compare Algorithm's inputs and outputs with local param settings,
   *
   * @param[in] A  The Algorithm object
   *
   * @return true for a 1 to 1 onto match
   */
  bool checkConsistency(const Algorithm &A) const;

  /**
   * Look through the inputs for a matching external name,
   * and convert to the associated internal name
   *
   * @param[in] externalName
   * @param[out] internalName  
   *
   * @return true if name was filled in, false if no externalName found
   */
  bool inputExternal2InternalName(const std::string externalName,
  				  std::string &internalName) const;

  /**
   * Look through the outputs for a matching internal name,
   * and convert to the associated external name
   *
   * @param[in] internalName
   * @param[out] externalName  The external (output) name
   *
   * @return true if name was filled in, false if no internalName found
   */
  bool outputInternal2ExternalName(const std::string internalName,
  				   std::string &externalName) const;

  /**
   * @return string URL to match a particular output field, empty
   * string for no match
   * @param[in] internalFieldName  Name to try and match
   */
  std::string matchingOutputUrl(const std::string &internalFieldName) const;

  /**
   * @return a string for a particular URL type 
   * @param[in] t  The URL type
   */
  static std::string sprintUrl(VirtVolParams::Url_t t);

  /**
   * @return a string for a particular data type
   * @param[in] t  The data type
   */
  static std::string sprintData(VirtVolParams::Data_t t);


  std::vector<UrlSpec> _virtvol_inputs;     /**< Inputs to the app */
  std::vector<UrlSpec> _virtvol_outputs;    /**< Outputs from the app */

protected:
private:

  bool _ok;      /**< True if object well formed */

  bool _init(void);
  bool _isInput(const std::string &internalName) const;
  bool _isOutput(const std::string &internalName) const;

};

#endif
