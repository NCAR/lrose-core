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
//   $Date: 2016/03/03 18:45:40 $
//   $Id: KavLtgFile.cc,v 1.6 2016/03/03 18:45:40 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * KavLtgFile.cc: Object controlling access to a raw Kavouras lightning
 *                data file.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>

#include <rapformats/KavLtgFile.hh>
#include <rapformats/kavltg.h>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>
using namespace std;




/*********************************************************************
 * Constructor
 */

KavLtgFile::KavLtgFile(bool debug_flag)
{
  _debugFlag = debug_flag;
  
  _strikeBuffer = (KAVLTG_strike_t *)NULL;
  _strikeBufferAlloc = 0;
  _strikeBufferUsed = 0;
  
}


/*********************************************************************
 * Destructor
 */

KavLtgFile::~KavLtgFile()
{
  // Free the local data buffer

  if (_strikeBuffer != (KAVLTG_strike_t *)NULL)
    ufree(_strikeBuffer);
}


/*********************************************************************
 * loadFile() - Load the given data file into the object.  Wait the
 *              given number of seconds before loading the file to make
 *              sure the creating process has time to finish writing
 *              the file.
 *
 * Returns true if the load was successful, false otherwise.
 */

bool KavLtgFile::loadFile(const char *filename,
			  int processing_delay)
{
  const char *routine_name = "loadFile()";
  
  FILE *ltg_file;
  
  // Allow the file activity to finish before processing the file

  if (processing_delay > 0)
    sleep(processing_delay);
  
  if (_debugFlag)
    fprintf(stderr,
	    "New data in file <%s>\n", filename);

  // Open the input file.

  if ((ltg_file = fopen(filename, "r")) == NULL)
  {
    fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr, "Error opening file <%s>\n", filename);
    
    return false;
  }
  
  // Read all of the data into a local buffer

  struct stat file_stat;
  
  if (ta_stat(filename, &file_stat) != 0)
  {
    fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr, "Error stating file <%s>\n", filename);
    
    return false;
  }
  
  _allocateBuffers(file_stat.st_size);
  
  int strikes_read;
  
  if ((strikes_read = fread((void *)_strikeBuffer,
			    sizeof(KAVLTG_strike_t),
			    _strikeBufferUsed,
			    ltg_file)) != _strikeBufferUsed)
  {
    fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr, "Error reading strike data from file <%s>\n",
	    filename);
    fprintf(stderr, "Expected %d strikes, read %d strikes\n",
	    _strikeBufferUsed, strikes_read);
    
    return false;
  }
  
  fclose(ltg_file);
  
  // Make sure the strike data is in native format

  for (int i = 0; i < _strikeBufferUsed; i++)
    KAVLTG_strike_from_BE(&_strikeBuffer[i]);
  
  _currentStrike = 0;
  
  return true;
  
}


/*********************************************************************
 * getNextStrike() - Retrieves the next lightning strike from the
 *                   file.
 *
 * Returns a pointer to the strike data on success, returns NULL on
 * error.  Note that the returned pointer points to static memory that
 * should NOT be freed by the calling routine.
 */

KAVLTG_strike_t *KavLtgFile::getNextStrike(void)
{
  while (_currentStrike < _strikeBufferUsed)
  {
    if (_strikeBuffer[_currentStrike].time != 0)
      return &_strikeBuffer[_currentStrike++];
    
    _currentStrike++;
  }
  
  return (KAVLTG_strike_t *)NULL;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _allocateBuffers() - Allocate the internal data buffers based on the
 *                      number of bytes in the new lightning file.
 */

void KavLtgFile::_allocateBuffers(int file_bytes)
{
  _strikeBufferUsed = file_bytes / sizeof(KAVLTG_strike_t);
  
  if (_strikeBufferUsed > _strikeBufferAlloc)
  {
    _strikeBufferAlloc = _strikeBufferUsed;
    
    if (_strikeBuffer == (KAVLTG_strike_t *)NULL)
    {
      _strikeBuffer = (KAVLTG_strike_t *)umalloc(_strikeBufferAlloc *
						 sizeof(KAVLTG_strike_t));
    }
    else
    {
      _strikeBuffer = (KAVLTG_strike_t *)urealloc(_strikeBuffer,
						  _strikeBufferAlloc *
						  sizeof(KAVLTG_strike_t));
    }
  }
  
}
