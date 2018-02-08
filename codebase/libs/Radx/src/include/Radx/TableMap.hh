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
// TableMap.hh
//
// Contains BUFR table information stored as a hash map
// of (key, value) pairs.  A key is a BUFR table descriptor.
//
// Brenda Javornik, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2017
//
///////////////////////////////////////////////////////////////

#ifndef TableMap_HH
#define TableMap_HH

#include <map>
#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>
#include <iterator>
#include <sstream>
#include <stdio.h>
#include <Radx/TableMapElement.hh>
#include <Radx/Radx.hh>


class TableMap {

public: 

  TableMap();
  ~TableMap();
  void setDebug(bool debug); 

  int ImportTables();
  int ImportTables(unsigned int masterTableVersion, unsigned int generatingCenter,
		   unsigned int localTableVersion, char *tablePath);
  int ImportTables2(unsigned int masterTableVersion, unsigned int generatingCenter,
		   unsigned int localTableVersion);
  int ImportTablesFromPath(unsigned int masterTableVersion, unsigned int generatingCenter,
			   unsigned int localTableVersion, char *tablePath);

  void AddDescriptorFromBufrFile(unsigned char f, unsigned char x,
                                         unsigned char y, const string fieldName,  
                                         int scale, const string units,
                                         int referenceValue, int dataWidthBits);

  void AddToTableD(std::vector <string>  descriptors);
  TableMapElement Retrieve(unsigned short key);
  bool filled();
  //  bool isComment(string &line);
  bool isWhiteSpace(string &str);
  string trim(string &str);

private:
  std::map<unsigned short, TableMapElement> table;  // TODO: should be unordered_map
  /*
  std::map<int, const char* []> masterBTables;
  std::map<unsigned int, const char* []> masterDTables;
  std::map<pair<unsigned int, unsigned int>, const char* []> localBTables;
  std::map<pair<unsigned int, unsigned int>,  const char* []> localDTables;

  std::map<unsigned int, unsigned int> masterBTablesSize;
  std::map<unsigned int, unsigned int> masterDTablesSize;
  std::map<pair<unsigned int, unsigned int>, unsigned int> localBTablesSize;
  std::map<pair<unsigned int, unsigned int>, unsigned int> localDTablesSize;
  */
  vector<string> split(const std::string &s, char delim);
  int ReadInternalTableB(const char **internalBufrTable,
			 size_t n);
  int ReadInternalTableD(const char **internalBufrTable,
			 size_t n);
  /*
  int ReadInternalTableD(unsigned int masterTableVersion,
				 unsigned int generatingCenter,
				 unsigned int localTableVersion);
  */

  int ReadTableB(string fileName);
  int ReadTableD(string fileName);
  int ImportTablesOld();

  bool _debug;
  string _tablePath;
};
#endif
