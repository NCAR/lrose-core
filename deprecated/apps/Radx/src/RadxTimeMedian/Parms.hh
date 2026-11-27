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
   * @param[in] expandEnv  True to expand environment vars while reading
   */
  Parms(const std::string &parmFileName, bool expandEnv);

  /**
   * Constructor that reads a file and processes a fixed list of files
   * @param[in] parmFileName  Name of file to read
   * @param[in] files  Files to process
   * @param[in] expandEnv  True to expand environment vars while reading
   */
  Parms(const std::string &parmFileName, const std::vector<std::string> &files,
	bool expandEnv);

  /**
   * Destructor
   */
  virtual ~Parms(void);

  /**
   * Print (tdrp print) the params
   */
  void printParams(tdrp_print_mode_t printMode);

  /**
   * Print usage and base class usage
   */
  void printHelp(void);

  /**
   * Base class virtual methods
   */
  #include <radar/RadxAppParmsVirtualMethods.hh>

  /**
   * Hardwired filter names for volumes
   */
  static const std::string _volInitStr;
  static const std::string _volFinishStr;

  /**
   * Hardwired filter names for rays
   */
  static const std::string _histoAccumStr;

  /**
   * Hardwrired user data names
   */
  static const std::string _initStatus;
  static const std::string _finishStatus;

protected:
private:  

};

# endif
