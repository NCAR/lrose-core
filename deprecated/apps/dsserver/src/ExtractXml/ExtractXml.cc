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


#include <vector>
#include <iostream>
#include "tinyxml/tinyxml.h"
using namespace std;



int printMatches(
  int matches,
  int maxNumMatches,
  TiXmlElement * element,
  int bugs,
  bool printLineNum,
  char * tag,
  std::vector<char *> attrNameList,
  std::vector<char *> attrValueList,
  char * getSpec);

//====================================================================


void badparms( char * msg) {
  cerr << "\nError: " << msg << endl;
  cerr << "Parms: -d debugLevel -max maxNumMatches -num y/n -in infile -tag tag -attr attr[=value] -attr attr[=value] ... -get <_text_|attrname>" << endl;
  exit(1);
}



//====================================================================



// See doc in badparms().

int main( int argc, char * argv[])
{
  int bugs = 0;
  int maxNumMatches = 0;
  bool printLineNum = false;
  char * infile = NULL;
  char * tag = NULL;
  char * getSpec = NULL;
  std::vector<char *> attrNameList;
  std::vector<char *> attrValueList;

  if (argc % 2 != 1) badparms("parms must come in key/value pairs");
  for (int iarg = 1; iarg < argc - 1; iarg += 2) {
    char * key = argv[iarg];
    char * val = argv[iarg+1];

    if (0 == strcmp( key, "-d")) {
      char * endptr;
      bugs = strtol( val, &endptr, 10);
      if (endptr != val + strlen(val)) badparms("invalid -d spec");
    }
    else if (0 == strcmp( key, "-max")) {
      char * endptr;
      maxNumMatches = strtol( val, &endptr, 10);
      if (endptr != val + strlen(val)) badparms("invalid -max spec");
    }
    else if (0 == strcmp( key, "-num")) {
      if (0 == strcmp( val, "n")) printLineNum = false;
      else if (0 == strcmp( val, "y")) printLineNum = true;
      else badparms("invalid -num spec");
    }
    else if (0 == strcmp( key, "-in")) {
      infile = val;
    }
    else if (0 == strcmp( key, "-tag")) {
      tag = val;
    }
    else if (0 == strcmp( key, "-attr")) {
      int ix = strcspn( val, "=");
      char * attrName;
      char * attrValue;
      if (ix < strlen( val)) {
        attrName = new char[ ix + 1];
        memcpy( attrName, val, ix);
        attrName[ix] = '\0';
        attrValue = val + ix + 1;
      }
      else {
        attrName = val;
        attrValue = NULL;
      }
      attrNameList.push_back( attrName);
      attrValueList.push_back( attrValue);
    }
    else if (0 == strcmp( key, "-get")) {
      getSpec = val;
    }
    else badparms("unknown keyword");
  }
  char * bugstg = argv[1];

  if (infile == NULL) badparms("-in not specified");
  if (tag == NULL) badparms("-tag not specified");
  if (getSpec == NULL) badparms("-get not specified");

  TiXmlDocument * doc = new TiXmlDocument( infile);
  if (! doc->LoadFile()) badparms("could not load file");

  TiXmlElement * firstElement = doc->FirstChildElement();
  printMatches( 0, maxNumMatches, firstElement,
    bugs, printLineNum, tag, attrNameList, attrValueList, getSpec);

  return 0;
} // end main

//=========================================================================

int printMatches(
  int matches,
  int maxNumMatches,
  TiXmlElement * element,
  int bugs,
  bool printLineNum,
  char * tag,
  std::vector<char *> attrNameList,
  std::vector<char *> attrValueList,
  char * getSpec)
{
  if (bugs >= 2) {
    cout << "printMatches: matches: " << matches
      << "  ele: " << element->Value()
      << "  row: " << element->Row() << endl;
  }
  if (0 == strcmp( tag, element->Value())) {
    if (bugs >= 2) cout << "  tag match" << endl;
    bool allOk = true;
    for (int ii = 0; ii < attrNameList.size(); ii++) {
      char * attrName = attrNameList.at(ii);
      char * attrValue = attrValueList.at(ii);
      const char * val = element->Attribute( attrName);
      if (val == NULL) {
        if (bugs >= 2) cout << "  attr not found: " << attrName << endl;
        allOk = false;
        break;
      }
      if (attrValue != NULL && 0 != strcmp( attrValue, val)) {
        if (bugs >= 2) cout << "  attr value mismatch: " << attrName << endl;
        allOk = false;
        break;
      }
      if (bugs >= 2) cout << "  attr matched: " << attrName << endl;
    }
    if (allOk) {
      if (bugs >= 2) cout << "  full match" << endl;
      matches++;
      if (printLineNum) cout << element->Row() << " ";
      if (0 == strcmp( getSpec, "_text_")) {
        const char * val = element->GetText();
        if (val == NULL) cout << "NULL" << endl;
        else cout << val << endl;
      }
      else {
        const char * val = element->Attribute( getSpec);
        if (val == NULL) cout << "NULL" << endl;
        else cout << val << endl;
      }
    }
  }
  if (bugs >= 2) cout << "  matches: " << matches << endl;
  if (maxNumMatches == 0 || matches < maxNumMatches) {
    if (bugs >= 2) cout << "  try subs of: " << element->Value() << endl;
    for (
      TiXmlElement * sub = element->FirstChildElement();
      sub != NULL;
      sub = sub->NextSiblingElement()) 
    {
      if (maxNumMatches > 0 && matches >= maxNumMatches) break;
      matches = printMatches( matches, maxNumMatches, sub,
        bugs, printLineNum, tag, attrNameList, attrValueList, getSpec);
    }
  }

  return matches;
} // end printMatches


//=========================================================================


