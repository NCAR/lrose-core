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
 *   $Date: 2016/03/04 02:22:09 $
 *   $Id: Mdv2Ascii.hh,v 1.6 2016/03/04 02:22:09 dixon Exp $
 *   $Revision: 1.6 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Mdv2Ascii: Mdv2Ascii program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Mdv2Ascii_HH
#define Mdv2Ascii_HH

#include <string>
#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "Args.hh"
#include "Params.hh"
using namespace std;


class Mdv2Ascii
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Destructor
   */

  ~Mdv2Ascii(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static Mdv2Ascii *Inst(int argc, char **argv);
  static Mdv2Ascii *Inst();
  

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static Mdv2Ascii *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  Mdv2Ascii(int argc, char **argv);
  

  /*********************************************************************
   * _openOutputFile() - Open the proper output file.
   *
   * Returns a pointer to the output file on success, 0 on failure.
   */

  FILE *_openOutputFile(const time_t trigger_time, const time_t genTime);
  

  /*********************************************************************
   * _processData() - Process data for the given trigger time.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const time_t trigger_time);
  

  /*********************************************************************
   * _readInputFile() - Reads the specified input file.
   *
   * Returns true on success, false on failure.
   */

  bool _readInputFile(const time_t trigger_time,
		      DsMdvx &input_file);
  

  /*********************************************************************
   * _writeEsriHeader() - Writes the ESRI header to the output file.
   *
   * Returns true on success, false on failure.
   */
  
  bool _writeEsriHeader(FILE *output_file,
			const Mdvx::master_header_t &master_hdr,
			const Mdvx::field_header_t &field_hdr);
  

  /*********************************************************************
   * _writeUsdaHeader() - Writes the USDA header to the output file.
   *
   * Returns true on success, false on failure.
   */

  bool _writeUsdaHeader(FILE *output_file,
			const MdvxField &field);
  

};


#endif
