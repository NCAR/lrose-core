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
#include <FiltAlgVirtVol/UrlParms.hh>
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
   * param[in] expandEnv  True to expand $() things to values
   */
  Parms(const std::string &parmFileName, bool expandEnv=true);
  
  /**
   * Destructor
   */
  virtual ~Parms(void);

  /**
   * Print (tdrp print) the params
   * @param[in] printMode
   */
  void printParams(tdrp_print_mode_t printMode);

  /**
   * Print the app help
   */
  void printHelp(void);


  #include <FiltAlgVirtVol/AlgorithmParmsVirtualFunctions.hh>

  /**
   * Params for the 2 dimensional output
   */
  UrlParms _output2dUrl;

protected:
private:  

};

# endif
