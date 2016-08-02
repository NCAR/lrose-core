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
 *  $Id: DsDirListTrigger.cc,v 1.5 2016/03/03 18:06:33 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	DsDirListTrigger
//
// Author:	G M Cunning
//
// Date:	Thu Aug  2 15:25:23 2007
//
// Description: This trigger is intended to for non-realtime processing 
//		of files across multiple directories.
//
//


#include <dsdata/DsDirListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>

using namespace std;


/**********************************************************************
 * Constructors
 */

DsDirListTrigger::DsDirListTrigger() :
  DsTrigger(TYPE_FILE_TRIGGER),
  _objectInitialized(false)
{


}

/**********************************************************************
 * Destructor
 */

DsDirListTrigger::~DsDirListTrigger()
{
  for (size_t i = 0; i < _inputDirList.size(); i++) {
    delete _inputDirList[i];
    _inputDirList[i] = 0;
  }
  _inputDirList.erase(_inputDirList.begin(), _inputDirList.end());
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/**********************************************************************
 * init() - Initialize the object.  The object must be initialized
 *          before it can be used.
 *
 * input_dir_list: directories to process.
 *
 *  Returns 0 on success, -1 on error.
 *
 * Use getErrStr() for error message.
 */

int DsDirListTrigger::init(const vector<string> &input_dir_list,
			   const string &file_substring,
			   const bool recurse,
			   const string &exclude_substring)
{
  const string method_name = "DsDirListTrigger::init()";

  _clearErrStr();

  for (size_t i = 0; i < input_dir_list.size(); i++) {

    DsInputDirTrigger *dsInputDirTrigger = 0;
    dsInputDirTrigger = new DsInputDirTrigger();

    if (!dsInputDirTrigger) {
          _errStr = "ERROR - " + method_name + "\n";
	  _errStr += "Unable to allocate DsInputDirTrigger object.\n";
	  return -1;
    }

    int iret = dsInputDirTrigger->init(input_dir_list[i], file_substring, true,
				      0, recurse, exclude_substring);

    if ( iret)
      {
	_errStr = "WARNING - " + method_name + "\n";
	_errStr += "Unable to initaliz eDsInputDirTrigger object.\n";
	delete dsInputDirTrigger;
	return -1;
      }


    _inputDirList.push_back(dsInputDirTrigger);
  }

  _dirListPtr = 0;

  _objectInitialized = true;  

  return 0;
}



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsDirListTrigger::next()
{
  const string method_name = "DsDirListTrigger::next()";
  
  assert(_objectInitialized);

  _clearErrStr();
  
  // try to go to next directory

  if(_inputDirList[_dirListPtr]->endOfData()) {
    _dirListPtr++;
  }


  // Check for end of trigger data

  if (endOfData())
      return -1;

  TriggerInfo triggerInfo;
  int iret = _inputDirList[_dirListPtr]->next(triggerInfo);
  _triggerInfo = triggerInfo;

  if ( iret)
  {
    _errStr = "WARNING - " + method_name + "\n";
    _errStr += "Unable to get next file path.\n";
    return -1;
  }


  return 0;
}

/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsDirListTrigger::endOfData() const
{
  assert(_objectInitialized);
  
  if (_dirListPtr >= _inputDirList.size()) 
    return true;
  
  return false;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsDirListTrigger::reset()
{
  assert(_objectInitialized);
  
  _dirListPtr = 0;
}
