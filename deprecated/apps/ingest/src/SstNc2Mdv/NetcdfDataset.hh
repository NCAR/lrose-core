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
/*
-------------------------------------------------------------------------------
NetcdfDataset.hh - header file for class NetcdfDataset

NetcdfDataset acts pretty much like an NcFile object (from the Netcdf
library) except that it does not depend in any way on an open Netcdf file,
and all objects that may be accessed from the NetcdfDataset object
are owned by the NetcdfDataset object, not the caller (as are NcAtt
objects obtained from an NcFile or NcVar object).

Steve Carson, RAP, NCAR, Boulder, CO, 80307, USA
April 2005
-------------------------------------------------------------------------------
*/

#ifndef NetcdfDataset_h
#define NetcdfDataset_h

//
// SYSTEM INCLUDES
//

#include <string>
#include <vector>
#include <Ncxx/Nc3File.hh>

//
// PROJECT INCLUDES
//

//
// LOCAL INCLUDES
//

#include "SstNc2MdvGlobals.hh"
#include "NetcdfUtils.hh"

using namespace std;

//
// DEFINES
//

//
// CONSTS
//

//
// FORWARD REFERENCES
//

class NcdAtt;
class NcdDim;
class NcdVar;

//
// TYPEDEFS
//

// Maps "by number" of NcdVar*, NcdAtt*, NcdDim*

typedef map<int, NcdAtt*>  NcdAttsByNum_t;
typedef map<int, NcdDim*>  NcdDimsByNum_t;
typedef map<int, NcdVar*>  NcdVarsByNum_t;

// Map of numbers by name

typedef map<string, int>  NcdNumsByName_t;

static const string
   empty_str = "";

//-----------------------------------------------------------------------------
// Class "NetcdfDataset"
//-----------------------------------------------------------------------------

class NetcdfDataset
{
public:

   //-----
   // Data
   //-----

   //-----------------
   // Member Functions
   //-----------------
   
   //
   // Constructor
   //

   NetcdfDataset
      (
      const NcuPrtArgs_t  &arPrtArgs
      );

   //
   // Destructor
   //

   ~NetcdfDataset();

   //
   // Inquiry
   //

   //
   // Action
   //

   ///////////////////////////////////////////////////////////////////////////
   // LoadNetcdfFile
   //
   // Reads all global attributes, dimensions, and some or all variables
   // from the Netcdf input file named by apNcInputFilePath.
   // 
   // Takes 1 required and 1 optional args:
   //
   //    Arg                Type            Description
   //    ---------------------------------------------------------------------
   //    apNcInputFilePath  const string &  filename of netcdf input file
   //    aNcVarNames        vector<string>  list of variables to be loaded
   //
   // If apNcInputFilePath is present and the file it refers to is valid,
   // LoadNetcdfFile will ALWAYS load *all* the global attributes and
   // dimensions (but not variables) from the Netcdf file named by
   // apNcInputFilePath.
   //
   // If aNcVarNames is missing, LoadNetcdfFile will load *all* the variables
   // (headers and data) present in the input file.
   //
   // If aNcVarNames is present, LoadNetcdfFile will load *only* those
   // variables that are named in the vector of strings aNcVarNames.
   //
   // LoadNetcdfFile opens the Netcdf file and closes it when done.
   //

   int
   LoadNetcdfFile
      (
      const string          &apNcInputFilePath,
      const vector<string>  *aNcVarNames = NULL // default: load all variables
      );

   ///////////////////////////////////////////////////////////////////////////
   // add_att
   // add_dim
   // add_var
   //
   // Add global attributes, dimensions, and variables to the maps in the
   // NetcdfDataset object:
   //
   //    mAttsByName; // map<string,  NcdAtt*>
   //    mDimsByName; // map<string,  NcdDim*>
   //    mVarsByName; // map<string,  NcdVar*>
   //
   // These functions check to make sure the attribute, dimension, or
   // variable being added does not already exist in the map. If it does,
   // the existing attribute, dimension, or variable is NOT replaced
   // and a warning message is issued.
   //
   // Global attributes, dimensions, and variables are all owned by
   // the NetcdfDataset object to which they are added. The NetcdfDataset
   // destructor will take care of deleting all of these objects.
   //
   // Note that this effects the design of the NcdVar destructor; it must
   // NOT delete the objects pointed to by the NcdDim pointers in its
   // dimensions map, because these point to the very objects owned by
   // this NetcdfDataset object.

