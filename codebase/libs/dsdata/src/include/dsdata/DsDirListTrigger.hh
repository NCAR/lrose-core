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
 *  $Id: DsDirListTrigger.hh,v 1.2 2016/03/03 18:06:33 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	DsDirListTrigger
// 
// Author:	G M Cunning
// 
// Date:	Wed Aug  1 20:37:19 2007
// 
// Description:	This trigger is intended to for non-realtime processing 
//		of files across multiple directories.
// 
// 


# ifndef    DsDirListTrigger_HH_H
# define    DsDirListTrigger_HH_H

#include <string>
#include <vector>
#include <cassert>

#include <dsdata/DsTrigger.hh>


using namespace std;

class DsDirListTrigger : public DsTrigger 
{
  
public:


  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsDirListTrigger();

  /**********************************************************************
   * Destructor
   */

  virtual ~DsDirListTrigger();

  /**********************************************************************
   * init() - Initialize the object.  The object must be initialized
   *          before it can be used.
   *
   *
   * input_dir_list: list of directories through which to iterate for reads.
   *
   * file_substring: Only files whose names contain this substring will
   *                 act as triggers.
   *
   * recurse: If true, the object will recurse through the input directory
   *          looking for files.  If false, the object will only look in
   *          the specified input directory.
   *
   * exclude_substring: Files whose names contain this substring will not
   *                    act as triggers.
   *
   *  Returns 0 on success, -1 on error.
   *
   * Use getErrStr() for error message.
   */

  int init(const vector<string> &input_dir_list,
	   const string &file_substring,
	   const bool recurse = false,
	   const string &exclude_substring = "");


  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * next() - Get the next trigger and set the triggerInfo accordingly
   *
   * Returns 0 upon success, -1 upon failure.
   */

  int next();
  

  /**********************************************************************
   * endOfData() - Check whether we are at the end of the data.
   */

  bool endOfData() const;
  

  /**********************************************************************
   * reset() - Reset to start of data list
   */

  void reset();
  


private:

  /////////////////////
  // Private members //
  /////////////////////

  bool _objectInitialized;

  vector <DsTrigger *> _inputDirList;

  size_t _dirListPtr;

};

# endif     /* DsDirListTrigger_HH_H */
