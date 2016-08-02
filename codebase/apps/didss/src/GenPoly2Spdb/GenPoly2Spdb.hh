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

/**
 * @file GenPoly2Spdb
 * @brief  The algorithm that reads ascii and converts to GenPoly SPDB
 * @class GenPoly2Spdb
 * @brief  The algorithm that reads ascii and converts to GenPoly SPDB
 */

#ifndef GenPoly2Spdb_H
#define GenPoly2Spdb_H

#include "Args.hh"
#include "Params.hh"
#include <string>

class DsInputPath;
class GenPoly;

class GenPoly2Spdb
{
public:
  /**
   * Constructor from command args
   * @param[in] argc
   * @param[in] argv
   */
  GenPoly2Spdb (int argc, char **argv);

  /**
   * Destructor
   */
  ~GenPoly2Spdb(void);

  /**
   * Run the algorithm
   * @return 0 for success, -1 for error
   */
  int Run(void);

  /**
   * True if object is correct
   */
  bool isOK;

protected:
private:

  std::string _progName;  /**< name of program */
  char *_paramsPath;      /**< name of parameter file */
  Args _args;             /**< Arguments handler */
  Params _params;         /**< Program parameters */
  time_t _time0;          /**< Lower time limit when times are restricted */
  time_t _time1;          /**< Upper time limit when times are restricted */


  DsInputPath *_setInput(void) const;
  int _parseInput (const string &inputFilePath, int index);
  bool _parseInit(const string &inputFilePath, int &year, int &month, int &day);
  void _parseInputLine(FILE *in, int year, int month, int day,
		       bool &first, GenPoly &pt, time_t &time0,
		       time_t &time1);
  bool _inRange(const time_t &t) const;
};

#endif

