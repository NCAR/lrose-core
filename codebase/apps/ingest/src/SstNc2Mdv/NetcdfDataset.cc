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
NetcdfDataset.cc - class to hold data from netcdf file

Steve Carson, RAP, NCAR, Boulder, CO, 80307, USA
April 2005
-------------------------------------------------------------------------------
*/

#include "NetcdfDataset.hh"

using namespace std;

//-----------------------------------------------------------------------------
// Class NetcdfDataset
//-----------------------------------------------------------------------------

//-------------------------------------------
// NetcdfDataset::NetcdfDataset (constructor)
//-------------------------------------------

NetcdfDataset::
NetcdfDataset( const NcuPrtArgs_t  &arPrtArgs )

{ // begin NetcdfDataset::NetcdfDataset (constructor)

const string
   METHOD_NAME = "NetcdfDataset(constructor)";

CLASS_NAME = "NetcdfDataset";

//
// Save diagnostic file info in member variables:
//    mNcuPrtArgs   : struct containing all diagnostic print info
//    mDiagOut      : pointer to diagnostic output file
//    mDiagOutLevel : diagnostic file output verbosity level
//    mLog          : logging object
//

mNcuPrtArgs   = arPrtArgs;

mDiagOut      = arPrtArgs.diag;
mDiagOutLevel = arPrtArgs.diag_lvl;
mLog          = arPrtArgs.log;

//
// Set dataset variables to empty values
//

mUnlimitedDimSize = 0;

mAttsByNum.clear(); // map<int, NcdAtt*>
mDimsByNum.clear(); // map<int, NcdDim*>
mVarsByNum.clear(); // map<int, NcdVar*>

mAttNumsByName.clear(); // map<string, int>
mDimNumsByName.clear(); // map<string, int>
mVarNumsByName.clear(); // map<string, int>

mNumAtts = 0;
mNumDims = 0;
mNumVars = 0;

mNetcdfFilename = "";
mNetcdfFile = NULL;

mIsValid = true;

//
// Print message to diagnostic output file
//

if((mDiagOut != NULL) && (mDiagOutLevel >= 2))
   {
   fprintf(mDiagOut, "\n%s::%s:\n", CLASS_NAME.c_str(), METHOD_NAME.c_str());
   fprintf(mDiagOut, "   Constructed NetcdfDataset object\n");
   fprintf(mDiagOut, "   mDiagOutLevel          = %d\n",
      mDiagOutLevel);
   fprintf(mDiagOut, "   mDiagOut               = %8.8lX\n",
      (unsigned long) mDiagOut);
   }

} // end NetcdfDataset::NetcdfDataset (constructor)

//-------------------------------------------
// NetcdfDataset::~NetcdfDataset (destructor)
//-------------------------------------------

NetcdfDataset::
~NetcdfDataset()

{ // begin NetcdfDataset::~NetcdfDataset

FreeMemory();

} // end NetcdfDataset::~NetcdfDataset

//------------------------------
// NetcdfDataset::LoadNetcdfFile
//------------------------------

int
NetcdfDataset::
LoadNetcdfFile
   (
   const string         &arNcInputFilePath,
   const vector<string> *aNcVarNames // default = NULL; load all variables
   )

{ // begin NetcdfDataset::LoadNetcdfFile

const string
   METHOD_NAME = "LoadNetcdfFile";

int
   status;

//
// Open the netcdf input file (sets mNetcdfFile)
//

status = OpenNetcdfFile(arNcInputFilePath, Nc3File::ReadOnly);

//
// If Netcdf file was not opened successfully, return FAILURE
//

if( status == FAILURE )
   {
   if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
      {
      fprintf(mDiagOut, "\nWARNING: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut, "   Unable to open Netcdf intput file '%s'\n",
         arNcInputFilePath.c_str());
      fprintf(mDiagOut, "   No attributes, dimensions, or variables loaded\n");
      }

   return FAILURE;
   }

//
// If Netcdf input file object open, but not valid, return FAILURE
//

if( (mNetcdfFile != NULL) && (!mNetcdfFile->is_valid()) )
   {
   if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
      {
      fprintf(mDiagOut, "\nWARNING: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut, "   Netcdf input file opened but not valid: '%s'\n",
         arNcInputFilePath.c_str());
      fprintf(mDiagOut, "   No attributes, dimensions, or variables loaded\n");
      }

   return FAILURE;
   }

//
// If Netcdf input file is valid, load all global attributes, dimensions
// and some or all variables
//

if( (mNetcdfFile != NULL) && (mNetcdfFile->is_valid()) )
   {
   //
   // Load all global attributes
   //

   status = LoadGlobalAttributes();

   //
   // If "LoadGlobalAttributes" was not successful, return FAILURE
   //

   if( status != SUCCESS )
      {
      if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
         {
         fprintf(mDiagOut, "\nERROR: %s::%s:\n",
            CLASS_NAME.c_str(), METHOD_NAME.c_str());
         fprintf(mDiagOut, "   Unable to load global attributes from '%s'\n",
            mNetcdfFilename.c_str());
         }

      return FAILURE;
      }

   //
   // Load all dimensions
   //

   status = LoadDimensions();

   //
   // If "LoadDimensions" was not successful, return FAILURE
   //

   if( status != SUCCESS )
      {
      if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
         {
         fprintf(mDiagOut, "\nERROR: %s::%s:\n",
            CLASS_NAME.c_str(), METHOD_NAME.c_str());
         fprintf(mDiagOut, "   Unable to load dimensions from '%s'\n",
            mNetcdfFilename.c_str());
         }

      return FAILURE;
      }

   //
   // Load variables
   //

   status = LoadVariables( aNcVarNames );

   //
   // If "LoadVariables" was not successful, return FAILURE
   //

   if( status != SUCCESS )
      {
      if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
         {
         fprintf(mDiagOut, "\nERROR: %s::%s:\n",
            CLASS_NAME.c_str(), METHOD_NAME.c_str());
         fprintf(mDiagOut, "   Unable to load variables from '%s'\n",
            mNetcdfFilename.c_str());
         }

      return FAILURE;
      }

   } // end if Netcdf input file valid

CloseNetcdfFile();

return SUCCESS;

} // end NetcdfDataset::LoadNetcdfFile
	
//------------------------------
// NetcdfDataset::OpenNetcdfFile
//------------------------------

int
NetcdfDataset::
OpenNetcdfFile
   (
   const string      &arNcFileName,
   Nc3File::FileMode  aFileMode       // Default: Nc3File::ReadOnly
   )                                 // (see NetcdfDataset.hh)

{ // begin NetcdfDataset::OpenNetcdfFile

const string
   METHOD_NAME = "OpenNetcdfFile";

//
// Try opening the Netcdf input file
//

mNetcdfFile = new Nc3File(arNcFileName.c_str(), aFileMode);

//
// See if the Netcdf file was opened successfully
//

if( mNetcdfFile->is_valid() )
   {
   mNetcdfFilename = arNcFileName;

   if((mDiagOut != NULL) && (mDiagOutLevel >= 2))
      {
      fprintf(mDiagOut, "\n%s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut,
         "   Opened Netcdf file '%s'\n", arNcFileName.c_str() );
      }

   return SUCCESS;
   }
else
   {
   mNetcdfFilename = "";
   mNetcdfFile = NULL;

   POSTMSG("Unable to open Netcdf input file '%s'", arNcFileName.c_str());

   if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut,
         "   Unable to open Netcdf file '%s'\n", arNcFileName.c_str() );
      }

   return FAILURE;
   }

} // end NetcdfDataset::OpenNetcdfFile


//------------------------------------
// NetcdfDataset::CloseNetcdfFile
//------------------------------------

int
NetcdfDataset::
CloseNetcdfFile( void )

{ // begin NetcdfDataset::CloseNetcdfFile

const string
   METHOD_NAME = "CloseNetcdfFile";

//
// Check to see that Netcdf file is valid (open)
//

if( !NcFileIsValid() )
   {
   if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
      {
      fprintf(mDiagOut, "\nINFO: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut,
         "   No Netcdf file is currently open\n");
      }

   return FAILURE;
   }

//
// Close the Netcdf file
//

if( !mNetcdfFile->close() )
   {
   if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut,
         "   Error closing Netcdf file '%s'\n",
         mNetcdfFilename.c_str() );
      }
   return FAILURE;
   }

if((mDiagOut != NULL) && (mDiagOutLevel >= 2))
   {
   fprintf(mDiagOut, "\n%s::%s:\n",
      CLASS_NAME.c_str(), METHOD_NAME.c_str());
   fprintf(mDiagOut,
      "   Closed Netcdf file '%s'\n", mNetcdfFilename.c_str());
   }

mNetcdfFilename = "";
delete mNetcdfFile;
mNetcdfFile     = NULL;

return SUCCESS;

} // end NetcdfDataset::CloseNetcdfFile

//-----------------------------
// NetcdfDataset::NcFileIsValid
//-----------------------------

bool
NetcdfDataset::
NcFileIsValid( void )

{ // begin NetcdfDataset::NcFileIsValid

if(mNetcdfFile != NULL)
   {
   if(mNetcdfFile->is_valid())
      {
      return true;
      }
   else
      {
      return false;
      }
   }
else
   {
   return false;
   }

} // end NetcdfDataset::NcFileIsValid

//------------------------------------
// NetcdfDataset::LoadGlobalAttributes
//------------------------------------

int
NetcdfDataset::
LoadGlobalAttributes( void )

{ // begin NetcdfDataset::LoadGlobalAttributes

const string
   METHOD_NAME = "LoadGlobalAttributes";

string
   att_name_str;

int
   status;

//
// Make sure Netcdf input file object is valid
//

if( !NcFileIsValid() )
   {
   if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut, "   Netcdf file is not valid\n");
      }

   return FAILURE;
   }

//
// Load all global attributes from Netcdf input file
//