   int add_att( NcdAtt *aNcdAtt );
   int add_dim( NcdDim *aNcdDim );
   int add_var( NcdVar *aNcdVar );

   ///////////////////////////////////////////////////////////////////////////
   // repl_att
   // repl_dim
   // repl_var
   //
   // Replace global attributes, dimensions, and variables in the maps in the
   // NetcdfDataset object:
   //
   //    mAttsByName; // map<string,  NcdAtt*>
   //    mDimsByName; // map<string,  NcdDim*>
   //    mVarsByName; // map<string,  NcdVar*>
   //
   // These functions check to see whether the attribute, dimension, or
   // variable being added already exists in the appropriate map. If it does,
   // the existing attribute, dimension, or variable is DELETED and the new
   // new one is inserted in its place.

   int repl_att( NcdAtt *aNcdAtt );
   int repl_dim( NcdDim *aNcdDim );
   int repl_var( NcdVar *aNcdVar );

   ///////////////////////////////////////////////////////////////////////////
   // (del_att)
   // (del_dim)
   // del_var
   //
   // Delete global attributes, dimensions, and variables in the maps in the
   // NetcdfDataset object:
   //
   //    mAttsByName; // map<string,  NcdAtt*>
   //    mDimsByName; // map<string,  NcdDim*>
   //    mVarsByName; // map<string,  NcdVar*>
   //
   // These functions check to see whether the attribute, dimension, or
   // variable being added already exists in the appropriate map. If it does,
   // the existing attribute, dimension, or variable is DELETED and the new
   // new one is inserted in its place.

   //int del_att( NcdAtt *aNcdAtt );
   //int del_dim( NcdDim *aNcdDim );
   int del_var( NcdVar *aNcdVar );
   int del_var( string aVarName );

   ///////////////////////////////////////////////////////////////////////////
   // get_att
   // get_dim
   // get_var
   //
   // Get pointers to objects of type NcdVar, NcdDim, and NcdAtt from
   // the maps in this NetcdfDataset object.
   //
   // Since these functions get quantities from STL "map" objects,
   // they first check to make sure the requested element already
   // exists in the map; otherwise, merely requesting an element that
   // does not exist will actually *create* an entry for that element
   // with an empty value!

   NcdAtt*
      get_att( const string &aAttName );
   NcdAtt*
      get_att( int aAttNum );

   NcdDim*
      get_dim( const string &aDimName );
   NcdDim*
      get_dim( int aDimNum );

   NcdVar*
      get_var( const string &aVarName );
   NcdVar*
      get_var( int aVarNum );

   ///////////////////////////////////////////////////////////////////////////
   // copy_var
   //

   int
   copy_var
      (
      const string   &arVarName,
      NetcdfDataset  &arNcdSource
      );

   ///////////////////////////////////////////////////////////////////////////
   // WriteNetcdfFile
   //
   // Writes all global attributes, dimensions, and some or all variables
   // to a Netcdf output file.
   //
   // Takes 2 required and 1 optional args:
   //
   //    Arg                 Type              Description
   //    ---------------------------------------------------------------------
   //    arNcOutputFilePath  const string &    filename of netcdf output file
   //    apNcVarNames        vector<string> *  list of variables to be written
   //
   // If arNcOutputFilePath is present and the file it refers to is valid,
   // WriteNetcdfFile will automatically write *all* the global attributes and
   // dimensions from the Netcdf file named by apNcInputFilePath.
   //
   // If aNcVarNames is missing, LoadNetcdfFile will load *all* the variables
   // present in the input file. If aNcVarNames is present, LoadNetcdfFile
   // will load *only* those variable names that are in the vector of
   // strings aNcVarNames.
   //
   // WriteNetcdfFile opens the Netcdf file and closes it when done.
   //

   int
   WriteNetcdfFile
      (
      const string         &arNcOutputFilePath,
      const vector<string> *apNcVarNames = NULL // default: write all variables
      );

   //
   // Provide access to log file
   //

   MsgLog&
      GetMsgLog() { return *mLog; }

   //
   // Free all memory that was allocated by the NetcdfDataset object
   //

   void
      FreeMemory( void );

   int
      num_atts( void ) { return mNumAtts; }

   int
      num_dims( void ) { return mNumDims; }

   int
      num_vars( void ) { return mNumVars; }

   bool
      is_valid( void ) { return mIsValid; }

private:

   //------------
   // Member Data
   //------------

   string CLASS_NAME;

   //
   // Variables used to contain the dataset
   //

   bool  mIsValid;
   int   mUnlimitedDimSize;

