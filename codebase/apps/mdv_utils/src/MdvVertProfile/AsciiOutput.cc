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
//   $Date: 2016/03/04 02:22:13 $
//   $Id: AsciiOutput.cc,v 1.2 2016/03/04 02:22:13 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * AsciiOutput: Class for outputing the profiles to an ASCII file.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/MdvxField.hh>
#include <toolsa/Path.hh>

#include "AsciiOutput.hh"

using namespace std;

/*********************************************************************
 * Constructor
 */

AsciiOutput::AsciiOutput(const string &output_dir,
			 const string &format_string,
			 const string &delimiter,
			 const bool debug_flag) :
  Output(output_dir, debug_flag),
  _formatString(format_string),
  _delimiter(delimiter)
{
}


/*********************************************************************
 * Destructor
 */

AsciiOutput::~AsciiOutput()
{
}


/*********************************************************************
 * writeProfiles() - Write the profiles to the appropriate output file.
 */

bool AsciiOutput::writeProfiles(const DateTime &data_time,
				const vector< ProfileSet > &profile_sets)
{
  static const string method_name = "AsciiOutput::writeProfiles()";
  
  // Create the output directory and open the output file

  char output_path_string[MAX_PATH_LEN];
  sprintf(output_path_string, "%s/%04d%02d%02d/%02d%02d%02d.txt",
	  _outputDir.c_str(),
	  data_time.getYear(), data_time.getMonth(), data_time.getDay(),
	  data_time.getHour(), data_time.getMin(), data_time.getSec());
  
  Path output_path(output_path_string);
  
  if (output_path.makeDirRecurse() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output directory: "
	 << output_path.getDirectory() << endl;
    
    return false;
  }
  
  FILE *output_file;
  
  if ((output_file = fopen(output_path_string, "w")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening output file for write: "
	 << output_path_string << endl;
    
    return false;
  }
  
  // Write the profiles

  vector< ProfileSet >::const_iterator set;
  
  for (set = profile_sets.begin(); set != profile_sets.end(); ++set)
  {
    double lat, lon;
    
    set->getLocation(lat, lon);
    
    fprintf(output_file, "# lat %f lon %f\n", lat, lon);
    
    const vector< VertProfile > profiles = set->getProfiles();
    vector< VertProfile >::const_iterator profile;
    
    for (profile = profiles.begin(); profile != profiles.end(); ++profile)
    {
      fprintf(output_file, "%s", profile->getFieldName().c_str());
      
      const vector< double > values = profile->getValues();
      vector< double >::const_iterator value;
      
      string format_string = _delimiter + _formatString;
      
      for (value = values.begin(); value != values.end(); ++value)
	fprintf(output_file, format_string.c_str(), *value);
      
      fprintf(output_file, "\n");
      
    } /* endfor - profile */
    
  } /* endfor - set */
  
  // Close the output file

  fclose(output_file);
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