for( int i_att = 0; i_att < mNetcdfFile->num_atts(); ++i_att )
   {
   //
   // Get an Nc3Att object from the input file
   //

   Nc3Att *att = mNetcdfFile->get_att(i_att);

   //
   // Get attribute name as a C++ string
   //

   att_name_str = (char *) att->name();

   //
   // Create an NcdAtt object
   //

   Nc3Values
      *att_values = att->values();

   NcdAtt
   *ncd_att =
   new NcdAtt
      (
      att_name_str,
      att->type(),
      att->num_vals(),
      att_values->base()
      );

   delete att_values;

   //
   // Store a pointer to the new NcdAtt object in map mAttsByName,
   // after checking to see that there is no attribute by that
   // name already in the map.
   //

   status = add_att( ncd_att );

   //
   // If the new attribute was not added to the map mAttsByName,
   // we must delete it here, since it will not be deleted by
   // the NetcdfDataset destructor.
   //

   if( status != SUCCESS )
      {
      delete ncd_att;
      }

   //
   // Delete the Nc3Att object since we own it
   //

   delete att;

   } // end for i_att

//
// Print out the global attributes that were loaded
// (if diagnostic printout is turned on)
//

if( (mDiagOut != NULL) && (mDiagOutLevel >= 2) )
   {
   fprintf(mDiagOut, "\n%s::%s:\n",
      CLASS_NAME.c_str(), METHOD_NAME.c_str());
   fprintf(mDiagOut, "   Found %d global attributes in '%s'\n",
      mNetcdfFile->num_atts(), mNetcdfFilename.c_str());

   NcdAtt
      *ncd_att;
   
   for(int i_att = 0; i_att < mNumAtts; ++i_att )
      {
      ncd_att = mAttsByNum[ i_att ];

      Nc3Type        att_type = ncd_att->type();
      long int  att_num_vals = ncd_att->num_vals();
      void          *att_val = ncd_att->values();
      const char   *att_name = ncd_att->name().c_str();

      if( (att_type == nc3Char) || (att_num_vals == 1) ) // att is scalar
         {

         //char *att_str = new char[att_num_vals+1];

         switch( att_type )
            {
            case nc3Byte:
               fprintf(mDiagOut, "   Att# %3d:'%32s' = %d\n",
                  i_att, att_name, *((ncbyte *) att_val) );
               break;
            case nc3Char:

               //strncpy(att_str, (char *) att_val, att_num_vals);
               //att_str[att_num_vals] = '\0';

               fprintf(mDiagOut, "   Att# %3d:'%32s' = '%s'\n",
                  i_att, att_name, (char *) att_val );
               break;
            case nc3Short:
               fprintf(mDiagOut, "   Att# %3d:'%32s' = %d\n",
                  i_att, att_name, *((short int *) att_val) );
               break;
            case nc3Int:
               fprintf(mDiagOut, "   Att# %3d:'%32s' = %d\n",
                  i_att, att_name, *((int *) att_val) );
               break;
            case nc3Float:
               fprintf(mDiagOut, "   Att# %3d:'%32s' = %g\n",
                  i_att, att_name, *((float *) att_val) );
               break;
            case nc3Double:
               fprintf(mDiagOut, "   Att# %3d:'%32s' = %g\n",
                  i_att, att_name, *((double *) att_val) );
               break;
            case nc3NoType:
               break;
            } // end switch att_type

         //delete [] att_str;

         }
      else // att is vector
         {

         fprintf(mDiagOut, "   Att# %3d:'%32s', #values = %ld:\n",
            i_att, att_name, att_num_vals );

         for( int j = 0; j < (int) att_num_vals; ++j )
            {

            //char *att_str = new char[att_num_vals+1];

            switch( att_type )
               {
               case nc3Byte:
                  fprintf(mDiagOut, "      %d\n", ((ncbyte *) att_val)[j] );
                  break;
               case nc3Char:

                  //strncpy(att_str, &(((char *) att_val)[j]), att_num_vals);
                  //att_str[att_num_vals] = '\0';

                  fprintf(mDiagOut, "      '%s'\n", (char *) att_val );
                  break;
               case nc3Short:
                  fprintf(mDiagOut, "      %d\n", ((short int *) att_val)[j] );
                  break;
               case nc3Int:
                  fprintf(mDiagOut, "      %d\n", ((int *) att_val)[j] );
                  break;
               case nc3Float:
                  fprintf(mDiagOut, "      %g\n", ((float *) att_val)[j] );
                  break;
               case nc3Double:
                  fprintf(mDiagOut, "      %g\n", ((double *) att_val)[j] );
               case nc3NoType:
                  break;
               } // end switch att_type

            //delete [] att_str;

            } // end for j (loop over att values)
         } // end if( (att_type == ncChar) || (att_num_vals == 1) )

      } // end for i (loop over attributes)
   } // end if diag print

return SUCCESS;

} // end NetcdfDataset::LoadGlobalAttributes


//------------------------------
// NetcdfDataset::LoadDimensions
//------------------------------

int
NetcdfDataset::
LoadDimensions( void )

{ // begin NetcdfDataset::LoadDimensions

const string
   METHOD_NAME = "LoadDimensions";

string
   dim_name_str;

NcdDim
   *ncd_dim;

Nc3Dim
   *nc_dim;

int
   status;

//
// If Netcdf input file object is not valid,
// do nothing and return FAILURE.
//

if( !NcFileIsValid() )
   {
   if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut, "   Invalid Netcdf input file object\n");
      }
   return FAILURE;
   }

//
// Load all dimensions from Netcdf input file.
//

bool
   ncdim_is_unlimited = false;

for( int i_dim = 0; i_dim < mNetcdfFile->num_dims(); ++i_dim )
   {
   nc_dim = mNetcdfFile->get_dim(i_dim);

   //
   // See if dimension is "unlimited"
   //

   if( nc_dim->is_unlimited() )
      {
      mUnlimitedDimSize = nc_dim->size();
      ncdim_is_unlimited = true;
      }
   else
      {
      ncdim_is_unlimited = false;
      }

   //
   // Get dimension name as a C++ string
   //

   dim_name_str = nc_dim->name();

   //
   // Create a new NcdDim object
   //

   ncd_dim =
   new NcdDim
      (
      dim_name_str,
      nc_dim->size(),
      ncdim_is_unlimited
      );

   //
   // Store a pointer to the new NcdDim object in map mDimsByName,
   // after checking to see that there is no attribute by that
   // name already in the map.
   //

   status = add_dim( ncd_dim );

   //
   // If the new dimension was not added to the map mDimsByName,
   // we must delete it here, since it will not be deleted by
   // the NetcdfDataset destructor.
   //

   if( status != SUCCESS )
      {
      delete ncd_dim;
      }

   } // end for i_dim

if( (mDiagOut != NULL) && (mDiagOutLevel >= 2) )
   {
   fprintf(mDiagOut, "\n%s::%s:\n",
      CLASS_NAME.c_str(), METHOD_NAME.c_str());
   fprintf(mDiagOut, "   Found %d dimensions in '%s'\n",
      mNetcdfFile->num_dims(), mNetcdfFilename.c_str());

   for(int i_dim = 0; i_dim < mNumDims; ++i_dim )
      {
      ncd_dim = mDimsByNum[ i_dim ];

      fprintf(mDiagOut, "   Dim# %3d:%9ld:'%s'\n",
         i_dim, ncd_dim->size(), ncd_dim->name().c_str() );
      }
   }

return SUCCESS;

} // end NetcdfDataset::LoadDimensions

//-----------------------------
// NetcdfDataset::LoadVariables
//-----------------------------

int
NetcdfDataset::
LoadVariables
   (
   const vector< string > *aNcVarNames
   )

{ // begin NetcdfDataset::loadVariables

const string
   METHOD_NAME = "LoadVariables";

vector<string>
   nc_var_names;

string
   tmp_string;

NcuPrtArgs_t
   ncu_prt_args;

int
   status;

mNcuPrtArgs.callerID = CLASS_NAME + "::" + METHOD_NAME;

//
// Make sure Netcdf input file object is valid
//

if( !NcFileIsValid() )
   {
   if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut, "   Invalid Netcdf input file object\n");
      fprintf(mDiagOut, "   No variables loaded\n");
      }

   return FAILURE;
   }

//
// If aNcVarNames is empty, or if the first element is "all",
// load all the variables in the netcdf file, otherwise use
// the list of variable names passed in via aNcVarNames
//

if (
   (aNcVarNames == NULL) ||
   (aNcVarNames->size() > 0 && !strcmp( (*aNcVarNames)[0].c_str(), "all" ))
   )
   {
   // load all variable names from netcdf file
   for(int i = 0; i < mNetcdfFile->num_vars(); ++i)
      nc_var_names.push_back( mNetcdfFile->get_var(i)->name() );
   }
else
   {
   // use explicit list of variable names
   for(int i = 0; i < (int) aNcVarNames->size(); ++i)
      nc_var_names.push_back((*aNcVarNames)[i]);
   }

if( (mDiagOut != NULL) && (mDiagOutLevel >= 2) )
   {
   fprintf(mDiagOut, "\n%s::%s:\n",
      CLASS_NAME.c_str(), METHOD_NAME.c_str());
   fprintf(mDiagOut, "   Loading the following variables:\n");

   for(int i = 0; i < (int) nc_var_names.size(); ++i)
      {
      fprintf(mDiagOut, "   %3d '%s'\n", i, nc_var_names[i].c_str());
      }
   }

string
   var_name;

NcdVar
   *ncd_var;

NcdDimsByNum_t
   ncd_dims;  // map<int, NcdDim*>

// Pointers for receiving data from NcVar object.

ncbyte  *ptr_byte;
char    *ptr_char;
short   *ptr_short;
int     *ptr_int;
float   *ptr_float;
double  *ptr_double;
void    *ptr_void;

//
// Load all requested variables from Netcdf file
//

