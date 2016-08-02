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
/////////////////////////////////////////////////////////////
// ReadStates.cc
//
// USStates object
//
// Deirdre Garvey RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2003
//
///////////////////////////////////////////////////////////////
//
// ReadStates returns the set of states
//


#include <cstdio>
#include <string>
#include <vector>
#include <cerrno>
#include <iostream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/http.h>
#include <cstring>
#include <cstdlib>
#include "ReadStates.hh"

using namespace std;

// constructor
ReadStates::ReadStates()
{

}


// destructor
ReadStates::~ReadStates()
{
}


//////////////////////////////////////////////////////////////////
/// PrintAll()
/// PrintAll function can print all content in the file which was 
/// specified in the Main function out.
//////////////////////////////////////////////////////////////////

void ReadStates::PrintAll()
{
  
  for (size_t j=0;j<v1.size();j++)
    {
      cout << "name= " << v1[j].iname << "..." <<  endl;
      cout << "abbrev= "  << v1[j].iabbr << "..." << endl;
    }
}

//////////////////////////////////////////////////////////////////
/// IsStateAbbr
///
/// Given state abbreviation, does it exist
//
// Returns true on success, false on failure.
//
//////////////////////////////////////////////////////////////////

bool ReadStates::IsStateAbbr(const string &abbr)

{

  for (size_t j=0;j<v1.size();j++)
  {

     if (abbr==v1[j].iabbr)
       return true;

  }
  return false;
}  

//////////////////////////////////////////////////////////////////
/// ReadData() 
///
/// ReadData function needs one const character parameter, which 
/// gives the file name this program should open to read. If this
/// file can not be opened or opened error, it returns -1. 
/// Otherwise, it returns 0.
//////////////////////////////////////////////////////////////////

int ReadStates::ReadData(const char *FilePath)
{
 
  // Handle a http URL
  if(strncasecmp(FilePath,"http:",5) == 0) { return ReadDataHttp(FilePath); }

  FILE *IN;
  // File must be local
  if((IN=fopen(FilePath,"r"))==NULL) {
    cout << "ReadStates::ReadData open failed for file:" << FilePath; 
    return(-1);
  }
 
  char line[256];
  Info info;
  int i;
 
      
  while (!feof(IN)) {

    if(fgets(line, 256, IN)) { 
      for(i=0;i<256;i++) {
	if(line[i]=='\0')
	  {i=256;}
      } 

      const string strline=(string)line;

      // Skip lines beginning with #

      if (strline.find("#", 0) != 0) {
      
	// Split the line on commas

	vector <string> toks;
	Tokenize(strline, ",", toks);

	// Save the state name, abbreviation

	if (toks.size() == 2) {
	  info.iname=toks[0];

	  // Strip leading blanks from abbreviation

	  vector <string> abbrtoks;
	  Tokenize(toks[1], " \n\t\r", abbrtoks);
	  info.iabbr=abbrtoks[0];

	  v1.push_back(info);   //push data fields into info class 
	}
      }
    }
  }
  fclose(IN);

  return(0);
}

//////////////////////////////////////////////////////////////////
/// ReadDataBuffer() 
///
/// On Error, it returns -1. 
/// Otherwise, it returns 0.
//////////////////////////////////////////////////////////////////

int ReadStates::ReadDataHttp(const char *url)
{
 
  char *buffer = NULL;
  char *str_ptr,*local_ptr;
  int buf_len,ret_stat;
  Info info;
 
   // allow 5 seconds to pull in states data
   ret_stat =  HTTPgetURL((char *)url,5000, &buffer, &buf_len);
   if(ret_stat <= 0 || buf_len <= 0) {
    cout << "ReadStates::ReadDataHttp get failed for URL:" << url; 
    return(-1);
  }

   // Prime strtok_r;
  str_ptr = strtok_r(buffer,"\n",&local_ptr);

  while (str_ptr != NULL ) {

    const string line=(string)str_ptr;

    // Skip lines beginning with #

    if (line.find("#", 0) != 0) {
      
      // Split the line on commas

      vector <string> toks;
      Tokenize(line, ",", toks);

      // Save the state name, abbreviation

      if (toks.size() == 2) {
	info.iname=toks[0];

	// Strip leading blanks from abbreviation

	vector <string> abbrtoks;
	Tokenize(toks[1], " \n\t\r", abbrtoks);
	info.iabbr=abbrtoks[0];

	v1.push_back(info);   //push data fields into info class 
      }
    }  

    // Get next line
    str_ptr = strtok_r(NULL,"\n",&local_ptr);
  }

  if(buffer != NULL) free(buffer);

  return(0);
} 


//////////////////////////////////////////////
// tokenize a string into a vector of strings

void ReadStates::Tokenize(const string &str,
			  const string &spacer,
			  vector<string> &toks)
{
  toks.clear();
  size_t pos = 0;
  while (true) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      return;
    } else if (end == string::npos) {
      string tok;
      tok.assign(str, start, string::npos);
      toks.push_back(tok);
      return;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}
