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
 *  $Id: MdvCombine.hh,v 1.4 2016/03/04 02:22:10 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header: MdvCombine
// 
// Author: G M Cunning
// 
// Date:	Tue Jan 16 11:33:31 2001
// 
// Description:	This program collects MDV data from several input 
//		directories and copies data to a single output directory.
// 
// 
// 
// 


# ifndef    MDV_COMBINE_H
# define    MDV_COMBINE_H

// C++ include files
#include <string>

// System/RAP include files
#include <dsdata/DsTrigger.hh>
using namespace std;

// Local include files

/////////////////////////
// Forward declarations
class Args;
class Params;
class Trigger;

class MdvCombine {
  
public:

  ////////////////////
  // public methods //
  ////////////////////


  // instance -- create the Singleton
  static MdvCombine *instance(int argc, char **argv);
  static MdvCombine *instance();

  // destructor
  ~MdvCombine();

  // isOK -- check on class
  bool isOk() const { return _isOk; }

  // getErrStr -- returns error message
  string getErrStr() const { return _errStr; }

  // execute -- executes the class
  bool execute();




protected:

  ///////////////////////
  // protected members //
  ///////////////////////
  

  ///////////////////////
  // protected methods //
  ///////////////////////

private:

  /////////////////////
  // private members //
  /////////////////////
  bool _debug;
  bool _isOk;
  string _progName;
  string _errStr;
  Args *_args;
  Params *_params;
  string _outputUrl;

  static const string _className;

  // Singleton instance pointer
  static MdvCombine *_instance;  

  /////////////////////
  // private methods //
  /////////////////////

  void _clearErrStr() { _errStr = ""; }
  DsTrigger* _createTrigger();

  // hide -- singleton
  MdvCombine(int argc, char **argv);
  //MdvCombine &operator=(const MdvCombine &);
  //MdvCombine(const MdvCombine &);

};



# endif     /* MDV_COMBINE_H */