for(int i_var = 0; i_var < (int) nc_var_names.size(); ++i_var)
   {

   // Get name of ith variable in nc_var_names[]

   var_name = nc_var_names[ i_var ];

   // Get NcVar pointer for variable nc_var_names[ i_var ]

   Nc3Var *nc_var = mNetcdfFile->get_var( var_name.c_str() );

   // Load the data from this variable using the appropriate type
   // pointer. Then assign the typed pointer to a void pointer
   // for subsequent use in the ncd_var struct which gets
   // saved in map mVarsByName

   if( nc_var != NULL )
      {

      switch( nc_var->type() )
         {
         case nc3Byte:
            NCU::LoadNcVar(mNcuPrtArgs, *mNetcdfFile, var_name, &ptr_byte);
            ptr_void = ptr_byte;
            break;
         case nc3Char:
            NCU::LoadNcVar(mNcuPrtArgs, *mNetcdfFile, var_name, &ptr_char);
            ptr_void = ptr_char;
            break;
         case nc3Short:
            NCU::LoadNcVar(mNcuPrtArgs, *mNetcdfFile, var_name, &ptr_short);
            ptr_void = ptr_short;
            break;
         case nc3Int:
            NCU::LoadNcVar(mNcuPrtArgs, *mNetcdfFile, var_name, &ptr_int);
            ptr_void = ptr_int;
            break;
         case nc3Float:
            NCU::LoadNcVar(mNcuPrtArgs, *mNetcdfFile, var_name, &ptr_float);
            ptr_void = ptr_float;
            break;
         case nc3Double:
            NCU::LoadNcVar(mNcuPrtArgs, *mNetcdfFile, var_name, &ptr_double);
            ptr_void = ptr_double;
            break;
         case nc3NoType:
            break;
         }

      //
      //---------------------------------
      // Get dimensions for this variable
      //

      // Get dimensions for this variable from the NetcdfDataset object
      // (instead of creating new NcdDim objects) and save pointers to them
      // in map "ncd_dims". This assumes that "LoadDimensions" has already
      // been called and that this Netcdf file is internally consistent.
      // (in other words, there SHOULD NOT be any dimensions associated
      // with this variable that are not already in the NetcdfDataset)
      //
      // This approach means that the dimension maps for each variable
      // in this NetcdfDataset will contain pointers to the very same
      // dimension objects that are owned by the NetcdfDataset object.
      // (instead of their own copies of "identical" NcdDim objects)
      //
      // The NcdVar destructor must NOT delete the dimensions in
      // its dimension map; dimensions will be deleted by the
      // NetcdfDataset destructor.

      int
         n_dims = nc_var->num_dims();

      // Clear map of pointers to NcdDim objects

      ncd_dims.clear();

      // Create map of all dimensions for this variable

      for(int i_dim = 0; i_dim < n_dims; ++i_dim)
         {
         string
            nc_dim_name_str = nc_var->get_dim(i_dim)->name();

         NcdDim
            *ncd_dim = get_dim( nc_dim_name_str );

         ncd_dims[ i_dim ] = ncd_dim;
         }

      //---------------------------
      // Create a new NcdVar object

      ncd_var =
      new NcdVar
         (
         var_name,
         nc_var->type(),
         ncd_dims,
         ptr_void
         );

      //
      //-----------------------------------------
      // Add attributes to this new NcdVar object
      //

      int
         n_atts = nc_var->num_atts();

      // Create new NcdAtt objects and add them to the NcdVar object

      for(int i_att = 0; i_att < n_atts; ++i_att)
         {
         Nc3Att
            *nc_att = nc_var->get_att(i_att);

         string
            nc_att_name_str = nc_att->name();

         Nc3Values
            *nc_values = nc_att->values();

         NcdAtt
         *ncd_att =
         new NcdAtt
            (
            nc_att_name_str,
            nc_att->type(),
            nc_att->num_vals(),
            nc_values->base()
            );

         status = ncd_var->add_att( ncd_att );
         if(status != SUCCESS) delete ncd_att;

         delete nc_values;
         delete nc_att;

         } // end for i_att

      //
      // Add the new NcdVar object to the NetcdfDataset object
      //

      status = add_var( ncd_var );

      //
      // If the new variable was not added to the map mVarssByName,
      // we must delete it here, since it will not be deleted by
      // the NetcdfDataset destructor.
      //

      if( status != SUCCESS )
         {
         delete ncd_var;
         }

      }
   else // variable not found in mNetcdfFile
      {

      if( mDiagOut != NULL )
         {
         fprintf(mDiagOut, "\nWARNING: %s::%s:\n",
            CLASS_NAME.c_str(), METHOD_NAME.c_str());
         fprintf(mDiagOut, "   Variable '%s' not found in '%s'\n",
            var_name.c_str(), mNetcdfFilename.c_str());
         }

      } // end if( nc_var != NULL )

   } // end for i_var

nc_var_names.clear();

return SUCCESS;

} // end NetcdfDataset::LoadVariables

//-----------------------
// NetcdfDataset::add_att
//-----------------------

int
NetcdfDataset::
add_att
   (
   NcdAtt *aNcdAtt
   )

{ // begin NetcdfDataset::add_att

const string
   METHOD_NAME = "add_att";

//
// If this attribute does NOT already exist in this NetcdfDataset object
// add it and return SUCCESS, otherwise, DO NOTHING and return FAILURE.
//

string
   att_name = aNcdAtt->name();

if( mAttNumsByName.find( att_name ) == mAttNumsByName.end() )
   {
   ++mNumAtts;
   int att_indx = mNumAtts - 1; // mAttsByNum is zero-based

   mAttsByNum[ att_indx ]     = aNcdAtt;
   mAttNumsByName[ att_name ] = att_indx;

   return SUCCESS;
   }
else // attribute already in this NetcdfDataset object
   {
   if( mDiagOut != NULL )
      {
      fprintf(mDiagOut,
         "\nWARNING: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut,
         "   Attribute '%s' already exists in NetcdfDataset; not replaced\n",
         att_name.c_str());
      }

   return FAILURE;
   }

} // end NetcdfDataset::add_att

//-----------------------
// NetcdfDataset::add_dim
//-----------------------

int
NetcdfDataset::
add_dim
   (
   NcdDim *aNcdDim
   )

{ // begin NetcdfDataset::add_dim

const string
   METHOD_NAME = "add_dim";

//
// If this dimension does NOT already exist in this NetcdfDataset object
// add it and return SUCCESS, otherwise, DO NOTHING and return FAILURE.
//

string
   dim_name = aNcdDim->name();

if( mDimNumsByName.find( dim_name ) == mDimNumsByName.end() )
   {
   ++mNumDims;
   int dim_indx = mNumDims - 1; // mDimsByNum is zero-based

   mDimsByNum[ dim_indx ]     = aNcdDim;
   mDimNumsByName[ dim_name ] = dim_indx;

   return SUCCESS;
   }
else // dimension already in this NetcdfDataset object
   {
   if( mDiagOut != NULL )
      {
      fprintf(mDiagOut,
         "\nWARNING: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut,
         "   Dimension '%s' already exists in NetcdfDataset; not replaced\n",
         dim_name.c_str());
      }

   return FAILURE;
   }

} // end NetcdfDataset::add_dim

//-----------------------
// NetcdfDataset::add_var
//-----------------------

int
NetcdfDataset::
add_var
   (
   NcdVar *aNcdVar
   )

{ // begin NetcdfDataset::add_var

const string
   METHOD_NAME = "add_var";

//
// If this variable does NOT already exist in this NetcdfDataset object
// add it and return SUCCESS, otherwise, DO NOTHING and return FAILURE.
//

string
   var_name = aNcdVar->name();

if( mVarNumsByName.find( var_name ) == mVarNumsByName.end() )
   {
   ++mNumVars;
   int var_indx = mNumVars - 1; // mVarsByNum is zero-based

   mVarsByNum[ var_indx ]     = aNcdVar;
   mVarNumsByName[ var_name ] = var_indx;

   return SUCCESS;
   }
else if ( mVarsByNum[mVarNumsByName[ var_name ]] == NULL )
   {
   mVarsByNum[mVarNumsByName[ var_name ]] = aNcdVar;

   return SUCCESS;
   }
else // variable already in this NetcdfDataset object
   {
   if( mDiagOut != NULL )
      {
      fprintf(mDiagOut,
         "\nWARNING: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut,
         "   Variable '%s' already exists in NetcdfDataset; not replaced\n",
         var_name.c_str());
      }

   return FAILURE;
   }

} // end NetcdfDataset::add_var

//------------------------
// NetcdfDataset::repl_att
//------------------------

int
NetcdfDataset::
repl_att
   (
   NcdAtt *aNcdAtt
   )

{ // begin NetcdfDataset::repl_att

const string
   METHOD_NAME = "repl_att";

//
// If this attribute does NOT already exist in this NetcdfDataset object
// add it and return SUCCESS, otherwise, DELETE the existing attribute,
// replace it with the new one, and return SUCCESS.
//

string
   att_name = aNcdAtt->name();

if( mAttNumsByName.find( att_name ) == mAttNumsByName.end() )
   {
   // Add attribute to this NetcdfDataset object

   ++mNumAtts;
   int att_indx = mNumAtts - 1; // mAttsByNum is zero-based

   mAttsByNum[ att_indx ]     = aNcdAtt;
   mAttNumsByName[ att_name ] = att_indx;
   }
else // replace attribute in this NetcdfDataset object
   {
   delete mAttsByNum[ mAttNumsByName[ att_name ] ];
   mAttsByNum[ mAttNumsByName[ att_name ] ] = aNcdAtt;
   }

return SUCCESS;

} // end NetcdfDataset::repl_att

//------------------------
// NetcdfDataset::repl_dim
//------------------------

int
NetcdfDataset::
repl_dim
   (
   NcdDim *aNcdDim
   )

{ // begin NetcdfDataset::repl_dim

const string
   METHOD_NAME = "repl_dim";

//
// If this dimension does NOT already exist in this NetcdfDataset object
// add it and return SUCCESS, otherwise, DELETE the existing dimension,
// replace it with the new one, and return SUCCESS.
//

string
   dim_name = aNcdDim->name();

if( mDimNumsByName.find( dim_name ) == mDimNumsByName.end() )
   {
   // Add dimension to this NetcdfDataset object

   ++mNumDims;
   int dim_indx = mNumDims - 1; // mDimsByNum is zero-based

   mDimsByNum[ dim_indx ]     = aNcdDim;
   mDimNumsByName[ dim_name ] = dim_indx;
   }
else // replace dimension in this NetcdfDataset object
   {
   delete mDimsByNum[ mDimNumsByName[ dim_name ] ];
   mDimsByNum[ mDimNumsByName[ dim_name ] ] = aNcdDim;
   }

return SUCCESS;

} // end NetcdfDataset::repl_dim

//------------------------
// NetcdfDataset::repl_var
//------------------------

