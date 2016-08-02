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
//   $Date: 2016/03/03 18:03:31 $
//   $Id: DsAccessFile.cc,v 1.3 2016/03/03 18:03:31 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsAccessFile.cc: Class for manipulating the _dsaccess files that
 *                  control access to DsServer directories.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2000
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <didss/DsAccess.hh>
#include <didss/DsAccessFile.hh>
#include <toolsa/str.h>
using namespace std;


/*
 * Initialize constants
 */

const string DsAccessFile::CLASS_NAME = "DsAccessFile";

const string DsAccessFile::fileName = "_dsaccess";

const int DsAccessFile::MAX_TOKENS = 10;
const int DsAccessFile::MAX_TOKEN_LEN = 100;

const string DsAccessFile::LIMIT_DIRECTIVE = "<Limit";
const string DsAccessFile::LIMIT_END_DIRECTIVE = "</Limit>";
const string DsAccessFile::ORDER_DIRECTIVE = "Order";
const string DsAccessFile::DENY_DIRECTIVE = "Deny";
const string DsAccessFile::ALLOW_DIRECTIVE = "Allow";

const string DsAccessFile::DENY_ALLOW_TOKEN = "Deny,Allow";
const string DsAccessFile::ALLOW_DENY_TOKEN = "Allow,Deny";

const string DsAccessFile::FROM_TOKEN = "from";
const string DsAccessFile::ALL_TOKEN = "all";


/**********************************************************************
 * Constructors
 */

DsAccessFile::DsAccessFile(const bool debug_flag) :
  _debugFlag(debug_flag),
  _pathInitialized(false),
  _filePath(""),
  _searchDirectories(false),
  _dirPath("")
{
  // Allocate space for the token parsing members

  tokens = new char*[MAX_TOKENS];
  
  for (int i = 0; i < MAX_TOKENS; ++i)
    tokens[i] = new char[MAX_TOKEN_LEN];
  
}


/**********************************************************************
 * Destructor
 */

DsAccessFile::~DsAccessFile()
{
  // Free up the space used for token parsing

  for (int i = 0; i < MAX_TOKENS; ++i)
    delete tokens[i];
  
  delete tokens;
}


/**********************************************************************
 * getReadAccess() - Retrieve the read access from the current access
 *                   file.
 *
 * You must call either setAccessFile() or setAccessDir() before calling
 * this method.
 *
 * Returns a pointer to the DsAccess structure indicating the allowed
 * read access.  If setAccessDir() is used for the access search,
 * returns a DsAccess structure with complete public access if there are
 * no _dsaccess files at any level in the directory structure.
 *
 * Returns 0 if the search path has not been initialized with either
 * setAccessFile() or setAccessDir() or if there is some kind of error.
 *
 * Note that control of the allocated memory is transferred to the
 * client of this method, who must delete the memory when no longer
 * needed.
 */

DsAccess *DsAccessFile::getReadAccess()
{
  return _getAccess("READ");
}


/**********************************************************************
 * getWriteAccess() - Retrieve the write access from the current access
 *                    file.
 *
 * You must call either setAccessFile() or setAccessDir() before calling
 * this method.
 *
 * Returns a pointer to the DsAccess structure indicating the allowed
 * write access.  If setAccessDir() is used for the access search,
 * returns a DsAccess structure with complete public access if there are
 * no _dsaccess files at any level in the directory structure.
 *
 * Returns 0 if the search path has not been initialized with either
 * setAccessFile() or setAccessDir() or if there is some kind of error.
 *
 * Note that control of the allocated memory is transferred to the
 * client of this method, who must delete the memory when no longer
 * needed.
 */

DsAccess *DsAccessFile::getWriteAccess()
{
  return _getAccess("WRITE");
}


/**********************************************************************
 * PRIVATE METHODS
 **********************************************************************/

/**********************************************************************
 * _getAccess() - Retrieve the indicated access from the current access
 *                file.
 *
 * You must call either setAccessFile() or setAccessDir() before calling
 * this method.
 *
 * Returns a pointer to the DsAccess structure indicating the allowed
 * access of the given type.  If setAccessDir() is used for the access
 * search, returns a DsAccess structure with complete public access if
 * there are no _dsaccess files at any level in the directory structure.
 *
 * Returns 0 if the search path has not been initialized with either
 * setAccessFile() or setAccessDir() or if there is some kind of error.
 *
 * Note that control of the allocated memory is transferred to the
 * client of this method, who must delete the memory when no longer
 * needed.
 */

