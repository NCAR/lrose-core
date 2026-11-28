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
 * RunForest: 
 *
 * Jason Craig
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2008
 ************************************************************************/

#ifndef RunForest_HH
#define RunForest_HH

#include <dsdata/DsTrigger.hh>

#include "Args.hh"
#include "Params.hh"
#include "ReadForest.hh"
#include "MdvReader.hh"

using namespace std;


class RunForest
{
 public:

  ~RunForest(void);

  bool okay;

  //
  // Inst() - Retrieve the singleton instance of this class.
  static RunForest *Inst(int argc, char **argv);
  static RunForest *Inst();
  
  // Initialize the local data.
  //
  // Returns true if the initialization was successful, false otherwise.
  bool init();

  //
  // run() - run the program.
  int run();

 private:

  // Singleton instance pointer
  static RunForest *_instance;
  
  // Constructor -- private because this is a singleton object
  RunForest(int argc, char **argv);

  bool _read();
  void _createOutputFields(time_t trigger_time, Mdvx::master_header_t &mhdr, 
			   Mdvx::vlevel_header_t *vHeader,
			   string *filePaths);

  void _traverseForestGrid();
  
  //int _classify_tree(node *nptr, int &i, int &j, int &k, int &nx, int &ny,
  //	     attribute *attributes, vector<RunForest::input_field> *inFields);

  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  DsTrigger *_dataTrigger;
  ReadForest *_input;
  int _nx, _ny, _nz;

  attribute *_attributes;
  vector<InputField> _inFields;
  vector<float *> _outFields;
  vector<string> _outFieldsNames;
  vector<string> _outFieldsUnits;

  float *_poly_lats;
  float *_poly_lons;
  int _nPolygonPoints;
  MdvxProj _outproj;
};


#endif