int
NetcdfDataset::
repl_var
   (
   NcdVar *aNcdVar
   )

{ // begin NetcdfDataset::repl_var

const string
   METHOD_NAME = "repl_var";

//
// If this variable does NOT already exist in this NetcdfDataset object
// add it and return SUCCESS, otherwise, DELETE the existing variable,
// replace it with the new one, and return SUCCESS.
//

string
   var_name = aNcdVar->name();

if( mVarNumsByName.find( var_name ) == mVarNumsByName.end() )
   {
   // Add new variable to this NetcdfDataset object

   ++mNumVars;
   int var_indx = mNumVars - 1; // mVarsByNum is zero-based

   mVarsByNum[ var_indx ]     = aNcdVar;
   mVarNumsByName[ var_name ] = var_indx;
   }
else // replace existing variable in this NetcdfDataset object
   {
   NcdVar *ncv = mVarsByNum[ mVarNumsByName[ var_name ] ];

   if(ncv != NULL)
      {
      delete ncv;
      }

   mVarsByNum[ mVarNumsByName[ var_name ] ] = aNcdVar;
   }

return SUCCESS;

} // end NetcdfDataset::repl_var

//-------------------------------
// NetcdfDataset::WriteNetcdfFile
//-------------------------------

int
NetcdfDataset::
WriteNetcdfFile
   (
   const string          &arNcOutputFilePath,
   const vector<string>  *apNcVarNames
   )

{ // begin NetcdfDataset::WriteNetcdfFile

string
   METHOD_NAME = "WriteNetcdfFile";

int
   status;

//
// Open the netcdf output file (sets mNetcdfFile)
//

status = OpenNetcdfFile(arNcOutputFilePath.c_str(), Nc3File::Replace);

mNetcdfFile->set_fill(Nc3File::NoFill);

//
// If Netcdf output file was not opened successfully return FAILURE.
//

if( status == FAILURE )
   {
   if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut, "   Unable to open Netcdf output file'%s'\n",
         arNcOutputFilePath.c_str());
      }

   return FAILURE;
   }

//
// If Netcdf file is valid, write all global attributes
// dimensions, and some or all variables.
//

if( NcFileIsValid() )
   {
   //
   // Write all global attributes
   //

   status = WriteGlobalAttributes();

   //
   // If "WriteGlobalAttributes" was not successful close the Netcdf file
   // and return FAILURE.
   //

   if( status != SUCCESS )
      {
      if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
         {
         fprintf(mDiagOut, "\nERROR: %s::%s:\n",
            CLASS_NAME.c_str(), METHOD_NAME.c_str());
         fprintf(mDiagOut, "   Unable to write global attributes to '%s'\n",
            mNetcdfFilename.c_str());
         }

      CloseNetcdfFile();
      return FAILURE;
      }

   //
   // Write all dimensions
   //

   status = WriteDimensions();

   //
   // If "WriteDimensions" was not successful close the Netcdf file
   // and return FAILURE.
   //

   if( status != SUCCESS )
      {
      if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
         {
         fprintf(mDiagOut, "\nERROR: %s::%s:\n",
            CLASS_NAME.c_str(), METHOD_NAME.c_str());
         fprintf(mDiagOut, "   Unable to write dimensions to '%s'\n",
            mNetcdfFilename.c_str());
         }

      CloseNetcdfFile();
      return FAILURE;
      }

   //
   // Write some or all variables
   //

   status = WriteVariables( apNcVarNames );

   //
   // If "WriteVariables" was not successful close the Netcdf file
   // and return FAILURE.
   //

   if( status != SUCCESS )
      {
      if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
         {
         fprintf(mDiagOut, "\nERROR: %s::%s:\n",
            CLASS_NAME.c_str(), METHOD_NAME.c_str());
         fprintf(mDiagOut, "   Unable to write variables to '%s'\n",
            mNetcdfFilename.c_str());
         }

      CloseNetcdfFile();
      return FAILURE;
      }

   } // end if Netcdf input file valid

CloseNetcdfFile();
return SUCCESS;

} // end NetcdfDataset::WriteNetcdfFile

//-------------------------------------
// NetcdfDataset::WriteGlobalAttributes
//-------------------------------------

int
NetcdfDataset::
WriteGlobalAttributes( void )

{ // begin WriteGlobalAttributes

const string
   METHOD_NAME = "WriteGlobalAttributes";

NcdAtt
   *ncd_att;

Nc3Type
   att_type;

void
   *att_vals;

long int
   att_nvals;

int
   att_status,
   status;

string
   att_name;

//
// Check to see that Netcdf output file is valid
//

if( !NcFileIsValid() )
   {
   if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut,
         "   Netcdf output file not valid\n");
      }

   return FAILURE;
   }

//
// Loop over all global attributes
//

if((mDiagOut != NULL) && (mDiagOutLevel >= 3))
   {
   fprintf(mDiagOut, "\n%s::%s:\n",
      CLASS_NAME.c_str(), METHOD_NAME.c_str());
   fprintf(mDiagOut,
      "   Writing global attributes to '%s':\n",
      mNetcdfFilename.c_str());
   }

status = SUCCESS;

for(int i_att = 0; i_att < mNumAtts; ++i_att)
   {
   ncd_att   = mAttsByNum[ i_att ];

   att_name  = ncd_att->name();
   att_type  = ncd_att->type();
   att_nvals = ncd_att->num_vals();
   att_vals  = ncd_att->values();

   //
   // Write out attribute with appropriate "add_att" call
   //

   att_status =
   NCU::AddGlobalAtt
      (
      mNcuPrtArgs,
      mNetcdfFile,
      att_name,
      att_type,
      att_nvals,
      (void *) att_vals
      );

   if( att_status != SUCCESS )
      {
      status = FAILURE;
      }

   } // end for i_att

return status;

} // end NetcdfDataset::WriteGlobalAttributes

//-------------------------------
// NetcdfDataset::WriteDimensions
//-------------------------------

int
NetcdfDataset::
WriteDimensions( void )

{ // begin NetcdfDataset::WriteDimensions

const string
   METHOD_NAME = "WriteDimensions";

NcdDim
   *ncd_dim;

int
   unlimited_dim_count = 0;

bool
   statusOK = true;

//
// Check to see that Netcdf output file is valid
//

if( !NcFileIsValid() )
   {
   if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut,
         "   No current open Netcdf output file\n");
      }

   return FAILURE;
   }

//
// Loop over all NcdDim objects in mDimsByName
//

if((mDiagOut != NULL) && (mDiagOutLevel >= 3))
   {
   fprintf(mDiagOut, "\n%s::%s:\n",
      CLASS_NAME.c_str(), METHOD_NAME.c_str());
   fprintf(mDiagOut,
      "   Writing dimensions to '%s':\n", mNetcdfFilename.c_str());
   }

int
   i_dim = 0;

while( (i_dim < mNumDims) && statusOK )
   {
   //
   // Get NcdDim pointer from map
   //

   ncd_dim = mDimsByNum[ i_dim ];

   //
   // If this is the "UNLIMITED" Netcdf dimension,
   // make sure there is not already an UNLIMITED dimension
   // in the output file.
   //

   if( ncd_dim->is_unlimited() )
      {
      ++unlimited_dim_count;

      if( unlimited_dim_count == 1 )
         {
         mNetcdfFile->add_dim( ncd_dim->name().c_str() );

         if((mDiagOut != NULL) && (mDiagOutLevel >= 3))
            {
            fprintf(mDiagOut,
               "   Dim %3d name = '%-24s' size = UNLIMITED\n",
               i_dim, ncd_dim->name().c_str());
            }
         }
      else // error: only 1 UNLIMITED dimension allowed in a Netcdf file
         {
         statusOK = false;

         if( mDiagOut != NULL )
            {
            fprintf(mDiagOut,
            "   ERROR writing Dim %3d name = '%-24s' size = UNLIMITED\n",
               i_dim, ncd_dim->name().c_str());
            fprintf(mDiagOut,
            "      Only one UNLIMITED dimension allowed\n");
            }
         }

      }
   else // dimension is not UNLIMITED
      {
      mNetcdfFile->add_dim( ncd_dim->name().c_str(), ncd_dim->size() );

      if((mDiagOut != NULL) && (mDiagOutLevel >= 3))
         {
         fprintf(mDiagOut,
            "   Dim %3d name = '%-24s' size = %9ld\n",
            i_dim, ncd_dim->name().c_str(), ncd_dim->size());
         }
      } // end if ncd_dim->is_unlimited()

   //
   // Increment loop iterator
   //

   ++i_dim;

   } // end while (iDim < mNumDims) && statusOK

//   } // end for i loop over dimensions

return SUCCESS;

} // end NetcdfDataset::WriteDimensions

//------------------------------
// NetcdfDataset::WriteVariables
//------------------------------

int
NetcdfDataset::
WriteVariables
   (
   const vector<string> *aNcVarNames
   )

{ // begin NetcdfDataset::WriteVariables

const string
   METHOD_NAME = "WriteVariables";

vector<string>
   nc_var_names;

//
// If Netcdf output file object is NOT valid, return FAILURE.
//

if( !NcFileIsValid() )
   {
   if((mDiagOut != NULL) && (mDiagOutLevel >= 1))
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut, "   Invalid Netcdf output file object\n");
      }

   return FAILURE;
   }

//
// If aNcVarNames is empty, or if the first element is "all",
// write all the variables in mVarsByName to the Netcdf file, 
// otherwise use the list of variable names passed in via aNcVarNames
//

if (
   (aNcVarNames == NULL) ||
   (aNcVarNames->size() > 0 && !strcmp((*aNcVarNames)[0].c_str(), "all"))
   )
   {
   // write all variables to netcdf file
   for(int i_var = 0; i_var < mNumVars; ++i_var)
      {
      NcdVar *ncd_var = mVarsByNum[ i_var ];
      if(ncd_var != NULL)
         {
         nc_var_names.push_back( ncd_var->name() );
         }
      }
   }
else
   {
   // use explicit list of variable names
   for(int i_var = 0; i_var < (int) aNcVarNames->size(); ++i_var)
      nc_var_names.push_back( (*aNcVarNames)[ i_var ]);
   }

