// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*********************************************************************
 * Args.cc: class controlling the command line arguments for the
 *          program.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <string.h>

#include <toolsa/os_config.h>
#include <tdrp/tdrp.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Args.hh"
#include "Params.hh"
/*using namespace std;*/

/**********************************************************************
 * Constructor
 */

Args::Args (int argc, char **argv, char *prog_name)
{
  char tmp_str[BUFSIZ];

  // Intialize

  okay = true;

  _nFiles = 0;
  _printSummary = false;
  _printSections = false;
  _printVarList = false;

  //*_fileList = NULL;
  
  TDRP_init_override(&override);
  
  // search for command options
  
  for (int i =  1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
        STRequal_exact(argv[i], "-help") ||
        STRequal_exact(argv[i], "-h") ||
        STRequal_exact(argv[i], "-man"))
    {
      _usage(prog_name, stdout);
      exit(0);
    }
    else if (STRequal_exact(argv[i], "-d") ||
             STRequal_exact(argv[i], "-debug"))
    {
      sprintf(tmp_str, "debug = 1;");
      TDRP_add_override(&override, tmp_str);
    }
    else if (STRequal_exact(argv[i], "-v") ||
             STRequal_exact(argv[i], "-verbose"))
    {
      sprintf(tmp_str, "debug = 2;");
      TDRP_add_override(&override, tmp_str);
    }
    else if (STRequal_exact(argv[i], "-printSummary"))
    {
      _printSummary = true;
      if (i < argc - 1) {

        if( argv[i+1][0] != '-') {
          _nFiles = 1;
          _fileList = argv + i + 1;
        }
      }
 
    }
    else if (STRequal_exact(argv[i], "-printVarList"))
    {
      _printVarList = true;
      if (i < argc - 1) {

        if( argv[i+1][0] != '-') {
          _nFiles = 1;
          _fileList = argv + i + 1;
        }
      }
 
    }
    else if(STRequal_exact(argv[i], "-o_f")) {
      if (i < argc - 1) {
        
        sprintf(tmp_str, "write_forecast = TRUE;");
        TDRP_add_override(&override, tmp_str);

        sprintf(tmp_str, "write_non_forecast = FALSE;");
        TDRP_add_override(&override, tmp_str);

        sprintf(tmp_str, "forecast_mdv_url = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);

      }
      else {
        cerr << "User must provide forecast output url with -o_f option" << endl;
        
        okay = false;
      }
    }
    else if(STRequal_exact(argv[i], "-o_n")) {
      if (i < argc - 1) {
        sprintf(tmp_str, "write_forecast = FALSE;");
        TDRP_add_override(&override, tmp_str);

        sprintf(tmp_str, "write_non_forecast = TRUE;");
        TDRP_add_override(&override, tmp_str);

        sprintf(tmp_str, "non_forecast_mdv_url = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      }
      else {
        cerr << "User must provide non-forecast output url with -o_n option" << endl;
        
        okay = false;
      }
    }
    else if (STRequal_exact(argv[i], "-printSections"))
    {
      _printSections = true;
      if (i < argc - 1) {

        if( argv[i+1][0] != '-') {
          _nFiles = 1;
          _fileList = argv + i + 1;
        }
      }
 
    }
    else if (STRequal_exact(argv[i], "-file") || STRequal_exact(argv[i], "-f"))
    {
      if (i < argc - 1) {

        int j;
        for( j = i+1; j < argc; j++ ) {
          if( argv[j][0] == '-')
            break;
        }
        _nFiles = j - i - 1;
        _fileList = argv + i + 1;
      }
 
    } else if (STRequal_exact(argv[i], "-writeLdataInfo")) {
      
      if (i < argc - 1) {

        sprintf(tmp_str, "writeLdataInfo = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
        i=i+1; if (i>argc) continue;
      } else {
        okay = false;
      }
      
    }
  } /* i */

  if (_printSummary && _nFiles != 1)
    okay = FALSE;

  if (_printSections && _nFiles != 1)
    okay = FALSE;

  if (!okay)
  {
    _usage(prog_name, stderr);
  }
    
}


/**********************************************************************
 * Destructor
 */

Args::~Args(void)
{
  TDRP_free_override(&override);
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/


/**********************************************************************
 * _usage() - Print the usage for this program.
 */

void Args::_usage(char *prog_name, FILE *stream)
{
  fprintf(stream, "%s%s%s",
          "Usage:\n\n", prog_name, " [options] as below:\n\n"
          "       [ --, -help, -h, -man ] produce this list.\n"
          "       [ -d, -debug ] debugging on\n"
          "       [ -printVarList ] file - print a list of variables in the grib file \n"
          "       [ -printSummary ] file - print a summary of fields in the grib file \n"
          "       [ -printSections ] file - print the sections in the grib file (can be large)\n"
          "       [ -file | -f ] filelist - for processing particular Grib files\n"
          "       [ -writeLdataInfo ? ] write LdataInfo files for output data\n"
          "       [ -o_f url] forecast output url\n"
          "       [ -o_n url] non-forecast output url\n"
          "       [ -v, -verbose ] verbose debugging on\n"
          "\n");


  TDRP_usage(stream);
}






