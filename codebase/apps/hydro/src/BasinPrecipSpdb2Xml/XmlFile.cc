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
//   $Date: 2016/03/07 18:36:49 $
//   $Id: XmlFile.cc,v 1.4 2016/03/07 18:36:49 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * XmlFile: Class controlling the XML file.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>

#include <toolsa/file_io.h>

#include "XmlFile.hh"
using namespace std;


/**********************************************************************
 * Constructor
 */

XmlFile::XmlFile(const bool debug_flag) :
  _debug(debug_flag),
  _filePtr(0),
  _fileTime(DateTime::NEVER)
{
}


/**********************************************************************
 * Destructor
 */

XmlFile::~XmlFile(void)
{
  if (_filePtr != 0)
    fclose(_filePtr);
}
  

/**********************************************************************
 * init() - Initialize the XML file.  Initialization includes making
 *          the output directory, opening the file and writing the
 *          first couple of lines to the file.
 *
 * Returns true on success, false on failure.
 */

bool XmlFile::init(const string &output_dir,
		   const DateTime &file_time)
{
  static const string method_name = "XmlFile::init()";
  
  // Save the file time for use later on

  _fileTime = file_time;
  
  // Make the output directory

  if (ta_makedir_recurse(output_dir.c_str()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating XML output directory: " << output_dir << endl;
    
    return false;
  }
  
  // Construct the output file name from the file time

  char *xml_filename = new char[strlen(output_dir.c_str()) + 50];

  sprintf(xml_filename, "%s/%04d%02d%02d_%02d%02d%02d.xml",
	  output_dir.c_str(),
	  file_time.getYear(), file_time.getMonth(),
	  file_time.getDay(),
	  file_time.getHour(), file_time.getMin(),
	  file_time.getSec());
  
  if (_debug)
    cerr << "   Will write XML to file path: <" << xml_filename << ">" << endl;
  
  // Open the output file

  if ((_filePtr = fopen(xml_filename, "w")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening output file for write" << endl;
    perror(xml_filename);
    
    delete xml_filename;
    
    return false;
  }
  
  delete xml_filename;
  
  // Write the first line to the output file

  fprintf(_filePtr, "<?xml version=\"1.0\" ?>\n");
  
  return true;
}


/**********************************************************************
 * writeBasinBeginTag() - Writes the indicated basin beginning tag to
 *                        the XML file.
 */

void XmlFile::writeBasinBeginTag(const int basin_id) const
{
  fprintf(_filePtr, "   <Basin ID=\"%d\"\n", basin_id);
}


/**********************************************************************
 * writeBasinEndTag() - Writes the basin end tag to the XML file.
 */

void XmlFile::writeBasinEndTag(void) const
{
  fprintf(_filePtr, "   />\n");
}


/**********************************************************************
 * writeDataValue() - Writes the indicated data value line to the XML
 *                    file.
 */

void XmlFile::writeDataValue(const int time_offset_minutes,
			     const double data_value) const
{
  if (time_offset_minutes >= 0)
    fprintf(_filePtr, "      t_m%d=\"%f\"\n",
	    time_offset_minutes, data_value);
  else
    fprintf(_filePtr, "      tf_m%d=\"%f\"\n",
	    (time_offset_minutes * -1), data_value);
}


/**********************************************************************
 * writeFieldBeginTag() - Writes the indicated field beginning tag to
 *                        the XML file.  The NOW tag following the field
 *                        begin tag is also written to the file.
 */

void XmlFile::writeFieldBeginTag(const string &field_string) const
{
//  if (i != 0)
//    fprintf(_filePtr, "\n");
    
  fprintf(_filePtr, "  <%s>\n", field_string.c_str());
    
  // Print out the time tag

  fprintf(_filePtr, "   <NOW TIME=\"%04d-%02d-%02dT%02d:%02d:%02d\" />\n",
	  _fileTime.getYear(), _fileTime.getMonth(),
	  _fileTime.getDay(),
	  _fileTime.getHour(), _fileTime.getMin(),
	  _fileTime.getSec());
}


/**********************************************************************
 * writeFieldEndTag() - Writes the indicated field end tag to the XML
 *                      file.
 */

void XmlFile::writeFieldEndTag(const string &field_string) const
{
  fprintf(_filePtr, "  </%s>\n", field_string.c_str());
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
