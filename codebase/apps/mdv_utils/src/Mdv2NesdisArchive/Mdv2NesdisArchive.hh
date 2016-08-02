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
/*
 *  $Id: Mdv2NesdisArchive.hh,v 1.4 2016/03/04 02:22:10 dixon Exp $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////
//
// Class:	Mdv2NesdisArchive
//
// Author:	G. M. Cunning
//
// Date:	Mon Dec  3 09:33:27 2007
//
// Description: 
//
//

#ifndef    MDV2NESDISARCHIVE_H
#define    MDV2NESDISARCHIVE_H

#include <string>
#include <vector>

#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>

using namespace std;

/////////////////////////
// Forward declarations
class Params;
class Args;
class DsMdvxInput;
class DsMdvx;
class MdvxField;


/////////////////////////
// Class declaration
class Mdv2NesdisArchive {
  
public:

  // singleton instance invocations
  static Mdv2NesdisArchive *instance(int argc, char **argv);
  static Mdv2NesdisArchive *instance();

  // destructor  
  ~Mdv2NesdisArchive();

  // getErrStr -- returns error message
  string getErrStr() const { return _errStr; }  // run 
  
  // isOK -- check on class state
  bool isOK() const { return _isOK; }
  
  // run -- Execution of processing
  int run();

protected:
  
private:

  bool _isOK;
  string _progName;
  string _errStr;
  Args *_args;
  Params *_params;

  static const string _className;
  static const string DATA_EXTENSION;
  static const string LAT_EXTENSION;
  static const string LON_EXTENSION;

  static const float EPSILON = 0.000001;

  // Singleton instance pointer
  static Mdv2NesdisArchive *_instance; 

  DsMdvxInput *_dsInput;
  DsMdvx *_mdvxIn;
  MdvxPjg _proj;
  DateTime _obsTime;
  string _fieldName;

  string _pathInUse;

  // hide the constructors and copy consturctor
  Mdv2NesdisArchive();
  Mdv2NesdisArchive(int argc, char **argv);
  Mdv2NesdisArchive(const Mdv2NesdisArchive &);
  Mdv2NesdisArchive  &operator=(const Mdv2NesdisArchive &);

  bool _processFile(DsMdvx*);
  string _buildOutputPath(const string& base_dir, const string& field_name, const string& ext); 
  bool _writeData(int out_fd,int num_pts, float* data);
  bool _writeNavigation(const string& base_dir, const string& field_name);
  bool _writePad(int out_fd, int  fld_size);
  void _setupInput();
  bool _processFields(vector< const MdvxField* >& input_fields);
  vector<const MdvxField* > _readInput();

};

# endif   // MDV2NESDISARCHIVE_H
