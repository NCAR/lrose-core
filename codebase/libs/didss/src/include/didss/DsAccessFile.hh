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

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 18:03:31 $
 *   $Id: DsAccessFile.hh,v 1.3 2016/03/03 18:03:31 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsAccessFile.hh: Class for manipulating the _dsaccess files that
 *                  control access to DsServer directories.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2000
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef DsAccessFile_HH
#define DsAccessFile_HH

/*
 **************************** includes **********************************
 */

#include <cassert>
#include <cstdio>
#include <string>

#include <didss/DsAccess.hh>
using namespace std;


/*
 ************************* class definitions ****************************
 */

class DsAccessFile
{

public:

  //////////////////////
  // Public constants //
  //////////////////////

  static const string fileName;
  

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsAccessFile(const bool debug_flag = false);


  /**********************************************************************
   * Destructor
   */

  virtual ~DsAccessFile();


  ///////////////////
  // Input methods //
  ///////////////////

  /**********************************************************************
   * getReadAccess() - Retrieve the read access indicated by the file.
   *
   * You must call either setAccessFile() or setAccessDir() before calling
   * this method.
   *
   * Returns a pointer to the DsAccess structure indicating the allowed
   * read access.  If setAccessDir() is used for the access search,
   * returns a DsAccess structure with complete public access if there are
   * no _dsaccess files at any level in the directory structure.
   *
   * Returns 0 if the search path has not been initialized with either
   * setAccessFile() or setAccessDir() or if there is some kind of error.
   *
   * Note that control of the allocated memory is transferred to the
   * client of this method, who must delete the memory when no longer
   * needed.
   */

  DsAccess *getReadAccess();
  

  /**********************************************************************
   * getWriteAccess() - Retrieve the write access from the current access
   *                    file.
   *
   * You must call either setAccessFile() or setAccessDir() before calling
   * this method.
   *
   * Returns a pointer to the DsAccess structure indicating the allowed
   * write access.  If setAccessDir() is used for the access search,
   * returns a DsAccess structure with complete public access if there are
   * no _dsaccess files at any level in the directory structure.
   *
   * Returns 0 if the search path has not been initialized with either
   * setAccessFile() or setAccessDir() or if there is some kind of error.
   *
   * Note that control of the allocated memory is transferred to the
   * client of this method, who must delete the memory when no longer
   * needed.
   */
  
  DsAccess *getWriteAccess();
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * setAccessFile() - Set the explicit path for the access file.  When
   *                   this is used, the access file can be named anything.
   *
   * Note that use of this method overrides any previous calls to
   * setAccessFile() or setAccessDir().
   */
  
  inline void setAccessFile(const string &file_path)
  {
    _filePath = file_path;
    _dirPath = "";
    _pathInitialized = true;
    _searchDirectories = false;
  }
  

  /**********************************************************************
   * getAccessFile() - Retrieve the current file path that will be used
   *                   to determine data access privileges.
   *
   * If there is no current file path, returns the empty string ("").
   */
  
  inline const string &getAccessFile(void) const
  {
    return _filePath;
  }
  

  /**********************************************************************
   * setAccessDir() - Set the directory path, using the $RAP_DATA_DIR
   *                  conventions, for the access file.  When this is used,
   *                  the access file must be named _dsaccess.  The
   *                  _dsaccess file in the closest subdirectory will be
   *                  used to determine the access permissions for the
   *                  directory path.  If no _dsaccess file is found in
   *                  the directory path, the public has full access to
   *                  the files in the directory.
   *
   * Note that use of this method overrides any previous calls to
   * setAccessFile() or setAccessDir().
   */
  
  inline void setAccessDir(const string &dir_path)
  {
    _filePath = "";
    _dirPath = dir_path;
    _pathInitialized = true;
    _searchDirectories = true;
  }
  

  /**********************************************************************
   * getAccessDir() - Retrieve the current directory path that will be used
   *                  to determine data access privileges.
   *
   * If there is no current directory path, returns the empty string ("").
   */
  
  inline const string &getAccessDir(void) const
  {
    return _dirPath;
  }
  

private:

  /////////////////////
  // Private members //
  /////////////////////

  static const string CLASS_NAME;
  
  bool _debugFlag;

  // Members describing the path for the access file

  bool _pathInitialized;
  
  string _filePath;

  bool _searchDirectories;
  string _dirPath;
  
  // Members for parsing the access file

  static const int MAX_TOKENS;
  static const int MAX_TOKEN_LEN;
  
  char **tokens;
  
  static const string LIMIT_DIRECTIVE;
  static const string LIMIT_END_DIRECTIVE;
  static const string ORDER_DIRECTIVE;
  static const string DENY_DIRECTIVE;
  static const string ALLOW_DIRECTIVE;
  
  static const string DENY_ALLOW_TOKEN;
  static const string ALLOW_DENY_TOKEN;
  
  static const string FROM_TOKEN;
  static const string ALL_TOKEN;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /**********************************************************************
   * _getAccess() - Retrieve the indicated access from the current access
   *                file.
   *
   * You must call either setAccessFile() or setAccessDir() before calling
   * this method.
   *
   * Returns a pointer to the DsAccess structure indicating the allowed
   * access of the given type.  If setAccessDir() is used for the access
   * search, returns a DsAccess structure with complete public access if
   * there are no _dsaccess files at any level in the directory structure.
   *
   * Returns 0 if the search path has not been initialized with either
   * setAccessFile() or setAccessDir() or if there is some kind of error.
   *
   * Note that control of the allocated memory is transferred to the
   * client of this method, who must delete the memory when no longer
   * needed.
   */

  DsAccess *_getAccess(const string &access_type);
  

  /**********************************************************************
   * _getAccessFilename() - Determine the path of the access file to use.
   *                        Returns 0 if there isn't an access file in
   *                        the directory.
   */

  string *_getAccessFilename(void);
  

  /**********************************************************************
   * _readAccessInfo() - Reads the indicated access information from the
   *                     given file.
   *
   * Returns a pointer to the appropriate access structure, or 0 if
   * there was an error of some type.  The client is given control of
   * the DsAccess pointer after this method call and so must delete the
   * pointer when finished with it.
   */

  DsAccess *_readAccessInfo(const string &access_filename,
			    const string &access_type);
  
  DsAccess *_readAccessInfo(FILE *access_file,
			    const string &access_type);
  

};

#endif /* DsAccessFile_HH */