if( (mDiagOut != NULL) && (mDiagOutLevel >= 3) )
   {
   fprintf(mDiagOut, "\n%s::%s:\n",
      CLASS_NAME.c_str(), METHOD_NAME.c_str());
   fprintf(mDiagOut, "   Writing the following variables:\n");
   }

//------------------------------------------
// Enter "define" mode in Netcdf output file
//------------------------------------------

//
// Add all variables and their attributes but do NOT write any
// data yet; this will be done below in a second, separate loop
// over all variables in order to avoid switching back and forth
// between "define" and "data" modes, which can be vey expensive.
//

NcdVar
   *ncd_var;

NcdDim
   *ncd_dim;

NcdAtt
   *ncd_att;

Nc3Var
   *nc_var,
   *nc_tmp;

Nc3Type
   var_type;

Nc3Dim
   **nc_dims;

int
   ndims,
   natts,
   elem_size;

const char
   *dim_name,
   *var_name;

string
   var_name_str,
   type_name;

//
// Loop 1: write variable headers and attrbutes
//

for(int i_var = 0; i_var < (int) nc_var_names.size(); ++i_var)
   { // begin for i_var (first loop)

   // Get name of variable as both string and char*

   var_name_str = nc_var_names[ i_var ];
   var_name = var_name_str.c_str();

   // Get the NcdVar object for the variable named nc_var_names[i_var]
   // from this NetcdfDataset object

   ncd_var  = get_var( var_name_str );

   // If nc_var_names[ i_var ] exists in this NetcdfDataset object
   // go ahead and write its header info to the output file.
   //
   // Otherwise, DO NOTHING and SILENTLY go to the next variable name

   if( ncd_var != NULL )
      {

      // For the current variable, get the number of dimensions,
      // number of attributes, and the type.

      ndims    = ncd_var->num_dims();
      natts    = ncd_var->num_atts();
      var_type = ncd_var->type();

      // Get array of Nc3Dim pointers for this variable

      if( ndims > 0 )
         {
         nc_dims = new Nc3Dim*[ ndims ];

         // Get Nc3Dim pointers for this variable from the Netcdf output file
         // (Assumes "WriteDimensions" has already been called)

         for(int i_dim = 0; i_dim < ndims; ++i_dim)
            {
            ncd_dim          = ncd_var->get_dim( i_dim );
            dim_name         = ncd_dim->name().c_str();
            nc_dims[ i_dim ] = mNetcdfFile->get_dim( dim_name );
            }
         }

      // See if current variable has already been added to
      // the Netcdf output file. 
      //
      // Note we must temporarily change the error behavior of the Netcdf
      // library so that the program does not stop if the variable name is not
      // found in the output file, otherwise we would get an error of the form:
      //    ncvarid: ncid 5: Variable not found
      // and excution would terminate.

      Nc3Error *nc_err = new Nc3Error( Nc3Error::silent_nonfatal );
      nc_tmp = mNetcdfFile->get_var( var_name );
      delete nc_err;

      if( nc_tmp == NULL ) // variable not yet in output Netcdf file
         {
         // Add the current variable to the Netcdf output file as an
         // array or scalar as appropriate. Save pointer to the new
         // Netcdf output variable "nc_var";

         if( ndims > 0 )
            {
            nc_var =
            mNetcdfFile->add_var(
               var_name, var_type, ndims, (const Nc3Dim **) nc_dims);
            }
         else
            {
            nc_var =
            mNetcdfFile->add_var(var_name, var_type);
            }
         }
      else // variable already exists in output file
         {
         nc_var = nc_tmp;
         }

      if( ndims > 0 ) delete [] nc_dims;

      // Add attributes to the new Netcdf output variable.

      if( natts > 0 )
         {
         for(int i_att = 0; i_att < natts; ++i_att)
            {

            ncd_att = ncd_var->get_att( i_att );

            int      att_size  = ncd_att->num_vals();
            Nc3Token  att_name  = ncd_att->name().c_str();
            Nc3Type   att_type  = ncd_att->type();
            void     *att_vals = ncd_att->values();

            NCU::AddVarAtt
               (
               mNcuPrtArgs,
               nc_var,
               att_name,
               att_type,
               att_size,
               att_vals
               );

            } // end for i_att
         } // end if natts > 0

      } // end if ncd_var != NULL

   } // end for i_var (first loop)

//----------------------------------
// "data" mode in Netcdf output file

//
// Write the data contents of each variable to the Netcdf output file.
// This keeps the output file in "data" mode continuously while
// writing out data, which is much more efficient than switching
// between "data" and "define" modes.
//

//
// Second loop over all variables to be written
//

for(int i_var = 0; i_var < (int) nc_var_names.size(); ++i_var)
   {

   // Get name of variable as both string and char*

   var_name_str = nc_var_names[i_var];
   var_name = var_name_str.c_str();

   //
   // Get NcdVar pointer from map
   // (will need this to get pointer to data block)
   //

   ncd_var = get_var( var_name_str );

   // If nc_var_names[ i_var ] exists in this NetcdfDataset object
   // go ahead and write its data values to the output file.
   //
   // Otherwise DO NOTHING and SILENTLY go to the next variable name

   if( ncd_var != NULL )
      {
      void *data = ncd_var->get_data();

      NCU::WriteVarData
         (
         mNcuPrtArgs,
         *mNetcdfFile,
         var_name,
         ncd_var->edges(),
         data
         );

      if( (mDiagOut != NULL) && (mDiagOutLevel >= 3) )
         {
         elem_size = NCU::ElementSize( ncd_var->type() );
         type_name = NCU::NcTypeName( ncd_var->type() );

         fprintf(
            mDiagOut,
            "   %3d '%24s' (%8s) : ndim = %2d, bytes = %9d, edges =",
            i_var,
            ncd_var->name().c_str(),
            type_name.c_str(),
            ncd_var->num_dims(),
            ncd_var->num_vals() * elem_size
            );

         for(int i_dim = 0; i_dim < mNumDims; ++i_dim)
            {
            fprintf(mDiagOut,", %ld", ncd_var->get_dim(i_dim)->size());
            }

         fprintf(mDiagOut,"\n");

         } // end diag printout

      } // end if ncd_var != NULL

   } // end for i_var (second loop)

nc_var_names.clear();

return SUCCESS;

} // end NetcdfDataset::WriteVariables

//---------------------------------
// NetcdfDataset::get_att - by name
//---------------------------------

// Get an attribute from this NetcdfDataset while being careful not to ask
// for an attribute which does not exist - which would cause an empty entry
// for the attribute to be inserted into the NetcdfDataset!

NcdAtt*
NetcdfDataset::
get_att
   (
   const string &aNcdAttName
   )

{ // begin NetcdfDataset::get_att - by name

const string
   METHOD_NAME = "get_att - by name";

// Check to see that requested variable name is NOT already in map;
// otherwise, just requesting a name that is not already in the map
// creates a blank entry for the requested name!

if( mAttNumsByName.find( aNcdAttName ) != mAttNumsByName.end() )
   {
   int att_num = mAttNumsByName[ aNcdAttName ];
   return mAttsByNum[ att_num ];
   }
else
   {
   if( mDiagOut != NULL )
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut, "   Attribute '%s' not found in NetcdfDataset\n",
         aNcdAttName.c_str());
      }

   return NULL;
   }

} // end NetcdfDataset::get_att - by name

//-----------------------------------
// NetcdfDataset::get_att - by number
//-----------------------------------

NcdAtt*
NetcdfDataset::
get_att
   (
   int aAttNum
   )

{ // begin NetcdfDataset::get_att - by number

const string
   METHOD_NAME = "get_att - by number";

if( (aAttNum >= 0) && (aAttNum < mNumAtts) )
   {
   return mAttsByNum[ aAttNum ];
   }
else
   {
   if( mDiagOut != NULL )
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut, "   Attribute %d not found in NetcdfDataset\n",
         aAttNum);
      }

   return NULL;
   }

} // end NetcdfDataset::get_att - by number

//---------------------------------
// NetcdfDataset::get_dim - by name
//---------------------------------

// Get a dimension from this NetcdfDataset while being careful not to ask
// for a dimension which does not exist - which would cause an empty entry
// for the dimension to be inserted into the NetcdfDataset!

NcdDim*
NetcdfDataset::
get_dim
   (
   const string  &aNcdDimName
   )

{ // begin NetcdfDataset::get_dim - by name

const string
   METHOD_NAME = "get_dim - by name";

// Check to see that requested variable name is NOT already in map;
// otherwise, just requesting a name that is not already in the map
// creates a blank entry for the requested name!

if( mDimNumsByName.find( aNcdDimName ) != mDimNumsByName.end() )
   {
   int dim_num = mDimNumsByName[ aNcdDimName ];
   return mDimsByNum[ dim_num ];
   }
else
   {
   if( mDiagOut != NULL )
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut, "   Dimension '%s' not found in NetcdfDataset\n",
         aNcdDimName.c_str());
      }

   return NULL;
   }

} // end NetcdfDataset::get_dim - by name

//-----------------------------------
// NetcdfDataset::get_dim - by number
//-----------------------------------

NcdDim*
NetcdfDataset::
get_dim
   (
   int aDimNum
   )

{ // begin NetcdfDataset::get_dim - by number

const string
   METHOD_NAME = "get_dim - by number";

if( (aDimNum >= 0) && (aDimNum < mNumDims) )
   {
   return mDimsByNum[ aDimNum ];
   }
else
   {
   if( mDiagOut != NULL )
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut, "   Dimension %d not found in NetcdfDataset\n",
         aDimNum);
      }

   return NULL;
   }

} // end NetcdfDataset::get_dim - by number

//---------------------------------
// NetcdfDataset::get_var - by name
//---------------------------------

// Get an NcdVar* from map "mVarsByName" while being careful not to ask for
// a variable which does not exist - which would cause an empty entry
// for the variable to be inserted into the map!

NcdVar*
NetcdfDataset::
get_var
   (
   const string &aNcdVarName
   )

