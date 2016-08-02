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

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/07 18:36:49 $
 *   $Id: XmlFile.hh,v 1.3 2016/03/07 18:36:49 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * XmlFile: Class controlling the XML file.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef XmlFile_HH
#define XmlFile_HH

#include <string>

#include <toolsa/DateTime.hh>
using namespace std;


class XmlFile
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**********************************************************************
   * Constructor
   */

  XmlFile(const bool debug_flag = false);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~XmlFile(void);
  

  /**********************************************************************
   * init() - Initialize the XML file.  Initialization includes making
   *          the output directory, opening the file and writing the
   *          first couple of lines to the file.
   *
   * Returns true on success, false on failure.
   */

  bool init(const string &output_dir,
	    const DateTime &file_time);
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  /**********************************************************************
   * writeBasinBeginTag() - Writes the indicated basin beginning tag to
   *                        the XML file.
   */

  void writeBasinBeginTag(const int basin_id) const;
  

  /**********************************************************************
   * writeBasinEndTag() - Writes the basin end tag to the XML file.
   */

  void writeBasinEndTag(void) const;
  

  /**********************************************************************
   * writeDataValue() - Writes the indicated data value line to the XML
   *                    file.
   */

  void writeDataValue(const int time_offset_minutes,
		      const double data_value) const;
  

  /**********************************************************************
   * writeFieldBeginTag() - Writes the indicated field beginning tag to
   *                        the XML file.  The NOW tag following the field
   *                        begin tag is also written to the file.
   */

  void writeFieldBeginTag(const string &field_string) const;
  

  /**********************************************************************
   * writeFieldEndTag() - Writes the indicated field end tag to the XML
   *                      file.
   */

  void writeFieldEndTag(const string &field_string) const;
  

  //////////////////////
  // Accessor methods //
  //////////////////////

  /**********************************************************************
   * setDebugFlag() - Sets the debug flag for this object.
   */

  void setDebugFlag(const bool debug_flag)
  {
    _debug = debug_flag;
  }
  

 protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  FILE *_filePtr;
  DateTime _fileTime;
  

};


#endif
