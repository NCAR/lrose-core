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
 * GldnLtgFile.cc: Object controlling access to raw GLDN lightning
 *                data file.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2004
 *
 * Gary Blackburn/Kay Levesque
 *
 *********************************************************************/


#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>

#include <rapformats/GldnLtgFile.hh>
#include <rapformats/ltg.h>
#include <toolsa/str.h>
#include <toolsa/udatetime.h>

using namespace std;

// Global constants

const int GldnLtgFile::MAX_TOKENS = 7;
const int GldnLtgFile::MAX_TOKEN_LEN = 20;


/*********************************************************************
 * Constructor
 */

GldnLtgFile::GldnLtgFile(bool debug_flag)
{
  _debugFlag = debug_flag;
  
  // Allocate space for the token buffers  

  _tokens = new char*[MAX_TOKENS];
  for (int i = 0; i < MAX_TOKENS; i++)
    _tokens[i] = new char[MAX_TOKEN_LEN];

  _strikeIterator = _strikeBuffer.end();

}


/*********************************************************************
 * Destructor
 */

GldnLtgFile::~GldnLtgFile()
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

LTG_strike_t *GldnLtgFile::getFirstStrike(void)
{
  _strikeIterator = _strikeBuffer.begin();

  if (_strikeIterator == _strikeBuffer.end())
    return (LTG_strike_t *) NULL;

  return *_strikeIterator;

}

/**********************************************************************
 * getNextStrike() - Retrieve the next strike from the buffer.  Returns
 *                   NULL if there are no more stikes in the vecter.
 *
 * Note that getFirstStrike() MUST be called before using this method.
 */

LTG_strike_t *GldnLtgFile::getNextStrike(void)
{

  _strikeIterator++;

  if (_strikeIterator == _strikeBuffer.end())
    return (LTG_strike_t *)NULL;

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

bool GldnLtgFile::loadFile(const char *filename,
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

//    if ((num_tokens = STRparse(file_line, _tokens, strlen(file_line),
    if ((num_tokens = STRparse_delim(file_line, _tokens, strlen(file_line),
			       ":", 6, MAX_TOKEN_LEN))
	!= 6)
    {
      fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
      fprintf(stderr, "Error parsing GLDN lightning line into tokens: <%s>\n",
	      file_line);
      fprintf(stderr, "Expected 6 tokens, got %d tokens\n", num_tokens);
      
      continue;
      
    }

    // Gldn time is in seconds since the epoch

    LTG_strike_t *strike = new LTG_strike_t;
    strike->time = atoi(_tokens[0]);    
    strike->latitude = atof(_tokens[1]);
    strike->longitude = atof(_tokens[2]);

    switch(atoi(_tokens[3]))
    {
    case GLDNLTG_GROUND_STROKE:
      strike->type = LTG_GROUND_STROKE;
      break;
    case GLDNLTG_CLOUD_STROKE:
      strike->type = LTG_CLOUD_STROKE;
      break;
    default:
      strike->type = LTG_TYPE_UNKNOWN;
      break;
    }
    
    strike->amplitude = atoi(_tokens[4]);
   
    _strikeBuffer.push_back (strike);


  }  // endwhile

  fclose(ltg_file);
  
  return true;
  
}