{ // begin NetcdfDataset::get_var - by name

const string
   METHOD_NAME = "get_var - by name";

// Check to see that requested variable name is NOT already in map;
// otherwise, just requesting a name that is not already in the map
// creates a blank entry for the requested name!

NcdVar *ncv_ptr_return = NULL;

if( mVarNumsByName.find( aNcdVarName ) != mVarNumsByName.end() )
   {
   int var_num = mVarNumsByName[ aNcdVarName ];
   ncv_ptr_return = mVarsByNum[ var_num ];
   }

//
// "ncv_ptr_return" might be NULL if the variable named by aNcdVarName
// was previously deleted using "NetcdfDataset::del_var"
//

if( ncv_ptr_return == NULL && mDiagOut != NULL )
   {
   fprintf(mDiagOut, "\nERROR: %s::%s:\n",
      CLASS_NAME.c_str(), METHOD_NAME.c_str());
   fprintf(mDiagOut, "   Variable '%s' not found in NetcdfDataset\n",
      aNcdVarName.c_str());
   }

return ncv_ptr_return;

} // end NetcdfDataset::get_var - by name

//-----------------------------------
// NetcdfDataset::get_var - by number
//-----------------------------------

NcdVar*
NetcdfDataset::
get_var
   (
   int aVarNum
   )

{ // begin NetcdfDataset::get_var - by number

const string
   METHOD_NAME = "get_var - by number";

NcdVar *ncv_ptr_return = NULL;

if( (aVarNum >= 0) && (aVarNum < mNumVars) )
   {
   ncv_ptr_return = mVarsByNum[ aVarNum ];
   }

//
// "ncv_ptr_return" might be NULL if the variable named by aNcdVarName
// was previously deleted using "NetcdfDataset::del_var"
//

if( ncv_ptr_return == NULL && mDiagOut != NULL )
   {
   fprintf(mDiagOut, "\nERROR: %s::%s:\n",
      CLASS_NAME.c_str(), METHOD_NAME.c_str());
   fprintf(mDiagOut, "   Variable %d not found in NetcdfDataset\n",
      aVarNum);
   }

return ncv_ptr_return;

} // end NetcdfDataset::get_var - by number

//--------------------------
// NetcdfDataset::FreeMemory
//--------------------------

void
NetcdfDataset::
FreeMemory()

{ // begin NetcdfDataset::FreeMemory

//-------------------------
// Delete global attributes

NcdAtt
   *ncd_att;

//
// Delete global attributes pointed to by map "mAttsByNum".
//

for(int i_att = 0; i_att < mNumAtts; ++i_att)
   {
   ncd_att = mAttsByNum[ i_att ];

   // Delete the attribute

   if(ncd_att != NULL)
      {
      delete ncd_att;
      }
   }

mAttsByNum.clear();
mAttNumsByName.clear();

//------------------
// Delete dimensions

NcdDim
   *ncd_dim;

//
// Delete dimensions pointed to by map "mDimsByNum".
//

for(int i_dim = 0; i_dim < mNumDims; ++i_dim)
   {
   ncd_dim  = mDimsByNum[ i_dim ];

   // Delete the dimension

   if(ncd_dim != NULL)
      {
      delete ncd_dim;
      }
   }

mDimsByNum.clear();
mDimNumsByName.clear();

//-----------------
// Delete variables

NcdVar
   *ncd_var;

//
// Delete variables pointed to by map "mVarsByNum".
//

for(int i_var = 0; i_var < mNumVars; ++i_var)
   {
   ncd_var  = mVarsByNum[ i_var ];

   // Delete the variable

   if(ncd_var != NULL)
      {
      delete ncd_var;
      }
   }

mVarsByNum.clear();
mVarNumsByName.clear();

} // end NetcdfDataset::FreeMemory

//-----------------------------
// NetcdfDataset::NewMethodName
//-----------------------------

int
NetcdfDataset::
NewMethodName( void )

{ // begin NetcdfDataset::NewMethodName

const string
   METHOD_NAME = "NewMethodName";

return SUCCESS;

} // end NetcdfDataset::NewMethodName

//-----------------------------------------------------------------------------
// Class NcdAtt
//-----------------------------------------------------------------------------

//-----------------------------
// NcdAtt::NcdAtt (constructor)
//-----------------------------

NcdAtt::
NcdAtt
   (
   const string  &aName,
   Nc3Type        aType,
   long int      aNumVals,
   void          *aValues
   )

{ // begin NcdAtt::constructor

string
   FUNCTION_NAME = "NcdAtt";

CLASS_NAME = "NcdAtt";

mName    = aName;
mType    = aType;
mNumVals = aNumVals;

mIsValid = false;

//
// Determine the element size
//

int
   elem_size_bytes = NCU::ElementSize( mType );

//
// Allocate space for the new attribute
//

if(aNumVals >= 1)
   {
   switch( mType )
      {
      case nc3Byte:
         mValues = (void *) new ncbyte    [ mNumVals ];
         break;
      case nc3Char:
         mValues = (void *) new char      [ mNumVals + 1 ];
         break;
      case nc3Short:
         mValues = (void *) new short int [ mNumVals ];
         break;
      case nc3Int:
         mValues = (void *) new int       [ mNumVals ];
         break;
      case nc3Float:
         mValues = (void *) new float     [ mNumVals ];
         break;
      case nc3Double:
         mValues = (void *) new double    [ mNumVals ];
         break;
      case nc3NoType:
         return;
         break;
      default:
         return;
         break;
      } // end switch mType
   }
else // ERROR: aNumVals < 1
   {
   mIsValid = false;
   return;
   }

//
// Copy the entire block of values into the space pointed to
// by member variable "mValues".
//

int
   n_bytes_to_copy = mNumVals * elem_size_bytes;

memcpy( (char *) mValues, (char *) aValues, n_bytes_to_copy );

//
// Make sure that an ncChar attribute is a zero-terminated C string (char *)
//
// Note that for an ncChar attribute, we allocated (mNumVals+1) bytes
//

if( mType == nc3Char )
   {
   ((char *) mValues)[ mNumVals ] = '\0';
   }

mIsValid = true;

} // end NcdAtt::constructor

//----------------------------------
// NcdAtt::NcdAtt (copy constructor)
//----------------------------------

NcdAtt::
NcdAtt
   (
   const NcdAtt  &aNcdAtt
   )

{ // begin NcdAtt::NcdAtt copy constructor

CLASS_NAME = "NcdAtt";

mName    = aNcdAtt.name();
mType    = aNcdAtt.type();
mNumVals = aNcdAtt.num_vals();

mIsValid = false;

//
// Determine the element size
//

int
   elem_size_bytes = NCU::ElementSize( mType );

//
// Allocate space for the new attribute
//

if(mNumVals >= 1)
   {
   switch( mType )
      {
      case nc3Byte:
         mValues = (void *) new ncbyte    [ mNumVals ];
         break;
      case nc3Char:
         mValues = (void *) new char      [ mNumVals + 1 ];
         break;
      case nc3Short:
         mValues = (void *) new short int [ mNumVals ];
         break;
      case nc3Int:
         mValues = (void *) new int       [ mNumVals ];
         break;
      case nc3Float:
         mValues = (void *) new float     [ mNumVals ];
         break;
      case nc3Double:
         mValues = (void *) new double    [ mNumVals ];
         break;
      case nc3NoType:
         return;
         break;
      default:
         return;
         break;
      } // end switch mType
   }
else // ERROR: mNumVals < 1
   {
   mIsValid = false;
   return;
   }

//
// Copy the entire block of values into the space pointed to
// by member variable "mValues".
//

int
   n_bytes_to_copy = mNumVals * elem_size_bytes;

memcpy( (char *) mValues, (char *) aNcdAtt.values(), n_bytes_to_copy );

//
// Make sure that an ncChar attribute is a zero-terminated C string (char *)
//
// Note that for an ncChar attribute, we allocated (mNumVals+1) bytes
//

if( mType == nc3Char )
   {
   ((char *) mValues)[ mNumVals ] = '\0';
   }

mIsValid = true;

} // end NcdAtt::NcdAtt copy constructor

//------------------
// NcdAtt::operator=
//------------------

const NcdAtt&
NcdAtt::
operator=
   (
   const NcdAtt &att_rhs
   )

{ // begin NcdAtt::operator=

if( &att_rhs != this )
   {
   mName    = att_rhs.name();
   mType    = att_rhs.type();
   mNumVals = att_rhs.num_vals();

   mIsValid = false;

   //
   // Deallocate the old attribute
   //

   switch( mType )
      {
      case nc3Byte:
         delete [] (ncbyte *) mValues;
         break;
      case nc3Char:
         delete [] (char *) mValues;
         break;
      case nc3Short:
         delete [] (short int *) mValues;
         break;
      case nc3Int:
         delete [] (int *) mValues;
         break;
      case nc3Float:
         delete [] (float *) mValues;
         break;
      case nc3Double:
         delete [] (double *) mValues;
         break;
      case nc3NoType:
         break;
      default:
         break;
      } // end switch mType

   //
   // Determine the element size
   //

   int
      elem_size_bytes = NCU::ElementSize( mType );

   //
   // Allocate space for the new attribute
   //

   if(mNumVals >= 1)
      {
      switch( mType )
         {
         case nc3Byte:
            mValues = (void *) new ncbyte    [ mNumVals ];
            break;
         case nc3Char:
            mValues = (void *) new char      [ mNumVals + 1 ];
            break;
         case nc3Short:
            mValues = (void *) new short int [ mNumVals ];
            break;
         case nc3Int:
            mValues = (void *) new int       [ mNumVals ];
            break;
         case nc3Float:
            mValues = (void *) new float     [ mNumVals ];
            break;
         case nc3Double:
            mValues = (void *) new double    [ mNumVals ];
            break;
         case nc3NoType:
            break;
         default:
            break;
         } // end switch mType
      }
   else // ERROR: mNumVals < 1
      {
      mIsValid = false;
      }

   //
   // Copy the entire block of values into the space pointed to
   // by member variable "mValues".
   //

   int
      n_bytes_to_copy = mNumVals * elem_size_bytes;

   memcpy( (char *) mValues, (char *) att_rhs.values(), n_bytes_to_copy );

   //
   // Make sure that an ncChar attribute is a zero-terminated C string (char *)
   //
   // Note that for an ncChar attribute, we allocated (mNumVals+1) bytes
   //

   if( mType == nc3Char )
      {
      ((char *) mValues)[ mNumVals ] = '\0';
      }

   mIsValid = true;

   } // end if( right != this )

return *this;

} // end NcdAtt::operator=

