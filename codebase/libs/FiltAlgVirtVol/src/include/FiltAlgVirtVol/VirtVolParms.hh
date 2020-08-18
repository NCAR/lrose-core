/**
 * @file VirtVolParms.hh 
 * @brief Parameters, derived from VirtVolParams
 * @class VirtVolParms
 * @brief Parameters, derived from VirtVolParams
 */

#ifndef VIRT_VOL_PARMS_H
#define VIRT_VOL_PARMS_H

#include <FiltAlgVirtVol/VirtVolParams.hh>
#include <FiltAlgVirtVol/UrlParms.hh>
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
   * @return string URL to match a particular output field, empty
   * string for no match
   * @param[in] fieldName  Name to try and match
   */
  std::string matchingOutputUrl(const std::string &fieldName) const;

  /**
   * @return true if the field is an output field in some url
   * @param[in] fieldName
   */
  bool hasOutputField(const std::string &fieldName) const;

  std::vector<UrlParms> _inputUrl;  /**< Input URL specifications */
  std::vector<UrlParms> _outputUrl; /**< Output URL specifications */

protected:
private:

  bool _ok;      /**< True if object well formed */

  bool _init(void);
  bool _isInput(const std::string &name) const;
  bool _isOutput(const std::string &name) const;

};

#endif
