/**
 * @file RadxAppParms.hh 
 * @brief Parameters, derived from RadxAppParams
 * @class RadxAppParms
 * @brief Parameters, derived from RadxAppParams
 */

#ifndef RADX_APP_PARMS_H
#define RADX_APP_PARMS_H

#include <radar/RadxAppParams.hh>
#include <radar/RadxAppConfig.hh>
#include <tdrp/tdrp.h>
#include <vector>
#include <string>

//------------------------------------------------------------------
class RadxAppParms  : public RadxAppParams
{
public:
  /**
   * Constructor empty.
   */
  RadxAppParms(void);

  /**
   * Construct from command line arguments
   * @param[in] argc
   * @param[in] argv
   */
  RadxAppParms(int argc, char **argv);
  
  /**
   * Constructor from file name
   * @param[in] fileName
   * @param[in] expandEnv  True to expand environment variables when reading
   */
  RadxAppParms(const std::string &fileName, bool expandEnv);

  /**
   * Constructor from file name
   * @param[in] fileName
   * @param[in] files  File List mode files to process
   * @param[in] expandEnv  True to expand environment variables when reading
   */
  RadxAppParms(const std::string &fileName,
	       const std::vector<std::string> &files, bool expandEnv);

  /**
   * Destructor
   */
  virtual ~RadxAppParms(void);

  /**
   * @return true if object well formed
   */
  inline bool isOk(void) const {return _ok;}

  /**
   * TDRP print the base class params
   * @param[in] mode
   */
  void printParams(tdrp_print_mode_t mode = PRINT_LONG);

  /**
   * TDRP print the base class params, plus the one thing about operators
   */
  void printHelp(void);

  /**
   * @return true if input string is a fixed constant 
   * @param[in] s
   */
  bool matchesFixedConst(const std::string &s) const;

  #define RADX_APP_PARMS_BASE
  #include <radar/RadxAppParmsVirtualMethods.hh>
  #undef RADX_APP_PARMS_BASE


  /**
   * Look at command line args and decide if it is a -print_params or
   * --print_params
   * @return true if it is a print_params
   * @param[in] argc
   * @param[in] argv
   * @param[out] printMode  Set on return if true
   * @param[out] expandEnv  Set on return if true
   */
  static bool isPrintParams(int argc, char **argv,
			    tdrp_print_mode_t &printMode, int &expandEnv);

  /**
   * @return true if command line arguments contain '-print_operators'
   * @param[in] argc
   * @param[in] argv
   */
  static bool isPrintOperators(int argc, char **argv);

  /**
   * @return true if command line arguments contain -h, -help, -man
   * @param[in] argc
   * @param[in] argv
   */
  static bool isHelp(int argc, char **argv);
    
  /**
   * @return true if command line arguments contain '-f' or '-path'
   * @param[in] argc
   * @param[in] argv
   * @param[out] files  List of args after '-f'/'-path' if return is true
   */
  static bool isFileList(int argc, char **argv,
			 std::vector<std::string> &files);

  /**
   * @return true if command line arguments contain -params <filename>
   * @param[in] argc
   * @param[in] argv
   * @param[out] fileName  Set if true
   */
  static bool isSetParams(int argc, char **argv, std::string &fileName);


  RadxAppConfig _inputs;  /**< Class that handles multiple input sources */
  bool _isFileList;       /**< True if the input data is a list of files */
  std::vector<std::string> _fileList; /**< If _isFileList this is the list */

  bool _outputAllFields;  /**< True to output all fields */
  std::vector<std::string> _outputFieldList; /**< If not _outputAllFields, these
					      * are the fields to output */

  /**
   * app parameters, fixed constants, set by virtual method
   */
  std::vector<std::string> _fixedConstants;

  /**
   * app parameters, user data strings, set by virtual method
   */
  std::vector<std::string> _userData;

  /**
   * app parameters, strings for filters over entire volume done before
   * the sweep filters, set by virtual method
   */
  std::vector<std::string> _volumeBeforeFilters;

  /**
   * app parameters, sweep filter strings, set by virtual method
   */
  std::vector<std::string> _sweepFilters;

  /**
   * app parameters, ray filter strings, set by virtual method
   */
  std::vector<std::string> _rayFilters;

  /**
   * app parameters, strings for filters over entire volume done after 
   * the sweep filters, set by virtual method
   */
  std::vector<std::string> _volumeAfterFilters;




protected:
private:

  bool _ok;      /**< True if object well formed */

};

#endif
