/**
 * @file AlgorithmParms.hh 
 * @brief Parameters, derived from AlgorithmParams.
 * @class AlgorithmParms
 * @brief Parameters, derived from AlgorithmParams.
 *
 * The derived class has a few methods, plus two pure virtual methods
 * that will implemented by the app.
 */

#ifndef ALGORITHM_PARMS_H
#define ALGORITHM_PARMS_H

#include <FiltAlgVirtVol/AlgorithmParams.hh>
#include <tdrp/tdrp.h>
#include <vector>
#include <string>


//------------------------------------------------------------------
class AlgorithmParms  : public AlgorithmParams
{
public:
  /**
   * Constructor empty.
   */
  AlgorithmParms(void);

  /**
   * Constructor..sets base class to input
   *
   * @param[in] p  Base class values to use
   */
  AlgorithmParms(const AlgorithmParams &p);

  /**
   * Destructor
   */
  virtual ~AlgorithmParms(void);

  /**
   * @return true if object well formed
   */
  inline bool isOk(void) const {return _ok;}

  /**
   * Set the base class to input, then set local members
   * @param[in] a  Base class object
   */
  void set(const AlgorithmParams &a);

  /**
   * Print TDRP base class to stdout
   * @param[in] mode
   */
  void printParams(tdrp_print_mode_t mode = PRINT_LONG);

  /**
   * @return true if input string is configured as a fixed constant
   * @param[in] s  
   */
  bool matchesFixedConst(const std::string &s) const;

  /**
   * Substitute fixed constants for values in al the filtering strings
   */
  void substituteFixedConst(void);
  
  /**
   * Add a fixed constant to the state, assumed of form conststring=valuestring
   * @param[in] item
   * @return true for success false for failure
   */
  bool addFixedConstant(const std::string &item);

  #define ALGORITHM_PARMS_BASE
  #include <FiltAlgVirtVol/AlgorithmParmsVirtualFunctions.hh>
  #undef ALGORITHM_PARMS_BASE

  /**
   * app parameters, fixed constants, set by virtual method setFiltersFromParms
   * subsituted into the filters
   */
  std::vector<std::pair<std::string, std::string> > _fixedConstants;

  /**
   * The fixed constant strings
   */
  std::vector<std::string> _fixedConstantNames;

  /**
   * app parameters, user data strings, set by virtual method setFiltersFromParms
   */
  std::vector<std::string> _userData;

  /**
   * app parameters, strings for filters over entire volume done before
   * the sweep filters, set by virtual method setFiltersFromParms
   */
  std::vector<std::string> _volumeBeforeFilters;

  /**
   * app parameters, sweep filter strings, set by virtual method setFiltersFromParms
   */
  std::vector<std::string> _sweepFilters;

  /**
   * app parameters, strings for filters over entire volume done after 
   * the sweep filters, set by virtual method setFiltersFromParms
   */
  std::vector<std::string> _volumeAfterFilters;

protected:
private:

  bool _ok;      /**< True if object well formed */

  void _subsituteFixedConst(std::string &filterStr);

};

#endif
