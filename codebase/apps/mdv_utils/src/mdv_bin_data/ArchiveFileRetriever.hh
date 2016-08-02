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
 *   $Date: 2016/03/04 02:22:15 $
 *   $Id: ArchiveFileRetriever.hh,v 1.4 2016/03/04 02:22:15 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * ArchiveFileRetriever.hh: Class for retrieving archive DsMdvx files.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2000
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef ArchiveFileRetriever_HH
#define ArchiveFileRetriever_HH

/*
 **************************** includes **********************************
 */

#include <sys/time.h>

#include <Mdv/DsMdvx.hh>

#include "FileRetriever.hh"
using namespace std;


/*
 ******************************* defines ********************************
 */


/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

class ArchiveFileRetriever : public FileRetriever
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  ////////////////////////////////
  // Constructors & Destructors //
  ////////////////////////////////

  // Constructor
  //
  // Note that this constructor exits the program if there is an error
  // compiling the time list to be processed.

  ArchiveFileRetriever(const string& input_url,
		       const time_t start_time,
		       const time_t end_time,
		       const bool debug_flag = false);
  
  // Destructor

  ~ArchiveFileRetriever(void);
  
  ///////////////////////////
  // Miscellaneous methods //
  ///////////////////////////

  // Method for retrieving the next file to process.

  DsMdvx *next(void);
  
 private:

  /////////////////////
  // Private members //
  /////////////////////

  vector< time_t > _archiveTimeList;
  

  /////////////////////
  // Private methods //
  /////////////////////

  DsMdvx *_readFile(const time_t search_time);

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("ArchiveFileRetriever");
  }
  
};


#endif
