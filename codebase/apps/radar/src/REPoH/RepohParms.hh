/**
 * @file RepohParms.hh
 * @brief all parms, instantiates virtual methods in the AlgorithmParms class
 * @class RepohParms
 * @brief all parms, instantiates virtual methods in the AlgorithmParms class
 */

# ifndef   REPOH_PARMS_HH
# define   REPOH_PARMS_HH

#include "RepohParams.hh"
#include <FiltAlgVirtVol/AlgorithmParms.hh>
#include <string>

//------------------------------------------------------------------
class RepohParms : public AlgorithmParms
{
public:

  /**
   * default constructor
   */
  RepohParms(void);

  /**
   * Constructor that reads a file
   * @param[in] parmFileName  Name of file to read
   */
  RepohParms(const std::string &parmFileName);
  
  /**
   * Destructor
   */
  virtual ~RepohParms(void);

  /**
   * Print (tdrp print) the _main params and the algorithm params
   */
  void print(void);

  /**
   * Print the inputs and outputs to stdout (debugging)
   */
  void printInputOutputs(void) const;

  /**
   * The App parameters, public intentionally
   */
  RepohParams _main;

  // /**
  //  * Mapping from strings to app parameter file type
  //  * @param[in] name  Name to map to a type
  //  * @return the type
  //  */
  // static RepohParams::app_filter_t nameToType(const string &name);


  #include <FiltAlgVirtVol/AlgorithmParmsVirtualFunctions.hh>

protected:
private:  

};

# endif
