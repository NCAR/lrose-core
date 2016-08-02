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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:22:12 $
//   $Id: AsciiTablePlotter.cc,v 1.4 2016/03/04 02:22:12 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * AsciiTablePlotter: Class that creates scatter plots as ASCII tables
 *                    that can be read into other programs for creating
 *                    the plots.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/MdvxPjg.hh>

#include "AsciiTablePlotter.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

AsciiTablePlotter::AsciiTablePlotter(const string &delimiter,
				     const bool include_header,
				     const string &output_dir,
				     const string &output_ext,
				     const string &x_field_name,
				     const string &y_field_name,
				     const bool debug_flag) :
  Plotter(output_dir, output_ext, x_field_name, y_field_name, debug_flag),
  _delimiter(delimiter),
  _includeHeader(include_header)
{
}

  
/*********************************************************************
 * Destructor
 */

AsciiTablePlotter::~AsciiTablePlotter()
{
}


/*********************************************************************
 * createPlot() - Create the scatter plot for the given fields.
 *
 * Returns true on success, false on failure.
 */

bool AsciiTablePlotter::createPlot(const DateTime &data_time,
				   const MdvxField &x_field,
				   const MdvxField &y_field)
{
  static const string method_name = "AsciiTablePlotter::createPlot()";
  
  // Generate the output file path.  Note that _generateOutputPath
  // also creates the directories for us.

  Path output_path = _generateOutputPath(data_time);
  
  return createPlot(output_path.getPath(),
		    data_time, x_field, y_field);
}


bool AsciiTablePlotter::createPlot(const string &output_path,
				   const DateTime &data_time,
				   const MdvxField &x_field,
				   const MdvxField &y_field)
{
  static const string method_name = "AsciiTablePlotter::createPlot()";
  
  // Open the output file

  FILE *output_file;

  if ((output_file = fopen(output_path.c_str(), "a")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening output file: " << output_path << endl;
    
    return false;
  }
  
  // Put in a header, if requested

  Mdvx::field_header_t x_field_hdr = x_field.getFieldHeader();
  Mdvx::field_header_t y_field_hdr = y_field.getFieldHeader();
  
  if (_includeHeader)
  {
    string x_field_name = _xFieldName;
    string y_field_name = _yFieldName;
    
    if (x_field_name == "")
      x_field_name = x_field_hdr.field_name;
    
    if (y_field_name == "")
      y_field_name = y_field_hdr.field_name;
    
    fprintf(output_file, "lat%slon%sdate%stime%s%s%s%s\n",
	    _delimiter.c_str(), _delimiter.c_str(),
	    _delimiter.c_str(), _delimiter.c_str(),
	    x_field_name.c_str(), _delimiter.c_str(),
	    y_field_name.c_str());
  }
  
  // Put the data in the output file

  fl32 *x_field_data = (fl32 *)x_field.getVol();
  fl32 *y_field_data = (fl32 *)y_field.getVol();
  
  MdvxPjg proj(x_field_hdr);
  
  for (int x = 0; x < x_field_hdr.nx; ++x)
  {
    for (int y = 0; y < x_field_hdr.ny; ++y)
    {
      int i = y * x_field_hdr.nx + x;
      
      // Don't process missing data

      if (x_field_data[i] == x_field_hdr.missing_data_value ||
	  x_field_data[i] == x_field_hdr.bad_data_value)
	continue;
    
      if (y_field_data[i] == y_field_hdr.missing_data_value ||
	  y_field_data[i] == y_field_hdr.bad_data_value)
	continue;
    
      // Calculate the needed values

      double lat, lon;
      
      proj.xyIndex2latlon(x, y, lat, lon);
      
      // Print out the data

      fprintf(output_file, "%f%s%f%s%s%s%s%s%f%s%f\n",
	      lat, _delimiter.c_str(),
	      lon, _delimiter.c_str(),
	      data_time.getDateStr().c_str(), _delimiter.c_str(),
	      data_time.getTimeStr().c_str(), _delimiter.c_str(),
	      x_field_data[i], _delimiter.c_str(),
	      y_field_data[i]);
      
    } /* endfor - y */
  } /* endfor - x */
  
  // Close the output file

  fclose(output_file);
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