DsAccess *DsAccessFile::_getAccess(const string &access_type)
{
  const string method_name = CLASS_NAME + string("::_getAccess()");
  
  if (!_pathInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Path not initialized" << endl;
    
    return 0;
  }
  
  // Determine the path of the _dsaccess file to use.  If no _dsaccess
  // file is found, allow universal access to the directory.

  string *access_filename;
  
  if ((access_filename = _getAccessFilename()) == 0)
    return new DsAccess(DsAccess::DENY_ALLOW, true, _debugFlag);
  
  // Read the access information from the access file

  DsAccess *access;
  
  if ((access = _readAccessInfo(*access_filename, access_type)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading access information from file: " <<
      access_filename << endl;
    
    return 0;
  }
  
  return access;
}


/**********************************************************************
 * _getAccessFilename() - Determine the path of the access file to use.
 *                        Returns 0 if there isn't an access file in
 *                        the directory.
 */

string *DsAccessFile::_getAccessFilename(void)
{
  assert(_pathInitialized);
  
  string *access_file_path = 0;
  
  // Determine the name of the access file

  if (_searchDirectories)
  {
    return access_file_path;
  }
  else
  {
    access_file_path = new string(_filePath);
    
    return access_file_path;
  }
  
  // We should never get here.

  return access_file_path;
}


/**********************************************************************
 * _readAccessInfo() - Reads the indicated access information from the
 *                     given file.
 *
 * Returns a pointer to the appropriate access structure, or 0 if
 * there was an error of some type.  The client is given control of
 * the DsAccess pointer after this method call and so must delete the
 * pointer when finished with it.
 */

DsAccess *DsAccessFile::_readAccessInfo(const string &access_filename,
					const string &access_type)
{
  const string method_name = CLASS_NAME + string("::_readAccessInfo()");
  
  // Open the access file

  FILE *access_file;
  
  if ((access_file = fopen(access_filename.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening access file: " << access_filename << endl;
    
    return 0;
  }
  
  // Read the access information

  DsAccess *access = _readAccessInfo(access_file, access_type);
  
  // Close the access file

  fclose(access_file);
  
  return access;
}


DsAccess *DsAccessFile::_readAccessInfo(FILE *access_file,
					const string &access_type)
{
  const string method_name = CLASS_NAME + string("::_readAccessInfo()");
  
  char line[BUFSIZ];
  
  // Skip lines in the file until we find a "Limit" directive
  // of the correct type.  If there is no "Limit" directive of
  // the given type, give the user full access.

  bool limit_found = false;
  
  while (fgets(line, BUFSIZ, access_file) != 0)
  {
    // Remove leading blanks so we can check the first
    // token on the line.

    STRblnk(line);
    
    // Look for the "Limit" directive.  This line should be of
    // the form:
    //
    // <Limit access_type>
    // 
    // Where: access_type is replaces with the value of the
    //          access_type string received by this method.

    if (line[0] != '<')
      continue;
    
    if (STRparse(line, tokens, BUFSIZ, MAX_TOKENS, MAX_TOKEN_LEN) != 2)
      continue;
    
    if (string(tokens[0]) != LIMIT_DIRECTIVE)
      continue;
    
    string access_token = access_type + string(">");
    
    if (string(tokens[1]) != access_token)
      continue;
    
    // If we get here, we found the "Limit" directive

    limit_found = true;
    break;
    
  } /* endwhile - fgets(...) */
  
  // If the appropriate limit directive wasn't found, universal
  // access is granted.

  if (!limit_found)
    return new DsAccess(DsAccess::DENY_ALLOW, true, _debugFlag);
  
  // Create the access structure to return if everything is
  // okay.

  DsAccess *access = new DsAccess(DsAccess::DENY_ALLOW, true, _debugFlag);
  
  // Process all of the directives until we reach the limit
  // end directive.  If there is no limit end directive, return
  // an error.

  bool limit_end_found = false;
  
  while (fgets(line, BUFSIZ, access_file) != 0)
  {
    // Parse the line into tokens

    int num_tokens = STRparse(line, tokens, BUFSIZ, MAX_TOKENS, MAX_TOKEN_LEN);
    
    // Skip empty lines

    if (num_tokens == 0)
      continue;
    
    // Check for the end limit token

    if (num_tokens == 1 &&
	string(tokens[0]) == LIMIT_END_DIRECTIVE)
    {
      limit_end_found = true;
      break;
    }
    
    // Process the limit directives

    string directive = tokens[0];
    
    if (directive == ORDER_DIRECTIVE)
    {
      bool error = false;
      
      if (num_tokens == 2)
      {
	string ad_token = string(tokens[1]);
	
	if (ad_token == DENY_ALLOW_TOKEN)
	  access->setOrder(DsAccess::DENY_ALLOW);
	else if (ad_token == ALLOW_DENY_TOKEN)
	  access->setOrder(DsAccess::ALLOW_DENY);
	else
	  error = true;
      }
      else
      {
	error = true;
      }
      
      if (error)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error parsing order directive line: " << line << endl;
	
	delete access;
	return 0;
      }
    }
    else if (directive == DENY_DIRECTIVE ||
	     directive == ALLOW_DIRECTIVE)
    {
      bool error = false;
      
      if (num_tokens == 3)
      {
	if (string(tokens[1]) == FROM_TOKEN)
	{
	  if (string(tokens[2]) == ALL_TOKEN)
	  {
	    if (directive == DENY_DIRECTIVE)
	      access->setDenyAllFlag(true);
	    else
	      access->setAllowAllFlag(true);
	  }
	  else if (strchr(tokens[2], '.') == 0)
	  {
	    if (directive == DENY_DIRECTIVE)
	      access->addUserToDenyList(tokens[2]);
	    else
	      access->addUserToAllowList(tokens[2]);
	  }
	  else
	  {
	    IPAddress address;
	    
	    if (!address.setFromString(tokens[2]))
	    {
	      cerr << "ERROR: " << method_name << endl;
	      cerr << "Error parsing IP address in line: " << line << endl;
	      
	      delete access;
	      return 0;
	    }
	    
	    if (directive == DENY_DIRECTIVE)
	      access->addIpToDenyList(address);
	    else
	      access->addIpToAllowList(address);
	  }
	}
	else
	{
	  error = true;
	}
      }
      else
      {
	error = true;
      }
      
      if (error)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error parsing allow/deny directive line: " << line << endl;
	
	delete access;
	return 0;
      }
    }
    else
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Unrecognized directive on line: " << line << endl;
      cerr << "Skipping line" << endl;
      
      continue;
    }
    
  }
  
  // Check for errors

  if (!limit_end_found)
  {
    delete access;
    return 0;
  }
  
  return access;
}
