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
/************************************************************************
 * ReadParf: 
 *
 * Jason Craig
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2008
 ************************************************************************/

#ifndef ReadParf_HH
#define ReadParf_HH

#include "ReadForest.hh"

using namespace std;

class ReadParf: public ReadForest
{
public:

  ReadParf();
  virtual ~ReadParf();

  virtual int readForest(const char *fileName) 
    { return readParfForest(fileName); };

  virtual forestType getForestType()
  {  return PARFOREST; };
private:

  int readParfForest(const char *baseFileName);
  void free_parf();
  void free_tree(tree *thetree);
  void free_node(node *nodeptr);
  int load_tree(tree *thetree, char *fileName);
  node *load_tree_internal(FILE *inFile);
  
  bool read_success;
  long int _instance_count;
};

#endif