   NcdAttsByNum_t  mAttsByNum; // map<int, NcdAtt*>
   NcdDimsByNum_t  mDimsByNum; // map<int, NcdDim*>
   NcdVarsByNum_t  mVarsByNum; // map<int, NcdVar*>

   NcdNumsByName_t 
      mAttNumsByName, // map<string, int>
      mDimNumsByName, // map<string, int>
      mVarNumsByName; // map<string, int>

   //
   // Count numbers of attributes, dimensions, and variables.
   //

   int
      mNumAtts,
      mNumDims,
      mNumVars;

   //
   // Netcdf file
   //

   string
      mNetcdfFilename;

   Nc3File
      *mNetcdfFile;

   //
   // Log and diagnostic output
   //

   MsgLog
      *mLog;

   FILE
      *mDiagOut;

   int
      mDiagOutLevel;

   NcuPrtArgs_t
      mNcuPrtArgs;

   //-----------------
   // Member Functions
   //-----------------

   //////////////////////////////////////////////////////////////////////////
   // LoadGlobalAttributes( void )
   // LoadDimensions( void )
   // LoadVariables( const vector<string> *aNcVarNames )
   //
   // Load global attributes, dimensions, and variables (headers and data)
   // from the current Netcdf input file into the NetcdfDataset.
   //

   int
   LoadGlobalAttributes( void );

   int
   LoadDimensions( void );

   int
   LoadVariables
      (
      const vector<string> *aNcVarNames = NULL // default: load all variables
      );

   //////////////////////////////////////////////////////////////////////////
   // OpenNetcdfFile
   // CloseNetcdfFile
   //
   // These member functions may be used to open and close the Netcdf file
   // explicitly. Normally it is not neccessary to call these functions
   // explicitly; they are called by the member functions LoadNetcdfFile
   // and WriteNetcdfFile.
   //
   // NcdFileIsValid
   //
   // NcdFileIsValid checks to see 1) if the Netcdf file pointer is
   // is NULL and if not 2) if the NcFile object "is_valid()".
   //
   // FileMode argument may be one of the following:
   //
   // NcFile::ReadOnly - read only; file must exist
   // NcFile::Write    - read or write; file must exit
   // NcFile::Replace  - read or write; writes over existing file
   // NcFile::New      - read or write; will not write over existing file
   //

   int
   OpenNetcdfFile
      (
      const string      &arNcInFileName,
      Nc3File::FileMode  aFileMode = Nc3File::ReadOnly
      );

   int
   CloseNetcdfFile( void );

   bool
   NcFileIsValid( void );

   ///////////////////////////////////////////////////////////////////////////
   // WriteGlobalAttributes( void )
   // WriteDimensions( void )
   // WriteVariables( const vector<string> *aNcVarNames )
   //

   int
   WriteGlobalAttributes( void );

   int
   WriteDimensions( void );

   int
   WriteVariables
      (
      const vector<string> *aNcVarNames = NULL
      );

   /////////////////
   // NewMethodName

   int
   NewMethodName( void );

}; // end class NetcdfDataset

//-----------------------------------------------------------------------------
// Class NcdAtt
//-----------------------------------------------------------------------------

class NcdAtt
{

public:

   // constructor

   NcdAtt
      (
      const string  &aName,
      Nc3Type        aType,
      long int      aNumVals,
      void          *aValues
      );

   // copy constructor

   NcdAtt( const NcdAtt  &aNcdAtt );

   // overloaded operator "="

   const NcdAtt& operator=( const NcdAtt  &aNcdAtt );

   // destructor

   ~NcdAtt( void );

   // inquiry

   bool
      is_valid( void ) const { return mIsValid; }

   const string &
      name( void ) const { return mName; }

   Nc3Type
      type( void ) const { return mType; }
   
   long int
      num_vals( void ) const { return mNumVals; }

   void *
      values( void ) const { return mValues; }

private:

   string CLASS_NAME;

   bool      mIsValid;
   string    mName;
   Nc3Type    mType;
   long int  mNumVals;
   void      *mValues;

}; // end class NcdAtt

//-----------------------------------------------------------------------------
// Class NcdDim
//-----------------------------------------------------------------------------

class NcdDim
{

public:

   // constructor

   NcdDim
      (
      const string  &aName,
      long int      aSize,
      bool          aIsUnlimited = false
      );

   // copy constructor

   NcdDim( const NcdDim &aNcdDim );

   // overloaded operator "="

   const NcdDim& operator=( const NcdDim  &aNcdDim );

