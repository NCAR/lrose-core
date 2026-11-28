//=============================================================================
//
//  (c) Copyright, 2008 University Corporation for Atmospheric Research (UCAR).
//      All rights reserved. 
//
//      File: $RCSfile: Args.hh,v $
//      Version: $Revision: 1.2 $  Dated: $Date: 2015/11/12 22:16:06 $
//
//=============================================================================

/**
 *
 * @file Args.hh
 *
 * @class Args
 *
 * Class controlling the command line arguments for this program.
 *  
 * @date 10/31/2008
 *
 */

#ifndef Args_HH
#define Args_HH

#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>

#include <tdrp/tdrp.h>

using namespace std;

/** 
 * @class Args
 */

class Args
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief TDRP overrides specified in the command line arguments.
   */

  tdrp_override_t override;
  

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   * @param[in] prog_name Program name.
   */

  Args(int argc, char **argv, char *prog_name);
  

  /**
   * @brief Destructor
   */

  ~Args(void);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the list of input files to process.
   *
   * @return Returns the list of files to process.
   */

  const vector< string > &getFileList(void)
  {
    return _inputFiles;
  }
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief The program name for error messages.
   */

  string _progName;

  /**
   * @brief The list of input files to process.
   */

  vector< string > _inputFiles;
  

  /////////////////////
  // Private methods //
  /////////////////////


  /**
   * @brief Print the usage for this program.
   *
   * @param[in,out] stream Stream to use for printing.
   */

  void _usage(FILE *stream);
  
};


#endif
