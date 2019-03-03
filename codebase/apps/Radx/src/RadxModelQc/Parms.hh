/**
 * @file Parms.hh
 * @brief all parms
 * @class Parms
 * @brief all parms
 */

# ifndef   PARMS_HH
# define   PARMS_HH

#include "Params.hh"
#include <radar/RadxAppParms.hh>
#include <string>

//------------------------------------------------------------------
class Parms : public RadxAppParms, public Params
{
public:

  /**
   * default constructor
   */
  Parms(void);

  /**
   * Constructor that reads a file
   * @param[in] parmFileName  Name of file to read
   * @param[in] expandEnv  True to expand environment variables in the params
   */
  Parms(const std::string &parmFileName, bool expandEnv);

  /**
   * Constructor that reads a file
   * @param[in] parmFileName  Name of file to read
   * @param[in] files  Names of Radx files to process
   * @param[in] expandEnv  True to expand environment variables in the params
   */
  Parms(const std::string &parmFileName, const std::vector<std::string> &files,
	bool expandEnv);

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
   * Print the usage, derived and base class
   */
  void printHelp(void);

  #include <radar/RadxAppParmsVirtualMethods.hh>

protected:
private:  

};

# endif
