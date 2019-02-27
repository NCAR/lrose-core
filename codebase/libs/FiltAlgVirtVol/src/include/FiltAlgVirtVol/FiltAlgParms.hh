/**
 * @file FiltAlgParms.hh 
 * @brief Parameters, derived from AlgorithmParms and VirtVolParms
 * @class FiltAlgParms
 * @brief Parameters, derived from AlgorithmParms and VirtVolParms
 */

#ifndef FILTALG_PARMS_H
#define FILTALG_PARMS_H

#include <FiltAlgVirtVol/AlgorithmParms.hh>
#include <FiltAlgVirtVol/VirtVolParms.hh>
#include <tdrp/tdrp.h>
#include <vector>
#include <string>

//------------------------------------------------------------------
class FiltAlgParms  : public AlgorithmParms, public VirtVolParms
{
public:
  /**
   * Constructor empty.
   */
  FiltAlgParms(void);

  /**
   * Constructor from file name
   * @param[in] fileName  File to parse
   * @param[in] expandEnv  True to expand environment variables while reading
   *                       the file
   */
  FiltAlgParms(const std::string &fileName, bool expandEnv=true);

  /**
   * Destructor
   */
  virtual ~FiltAlgParms(void);

  
  /**
   * @return true if object well formed
   */
  inline bool isOk(void) const {return _ok;}


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
   * TDRP print the base class params, plus the one thing about operators
   */
  void printHelp(void);

  /**
   * Print the TDRP params
   * @param[in] mode TDRP printing mode
   */
  void printParams(tdrp_print_mode_t mode = PRINT_LONG);

  /**
   * @return true if command line arguments contain '-print_operators'
   * @param[in] argc
   * @param[in] argv
   */
  static bool isPrintOperators(int argc, char **argv);

  /**
   * @return true if command line arguments contain -params <filename>
   * @param[in] argc
   * @param[in] argv
   * @param[out] fileName  Set if true
   */
  static bool isSetParams(int argc, char **argv, std::string &fileName);

  /**
   * @return true if command line arguments contain -h, -help, -man
   * @param[in] argc
   * @param[in] argv
   */
  static bool isHelp(int argc, char **argv);
    

protected:
private:

  bool _ok;      /**< True if object well formed */

};

#endif