   // destructor

   ~NcdDim( void );

   // inquiry

   long int
      size( void ) const { return mSize; }

   const string &
      name( void ) const { return mName; }

   bool
      is_unlimited( void ) const { return mIsUnlimited; }

   bool
      is_valid( void ) const { return mIsValid; }

private:

   string CLASS_NAME;

   bool      mIsValid;
   string    mName;
   long int  mSize;
   bool      mIsUnlimited;

}; // end class NcdDim

//-----------------------------------------------------------------------------
// Class NcdVar
//-----------------------------------------------------------------------------

class NcdVar
{

public:

   // constructor 1:
   // name, type, dims by number, data pointer

   NcdVar
      (
      const string     &aName,       // name
      Nc3Type           aType,        // type
      NcdDimsByNum_t   &aDimsByNum,  // map<int, NcdDim*>
      void             *aData        // pointer to data
      );

   // copy constructor
   //
   // NOTE: It is not possible to write a copy contructor for NcdVar!
   //
   // The NcdDim objects pointed to by map mDimsByNum are owned
   // by the NetcdfDataset object to which this NcdVar belongs. For this
   // reason the NcdVar destructor purposely does NOT delete the dimension
   // objects pointed to by its mDimsByNum map.
   //
   // Since a copy constructor only has one argument which is
   // a const reference to another NcdVar, it cannot have access
   // to the NetcdfDataset object to which this NcdVar will belong.
   // Thus it cannot obtain pointers to the actual NcdDim objects
   // owned by the NetcdfDataset object.
   //
   // If an NcdVar copy constructor simply made copies of the NcdDim
   // objects associated with the NcdVar to be copied and placed pointers
   // to them in mDimsByNum, a memory leak would result.
   //
   // In order to copy a variable from one NetcdfDataset object
   // to another, use the "copy_var" function in the target NetcdfDataset
   // object.

   // destructor

   ~NcdVar( void );

   // inquiry

   int
      num_dims( void ) { return mNumDims; }

   int
      num_atts( void ) { return mNumAtts; }

   Nc3Type
      type( void ) { return mType; }

   NcdAtt*
      get_att( const string &aAttName ); // C++ string OR (const char *) arg
   NcdAtt*
      get_att( int aAttNum );

   NcdDim*
      get_dim( const string &aDimName ); // C++ string OR (const char *) arg
   NcdDim*
      get_dim( int aDimNum );

   const NcdDimsByNum_t &
      get_dims( void ) { return mDimsByNum; }

   const string &
      name( void ) { return mName; }

   void *
      get_data( void ) { return mData; }

   bool
      is_valid( void ) { return mIsValid; }

   int
      num_vals( void );

   int
      data_bytes( void );

   const long int *
      edges( void ) { return mEdges; }

   // action

   int add_att( NcdAtt *aNcdAtt );

   int add_att( Nc3Token aName, ncbyte     aValue );
   int add_att( Nc3Token aName, char       aValue );
   int add_att( Nc3Token aName, short      aValue );
   int add_att( Nc3Token aName, int        aValue );
   int add_att( Nc3Token aName, float      aValue );
   int add_att( Nc3Token aName, double     aValue );

   int add_att( Nc3Token aName, const char *aValue );

   int add_att( Nc3Token aName, int aSize, const ncbyte  *aValue );
   int add_att( Nc3Token aName, int aSize, const short   *aValue );
   int add_att( Nc3Token aName, int aSize, const int     *aValue );
   int add_att( Nc3Token aName, int aSize, const long    *aValue );
   int add_att( Nc3Token aName, int aSize, const float   *aValue );
   int add_att( Nc3Token aName, int aSize, const double  *aValue );

   int
   replace_att // replaces attribute even if it already exists
      (
      NcdAtt *aNcdAtt
      );

   int
   copy_att // copies attribute from another NcdVar
      (
      const string  &aAttName,
      NcdVar        *aNcdVarSrc
      );

private:

   string CLASS_NAME;

   bool            mIsValid;
   string          mName;
   Nc3Type          mType;
   long int        *mEdges;
   void            *mData;

   NcdDimsByNum_t  mDimsByNum; // map<int,  NcdDim*>
   NcdAttsByNum_t  mAttsByNum; // map<int,  NcdAtt*>

   NcdNumsByName_t
      mAttNumsByName, // map<string, int>
      mDimNumsByName; // map<string, int>

   int
      mNumAtts,
      mNumDims;

}; // end class NcdVar

#endif // NetcdfDataset_h