//-----------------------------
// NcdAtt::~NcdAtt (destructor)
//-----------------------------

NcdAtt::~NcdAtt()


{ // begin NcdAtt::~NcdAtt

switch( mType )
   {
   case nc3Byte:
      delete [] (ncbyte *) mValues;
      break;
   case nc3Char:
      delete [] (char *) mValues;
      break;
   case nc3Short:
      delete [] (short int *) mValues;
      break;
   case nc3Int:
      delete [] (int *) mValues;
      break;
   case nc3Float:
      delete [] (float *) mValues;
      break;
   case nc3Double:
      delete [] (double *) mValues;
      break;
   case nc3NoType:
      break;
   default:
      break;
   } // end switch mType

} // end NcdAtt::~NcdAtt

//-----------------------------------------------------------------------------
// Class NcdDim
//-----------------------------------------------------------------------------

//-----------------------------
// NcdDim::NcdDim (constructor)
//-----------------------------

NcdDim::
NcdDim
   (
   const string  &aName,
   long int      aSize,
   bool          aIsUnlimited // default: false (see NcdDim.hh)
   )

{ // begin NcdDim::NcdDim constructor

CLASS_NAME   = "NcdDim";

mName        = aName;
mSize        = aSize;
mIsUnlimited = aIsUnlimited;

} // end NcdDim::NcdDim constructor

//----------------------------------
// NcdDim::NcdDim (copy constructor)
//----------------------------------

NcdDim::
NcdDim
   (
   const NcdDim &aNcdDim
   )

{ // begin NcdDim:: copy constructor

CLASS_NAME   = "NcdDim";

mName        = aNcdDim.name();
mSize        = aNcdDim.size();
mIsUnlimited = aNcdDim.is_unlimited();

} // end NcdDim:: copy constructor

//------------------
// NcdDim::operator=
//------------------

const NcdDim&
NcdDim::
operator=
   (
   const NcdDim &dim_rhs
   )

{ // begin NcdDim::operator=

if( &dim_rhs != this )
   {
   CLASS_NAME   = "NcdDim";
   mName        = dim_rhs.name();
   mSize        = dim_rhs.size();
   mIsUnlimited = dim_rhs.is_unlimited();
   }

return *this;

} // end NcdDim::operator=

//-----------------------------
// NcdDim::~NcdDim (destructor)
//-----------------------------

NcdDim::~NcdDim()

{ // begin NcdDim::~NcdDim
} // end NcdDim::~NcdDim

//-----------------------------------------------------------------------------
// Class NcdVar
//-----------------------------------------------------------------------------

//-----------------------------
// NcdVar::NcdVar (constructor)
//-----------------------------

NcdVar::
NcdVar
   (
   const string    &arName,      // ref: name
   Nc3Type          aType,        // type
   NcdDimsByNum_t  &arDimsByNum, // ref: map<int, NcdDim*>
   void            *apData       // ptr: to data
   )

{ // begin NcdVar::NcdVar constructor

CLASS_NAME = "NcdVar";

string
   FUNCTION_NAME = "NcdVar";

//
// Make local copies of name, type, dimensions map, and data pointer
//

mName       = arName;
mType       = aType;
mDimsByNum  = arDimsByNum;
mData       = apData;

mNumAtts    = 0;
mNumDims    = mDimsByNum.size();

mIsValid    = true;
mEdges      = NULL;

// Construct "edges" array; will be owned by this NcdVar object
// so callers using the edges() function will not need to delete
// the edges array - that will be done by the NcdVar destructor
//
// Also construct the "mDimNumsByName" map

NcdDim
   *ncd_dim;

if( mNumDims > 0 )
   {
   mEdges = new long int[ mNumDims ];

   for(int i_dim = 0; i_dim < mNumDims; ++i_dim)
      {
      ncd_dim = mDimsByNum[ i_dim ];

      mEdges[ i_dim ] = ncd_dim->size();

      string
         dim_name = ncd_dim->name();

      mDimNumsByName[ dim_name ] = i_dim;
      }
   } // end if n_dims > 0

} // end NcdVar::NcdVar constructor

//-----------------------------
// NcdVar::~NcdVar (destructor)
//-----------------------------

NcdVar::~NcdVar()

{ // begin NcdVar::~NcdVar

//
// Delete attributes
//

for(int i_att = 0; i_att < mNumAtts; ++i_att)
   {
   NcdAtt
      *ncd_att = mAttsByNum[ i_att ];

   if( ncd_att != NULL )
      {
      delete ncd_att;
      }
   }

mAttsByNum.clear();
mAttNumsByName.clear();

//
// DO NOT DELETE DIMENSIONS; THEY ARE OWNED BY THE NetcdfDataset OBJECT
//

//
// Delete mEdges array
//

if(mNumDims > 0)
   {
   delete [] mEdges;
   }

//
// Delete data block
//

//if(mNumDims > 0)
//   {
   switch( mType )
      {
      case nc3Byte:
         delete [] (ncbyte *) mData;
         break;
      case nc3Char:
         delete [] (char *) mData;
         break;
      case nc3Short:
         delete [] (short int *) mData;
         break;
      case nc3Int:
         delete [] (int *) mData;
         break;
      case nc3Float:
         delete [] (float *) mData;
         break;
      case nc3Double:
         delete [] (double *) mData;
         break;
      case nc3NoType:
         break;
      default:
         break;
      } // end switch mType
//   } // end if mNumDims > 0

} // end NcdVar::~NcdVar

//-----------------
// NcdVar::num_vals
//-----------------

int
NcdVar::
num_vals( void )

{ // begin NcdVar::num_vals

int
   prod_of_dims = 1;

if( mNumDims > 0 )
   {
   for(int i_dim = 0; i_dim < mNumDims; ++i_dim)
      {
      prod_of_dims = prod_of_dims * mDimsByNum[ i_dim ]->size();
      }
   }

return prod_of_dims;

} // end NcdVar::num_vals

//-------------------
// NcdVar::data_bytes
//-------------------

int
NcdVar::
data_bytes( void )

{ // begin NcdVar::data_bytes

int
   prod_of_dims = 1;

if( mNumDims > 0 )
   {
   for(int i_dim = 0; i_dim < mNumDims; ++i_dim)
      {
      prod_of_dims = prod_of_dims * mDimsByNum[ i_dim ]->size();
      }
   }

return prod_of_dims * NCU::ElementSize( mType );

} // end NcdVar::num_vals



//----------------
// NcdVar::add_att
//----------------


//--- scalar attributes

// ncbyte
int NcdVar::add_att(Nc3Token aName, ncbyte aValue)
{
return add_att( new NcdAtt(aName, nc3Byte, (long) 1, (void *) &aValue) );
}

// char
int NcdVar::add_att(Nc3Token aName, char aValue)
{
return add_att( new NcdAtt(aName, nc3Char, (long) 1, (void *) &aValue) );
}

// short
int NcdVar::add_att(Nc3Token aName, short aValue)
{
return add_att( new NcdAtt(aName, nc3Short, (long) 1, (void *) &aValue) );
}

// int
int NcdVar::add_att(Nc3Token aName, int aValue)
{
return add_att( new NcdAtt(aName, nc3Int, (long) 1, (void *) &aValue) );
}

// float
int NcdVar::add_att(Nc3Token aName, float aValue)
{
return add_att( new NcdAtt(aName, nc3Float, (long) 1, (void *) &aValue) );
}

// double
int NcdVar::add_att(Nc3Token aName, double aValue)
{
return add_att( new NcdAtt(aName, nc3Double, (long) 1, (void *) &aValue) );
}


//--- C-string (char *) attribute

// const char *
int NcdVar::add_att(Nc3Token aName, const char *aValue)
{
return
add_att( new NcdAtt(aName, nc3Char, (long) strlen(aValue), (void *) aValue));
}


//--- vector attributes

// ncbyte
int NcdVar::add_att(Nc3Token aName, int aSize, const ncbyte *aValue)
{
return add_att( new NcdAtt(aName, nc3Byte, (long) aSize, (void *) aValue) );
}

// short
int NcdVar::add_att(Nc3Token aName, int aSize, const short *aValue)
{
return add_att( new NcdAtt(aName, nc3Short, (long) aSize, (void *) aValue) );
}

// int
int NcdVar::add_att(Nc3Token aName, int aSize, const int *aValue)
{
return add_att( new NcdAtt(aName, nc3Int, (long) aSize, (void *) aValue) );
}

// float
int NcdVar::add_att(Nc3Token aName, int aSize, const float *aValue)
{
return add_att( new NcdAtt(aName, nc3Float, (long) aSize, (void *) aValue) );
}

// double
int NcdVar::add_att(Nc3Token aName, int aSize, const double *aValue)
{
return add_att( new NcdAtt(aName, nc3Double, (long) aSize, (void *) aValue) );
}


// NcdAtt*

int
NcdVar::
add_att
   (
   NcdAtt *aNcdAtt
   )

{ // begin NcdVar::add_att

const string
   METHOD_NAME = "add_att";

//
// If this attribute does NOT already exist in this NcdVar object
// add it and return SUCCESS, otherwise, DO NOTHING and return FAILURE.
//

string
   att_name = aNcdAtt->name();

if( mAttNumsByName.find( att_name ) == mAttNumsByName.end() )
   {
   ++mNumAtts;
   int att_indx = mNumAtts - 1; // mAttsByNum is zero-based

   mAttsByNum[ att_indx ]     = aNcdAtt;
   mAttNumsByName[ att_name ] = att_indx;

   return SUCCESS;
   }
else // attribute already in this NcdVar object
   {

   return FAILURE;
   }

} // end NcdVar::add_att

//----------------
// NcdVar::get_att
//----------------

// Get an NcdAtt* from map "mAttsByNum" while being careful not to ask for
// an attribute which does not exist - which would cause an empty entry
// for the attribute to be inserted into the map!

NcdAtt*
NcdVar::
get_att
   (
   const string &aNcdAttName
   )

{ // begin NcdVar::get_att - by name

string
   METHOD_NAME = "get_att";

// Check to see that requested attribute name is NOT already in map;
// otherwise, just requesting a name that is not already in the map
// creates a blank entry for the requested name!

if( mAttNumsByName.find( aNcdAttName ) != mAttNumsByName.end() )
   {
   int att_num = mAttNumsByName[ aNcdAttName ];
   return mAttsByNum[ att_num ];
   }
else
   {

   return NULL;
   }

} // end NcdVar::get_att - by name

