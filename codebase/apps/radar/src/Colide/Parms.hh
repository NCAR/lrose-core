/**
 * @file Parms.hh
 * @brief all parms, instantiates virtual methods in the AlgorithmParms class
 * @class Parms
 * @brief all parms, instantiates virtual methods in the AlgorithmParms class
 */

# ifndef   PARMS_HH
# define   PARMS_HH

#include "Params.hh"
#include <FiltAlgVirtVol/FiltAlgParms.hh>
#include <rapmath/FuzzyF.hh>
#include <tdrp/tdrp.h>
#include <string>

//------------------------------------------------------------------
class Parms : public FiltAlgParms, public Params
{
public:

  /**
   * default constructor
   */
  Parms(void);

  /**
   * Constructor that reads a file
   * @param[in] parmFileName  Name of file to read
   * @param[in] expandEnv  True to expand env variables when reading
   */
  Parms(const std::string &parmFileName, bool expandEnv=true);
  
  /**
   * Destructor
   */
  virtual ~Parms(void);

  /**
   * Print (tdrp print) the _main params and the algorithm params
   */
  void printParams(tdrp_print_mode_t printMode);

  /**
   * Print usage and base class useage
   */
  void printHelp(void);

  /**
   * Print the inputs and outputs to stdout (debugging)
   */
  void printInputOutputs(void) const;

  #include <FiltAlgVirtVol/AlgorithmParmsVirtualFunctions.hh>

  /**
   * Fuzzy functions for line detection
   */
  FuzzyF _lineDetectSide, _lineDetectCenter, _lineDetectStd;

  /**
   * Fuzzy function for shear detection
   */
  FuzzyF _shearDetectSide;

  /**
   * Fuzzy function for elliptical filter confidence
   */
  FuzzyF _ellipConf;

  /**
   * Fuzzy function for enhance filter 
   */
  FuzzyF _enhanceFuzzy;

  /**
   * Old data, pairs of field names and maximum seconds back
   */
  std::vector<std::pair<std::string, int> > _oldData;

protected:
private:  

};

# endif
