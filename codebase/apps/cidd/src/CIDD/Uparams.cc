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
////////////////////////////////////////////////////////////
// Uparams.hh
//
// Uparams class provides parameter utilities
//
// Mike Dixon, RAP, NCAR, Boulder, CO, USA
// March 2001
//
/////////////////////////////////////////////////////////////

#include "Uparams.hh"
#include <cerrno>
#include <iostream>
#include <cstdio>
#include <toolsa/umisc.h>
#include <toolsa/str.h>

// constructor

Uparams::Uparams()
{
}

// destructor

Uparams::~Uparams()
{
  clear();
}

// Clear the data base

void Uparams::clear()
{
  _plist.clear();
}

////////////////////////////////////////
// read in from param file
//
// returns 0 on success, -1 on failure

int Uparams::read(const char *file_path, const char *prog_name)

{


  // open file

  FILE *params_file;
  if ((params_file = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Uparams::read" << endl;
    cerr << "  Cannot read params from file: " << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // loop through file
  
  char line[BUFSIZ];

  while (!feof(params_file)) {
    
    // read a line
    
    if (fgets(line, BUFSIZ, params_file) == NULL) {
      break;
    }
    
    if (feof(params_file))
      break;

    // substitute in any environment variables
    
    usubstitute_env(line, BUFSIZ);

    // delete past any hash-bang

    char *sptr;
    if ((sptr = strstr(line, "#!")) != NULL) {
      *sptr = '\0';
    }
    
    // process only if the line has the program name followed by a period.
    
    char *name = line;
    if (strlen(name) < strlen(prog_name + 1)) {
      continue;
    }
    if (strncmp(prog_name, name, strlen(prog_name)) ||
	name[strlen(prog_name)] != '.') {
      continue;
    }

    // check that there is a colon

    char *colon = strchr(name, ':');
    if (!colon) {
      continue;
    }
    
    // back up past any white space
    
    char *end_of_name = colon - 1;
    while (*end_of_name == ' ' || *end_of_name == '\t') {
      end_of_name--;
    }

    // place null at end of name

    *(end_of_name + 1) = '\0';

    // get entry string

    char *entry = colon + 1;

    // advance past white space
    
    while (*entry == ' ' || *entry == '\t') {
      entry++;
    }

    // back up past white space
    
    char *end_of_entry = entry + strlen(entry);
    while (*end_of_entry == ' ' || *end_of_entry == '\t' ||
	   *end_of_entry == '\r' || *end_of_entry == '\n' ||
	   *end_of_entry == '\0') {
      end_of_entry--;
    }

    // place null at end of entry
    
    *(end_of_entry + 1) = '\0';

    // check that we do not already have this param
    
    bool previous_entry_found = false;
    for (size_t ii = 0; ii < _plist.size(); ii++) {
      if (_plist[ii].name == name) {
	_plist[ii].entry = entry;
	previous_entry_found = true;
	break;
      }
    } // ii

    // if previous entry was not found,
    // store name and entry pointers in params list

    if (!previous_entry_found) {
      Uparam_list_t ll;
      ll.name = name;
      ll.entry = entry;
      _plist.push_back(ll);
    }
      
  } /* while (!feof(params_file)) */

  // close file

  fclose(params_file);

  // debug print

//    for (size_t ii = 0; ii < _plist.size(); ii++) {
//      cerr << "name, val: " << _plist[ii].name << ", " << _plist[ii].entry << endl;
//    } // ii
//    cerr << "Param list size: " << _plist.size() << endl;

  return 0;

}

////////////////////////////////////////
// read from a param buffer
//
// returns 0 on success, -1 on failure

int Uparams::read(const char *buf,
		  int buf_len,
		  const char *prog_name)

{

  // loop through lines in buffer
  
  char line[BUFSIZ];
  const char *ptr = buf;
  
  while (ptr < buf + buf_len) {
  
    // find a line

    const char *eol = strchr(ptr, '\n');
    if (eol == NULL) {
      break;
    }
    
    int lineLen = eol - ptr + 1;
    int copyLen;
    if (lineLen > BUFSIZ) {
      copyLen = BUFSIZ;
    } else {
      copyLen = lineLen;
    }
    STRncopy(line, ptr, copyLen);
    ptr += lineLen;

    // substitute in any environment variables
    
    usubstitute_env(line, BUFSIZ);
    
    // delete past any hash-bang

    char *sptr;
    if ((sptr = strstr(line, "#!")) != NULL) {
      *sptr = '\0';
    }
    
    // process only if the line has the program name followed by a period.
    
    char *name = line;
    if (strlen(name) < strlen(prog_name + 1)) {
      continue;
    }
    if (strncmp(prog_name, name, strlen(prog_name)) ||
	name[strlen(prog_name)] != '.') {
      continue;
    }

    // check that there is a colon

    char *colon = strchr(name, ':');
    if (!colon) {
      continue;
    }
    
    // back up past any white space
    
    char *end_of_name = colon - 1;
    while (*end_of_name == ' ' || *end_of_name == '\t') {
      end_of_name--;
    }

    // place null at end of name

    *(end_of_name + 1) = '\0';

    // get entry string

    char *entry = colon + 1;

    // advance past white space
    
    while (*entry == ' ' || *entry == '\t') {
      entry++;
    }

    // back up past white space
    
    char *end_of_entry = entry + strlen(entry);
    while (*end_of_entry == ' ' || *end_of_entry == '\t' ||
	   *end_of_entry == '\r' || *end_of_entry == '\n' ||
	   *end_of_entry == '\0') {
      end_of_entry--;
    }

    // place null at end of entry
    
    *(end_of_entry + 1) = '\0';

    // check that we do not already have this param
    
    bool previous_entry_found = false;
    for (size_t ii = 0; ii < _plist.size(); ii++) {
      if (_plist[ii].name == name) {
	_plist[ii].entry = entry;
	previous_entry_found = true;
	break;
      }
    } // ii

    // if previous entry was not found,
    // store name and entry pointers in params list

    if (!previous_entry_found) {
      Uparam_list_t ll;
      ll.name = name;
      ll.entry = entry;
      _plist.push_back(ll);
    }
      
  } /* while (!feof(params_file)) */

  // debug print

//    for (size_t ii = 0; ii < _plist.size(); ii++) {
//      cerr << "name, val: " << _plist[ii].name << ", " << _plist[ii].entry << endl;
//    } // ii
//    cerr << "Param list size: " << _plist.size() << endl;

  return 0;

}

///////////////////////////////////////////////////////////////
// gets an entry from the param list
// returns NULL if this fails
///////////////////////////////////////////////////////////////

const char *Uparams::_get(const char *search_name) const

{
  
  for (size_t ii =  0; ii < _plist.size(); ii++) {
    if (_plist[ii].name == search_name) {
      return (_plist[ii].entry.c_str());
    }
  }

  return NULL;

}

///////////////////////////////////////////////////////////////
// returns the value of a double parameter
// If it cannot find the parameter, returns the default
///////////////////////////////////////////////////////////////

double Uparams::getDouble(const char *name, double default_val)
{

  const char *entryStr = _get(name);
  if (entryStr == NULL) {
    return default_val;
  } else {
    double val;
    if (sscanf(entryStr, "%lg", &val) == 1) {
      return val;
    } else {
      return default_val;
    }
  }
  
  return default_val;

}

///////////////////////////////////////////////////////////////
// returns the value of a float parameter
// If it cannot find the parameter, returns the default
///////////////////////////////////////////////////////////////

float Uparams::getFloat(const char *name, float default_val)
{

  const char *entryStr = _get(name);
  if (entryStr == NULL) {
    return default_val;
  } else {
    float val;
    if (sscanf(entryStr, "%g", &val) == 1) {
      return val;
    } else {
      return default_val;
    }
  }
  
  return default_val;

}

///////////////////////////////////////////////////////////////
// returns the value of a int parameter
// If it cannot find the parameter, returns the default
///////////////////////////////////////////////////////////////

int Uparams::getInt(const char *name, int default_val)

{

  const char *entryStr = _get(name);
  if (entryStr == NULL) {
    return default_val;
  } else {
    int val;
    if (sscanf(entryStr, "%d", &val) == 1) {
      return val;
    } else {
      return default_val;
    }
  }
  
  return default_val;

}

///////////////////////////////////////////////////////////////
// returns the value of a long parameter
// If it cannot find the parameter, returns the default
///////////////////////////////////////////////////////////////

long Uparams::getLong(const char *name, long default_val)

{

  const char *entryStr = _get(name);
  if (entryStr == NULL) {
    return default_val;
  } else {
    long val;
    if (sscanf(entryStr, "%ld", &val) == 1) {
      return val;
    } else {
      return default_val;
    }
  }
  
  return default_val;

}

///////////////////////////////////////////////////////////////
// returns the value of a string parameter
// If it cannot find the parameter, returns the default
///////////////////////////////////////////////////////////////

const char *Uparams::getString(const char *name, const char *default_val)

{

  const char *entryStr = _get(name);
  if (entryStr == NULL) {
    return default_val;
  } else {
    return (char *) entryStr;
  }
}