NcdAtt*
NcdVar::
get_att
   (
   int aAttNum
   )

{ // begin NcdVar::get_att - by number

if( (aAttNum >= 0) && (aAttNum < mNumAtts) )
   {
   return mAttsByNum[ aAttNum ];
   }
else
   {

   return NULL;
   }

} // end NcdVar::get_att - by number

//----------------
// NcdVar::get_dim
//----------------

// Get an NcdDim* from map "mDimsByName" while being careful not to ask for
// a variable which does not exist - which would cause an empty entry
// for the variable to be inserted into the map!

NcdDim*
NcdVar::
get_dim
   (
   const string  &aNcdDimName
   )

{ // begin NcdVar::get_dim - by name

const string
   METHOD_NAME = "get_dim";

// Check to see that requested variable name is NOT already in map;
// otherwise, just requesting a name that is not already in the map
// creates a blank entry for the requested name!

if( mDimNumsByName.find( aNcdDimName ) != mDimNumsByName.end() )
   {
   int dim_num = mDimNumsByName[ aNcdDimName ];
   return mDimsByNum[ dim_num ];
   }
else
   {

   return NULL;
   }

} // end NcdVar::get_dim - by name

NcdDim*
NcdVar::
get_dim
   (
   int aDimNum
   )

{ // begin NcdVar::get_dim - by number

if( (aDimNum >= 0) && (aDimNum < mNumDims) )
   {
   return mDimsByNum[ aDimNum ];
   }
else
   {

   return NULL;
   }

} // end NcdVar::get_dim - by number


//--------------------
// NcdVar::replace_att
//--------------------

int
NcdVar::
replace_att
   (
   NcdAtt *aNcdAtt
   )

{ // begin NcdVar::replace_att

const string
   METHOD_NAME = "replace_att";

// If attribute is new, place it in the map
// If attribute already exists, delete old attribute then add new one

string
   att_name = aNcdAtt->name();

if( mAttNumsByName.find( att_name ) == mAttNumsByName.end() )
   {
   // add new attribute

   ++mNumAtts;
   int att_indx = mNumAtts - 1; // mAttsByNum is zero-based

   mAttsByNum[ att_indx ]     = aNcdAtt;
   mAttNumsByName[ att_name ] = att_indx;
   }
else // aAttName already in map mAttsByName; replace
   {
   // replace existing attribute

   int
      att_num = mAttNumsByName[ att_name ];

   NcdAtt
      *att_old = mAttsByNum[ att_num ];

   // Avoid memory leak;
   // Delete object pointed to by existing map entry,
   // then replace that pointer with pointer to new attribute.

   delete att_old;

   mAttsByNum[ att_num ] = aNcdAtt;
   }

return SUCCESS;

} // end NcdVar::replace_att


//-----------------
// NcdVar::copy_att
//-----------------

int
NcdVar::
copy_att
   (
   const string  &aAttName,
   NcdVar        *aNcdVarSrc
   )

{ // begin NcdVar::copy_att

const string
   METHOD_NAME = "copy_att";

NcdAtt
   *src_att = NULL;

int
   status = SUCCESS;

// Get pointer to attribute in source NcdVar

src_att = aNcdVarSrc->get_att( aAttName );

if( src_att != NULL )
   {
   NcdAtt
      *dest_att = NULL;

   // Create destination attribute identical to source attribute

   dest_att =
   new NcdAtt
      (
      aAttName,
      src_att->type(),
      src_att->num_vals(),
      src_att->values()
      );

   // Store pointer to destination attribute in map,
   // replacing the attribute if it already exists

   if( dest_att != NULL )
      {
      replace_att( dest_att );
      }
   else // "replace_att" failure
      {
      status = FAILURE;
      }

   }
else // attribute not found in aNcdVarSrc
   {
   status = FAILURE;
   }

return status;

} // end NcdVar::copy_att

//------------------------
// NetcdfDataset::copy_var
//------------------------

int
NetcdfDataset::
copy_var
   (
   const string   &arVarName,
   NetcdfDataset  &arNcdSource
   )

{ // begin NetcdfDataset::copy_var

const string
   METHOD_NAME = "copy_var";

//
// Make sure variable does not already exist in THIS NetcdfDataset
//

if( mVarNumsByName.find( arVarName ) != mVarNumsByName.end() )
   {
   if( mVarsByNum[ mVarNumsByName[ arVarName ] ] != NULL )
      {
      if( mDiagOut != NULL )
         {
         fprintf(mDiagOut, "\nERROR: %s::%s:\n",
            CLASS_NAME.c_str(), METHOD_NAME.c_str());
         fprintf(mDiagOut,
            "   Variable '%s' already exists in target NetcdfDataset\n",
            arVarName.c_str());
         }

      return FAILURE;
      }
   }

//
// See if variable exists in the source NetcdfDataset
//

NcdVar
   *ncd_var_src = arNcdSource.get_var( arVarName );

if( ncd_var_src == NULL )
   {
   if( mDiagOut != NULL )
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut, "   Variable '%s' not found in NetcdfDataset\n",
         arVarName.c_str());
      }

   return FAILURE;
   }

//
// See if all the dimensions needed by this variable exist in
// THIS NetcdfDataset. If they do, build a dimension map for
// this variable containing pointers to NcdDim objects that are
// owned by THIS NetcdfDataset.
//

int
   num_dims_src = ncd_var_src->num_dims();

NcdDimsByNum_t
   dims_by_num_tgt;

bool
   dims_found = true;

if(num_dims_src > 0)
   {

   for(int i_dim = 0; i_dim < num_dims_src; ++i_dim)
      {
      string
         dim_name_src = ncd_var_src->get_dim(i_dim)->name();

      // Look for dimension named by "dim_name_src" in THIS NetcdfDataset

      if(mDimNumsByName.find(dim_name_src) != mDimNumsByName.end())
         {
         // Get pointer to dimension from THIS NetcdfDataset
         dims_by_num_tgt[i_dim] = mDimsByNum[mDimNumsByName[dim_name_src]];
         }
      else
         {
         // Dimension not found in THIS NetcdfDataset
         dims_found = false;

         if( mDiagOut != NULL )
            {
            fprintf(mDiagOut, "\nERROR: %s::%s:\n",
               CLASS_NAME.c_str(), METHOD_NAME.c_str());
            fprintf(mDiagOut,
               "   Dimension '%s' not found in this NetcdfDataset\n",
               dim_name_src.c_str());
            }
         } // end if dim_name_src found

      } // end for i_dim (dimensions)

   } // end if(num_dims_src > 0)

if( !dims_found )
   {
   if( mDiagOut != NULL )
      {
      fprintf(mDiagOut, "\nERROR: %s::%s:\n",
         CLASS_NAME.c_str(), METHOD_NAME.c_str());
      fprintf(mDiagOut,
      "   Variable '%s' requires dimensions not found in this NetcdfDataset\n",
         arVarName.c_str());
      }

   return FAILURE;
   }

//
// Allocate space for the target data
//

int
   data_bytes_tgt = ncd_var_src->data_bytes();

void
   *data_tgt = (void *) new char[ data_bytes_tgt ];

//
// Copy the source data to the target data
//

memcpy( (char *) data_tgt, (char *)  ncd_var_src->get_data(), data_bytes_tgt );

//
// Create a new NcdVar object identical to the source object.
//

NcdVar
*ncd_var_tgt =
new NcdVar
   (
   arVarName,           // string
   ncd_var_src->type(), // NcType
   dims_by_num_tgt,     // map<int, NcdDim*>
   data_tgt             // void*
   );

//
// Copy all attributes from source NcdVar to target NcdVar
//

for(int i_att = 0; i_att < ncd_var_src->num_atts(); ++i_att)
   {
   // Copy attribute from source NcdVar to THIS NcdVar
   ncd_var_tgt->copy_att( ncd_var_src->get_att(i_att)->name(), ncd_var_src );
   }

//
// Add new target NcdVar object to THIS NetcdfDataset
//

if( mVarNumsByName.find( arVarName ) == mVarNumsByName.end() )
   {
   // name does NOT already exist

   ++mNumVars;

   int
      var_indx_tgt = mNumVars - 1;

   mVarsByNum[ var_indx_tgt ] = ncd_var_tgt;
   mVarNumsByName[ arVarName ] = var_indx_tgt;
   }
else // name exists but NcdVar pointer is NULL
   {
   mVarsByNum[ mVarNumsByName[ arVarName ] ] = ncd_var_tgt;
   }

return SUCCESS;

} // end NetcdfDataset::copy_var

//------------------------
// NetcdfDataset::del_var
//------------------------

int
NetcdfDataset::
del_var
   (
   string aVarName
   )

{ // begin NetcdfDataset::del_var

const string
   METHOD_NAME = "del_var";

//
// If this variable does NOT already exist in this NetcdfDataset object
// do nothing and return SUCCESS, otherwise, DELETE the existing variable
// and return SUCCESS.
//

if( mVarNumsByName.find( aVarName ) != mVarNumsByName.end() )
   {
   delete
   mVarsByNum[ mVarNumsByName[ aVarName ] ];

   mVarsByNum[ mVarNumsByName[ aVarName ] ] = NULL;
   }

return SUCCESS;

} // end NetcdfDataset::del_var

//------------------------
// NetcdfDataset::del_var
//------------------------

int
NetcdfDataset::
del_var
   (
   NcdVar *aNcdVar
   )

{ // begin NetcdfDataset::del_var

const string
   METHOD_NAME = "del_var";

//
// If this variable does NOT already exist in this NetcdfDataset object
// do nothing and return SUCCESS, otherwise DELETE the existing variable
// and return SUCCESS.
//

string
   var_name = aNcdVar->name();

if( mVarNumsByName.find( var_name ) != mVarNumsByName.end() )
   {
   delete
   mVarsByNum[ mVarNumsByName[ var_name ] ];

   mVarsByNum[ mVarNumsByName[ var_name ] ] = NULL;
   }

return SUCCESS;

} // end NetcdfDataset::del_var
