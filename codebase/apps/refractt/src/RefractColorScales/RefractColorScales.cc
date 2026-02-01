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
/////////////////////////////////////////////////////////////////////////////
// RefractColorScales
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2026
//
/////////////////////////////////////////////////////////////////////////////
//
// This creates CIDD-type colorscale files that matches the colors used by
// Frederic Fabry in his original n_viewcalib program
//
/////////////////////////////////////////////////////////////////////////////

#include "RefractColorScales.hh"
#include "toolsa/ucopyright.h"

//---------------------------------------------------------------------

RefractColorScales::RefractColorScales(int argc, char **argv)

{

  // Initialize the okay flag.
  
  okay = true;
  
  // Set the program name.
  
  _progName = "RefractColorScales";
  
  // Display ucopyright message.

  ucopyright(_progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    okay = false;
    return;
  }

}

//---------------------------------------------------------------------
RefractColorScales::~RefractColorScales()
{

}

//---------------------------------------------------------------------
int RefractColorScales::run()
{

  if (_createStrengthColorscale()) {
    return -1;
  }

  if (_createQualityColorscale()) {
    return -1;
  }

  return 0;

}
//---------------------------------------------------------------------

int RefractColorScales::_createStrengthColorscale()
{

  // compute color scale path

  string path(_args.output_dir);
  path += "/";
  path += "refract_strength.colors";

  // open file
  
  FILE *file;
  if ((file = fopen(path.c_str(), "w")) == 0) {
    cerr << "Error opening strength colorscale file for output" << endl;
    perror(path.c_str());
    return -1;
  }
  
  for (int i = 0; i < 120; ++i) {

    double step = 6.0 * ((double)i / 120.0);
    
    short int red, green, blue;

    red = (short int)(step * 255.0 / 6.0);
    green = red;
    blue = red;
    
    double min_value = ((80.0 / 120.0) * (double)i) - 10.0;
    double max_value = ((80.0 / 120.0) * (double)(i+1)) - 10.0;
    
    fprintf(file, "%8.7f  %8.7f   #%02x%02x%02x\n",
	    min_value, max_value, red, green, blue);
    
  }
  
  fclose(file);

  cerr << "Wrote strength colorscale file: " << path << endl;

  return 0;
  
}

//---------------------------------------------------------------------
int RefractColorScales::_createQualityColorscale()
{

  // compute color scale path

  string path(_args.output_dir);
  path += "/";
  path += "refract_quality.colors";

  // open file
  
  FILE *file;
  if ((file = fopen(path.c_str(), "w")) == 0) {
    cerr << "Error opening quality colorscale file for output" << endl;
    perror(path.c_str());
    return -1;
  }
  
  for (int i = 0; i < 120; ++i) {
    
    double step = 6.0 * ((double)i / 120.0);
    
    short int red, green, blue;

    red = (short int)(step * 255.0 / 6.0);
    green = red;
    blue = red;
    
    double min_value = ((1.0 / 120.0) * (double)i);
    double max_value = ((1.0 / 120.0) * (double)(i+1));
    
    fprintf(file, "%8.7f  %8.7f   #%02x%02x%02x\n",
	    min_value, max_value, red, green, blue);
  }
  
  fclose(file);

  cerr << "Wrote quality colorscale file: " << path << endl;

  return 0;

}

