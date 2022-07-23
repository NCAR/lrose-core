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
// FirLoc.cc
//
// FirLoc object
//
// Feb 2003
//
///////////////////////////////////////////////////////////////
//
//

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <toolsa/toolsa_macros.h>
#include <Spdb/FirLoc.hh>
using namespace std;

// constructor
FirLoc::FirLoc()
{

}


// destructor
FirLoc::~FirLoc()
{
}


//////////////////////////////////////////////////////////////////
/// PrintAll()
/// PrintAll function can print all content in the file which was 
/// specified in the Main function out.
//////////////////////////////////////////////////////////////////

void FirLoc::PrintAll()
{
  
  for (size_t j=0;j<v1.size();j++)
    {
      cout << "sname=" << v1[j].sname << ", iname=" << v1[j].iname << ", npts=" << v1[j].npts << endl;

      for (size_t jj=0; jj< v1[j].npts; jj++) {
	cout << v1[j].lats[jj] << " " << v1[j].lons[jj] << endl;
      }
    }
}

//////////////////////////////////////////////////////////////////
/// PrintFir()
/// PrintFir function can print all content in the file for the
/// specified FIR.
//////////////////////////////////////////////////////////////////

void FirLoc::PrintFir(const string &name)
{

  for (size_t j=0;j<v1.size();j++)
  {
    if (MatchName(name, j)) {
      vector <double> lats;
      vector <double> lons;

      if (GetFirPoints(name, lats, lons)) {
	{
	  cout << "sname=" << v1[j].sname << ", iname=" << v1[j].iname << ", npts=" << v1[j].npts << endl;

	  for (size_t jj=0; jj< v1[j].npts; jj++) {
	    cout << v1[j].lats[jj] << " " << v1[j].lons[jj] << endl;
	  }
	}
      }
      break;
    }
  }
}


//////////////////////////////////////////////////////////////////
/// FirExists()
///
/// Given Fir name does it exist in the data file
//
// Returns true on success, false on failure.
//
//////////////////////////////////////////////////////////////////

bool FirLoc::FirExists(const string &name)
{

  // Go through all the info and search for the name to match 
  // either the station name or the long name
  
  for (size_t j=0;j<v1.size();j++)
  {
    if (MatchName(name, j))
      return true;
  }
  return false;
}  

//////////////////////////////////////////////////////////////////
/// GetFirPoints()
///
/// Given Fir name does it exist in the data file? If so get
/// and return the lat lons
//
// Returns true on success, false on failure.
//
//////////////////////////////////////////////////////////////////

bool FirLoc::GetFirPoints(const string &name, vector <double> &lats,
			    vector <double> &lons)

{

  lats.clear();
  lons.clear();

  // Convert the name to upper case 
  
  string ucname=name;
  AllToUpper(ucname);
  
  // Search for the name to match either the station name or the
  // long name

  // bool found=false;
  for (size_t j=0;j<v1.size();j++)
  {
    if (MatchName(name, j)) {
      // found=true;

      for (size_t jj=0; jj< v1[j].npts; jj++) {
	
	double lat=v1[j].lats[jj];
	double lon=v1[j].lons[jj];
      
	lats.push_back(lat);
	lons.push_back(lon);
      }
      return true;
    }
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
///
/// The input data file should be of the following format (yes, it
/// is comma-delimited!)
///             fir-name,
/// At this time, there are no lat,lons handled in the input file.
/// It is assumed that the entire line is the fir-name
///
///
//////////////////////////////////////////////////////////////////

int FirLoc::ReadData(const string &FilePath)
{

  // File must be local
  // Open input file for reading

  ifstream infile(FilePath.c_str());
  if (!infile) {
    cerr << "FirLoc::ReadData open failed for file:" << FilePath; 
    return(-1);
  }
 
  Info info;
      
  // get the lines from the file one-line-at-a-time. Lines come in 3 
  // different types:
  // station name and long name:     AGGG,Honiara,
  // keystring and number of points: POLYLINE 13
  // latlon:                         -4.833333 159.000000

  bool first_station=false;
  string keystring="POLYLINE ";
  string line;
  while (getline(infile, line, '\n')) {

    // Only station name lines have commas

    if (line.find(",", 0) != string::npos) {

      // If this is the first station do NOT push the previous station
      // into the class, otherwise do the push

      if (first_station) {
	first_station=false;
      } else {
	v1.push_back(info);   //push data fields into info class 
      }

      // Clear the info struct

      info.lats.clear();
      info.lons.clear();

      // Tokenize the line on commas
      
      vector <string> toks;
      Tokenize(line, ",", toks);

      // Find the station name of the FIR. This is the string before the first
      // comma

      string sname=toks[0];

      // Find the long name of the FIR. This is the string after the first comma
      // and before the second comma

      string iname=toks[1];

      // Convert names to uppercase before pushing into class
      
      string ucsname=sname;
      AllToUpper(ucsname);
      info.sname=ucsname;

      string uciname=iname;
      AllToUpper(uciname);
      info.iname=uciname;

      //      cout << "sname: " << sname << ", iname: " << iname <<endl;
    }

    // Is this a polyline and npoints line

    else if (line.find(keystring,0) != string::npos) {
	
      // Tokenize the line on blanks
      
      vector <string> toks;
      Tokenize(line, " \n\t\r", toks);

      // Get the number of points
      
      for (size_t ii=1; ii<toks.size(); ii++) {
	if (ii == 1) {
	  size_t npts=atoi(toks[ii].c_str());
	  info.npts=npts;

	  //	  cout <<"   npts: " << npts << endl;

	}
      }
    }

    // This must be a lat lon

    else {

      // Tokenize the line on blanks
      
      vector <string> toks;
      Tokenize(line, " \n\t\r", toks);

      // Get the lat lon

      for (size_t ii=0; ii<toks.size(); ii++) {
	if (ii == 0) {
	  double lat=strtod(toks[ii].c_str(), NULL);
	  info.lats.push_back(lat);
	  //	  cout << "   lat lon: " << lat << ", " ;
	}
	if (ii == 1) {
	  double lon=strtod(toks[ii].c_str(), NULL);
	  info.lons.push_back(lon);

	  //	  cout << lon << endl;

	}
      }
    }

  } //endwhile

  v1.push_back(info);   //push data fields into info class for last station
  
  infile.close();

  return(0);
}

//////////////////////////////////////////////////////////////////
/// AllToUpper
///
/// Convert the input string to all upper case
//
//
//////////////////////////////////////////////////////////////////

void FirLoc::AllToUpper(string &str)
{
  for (size_t ii=0; ii<str.size(); ii++) {
    if ((islower(str[ii]) != 0) && (isalpha(str[ii]) != 0)) {
      str[ii]=toupper(str[ii]);
    }
  }
}

//////////////////////////////////////////////////////////////////
/// Match name with v1 struct at specified index
///
/// Return true if match found, false otherwise
//
//////////////////////////////////////////////////////////////////

bool FirLoc::MatchName(const string &name, const size_t &idx)
{
  // Convert the name to upper case 
  
  string ucname=name;
  AllToUpper(ucname);
  
  // Search for the name to match either the station name or the
  // long name
  
  if (ucname==v1[idx].iname)
    return true;
  else if (ucname==v1[idx].sname)
    return true;

  return false;
}

//////////////////////////////////////////////
// tokenize a string into a vector of strings

void FirLoc::Tokenize(const string &str,
		      const string &spacer,
		      vector<string> &toks)
  
{
    
  toks.clear();
  size_t pos = 0;
  bool done = false;
  while (!done) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      done = true;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}
