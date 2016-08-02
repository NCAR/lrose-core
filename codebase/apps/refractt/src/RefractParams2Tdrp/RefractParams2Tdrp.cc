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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 18:17:27 $
//   $Id: RefractParams2Tdrp.cc,v 1.6 2016/03/07 18:17:27 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * RefractParams2Tdrp: RefractParams2Tdrp program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <math.h>
#include <signal.h>
#include <string>

#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "RefractParams2Tdrp.hh"

#include "RefractCalibParams.hh"
#include "RefractParams.hh"

using namespace std;

// Global variables

RefractParams2Tdrp *RefractParams2Tdrp::_instance =
     (RefractParams2Tdrp *)NULL;


/*********************************************************************
 * Constructor
 */

RefractParams2Tdrp::RefractParams2Tdrp(int argc, char **argv)
{
  static const string method_name = "RefractParams2Tdrp::RefractParams2Tdrp()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (RefractParams2Tdrp *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);

}


/*********************************************************************
 * Destructor
 */

RefractParams2Tdrp::~RefractParams2Tdrp()
{
  // Free contained objects

  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

RefractParams2Tdrp *RefractParams2Tdrp::Inst(int argc, char **argv)
{
  if (_instance == (RefractParams2Tdrp *)NULL)
    new RefractParams2Tdrp(argc, argv);
  
  return(_instance);
}

RefractParams2Tdrp *RefractParams2Tdrp::Inst()
{
  assert(_instance != (RefractParams2Tdrp *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run() - run the program.
 */

void RefractParams2Tdrp::run()
{
  static const string method_name = "RefractParams2Tdrp::run()";
  
  if (_args->nParamFileName == "")
  {
    cerr << "ERROR: " <<method_name << endl;
    cerr << "You must specify the name of the original parameter file on the" << endl;
    cerr << "command line using the -n_params option" << endl;
    
    return;
  }
  
  if (_args->refractParamFileName == "" &&
      _args->refractCalibParamFileName == "")
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "You must specify a Refract and/or a RefractCalib parameter file path" << endl;
    cerr << "on the command line using the -refract_params and/or -refract_calib_params" << endl;
    cerr << "command line options" << endl;
    
    return;
  }
  
  _processParams(_args->nParamFileName);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _getKeyVal() - Get the keyword value from the parameter file line.
 */

void RefractParams2Tdrp::_getKeyVal(char *line, string &keyword, string &value)
{
  int i;   // Index into parameter file line
  
  // Initialize the return values

  keyword = "";
  value = "";
  
  // Skip over white space

  for (i = 0; line[i] && isspace(line[i]); ++i);

  // Transfer nonwhitespaces from line to keyword

  for (; line[i] && !isspace(line[i]); ++i)
    keyword += line[i];

  // Skip over white space

  for (; line[i] && isspace(line[i]); ++i) ;

  // Transfer all remaining characters until whitespace or # to value
  // (unless in quotes)

  bool in_quotes = false;

  if (line[i] == '"')
  {
    in_quotes = true;
    ++i;
  }
  
  for ( ; line[i]; ++i)
  {
    // Finish if we are at the end of a quote

    if (in_quotes && line[i] == '"')
      break;
    
    // Finish if we are not in a quote and we reach a space or #

    if (!in_quotes && (isspace(line[i]) || line[i] == '#'))
      break;
    
    value += line[i];
  }

}


/*********************************************************************
 * _processParams() - Read the n_xtract-style parameter file.
 *
 * Returns true on success, false on failure.
 */

bool RefractParams2Tdrp::_processParams(const string &params_file)
{
  static const string method_name = "RefractParams2Tdrp::_processParams()";
  
  char	line[PATH_MAX] ;

  // Declare some variables we will need after parsing all of the parameters

  string n_out_num_azim = "";
  string n_out_num_ranges = "";
  string dn_out_num_azim = "";
  string dn_out_num_ranges = "";
  
  // Initialize the TDRP parameters

  RefractParams refract_tdrp_params;
  refract_tdrp_params.loadDefaults(false);
  string refract_tdrp_buffer = "";
  
  RefractCalibParams refract_calib_tdrp_params;
  refract_calib_tdrp_params.loadDefaults(false);
  string refract_calib_tdrp_buffer = "";
  
  // Open file, read data line by line

  FILE *fp_param = fopen(params_file.c_str(), "r");
  if (!fp_param)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot open parameter file: " << params_file << endl;

    return false;
  }

  int linenum = 0;

  while (fgets(line, PATH_MAX, fp_param) != NULL)
  {
    ++linenum;

    string keyword;
    string value;
    
    _getKeyVal(line, keyword, value);

    // Skip blank lines and comments

    if (keyword.length() == 0 || keyword[0] == '#')
      continue;
    
    // Set the appropriate parameter

    if (keyword == "NameOfRun")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "Author")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "ProjectName")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "Latitude")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "Longitude")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "AvTargetHeight")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "DataVersion")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "SubVersion")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "DebugLevel")
    {
      int debug_level = atoi(value.c_str());
      
      if (debug_level <= 0)
      {
	refract_tdrp_buffer += "debug_level = DEBUG_OFF;\n";
	refract_calib_tdrp_buffer += "debug_level = DEBUG_OFF;\n";
      }
      else if (debug_level == 1)
      {
	refract_tdrp_buffer += "debug_level = DEBUG_NORM;\n";
	refract_calib_tdrp_buffer += "debug_level = DEBUG_NORM;\n";
      }
      else if (debug_level == 2)
      {
	refract_tdrp_buffer += "debug_level = DEBUG_EXTRA;\n";
	refract_calib_tdrp_buffer += "debug_level = DEBUG_EXTRA;\n";
      }
      else
      {
	refract_tdrp_buffer += "debug_level = DEBUG_VERBOSE;\n";
	refract_calib_tdrp_buffer += "debug_level = DEBUG_VERBOSE;\n";
      }
    }
    else if (keyword == "DebugOutputFile")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "RealTimeMode")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "RTDataFileName")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "ResListFile")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "ResFirstFile")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "ResLastFile")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "RefFileName")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "FontName")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "DestinationPath")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "StatFileName")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "UseCircularQ")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "LatestIQ")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "LatestNPolar")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "LatestNCart")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "LatestDNPolar")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "LatestDNCart")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "McGillOutputN")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "TmpFile")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "NumAzim")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "NumRangeBins")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "IQPerAngle")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "GateSpacing")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "Frequency")
    {
      refract_tdrp_buffer += "frequency = " + value + ";\n";
    }
    else if (keyword == "PRF")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "FirstRange")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "RMin")
    {
      refract_tdrp_buffer += "r_min = " + value + ";\n";
      refract_calib_tdrp_buffer += "r_min = " + value + ";\n";
    }
    else if (keyword == "AzimFluct")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "SwitchEndianIn")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "BeamWidth")
    {
      refract_calib_tdrp_buffer += "beam_width = " + value + ";\n";
    }
    else if (keyword == "NOutNumAzim")
    {
      n_out_num_azim = value;
    }
    else if (keyword == "NOutNumRanges")
    {
      n_out_num_ranges = value;
    }
    else if (keyword == "NSmoothingSideLen")
    {
      refract_tdrp_buffer += "n_smoothing_side_len = " + value + ";\n";
    }
    else if (keyword == "NErrorThresh")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "DoMapDiff")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "NumMapDiff")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "DNOutNumAzim")
    {
      dn_out_num_azim = value;
    }
    else if (keyword == "DNOutNumRanges")
    {
      dn_out_num_ranges = value;
    }
    else if (keyword == "DNSmoothingSideLen")
    {
      refract_tdrp_buffer += "dn_smoothing_side_len = " + value + ";\n";
    }
    else if (keyword == "DNErrorThresh")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "MinConsistency")
    {
      refract_tdrp_buffer += "min_consistency = " + value + ";\n";
    }
    else if (keyword == "DoCartesianN")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "CartesianX")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "CartesianY")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "CartesianResol")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "SwitchEndianOut")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "DoRelax")
    {
      if (_parseBoolean(value))
	refract_tdrp_buffer += "do_relax = true;\n";
      else
	refract_tdrp_buffer += "do_relax = false;\n";
    }
    else if (keyword == "MinNTrace")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "MaxNTrace")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "DurationHistory")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "MinNDisplay")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "MaxNDisplay")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "AdaptDisplay")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "NumColors")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "GeogOverlay")
    {
      _printUnusedParamWarning(keyword);
    }
    else if (keyword == "SideLobePow")
    {
      refract_calib_tdrp_buffer += "side_lobe_pow = " + value + ";\n";
    }
    else
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Unrecognized keyword \"" << keyword << "\" in line "
	   << linenum << endl;
    }
  }
    
  fclose(fp_param);

  // Set the output dimensions

  if (n_out_num_azim != "" || dn_out_num_azim != "")
  {
    if (n_out_num_azim == "")
    {
      refract_tdrp_buffer += "num_azim = " + dn_out_num_azim + ";\n";
      refract_calib_tdrp_buffer += "num_azim = " + dn_out_num_azim + ";\n";
    }
    else if (dn_out_num_azim == "")
    {
      refract_tdrp_buffer += "num_azim = " + n_out_num_azim + ";\n";
      refract_calib_tdrp_buffer += "num_azim = " + n_out_num_azim + ";\n";
    }
    else if (n_out_num_azim == dn_out_num_azim)
    {
      refract_tdrp_buffer += "num_azim = " + n_out_num_azim + ";\n";
      refract_calib_tdrp_buffer += "num_azim = " + n_out_num_azim + ";\n";
    }
    else
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Different NOutNumAzim and DnOutNumAzim values specified in param file" << endl;
      cerr << "Using the value for NOutNumAzim for output" << endl;
      
      refract_tdrp_buffer += "num_azim = " + n_out_num_azim + ";\n";
      refract_calib_tdrp_buffer += "num_azim = " + n_out_num_azim + ";\n";
    }
  }
  
  if (n_out_num_ranges != "" || dn_out_num_ranges != "")
  {
    if (n_out_num_ranges == "")
    {
      refract_tdrp_buffer += "num_range_bins = " + dn_out_num_ranges + ";\n";
      refract_calib_tdrp_buffer += "num_range_bins = " + dn_out_num_ranges + ";\n";
    }
    else if (dn_out_num_ranges == "")
    {
      refract_tdrp_buffer += "num_range_bins = " + n_out_num_ranges + ";\n";
      refract_calib_tdrp_buffer += "num_range_bins = " + n_out_num_ranges + ";\n";
    }
    else if (n_out_num_ranges == dn_out_num_ranges)
    {
      refract_tdrp_buffer += "num_range_bins = " + n_out_num_ranges + ";\n";
      refract_calib_tdrp_buffer += "num_range_bins = " + n_out_num_ranges + ";\n";
    }
    else
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Different NOutNumRanges and DnOutNumRanges values specified in param file" << endl;
      cerr << "Using the value for NOutNumRanges for output" << endl;
      
      refract_tdrp_buffer += "num_range_bins = " + n_out_num_ranges + ";\n";
      refract_calib_tdrp_buffer += "num_range_bins = " + n_out_num_ranges + ";\n";
    }
  }
  
  // Print the TDRP parameter files

  if (_args->refractParamFileName != "")
    _writeTdrpFile(refract_tdrp_params, refract_tdrp_buffer,
		   _args->refractParamFileName);
  
  if (_args->refractCalibParamFileName != "")
    _writeTdrpFile(refract_calib_tdrp_params, refract_calib_tdrp_buffer,
		   _args->refractCalibParamFileName);
  
  return true;
}
