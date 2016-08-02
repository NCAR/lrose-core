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
// FirLoc.h
//
// FirLoc object
//
// Feb 2003
//
///////////////////////////////////////////////////////////////
//
// FirLoc 
//
///////////////////////////////////////////////////////////////

#ifndef FirLoc_HH
#define FirLoc_HH


#include <string>
#include <vector>
using namespace std;

class FirLoc
{
 
public:

  //////////////////////////////////////////
  /// This class, info, defines public variables. 
  /// These variables are for 
  /// storing the data which was read from input data file into 
  /// the system main memory. 

  class Info{
  public:
    string sname;
    string iname;
    size_t npts;
    vector <double> lats;
    vector <double> lons;
  };

  FirLoc();    // Constructor
  ~FirLoc();   // Destructor

  // read in the data file - this must be called before using
  // other functions

  int ReadData(const string &FilePath);
  
  // Print the fir data to standard output

  void PrintAll();

  // Print fir data for one fir
  
  void PrintFir(const string &name);
  
  // Given fir name does it exist in the data file
  // Returns true on success, false on failure.

  bool FirExists(const string &name);

  // Given fir name does it exist in the data file?
  // Returns true on success, false on failure.
  // Also return the array of points for the FIR polygon

  bool GetFirPoints(const string &name, 
                    vector <double> &lats,
                    vector <double> &lons);

private:

  vector<Info> v1;
  vector<Info>::iterator infoi;

  // Convert the string to all uppercase

  void AllToUpper(string &str);

  /// Match name with v1 struct at specified index

  bool MatchName(const string &name, const size_t &idx);

  // tokenize a string into a vector of strings

  void Tokenize(const string &str,
                const string &spacer,
                vector<string> &toks);

};

#endif
