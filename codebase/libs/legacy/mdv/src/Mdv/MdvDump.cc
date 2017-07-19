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
 * MdvDump.cc: MDV object code.  This object manipulates MDV
 *             information.  This file contains the private member
 *             functions that deal with dumping out the object in
 *             different formats.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <cstdio>
#include <cerrno>
#include <cassert>

#include <mdv/mdv_print.h>
#include <mdv/mdv_user.h>
#include <mdv/mdv_write.h>
#include <toolsa/str.h>
#include <toolsa/utim.h>
#include <toolsa/ldata_info.h>

#include <mdv/Mdv.h>
using namespace std;

/*
 * Global variables
 */


/*********************************************************************
 * _dumpBinary() - Dump the information to the indicated stream in
 *                 binary format.
 */

void Mdv::_dumpBinary(FILE *out_file,
		      int output_encoding_type)
{
  static char *routine_name = "_dumpBinary";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Write the master header

  if (MDV_write_master_header(out_file,
			      _masterHdr) != MDV_SUCCESS)
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n",
	    _className(), routine_name);
    fprintf(stderr,
	    "Error writing master header to output file\n");
    return;
  }
  
  // Write each of the fields

  int next_offset = MDV_get_first_field_offset(_masterHdr);
  int field_num;
  
  for (field_num = 0; field_num < (int)_fieldList->size(); field_num++)
  {
    int bytes_written = (*_fieldList)[field_num]->dump(out_file,
						       field_num,
						       next_offset,
						       output_encoding_type);
    
    next_offset += bytes_written + (2 * sizeof(si32));
  } /* endfor - field_num */
  
  return;
}


/*********************************************************************
 * _dumpAscii() - Dump the information to the indicated stream in
 *                ASCII format.
 */

void Mdv::_dumpAscii(FILE *stream,
		     int full_flag)
{
  static char *routine_name = "_dumpAscii";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  // Print the master header

  if (full_flag)
    MDV_print_master_header_full(_masterHdr, stream);
  else
    MDV_print_master_header(_masterHdr, stream);
  
  // Print out each of the fields

  for (int i = 0; i < _fieldList->size(); i++)
    (*_fieldList)[i]->print(stream, full_flag);

  return;
}


/*********************************************************************
 * _writeCurrentIndex() - Write the current index file for the data
 *                        file that was just written.
 *
 * Returns the full path for the written index file.
 */

char *Mdv::_writeCurrentIndex(char *output_dir,
			      char *output_ext,
			      time_t data_time)
{
  static char *routine_name = "_writeCurrentIndex";
  static int first_call = TRUE;
  static LDATA_handle_t ldata;
  char calling_sequence[BUFSIZ];

  if (first_call) {
    sprintf(calling_sequence, "%s::%s",
	    _className(), routine_name);
    LDATA_init_handle(&ldata, calling_sequence, 0);
    first_call = FALSE;
  }

  // Write the info file

  if (LDATA_info_write(&ldata, output_dir, data_time, output_ext,
		       (char *)NULL, (char *)NULL, 0, (int *)NULL)) {
    return (NULL);
  } else {
    return (ldata.file_path);
  }

}
