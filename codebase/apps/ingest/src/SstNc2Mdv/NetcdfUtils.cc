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
NetcdfUtils.cc - Utilities for reading & writing netcdf files

Steve Carson, RAP, NCAR, Boulder, CO, 80307, USA
April 2005

DESIGN NOTES

This file is NOT compiled directly by the Makefile; rather, it is
#included in NetcdfUtils.hh. This is how function templates must
be implemented.
-------------------------------------------------------------------------------
*/

const string NAMESPACE_NAME = "NetcdfUtils";

namespace NetcdfUtils

  //----------------------------
{ // begin namespace NetcdfUtils
  //----------------------------

/*
-------------------------------------------------------------------------------
NcTypeName
-------------------------------------------------------------------------------
*/

template< typename ArgType >
static string
NcTypeName( ArgType aNcType )

{ // begin NcTypeName

switch( aNcType )
   {
   case nc3Byte:
      return "ncByte";
      break;
   case nc3Char:
      return "ncChar";
      break;
   case nc3Short:
      return "ncShort";
      break;
   case nc3Int:
      return "ncInt";
      break;
   case nc3Float:
      return "ncFloat";
      break;
   case nc3Double:
      return "ncDouble";
      break;
   case nc3NoType:
      return "";
      break;
   default:
      return NULL;
   }

} // end NcTypeName

/*
-------------------------------------------------------------------------------
ElementSize
-------------------------------------------------------------------------------
*/

template< typename ArgType >
int
ElementSize( ArgType aNcType )

{ // begin ElementSize

switch( aNcType )
   {
   case nc3Byte:
      return sizeof( ncbyte );
      break;
   case nc3Char:
      return sizeof( char );
      break;
   case nc3Short:
      return sizeof( short int );
      break;
   case nc3Int:
      return sizeof( int );
      break;
   case nc3Float:
      return sizeof( float );
      break;
   case nc3Double:
      return sizeof( double );
      break;
   case nc3NoType:
      return 0;
      break;
   default:
      return FAILURE;
   }

} // end ElementSize

/*
-------------------------------------------------------------------------------
ReadNcVar

Read data from Netcdf file into previously allocated space
and handle error conditions
-------------------------------------------------------------------------------
*/

template< typename Nc3VarType >
int
ReadNcVar
   (
   const NcuPrtArgs_t  &arPrtArgs,   // const ref to print args
   const Nc3File        &arNcFile,    // const ref to netcdf file
   const string        &arNcVarName, // const ref to variable name
   Nc3VarType           *apNcVarData  // pointer to variable receiving data
   )

{ // begin ReadNcVar

//
// initialize log and diagnostic message strings
//

const string
   FUNCTION_NAME = "ReadNcVar";

string
   info_msg_prefix =
      NAMESPACE_NAME +
      "::" +
      FUNCTION_NAME +
      ": called from " +
      arPrtArgs.callerID +
      "\n",
   err_msg_prefix  = "ERROR: " + info_msg_prefix,
   warn_msg_prefix = "WARNING: " + info_msg_prefix,
   msg;

//
// Get pointer to netcdf variable object
//

Nc3Var *ncv = arNcFile.get_var( arNcVarName.c_str() );

Nc3Error nc_err = Nc3Error(Nc3Error::verbose_nonfatal);

//--- Check to see if pointer to netcdf variable object is NULL
if( ncv != NULL )
   {

   //--- Check to see if netcdf variable name is valid
   if ( ncv->is_valid() )
      {

      //--- get data for variable and check for success
      Nc3Bool  get_ok = true;
      long    *edges;

      int
      num_dims = ncv->num_dims();

      edges = ncv->edges();

      if( num_dims == 0 )
         {
         // scalar data
         get_ok = ncv->get(apNcVarData);
         }
      else
         {
         // n-dimensional data
         get_ok = ncv->get(apNcVarData, edges);
         }

      // int nc_err_num = nc_err.get_err();

      if( get_ok )
         {
         if((arPrtArgs.diag != NULL) && (arPrtArgs.diag_lvl >= 4))
            {
            fprintf(arPrtArgs.diag, "\n%s", info_msg_prefix.c_str());
            fprintf(arPrtArgs.diag, "   Read data for variable '%s'\n",
               arNcVarName.c_str());
            }
         }
      else // type mismatch (programming error!)
         {

         msg = err_msg_prefix +
            "   Type mismatch for netcdf variable "+arNcVarName+
            "; programming error?"+
            "\n";

         arPrtArgs.log->postMsg(ERROR, msg.c_str());

         if((arPrtArgs.diag != NULL) && (arPrtArgs.diag_lvl >= 4))
            {
            fprintf(arPrtArgs.diag, "\n%s", msg.c_str());
            }

         return( FAILURE );

         } // end if( get_ok )

      delete edges; // Netcdf object we own

      }
   else // variable name not valid
      {
      msg = err_msg_prefix +
         "netcdf variable name '"+arNcVarName+"' not valid\n";

      arPrtArgs.log->postMsg(ERROR, msg.c_str());

      if((arPrtArgs.diag != NULL) && (arPrtArgs.diag_lvl >= 4))
         {
         fprintf(arPrtArgs.diag, "\n%s", err_msg_prefix.c_str());
         fprintf(arPrtArgs.diag, "   Netcdf variable name '%s' not valid\n",
            arNcVarName.c_str());
         }

      return( FAILURE );
      }
   }
else // variable ncvName not found in netCDF file
   {
   msg = err_msg_prefix +
      "netcdf variable name '" + arNcVarName + "' not found\n";

   arPrtArgs.log->postMsg(ERROR, msg.c_str());

   if((arPrtArgs.diag != NULL) && (arPrtArgs.diag_lvl >= 4))
      {
      fprintf(arPrtArgs.diag, "\n%s", err_msg_prefix.c_str());
      fprintf(arPrtArgs.diag, "   Netcdf variable name '%s' not found\n",
         arNcVarName.c_str());
      }

   return( FAILURE );
   }

// DONT EVER DO THIS AGAIN!!!
//
// Explanation: when the user calls an NcFile method that returns
// a pointer to an NcVar or NcDim, the use must NOT call "delete"
// with that pointer; the object it points to is owned by the
// NcFile object and will be deleted by that object's destructor.
// If you delete the NcVar or NcDim object yourself, any subsequent
// call to the NcFile object's destructor will result in a
// segmentation violation.
//
//delete( ncv );

return( SUCCESS );

} // end ReadNcVar

/*
-------------------------------------------------------------------------------
LoadNcVar

Allocate space for and read in a netcdf variable
-------------------------------------------------------------------------------
*/

template< typename NcVarElemType >
int
LoadNcVar
   (
   const NcuPrtArgs_t  &arPrtArgs,     // const ref to print args
   const Nc3File        &arNcFile,      // const ref to netcdf file
   const string        &arNcVarName,   // const ref to variable name
   NcVarElemType       **apNcVarData   // pointer to pointer to data
   )

{ // begin LoadNcVar

const string
   FUNCTION_NAME = "LoadNcVar";

string
   info_msg_prefix =
      "\n" +
      NAMESPACE_NAME +
      "::" +
      FUNCTION_NAME +
      ": called from " +
      arPrtArgs.callerID +
      "\n",
   err_msg_prefix  = "ERROR: " + info_msg_prefix,
   warn_msg_prefix = "WARNING: " + info_msg_prefix,
   msg;

int
   n_dims,
   product_of_dims;

NcuPrtArgs_t
   prt_args;

//
// Get pointer to netcdf variable object
//

Nc3Var *ncv = arNcFile.get_var( arNcVarName.c_str() );

//
// Get dimensions of netcdf variable
//

n_dims = ncv->num_dims();

product_of_dims = 1;
for( int i = 0; i < n_dims; ++i )
   {
   product_of_dims *= ncv->get_dim(i)->size();
   }

//
// Allocate space for the data from the netcdf variable
// Caller is responsible for deleting
//

*apNcVarData = new NcVarElemType[ product_of_dims ];

if( (arPrtArgs.diag != NULL) && (arPrtArgs.diag_lvl >= 4) )
   {
   fprintf(arPrtArgs.diag, info_msg_prefix.c_str());
   fprintf(arPrtArgs.diag, "   Alocated %ld bytes for variable '%s'\n",
      (product_of_dims * sizeof(NcVarElemType)), arNcVarName.c_str());
   for( int i = 0; i < n_dims; ++i )
      {
      fprintf(arPrtArgs.diag, "   Dim# %2d size = %ld\n",
         i, ncv->get_dim(i)->size() );
      }
   }

//
// Read data from netcdf file into space just allocated
//

prt_args = arPrtArgs;
prt_args.callerID = FUNCTION_NAME;

int status =
ReadNcVar
   (
   prt_args,
   arNcFile,
   arNcVarName,
   *apNcVarData
   );

return status;

} // end LoadNcVar

/*
-------------------------------------------------------------------------------
CopyVarAtt

Copy a variable attribute (as opposed to a global attribute)
from one NcVar object to another NcVar object WITHOUT LEAKING MEMORY!

NOTE: this function is only intended for use with CharPtr = char *
Had to make it a template function in order to work correctly
within namespace "NetcdfUtils"
-------------------------------------------------------------------------------
*/

template< typename CharPtr >
void
CopyVarAtt
   (
   const NcuPrtArgs_t  &arPrtArgs,  // const ref to print args
   CharPtr             *aAttName,
   Nc3Var               *apInputNcVar,
   Nc3Var               *apOutputNcVar
   )

{ // begin CopyVarAtt

const string
   FUNCTION_NAME = "CopyVarAtt";

string
   info_msg_prefix =
      NAMESPACE_NAME +
      "::" +
      FUNCTION_NAME +
      ": called from " +
      arPrtArgs.callerID +
      "\n",
   err_msg_prefix  = "ERROR: " + info_msg_prefix,
   warn_msg_prefix = "WARNING: " + info_msg_prefix,
   msg;

// Turn off Netcdf error messages so we can see if the requested
// attribute exists in the input variable.

Nc3Error
   *nc_err = new Nc3Error( Nc3Error::silent_nonfatal );

// Get the attribute, if it exists

Nc3Att
   *att = apInputNcVar->get_att( aAttName );

// restore previous error behavior of Netcdf library

delete nc_err;

// If attribute does not exist in input variable, issue a warning
// message and return without doing anything.

if( att == NULL )
   {
   fprintf(arPrtArgs.diag, "\n%s", warn_msg_prefix.c_str());
   fprintf(arPrtArgs.diag, "   Attribute '%s' not found in variable '%s'\n",
      aAttName, apInputNcVar->name() );
   fprintf(arPrtArgs.diag, "   No attribute copied to variable '%s'\n",
      apOutputNcVar->name() );
   return;
   }

// Attribute exists, so get its value(s).

Nc3Values
   *att_vals = att->values();

long int
   att_num_vals = att->num_vals();

Nc3Type
   att_type = att->type();

// Add this attribute to the output variable

if( (att_type == nc3Char) || (att_num_vals == 1) ) // att is scalar
   {
   switch( att_type )
      {
      case nc3Byte:
         apOutputNcVar->add_att( aAttName, att_vals->as_ncbyte(0) );
         break;
      case nc3Char:
         apOutputNcVar->add_att( aAttName, att_vals->as_char(0) );
         break;
      case nc3Short:
         apOutputNcVar->add_att( aAttName, att_vals->as_short(0) );
         break;
      case nc3Int:
         apOutputNcVar->add_att( aAttName, att_vals->as_int(0) );
         break;
      case nc3Float:
         apOutputNcVar->add_att( aAttName, att_vals->as_float(0) );
         break;
      case nc3Double:
         apOutputNcVar->add_att( aAttName, att_vals->as_double(0) );
         break;
      case nc3NoType:
         break;
      }
   }
else // att is vector
   {
   switch( att_type )
      {
      case nc3Byte:
         apOutputNcVar->add_att(
            aAttName, att_num_vals, (ncbyte *) att_vals->base() );
         break;
      case nc3Char:
         apOutputNcVar->add_att(
            aAttName, att_num_vals, (char *) att_vals->base() );
         break;
      case nc3Short:
         apOutputNcVar->add_att(
            aAttName, att_num_vals, (short *) att_vals->base() );
         break;
      case nc3Int:
         apOutputNcVar->add_att(
            aAttName, att_num_vals, (int *) att_vals->base() );
         break;
      case nc3Float:
         apOutputNcVar->add_att(
            aAttName, att_num_vals, (float *) att_vals->base() );
         break;
      case nc3Double:
         apOutputNcVar->add_att(
            aAttName, att_num_vals, (double *) att_vals->base() );
         break;
      case nc3NoType:
         break;
      }
   }

// Delete the attribute and its value(s) or else we leak memory.

delete att;      // Netcdf object we own
delete att_vals; // Netcdf object we own

} // end CopyVarAtt

/*
------------------------------------------------------------------------------
GetScalarAttValue
------------------------------------------------------------------------------
*/

template< typename AttType >
void
GetScalarAttValue
   (
   const NcuPrtArgs_t  &arPrtArgs,  // const ref to print args
   const char          *aAttName,
   Nc3Var               *apInputNcVar,
   AttType             *apAttValue
   )

{ // begin GetScalarAttValue

const string
   FUNCTION_NAME = "GetScalarAttValue";

string
   info_msg_prefix =
      NAMESPACE_NAME +
      "::" +
      FUNCTION_NAME +
      ": called from " +
      arPrtArgs.callerID +
      "\n",
   err_msg_prefix  = "ERROR: " + info_msg_prefix,
   warn_msg_prefix = "WARNING: " + info_msg_prefix,
   msg;

// Turn off Netcdf error messages so we can see if the requested
// attribute exists in the input variable.

Nc3Error
   *nc_err = new Nc3Error( Nc3Error::silent_nonfatal );

Nc3Att
   *att = apInputNcVar->get_att( aAttName );

// restore previous error behavior of Netcdf library

delete nc_err;

// If attribute does not exist in input variable, issue a warning
// message and return without doing anything.

if( att == NULL )
   {
   fprintf(arPrtArgs.diag, "\n%s", warn_msg_prefix.c_str());
   fprintf(arPrtArgs.diag, "   Attribute '%s' not found in variable '%s'\n",
      aAttName, apInputNcVar->name() );
   return;
   }

Nc3Values
   *att_vals = att->values();

Nc3Type
   att_type = att->type();

switch( att_type )
   {
   case nc3Byte:
      *apAttValue = (AttType) att_vals->as_ncbyte(0);
      break;
   case nc3Char:
      *apAttValue = (AttType) att_vals->as_char(0);
      break;
   case nc3Short:
      *apAttValue = (AttType) att_vals->as_short(0);
      break;
   case nc3Int:
      *apAttValue = (AttType) att_vals->as_int(0);
      break;
   case nc3Float:
      *apAttValue = (AttType) att_vals->as_float(0);
      break;
   case nc3Double:
      *apAttValue = (AttType) att_vals->as_double(0);
      break;
   case nc3NoType:
      *apAttValue = (AttType) NULL;
      break;
   }

delete att;      // Netcdf object we own
delete att_vals; // Netcdf object we own

} // end GetScalarAttValue

/*
------------------------------------------------------------------------------
AddVarAtt
------------------------------------------------------------------------------
*/

template< typename VoidPtr >
int
AddVarAtt
   (
   const NcuPrtArgs_t  &arPrtArgs,  // const ref to print args
   Nc3Var               *apNcVar,    // const pointer to netcdf variable
   Nc3Token             aAttName,    // attribute name
   Nc3Type              aAttType,    // attribute type
   int                 aNumVals,    // number of attribute values
   VoidPtr             *aAttVals    // pointer to block of values
   )

{ // begin AddVarAtt

const string
   FUNCTION_NAME = "AddVarAtt";

string
   info_msg_prefix =
      NAMESPACE_NAME +
      "::" +
      FUNCTION_NAME +
      ": called from " +
      arPrtArgs.callerID +
      "\n",
   err_msg_prefix  = "ERROR: " + info_msg_prefix,
   warn_msg_prefix = "WARNING: " + info_msg_prefix,
   msg;

// Turn off Netcdf error messages so we can see if the requested
// attribute exists in the variable.

Nc3Error
   *nc_err = new Nc3Error( Nc3Error::silent_nonfatal );

Nc3Att
   *nc_att = apNcVar->get_att( aAttName );

// restore previous error behavior of Netcdf library

delete nc_err;

// If attribute already exist in variable, issue a warning
// message and return without doing anything.

if( nc_att != NULL )
   {
   if( arPrtArgs.diag != NULL )
      {
      fprintf(arPrtArgs.diag, "\n%s", warn_msg_prefix.c_str());
      fprintf(arPrtArgs.diag,
         "   Attribute '%s' already exists in variable '%s'\n",
         aAttName, apNcVar->name() );
      }

   return FAILURE;
   }

if( aNumVals <= 1 ) // scalar attribute
   {
   switch( aAttType )
      {
      case nc3Byte:
         apNcVar->add_att(aAttName, *((ncbyte *) aAttVals) );
         break;
      case nc3Char:
         apNcVar->add_att(aAttName, *((char *)   aAttVals) );
         break;
      case nc3Short:
         apNcVar->add_att(aAttName, *((short *)  aAttVals) );
         break;
      case nc3Int:
         apNcVar->add_att(aAttName, *((int *)    aAttVals) );
         break;
      case nc3Float:
         apNcVar->add_att(aAttName, *((float *)  aAttVals) );
         break;
      case nc3Double:
         apNcVar->add_att(aAttName, *((double *) aAttVals) );
         break;
      case nc3NoType:
         break;
      } // end switch( att_type )
   }
else // vector attribute
   {
   switch( aAttType )
      {
      case nc3Byte:
         apNcVar->add_att(aAttName, aNumVals, (ncbyte *) aAttVals);
         break;
      case nc3Char:
         apNcVar->add_att(aAttName, aNumVals, (char *) aAttVals);
         break;
      case nc3Short:
         apNcVar->add_att(aAttName, aNumVals, (short *) aAttVals);
         break;
      case nc3Int:
         apNcVar->add_att(aAttName, aNumVals, (int *) aAttVals);
         break;
      case nc3Float:
         apNcVar->add_att(aAttName, aNumVals, (float *) aAttVals);
         break;
      case nc3Double:
         apNcVar->add_att(aAttName, aNumVals, (double *) aAttVals);
         break;
      case nc3NoType:
         break;
      } // end switch( att_type )

   } // end if if( aNumVals <= 1 )

return SUCCESS;

} // end AddVarAtt

/*
------------------------------------------------------------------------------
AddGlobalAtt
------------------------------------------------------------------------------
*/

template< typename VoidPtr >
int
AddGlobalAtt
   (
   const NcuPrtArgs_t  &arPrtArgs,  // const ref to print args
   Nc3File              *apNcFile,   // const pointer to netcdf variable
   const string        &arAttName,  // const ref to attribute name
   Nc3Type              aAttType,    // attribute type
   long int            aNumVals,    // number of attribute values
   VoidPtr             *apAttVals    // pointer to block of values
   )

{ // begin AddGlobalAtt

const string
   FUNCTION_NAME = "AddGlobalAtt";

bool
   ok;

char
   *att_name = (char *) arAttName.c_str();

int
   n_vals = (int) aNumVals;

if( apNcFile == NULL )
   {
   if( arPrtArgs.diag != NULL )
      {
      fprintf(arPrtArgs.diag, "\nERROR: %s::%s:\n",
         NAMESPACE_NAME.c_str(), FUNCTION_NAME.c_str() );
      fprintf(arPrtArgs.diag, "   Netcdf file pointer is NULL\n");
      }

   return FAILURE;
   }

if( !apNcFile->is_valid() )
   {
   if( arPrtArgs.diag != NULL )
      {
      fprintf(arPrtArgs.diag, "\nERROR: %s::%s:\n",
         NAMESPACE_NAME.c_str(), FUNCTION_NAME.c_str() );
      fprintf(arPrtArgs.diag, "   Netcdf file not valid\n");
      }

   return FAILURE;
   }

if( aAttType == nc3Char )
   {
   //
   // Make sure string values in attributes are zero-terminated
   //

   char *att_str   = new char[ n_vals + 1 ];
   strncpy(att_str, (char *) apAttVals, n_vals);
   att_str[n_vals] = '\0';
   ok              = apNcFile->add_att(att_name, att_str);
   delete [] att_str;

   return SUCCESS;
   }

// attribute type not "ncChar"

if( n_vals == 1 ) // scalar attribute
   {
   switch( aAttType )
      {
      case nc3Byte:
         ok = apNcFile->add_att(att_name, *((ncbyte *) apAttVals));
         break;
      case nc3Short:
         ok = apNcFile->add_att(att_name, *((short int *) apAttVals));
         break;
      case nc3Int:
         ok = apNcFile->add_att(att_name, *((int *) apAttVals));
         break;
      case nc3Float:
         ok = apNcFile->add_att(att_name, *((float *) apAttVals));
         break;
      case nc3Double:
         ok = apNcFile->add_att(att_name, *((double *) apAttVals));
         break;
      case nc3NoType:
         break;
      case nc3Char:
         break;
      }
   }
else // vector (1D array) attribute
   {
   switch( aAttType )
      {
      case nc3Byte:
         ok = apNcFile->add_att(att_name, n_vals, (ncbyte *) apAttVals);
         break;
      case nc3Short:
         ok = apNcFile->add_att(att_name, n_vals, (short int *) apAttVals);
         break;
      case nc3Int:
         ok = apNcFile->add_att(att_name, n_vals, (int *) apAttVals);
         break;
      case nc3Float:
         ok = apNcFile->add_att(att_name, n_vals, (float *) apAttVals);
         break;
      case nc3Double:
         ok = apNcFile->add_att(att_name, n_vals, (double *) apAttVals);
         break;
      case nc3NoType:
         break;
      case nc3Char:
         break;
      }
   } // end if( n_vals == (long int) 1 )

if( !ok )
   {
   if(arPrtArgs.diag != NULL)
      {
      fprintf(arPrtArgs.diag,
     "   ERROR writing attribute: name = '%-24s' type = '%8s' n_vals = %6ld\n",
	      arAttName.c_str(), NCU::NcTypeName(aAttType).c_str(), aNumVals);
      }
   return FAILURE;
   }
else
   {
   if((arPrtArgs.diag != NULL) && (arPrtArgs.diag_lvl >= 3))
      {
      fprintf(arPrtArgs.diag,
         "   Wrote attribute: name = '%-24s' type = '%8s' n_vals = %6ld\n",
	      arAttName.c_str(), NCU::NcTypeName(aAttType).c_str(), aNumVals);
      }
   return SUCCESS;
   }

} // end AddGlobalAtt

/*
------------------------------------------------------------------------------
WriteVarData
------------------------------------------------------------------------------
*/

template< typename VoidPtr >
int
WriteVarData
   (
   const NcuPrtArgs_t  &arPrtArgs,  // const ref to print args
   Nc3File              &arNcFile,   // ref to netcdf file
   const char          *aVarName,   // const ptr to variable name
   const long int      *aEdges,     // edges array
   VoidPtr             *aDataPtr    // ptr to data block
   )

{ // begin WriteVarData

const string
   FUNCTION_NAME = "WriteVarData";

Nc3Var
   *nc_var = NULL;

Nc3Type
   nc_var_type;

int
   n_dims;

bool
   put_ok = false;

nc_var = arNcFile.get_var( aVarName );

if( nc_var == NULL )
   {
   if( arPrtArgs.diag != NULL )
      {
      fprintf(arPrtArgs.diag, "\nERROR: %s::%s:\n",
         NAMESPACE_NAME.c_str(), FUNCTION_NAME.c_str());
      fprintf(arPrtArgs.diag, "   Variable '%s' not found in Netcdf file\n",
         aVarName);
      fprintf(arPrtArgs.diag, "   No data written\n");
      }

   return FAILURE;
   }

nc_var_type = nc_var->type();
n_dims      = nc_var->num_dims();

if( n_dims > 0 ) // array variable
   {
   switch( nc_var_type )
      {
      case nc3Byte:
         put_ok = nc_var->put( (ncbyte *)    aDataPtr, aEdges );
         break;
      case nc3Char:
         put_ok = nc_var->put( (char *)      aDataPtr, aEdges );
         break;
      case nc3Short:
         put_ok = nc_var->put( (short int *) aDataPtr, aEdges );
         break;
      case nc3Int:
         put_ok = nc_var->put( (int *)       aDataPtr, aEdges );
         break;
      case nc3Float:
         put_ok = nc_var->put( (float *)     aDataPtr, aEdges );
         break;
      case nc3Double:
         put_ok = nc_var->put( (double *)    aDataPtr, aEdges );
         break;
      case nc3NoType:
         break;
      } // end switch var_type

   }
else // scalar variable
   {

   switch( nc_var_type )
      {
      case nc3Byte:
         put_ok = nc_var->put( (ncbyte *)    aDataPtr );
         break;
      case nc3Char:
         put_ok = nc_var->put( (char *)      aDataPtr );
         break;
      case nc3Short:
         put_ok = nc_var->put( (short int *) aDataPtr );
         break;
      case nc3Int:
         put_ok = nc_var->put( (int *)       aDataPtr );
         break;
      case nc3Float:
         put_ok = nc_var->put( (float *)     aDataPtr );
         break;
      case nc3Double:
         put_ok = nc_var->put( (double *)    aDataPtr );
         break;
      case nc3NoType:
         break;
      } // end switch var_type

   } // end if( n_dims > 0 )

if( !put_ok )
   {
   if( arPrtArgs.diag != NULL )
      {
      fprintf(arPrtArgs.diag, "\nERROR: %s::%s:\n",
         NAMESPACE_NAME.c_str(), FUNCTION_NAME.c_str());
      fprintf(arPrtArgs.diag, "   Error writing variable '%s'\n",
         aVarName);
      }
   return FAILURE;
   }
else
   {
   return SUCCESS;
   }

} // end WriteVarData


  //--------------------------
} // end namespace NetcdfUtils
  //--------------------------
