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

/*********************************************************************
 * GaiLtgFile.cc: Object controlling access to raw GAI lightning
 *                data file.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2002
 *
 * Gary Blackburn
 *
 *********************************************************************/


#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>

#include <rapformats/GaiLtgFile.hh>
#include <rapformats/gailtg.h>
#include <toolsa/str.h>
#include <toolsa/udatetime.h>
using namespace std;

// Global constants

const int GaiLtgFile::MAX_TOKENS = 7;
const int GaiLtgFile::MAX_TOKEN_LEN = 20;



/*********************************************************************
 * Constructor
 */

GaiLtgFile::GaiLtgFile(bool debug_flag, bool five_fields)
{
  _debugFlag = debug_flag;
  _fiveFields = five_fields;
  
  // Allocate space for the token buffers  

  _tokens = new char*[MAX_TOKENS];
  for (int i = 0; i < MAX_TOKENS; i++)
    _tokens[i] = new char[MAX_TOKEN_LEN];

  _strikeIterator = _strikeBuffer.end();

}


/*********************************************************************
 * Destructor
 */

GaiLtgFile::~GaiLtgFile()
{
  // Free token buffers  

  for (int i = 0; i < MAX_TOKENS; i++)
    delete [] _tokens[i];
  delete [] _tokens;

  // Free the local data buffer

  for (int i=0; i < (int) _strikeBuffer.size(); i++) {
    delete _strikeBuffer[i];
  }

  _strikeBuffer.erase(_strikeBuffer.begin(),_strikeBuffer.end());
    
}


//////////////////////////////
// Strike iteration methods //
//////////////////////////////

/*********************************************************************
 * getFirstStrike() - Retrieve the first strike from the buffer.  Returns
 *                    NULL if the end of the vector is reached
 */

GAILTG_strike_t *GaiLtgFile::getFirstStrike(void)
{
  _strikeIterator = _strikeBuffer.begin();

  if (_strikeIterator == _strikeBuffer.end())
    return (GAILTG_strike_t *) NULL;

  return *_strikeIterator;

}

/**********************************************************************
 * getNextStrike() - Retrieve the next strike from the buffer.  Returns
 *                   NULL if there are no more stikes in the vecter.
 *
 * Note that getFirstStrike() MUST be called before using this method.
 */

GAILTG_strike_t *GaiLtgFile::getNextStrike(void)
{

  _strikeIterator++;

  if (_strikeIterator == _strikeBuffer.end())
    return (GAILTG_strike_t *)NULL;

  return *_strikeIterator;

}

/*********************************************************************
 * loadFile() - Load the given data file into the object.  Wait the
 *              given number of seconds before loading the file to make
 *              sure the creating process has time to finish writing
 *              the file.
 *
 * Returns true if the load was successful, false otherwise.
 */

bool GaiLtgFile::loadFile(const char *filename,
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
  
  _strikeBuffer.clear ();
  // Process each line in the input file

  char file_line[BUFSIZ];

  while (fgets(file_line, BUFSIZ, ltg_file) != (char *)NULL)
  {
    int num_tokens;

    if (_fiveFields) 
    {
      if ((num_tokens = STRparse(file_line, _tokens, strlen(file_line),
                               5, MAX_TOKEN_LEN)) != 5)
      {
        fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
        fprintf(stderr, "Error parsing GAI lightning line into tokens: <%s>\n",
              file_line);
        fprintf(stderr, "Expected 5 tokens, got %d tokens\n", num_tokens);

        continue;

      }
    }
    else 
    {
      if ((num_tokens = STRparse(file_line, _tokens, strlen(file_line),
                               6, MAX_TOKEN_LEN))
        != 6)
      {
        fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
        fprintf(stderr, "Error parsing GAI lightning line into tokens: <%s>\n",
              file_line);
        fprintf(stderr, "Expected 6 tokens, got %d tokens\n", num_tokens);

        continue;

      }
    }

    // change day/time to unix time
    // GAI time is stored in two tokens
    // representing  mmddyy and hhmmss00
    GAILTG_strike_t *strike = new GAILTG_strike_t;

    date_time_t time;
    si32 mmddyy = atoi(_tokens[0]);
    time.month = (int) (mmddyy / 10000);
    time.day = (int) ((mmddyy - (time.month * 10000)) / 100);
    time.year = 2000 + (mmddyy - (time.month * 10000) - (time.day * 100));

    si32 hhmmss = atoi(_tokens[1]);
    time.hour = (int) (hhmmss / 1000000);
    time.min = (int) ((hhmmss - (time.hour * 1000000)) / 10000);
    time.sec = (int) ((hhmmss - (time.hour * 1000000) - (time.min * 10000)) / 100);

    strike->time = uunix_time(&time);
    strike->latitude = atoi(_tokens[2]);
    strike->longitude = atoi(_tokens[3]);
    //if (strike->longitude < 0)
    //   strike->longitude = strike->longitude + (si32) 3600000;
    strike->amplitude = atoi(_tokens[4]);
    strike->type = LTG_TYPE_UNKNOWN;
   
    _strikeBuffer.push_back (strike);


  }  // endwhile

  fclose(ltg_file);
  
  return true;
  
}
