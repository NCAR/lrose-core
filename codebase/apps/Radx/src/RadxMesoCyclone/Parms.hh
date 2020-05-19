/**
 * @file Parms.hh
 * @brief all parms
 * @class Parms
 * @brief all parms
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
   */
  Parms(const std::string &parmFileName, bool expandEnv=true);
  
  /**
   * Destructor
   */
  virtual ~Parms(void);

  /**
   * Print (tdrp print) the params
   */
  void printParams(tdrp_print_mode_t printMode);

  void printHelp(void);

  /**
   * Print the inputs and outputs to stdout (debugging)
   */
  void printInputOutputs(void) const;

  #include <FiltAlgVirtVol/AlgorithmParmsVirtualFunctions.hh>

  /**
   * Fuzzy functions for line detection
   */
  FuzzyF _detectSide;

  /**
   * Fuzzy function for nyquist velocity
   */
  FuzzyF _nyquistFuzzy;

  /**
   * Fuzzy function for radial length km
   */
  FuzzyF _radialFuzzy;

protected:
private:  

};

# endif
