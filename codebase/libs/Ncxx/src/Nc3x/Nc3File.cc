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
//////////////////////////////////////////////////////////////////////
//  Nc3x C++ classes for NetCDF3
//  Repackaged from original Unidata C++ API for NetCDF3.
//
//  Modification for LROSE made by:
//    Mike Dixon, EOL, NCAR
//    P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//  June 2017
//
//////////////////////////////////////////////////////////////////////
/*********************************************************************
 *   Original copyright:
 *   Copyright 1992, University Corporation for Atmospheric Research
 *   See netcdf/README file for copying and redistribution conditions.
 *   Purpose:   C++ class interface for netCDF
 *********************************************************************/

#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <Ncxx/Nc3File.hh>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

static const int ncGlobal = NC_GLOBAL; // psuedo-variable for global attributes

static const int ncBad = -1;	// failure return for netCDF C interface 

Nc3File::~Nc3File( void )
{
  (void) close();
}

Nc3Bool Nc3File::is_valid( void ) const
{
  return the_id != ncBad;
}

int Nc3File::num_dims( void ) const
{
  int num = 0;
  if (is_valid())
    Nc3Error::set_err(
            nc_inq_ndims(the_id, &num)
                      );
  return num;
}

int Nc3File::num_vars( void ) const
{
  int num = 0;
  if (is_valid())
    Nc3Error::set_err(
            nc_inq_nvars(the_id, &num)
                      );
  return num;
}

int Nc3File::num_atts( void ) const
{
  int num = 0;
  if (is_valid())
    Nc3Error::set_err(
            nc_inq_natts(the_id, &num)
                      );
  return num;
}

Nc3Dim* Nc3File::get_dim( Nc3Token name ) const
{
  int dimid;
  if(Nc3Error::set_err(
             nc_inq_dimid(the_id, name, &dimid)
                       ) != NC_NOERR)
    return 0;
  return get_dim(dimid);
}

Nc3Var* Nc3File::get_var( Nc3Token name ) const
{
  int varid;
  if(Nc3Error::set_err(
             nc_inq_varid(the_id, name, &varid)
                       ) != NC_NOERR)
    return 0;
  return get_var(varid);
}

Nc3Att* Nc3File::get_att( Nc3Token aname ) const
{
  return is_valid() ? globalv->get_att(aname) : 0;
}

Nc3Dim* Nc3File::get_dim( int i ) const
{
  if (! is_valid() || i < 0 || i >= num_dims())
    return 0;
  return dimensions[i];
}

Nc3Var* Nc3File::get_var( int i ) const
{
  if (! is_valid() || i < 0 || i >= num_vars())
    return 0;
  return variables[i];
}

Nc3Att* Nc3File::get_att( int n ) const
{
  return is_valid() ? globalv->get_att(n) : 0;
}

Nc3Dim* Nc3File::rec_dim( ) const
{
  if (! is_valid())
    return 0;
  int recdim;
  if(Nc3Error::set_err(
             nc_inq_unlimdim(the_id, &recdim)
                       ) != NC_NOERR)
    return 0;
  return get_dim(recdim);
}

Nc3Dim* Nc3File::add_dim(Nc3Token name, long size)
{
  if (!is_valid() || !define_mode())
    return 0;
  int n = num_dims();
  Nc3Dim* dimp = new Nc3Dim(this, name, size);
  dimensions[n] = dimp;	// for garbage collection on close()
  return dimp;
}

Nc3Dim* Nc3File::add_dim(Nc3Token name)
{
  return add_dim(name, NC_UNLIMITED);
}

// To create scalar, 1-dimensional, ..., 5-dimensional variables, just supply
// as many dimension arguments as necessary

Nc3Var* Nc3File::add_var(Nc3Token name, Nc3Type type, // scalar to 5D var
                         const Nc3Dim* dim0,
                         const Nc3Dim* dim1,
                         const Nc3Dim* dim2,
                         const Nc3Dim* dim3,
                         const Nc3Dim* dim4)
{
  if (!is_valid() || !define_mode())
    return 0;
  int dims[5];
  int ndims = 0;
  if (dim0) {
    ndims++;
    dims[0] = dim0->id();
    if (dim1) {
      ndims++;
      dims[1] = dim1->id();
      if (dim2) {
        ndims++;
        dims[2] = dim2->id();
        if (dim3) {
          ndims++;
          dims[3] = dim3->id();
          if (dim4) {
            ndims++;
            dims[4] = dim4->id();
          }
        }
      }
    }
  }
  int n = num_vars();
  int varid;
  if(Nc3Error::set_err(
             nc_def_var(the_id, name, (nc_type) type, ndims, dims, &varid)
                       ) != NC_NOERR)
    return 0;
  Nc3Var* varp =
    new Nc3Var(this, varid);
  variables[n] = varp;
  return varp;
}

// For variables with more than 5 dimensions, use n-dimensional interface
// with vector of dimensions.

Nc3Var* Nc3File::add_var(Nc3Token name, Nc3Type type, int ndims, const Nc3Dim** dims)
{
  if (!is_valid() || !define_mode())
    return 0;
  int* dimids = new int[ndims];
  for (int i=0; i < ndims; i++)
    dimids[i] = dims[i]->id();
  int n = num_vars();
  int varid;
  if(Nc3Error::set_err(
             nc_def_var(the_id, name, (nc_type) type, ndims, dimids, &varid)
                       ) != NC_NOERR)
    return 0;
  Nc3Var* varp =
    new Nc3Var(this, varid);
  variables[n] = varp;
  delete [] dimids;
  return varp;
}

// OLD MACRO DEFINITION - now explicitly expanded in the code
// #define Nc3File_add_scalar_att(TYPE)                    
//   Nc3Bool Nc3File::add_att(Nc3Token aname, TYPE val)    
//   {                                                     
//     return globalv->add_att(aname, val);                
//   }

// Nc3File_add_scalar_att(char)
// Nc3File_add_scalar_att(ncbyte)
// Nc3File_add_scalar_att(short)
// Nc3File_add_scalar_att(int)
// Nc3File_add_scalar_att(long)
// Nc3File_add_scalar_att(float)
// Nc3File_add_scalar_att(double)
// Nc3File_add_scalar_att(const char*)

Nc3Bool Nc3File::add_att(Nc3Token aname, char val)
{
  return globalv->add_att(aname, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, ncbyte val)
{
  return globalv->add_att(aname, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, short val)
{
  return globalv->add_att(aname, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, int val)
{
  return globalv->add_att(aname, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, long long val)
{
  return globalv->add_att(aname, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, long val)
{
  return globalv->add_att(aname, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, float val)
{
  return globalv->add_att(aname, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, double val)
{
  return globalv->add_att(aname, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, const char * val)
{
  return globalv->add_att(aname, val);
}

// OLD MACRO DEFINITION - now explicitly expanded in the code
// #define Nc3File_add_vector_att(TYPE)                                    
//   Nc3Bool Nc3File::add_att(Nc3Token aname, int n, const TYPE* val)      
//   {                                                                     
//     return globalv->add_att(aname, n, val);                             
//   }
//   Nc3File_add_vector_att(char)
//   Nc3File_add_vector_att(ncbyte)
//   Nc3File_add_vector_att(short)
//   Nc3File_add_vector_att(int)
//   Nc3File_add_vector_att(long)
//   Nc3File_add_vector_att(float)
//   Nc3File_add_vector_att(double)

Nc3Bool Nc3File::add_att(Nc3Token aname, int n, const char* val)
{
  return globalv->add_att(aname, n, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, int n, const ncbyte* val)
{
  return globalv->add_att(aname, n, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, int n, const short* val)
{
  return globalv->add_att(aname, n, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, int n, const int* val)
{
  return globalv->add_att(aname, n, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, int n, const long long* val)
{
  return globalv->add_att(aname, n, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, int n, const long* val)
{
  return globalv->add_att(aname, n, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, int n, const float* val)
{
  return globalv->add_att(aname, n, val);
}

Nc3Bool Nc3File::add_att(Nc3Token aname, int n, const double* val)
{
  return globalv->add_att(aname, n, val);
}

Nc3Bool Nc3File::set_fill( FillMode a_mode )
{
  int prev_mode;
  if (Nc3Error::set_err(
              nc_set_fill(the_id, a_mode, &prev_mode)
                        ) == NC_NOERR) {
    the_fill_mode = a_mode;
    return TRUE;
  }
  return FALSE;
}

Nc3File::FillMode Nc3File::get_fill( void ) const
{
  return the_fill_mode;
}

Nc3File::FileFormat Nc3File::get_format( void ) const
{
  int the_format;
  Nc3Error::set_err(
          nc_inq_format(the_id, &the_format)
                    );
  switch (the_format) {
    case NC_FORMAT_CLASSIC:
      return Classic;
    case NC_FORMAT_64BIT:
      return Offset64Bits;
    case NC_FORMAT_NETCDF4:
      return Netcdf4;
    case NC_FORMAT_NETCDF4_CLASSIC:
      return Netcdf4Classic;
    default:
      return BadFormat;
  }
}

Nc3Bool Nc3File::sync( void )
{
  if (!data_mode())
    return 0;
  if (Nc3Error::set_err(
              nc_sync(the_id)
                        ) != NC_NOERR)
    return 0;
  int i;
  for (i = 0; i < num_dims(); i++) {
    if (dimensions[i]->is_valid()) {
      dimensions[i]->sync();
    } else {		// someone else added a new dimension
      dimensions[i] = new Nc3Dim(this,i);
    }
  }
  for (i = 0; i < num_vars(); i++) {
    if (variables[i]->is_valid()) {
      variables[i]->sync();
    } else {		// someone else added a new variable
      variables[i] = new Nc3Var(this,i);
    }
  }
  return 1;
}

Nc3Bool Nc3File::close( void )
{
  int i;
    
  if (the_id == ncBad)
    return 0;
  for (i = 0; i < num_dims(); i++)
    delete dimensions[i];
  for (i = 0; i < num_vars(); i++)
    delete variables[i];
  delete [] dimensions;
  delete [] variables;
  delete globalv;
  int old_id = the_id;
  the_id = ncBad;
  return Nc3Error::set_err(
          nc_close(old_id)
                           ) == NC_NOERR;
}

Nc3Bool Nc3File::abort( void )
{
  return Nc3Error::set_err(
          nc_abort(the_id)
                           ) == NC_NOERR;
}

Nc3Bool Nc3File::define_mode( void )
{
  if (! is_valid())
    return FALSE;
  if (in_define_mode)
    return TRUE;
  if (Nc3Error::set_err(
              nc_redef(the_id)
                        ) != NC_NOERR)
    return FALSE;
  in_define_mode = 1;
  return TRUE;
}

Nc3Bool Nc3File::data_mode( void )
{
  if (! is_valid())
    return FALSE;
  if (! in_define_mode)
    return TRUE;
  if (Nc3Error::set_err(
              nc_enddef(the_id)
                        ) != NC_NOERR)
    return FALSE;
  in_define_mode = 0;
  return TRUE;
}

int Nc3File::id( void ) const
{
  return the_id;
}

Nc3File::Nc3File( const char* path, FileMode fmode, 
                  size_t* bufrsizeptr, size_t initialsize, FileFormat fformat  )
{
  Nc3Error err(Nc3Error::silent_nonfatal); // constructor must not fail

  int mode = NC_NOWRITE;
  the_fill_mode = Fill;
  int status;
    
  // If the user wants a 64-bit offset format, set that flag.
  if (fformat == Offset64Bits)
    mode |= NC_64BIT_OFFSET;
  else if (fformat == Netcdf4)
    mode |= NC_NETCDF4;
  else if (fformat == Netcdf4Classic)
    mode |= NC_NETCDF4|NC_CLASSIC_MODEL;

  switch (fmode) {
    case Write:
      mode |= NC_WRITE;
      /*FALLTHRU*/
    case ReadOnly:
      // use netcdf-3 interface to permit specifying tuning parameter
      status = Nc3Error::set_err(
              nc__open(path, mode, bufrsizeptr, &the_id)
                                 );
      if(status != NC_NOERR)
      {
        Nc3Error::set_err(status);
        the_id =  -1;
      }
      in_define_mode = 0;
      break;
    case New:
      mode |= NC_NOCLOBBER;
      /*FALLTHRU*/
    case Replace:
      // use netcdf-3 interface to permit specifying tuning parameters
      status = Nc3Error::set_err(
              nc__create(path, mode, initialsize,
                         bufrsizeptr, &the_id)
                                 );
      if(status != NC_NOERR)
      {
        Nc3Error::set_err(status);
        the_id =  -1;
      }
      in_define_mode = 1;
      break;
    default:
      the_id = ncBad;
      in_define_mode = 0;
      break;
  }
  if (is_valid()) {
    dimensions = new Nc3Dim*[NC_MAX_DIMS];
    variables = new Nc3Var*[NC_MAX_VARS];
    int i;
    for (i = 0; i < num_dims(); i++)
      dimensions[i] = new Nc3Dim(this, i);
    for (i = 0; i < num_vars(); i++)
      variables[i] = new Nc3Var(this, i);
    globalv = new Nc3Var(this, ncGlobal);
  } else {
    dimensions = 0;
    variables = 0;
    globalv = 0;
  }
}

Nc3Token Nc3Dim::name( void ) const
{
  return the_name;
}

long Nc3Dim::size( void ) const
{
  size_t sz = 0;
  if (the_file)
    Nc3Error::set_err(
            nc_inq_dimlen(the_file->id(), the_id, &sz)
                      );
  return sz;
}

Nc3Bool Nc3Dim::is_valid( void ) const
{
  return the_file->is_valid() && the_id != ncBad;
}

Nc3Bool Nc3Dim::is_unlimited( void ) const
{
  if (!the_file)
    return FALSE;
  int recdim;
  Nc3Error::set_err(
          nc_inq_unlimdim(the_file->id(), &recdim)
                    );
  return the_id == recdim;
}

Nc3Bool Nc3Dim::rename(Nc3Token newname)
{
  if (strlen(newname) > strlen(the_name)) {
    if (! the_file->define_mode())
      return FALSE;
  }
  Nc3Bool ret = Nc3Error::set_err(
          nc_rename_dim(the_file->id(), the_id, newname)
				  ) == NC_NOERR;
  if (ret) {
    delete [] the_name;
    the_name = new char[1 + strlen(newname)];
    strcpy(the_name, newname);
  }
  return ret;
}

int Nc3Dim::id( void ) const
{
  return the_id;
}

Nc3Bool Nc3Dim::sync(void) 
{    
  char nam[NC_MAX_NAME];
  if (the_name) {
    delete [] the_name;
  }
  if (the_file && Nc3Error::set_err(
              nc_inq_dimname(the_file->id(), the_id, nam)
                                    ) == NC_NOERR) {
    the_name = new char[strlen(nam) + 1]; 
    strcpy(the_name, nam);
    return TRUE;
  }
  the_name = 0;
  return FALSE;
}

Nc3Dim::Nc3Dim(Nc3File* nc, int id)
	: the_file(nc), the_id(id)
{
  char nam[NC_MAX_NAME];
  if (the_file && Nc3Error::set_err(
              nc_inq_dimname(the_file->id(), the_id, nam)
                                    ) == NC_NOERR) {
    the_name = new char[strlen(nam) + 1]; 
    strcpy(the_name, nam);
  } else {
    the_name = 0;
  }
}

Nc3Dim::Nc3Dim(Nc3File* nc, Nc3Token name, long sz)
	: the_file(nc)
{
  size_t dimlen = sz;
  if(Nc3Error::set_err(
             nc_def_dim(the_file->id(), name, dimlen, &the_id)
                       ) == NC_NOERR) {
    the_name = new char[strlen(name) + 1];
    strcpy(the_name, name);
  } else {
    the_name = 0;
  }
}

Nc3Dim::~Nc3Dim( void )
{
  delete [] the_name;
}

// OLD MACRO DEFINITION - now explicitly expanded in the code
// #define Nc_as(TYPE) name2(as_,TYPE)
// #define Nc3TypedComponent_as(TYPE)                      
//   TYPE Nc3TypedComponent::Nc_as(TYPE)( long n ) const   
//   {                                                     
//     Nc3Values* tmp = values();                          
//     TYPE rval = tmp->Nc_as(TYPE)(n);                    
//     delete tmp;                                         
//     return rval;                                        
//   }
// Nc3TypedComponent_as(ncbyte)
//   Nc3TypedComponent_as(char)
//   Nc3TypedComponent_as(short)
//   Nc3TypedComponent_as(int)
//   Nc3TypedComponent_as(nclong)
//   Nc3TypedComponent_as(long)
//   Nc3TypedComponent_as(float)
//   Nc3TypedComponent_as(double)

ncbyte Nc3TypedComponent::as_ncbyte( long n ) const {
  Nc3Values* tmp = values();
  if (tmp == NULL) {
    return 0;
  }
  ncbyte rval = tmp->as_ncbyte(n);
  delete tmp;
  return rval;
}

char Nc3TypedComponent::as_char( long n ) const {
  Nc3Values* tmp = values();
  if (tmp == NULL) {
    return 0;
  }
  char rval = tmp->as_char(n);
  delete tmp;
  return rval;
}

short Nc3TypedComponent::as_short( long n ) const {
  Nc3Values* tmp = values();
  if (tmp == NULL) {
    return 0;
  }
  short rval = tmp->as_short(n);
  delete tmp;
  return rval;
}

int Nc3TypedComponent::as_int( long n ) const {
  Nc3Values* tmp = values();
  if (tmp == NULL) {
    return 0;
  }
  int rval = tmp->as_int(n);
  delete tmp;
  return rval;
}

int64_t Nc3TypedComponent::as_int64( long n ) const {
  Nc3Values* tmp = values();
  if (tmp == NULL) {
    return 0;
  }
  int64_t rval = tmp->as_int64(n);
  delete tmp;
  return rval;
}

nclong Nc3TypedComponent::as_nclong( long n ) const {
  Nc3Values* tmp = values();
  if (tmp == NULL) {
    return 0;
  }
  nclong rval = tmp->as_nclong(n);
  delete tmp;
  return rval;
}

long Nc3TypedComponent::as_long( long n ) const {
  Nc3Values* tmp = values();
  if (tmp == NULL) {
    return 0;
  }
  long rval = tmp->as_long(n);
  delete tmp;
  return rval;
}

float Nc3TypedComponent::as_float( long n ) const {
  Nc3Values* tmp = values();
  if (tmp == NULL) {
    return 0;
  }
  float rval = tmp->as_float(n);
  delete tmp;
  return rval;
}

double Nc3TypedComponent::as_double( long n ) const {
  Nc3Values* tmp = values();
  if (tmp == NULL) {
    return 0;
  }
  double rval = tmp->as_double(n);
  delete tmp;
  return rval;
}
  
char* Nc3TypedComponent::as_string( long n ) const
{
  Nc3Values* tmp = values();
  if (tmp == NULL) {
    char* s = new char[n + 1];
    s[0] = '\0';
    return s;
  }
  char* rval = tmp->as_string(n);
  delete tmp;
  return rval;
}

Nc3TypedComponent::Nc3TypedComponent ( Nc3File* nc )
	: the_file(nc)
{}

Nc3Values* Nc3TypedComponent::get_space( long numVals ) const
{
  Nc3Values* valp;
  if (numVals < 1)
    numVals = num_vals();
  switch (type()) {
    case nc3Float:
      valp = new Nc3Values_float(numVals);
      break;
    case nc3Double:
      valp = new Nc3Values_double(numVals);
      break;
    case nc3Int:
      valp = new Nc3Values_int(numVals);
      break;
    case nc3Short:
      valp = new Nc3Values_short(numVals);
      break;
    case nc3Byte:
    case nc3Char:
      valp = new Nc3Values_char(numVals);
      break;
    case nc3NoType:
    default:
      valp = 0;
  }
  return valp;
}

Nc3Var::~Nc3Var( void )
{
  delete[] the_cur;
  delete[] cur_rec;
  delete[] the_name;
}

Nc3Token Nc3Var::name( void ) const
{
  return the_name;
}

Nc3Type Nc3Var::type( void ) const
{
  nc_type typ;
  Nc3Error::set_err(
          nc_inq_vartype(the_file->id(), the_id, &typ)
                    );
  return (Nc3Type) typ;
}

Nc3Bool Nc3Var::is_valid( void ) const
{
  return the_file->is_valid() && the_id != ncBad;
}

int Nc3Var::num_dims( void ) const
{
  int ndim;
  Nc3Error::set_err(
          nc_inq_varndims(the_file->id(), the_id, &ndim)
                    );
  return ndim;
}

// The i-th dimension for this variable
Nc3Dim* Nc3Var::get_dim( int i ) const
{
  int ndim;
  int dims[NC_MAX_DIMS];
  if(Nc3Error::set_err(
             nc_inq_var(the_file->id(), the_id, 0, 0, &ndim, dims, 0)
                       ) != NC_NOERR ||
     i < 0 || i >= ndim)
    return 0;
  return the_file->get_dim(dims[i]);
}

long* Nc3Var::edges( void ) const	// edge lengths (dimension sizes)
{
  long* evec = new long[num_dims()];
  for(int i=0; i < num_dims(); i++)
    evec[i] = get_dim(i)->size();
  return evec;
}

int Nc3Var::num_atts( void ) const // handles variable and global atts
{
  int natt = 0;
  if (the_file->is_valid()) {
    if (the_id == ncGlobal) {
      natt = the_file->num_atts();
    } else {
      Nc3Error::set_err(
              nc_inq_varnatts(the_file->id(), the_id, &natt)
                        );
    }
  }
  return natt;
}

Nc3Att* Nc3Var::get_att( Nc3Token aname ) const
{
  Nc3Att* att = new Nc3Att(the_file, this, aname);
  if (! att->is_valid()) {
    delete att;
    return 0;
  }
  return att;
}

Nc3Att* Nc3Var::get_att( int n ) const
{
  if (n < 0 || n >= num_atts())
    return 0;
  Nc3Token aname = attname(n);
  Nc3Att* ap = get_att(aname);
  delete [] (char*)aname;
  return ap;
}

long Nc3Var::num_vals( void ) const
{
  long prod = 1;
  for (int d = 0; d < num_dims(); d++)
    prod *= get_dim(d)->size();
  return  prod;
}

Nc3Values* Nc3Var::values( void ) const
{
  int ndims = num_dims();
  size_t crnr[NC_MAX_DIMS];
  size_t edgs[NC_MAX_DIMS];
  for (int i = 0; i < ndims; i++) {
    crnr[i] = 0;
    edgs[i] = get_dim(i)->size();
  }
  Nc3Values* valp = get_space();
  int status;
  switch (type()) {
    case nc3Float:
      status = Nc3Error::set_err(
              nc_get_vara_float(the_file->id(), the_id, crnr, edgs, 
                                (float *)valp->base())
                                 );
      break;
    case nc3Double:
      status = Nc3Error::set_err(
              nc_get_vara_double(the_file->id(), the_id, crnr, edgs, 
                                 (double *)valp->base())
                                 );
      break;
    case nc3Int:
      status = Nc3Error::set_err(
              nc_get_vara_int(the_file->id(), the_id, crnr, edgs, 
                              (int *)valp->base())
                                 );
      break;
    case nc3Short:
      status = Nc3Error::set_err(
              nc_get_vara_short(the_file->id(), the_id, crnr, edgs, 
                                (short *)valp->base())
                                 );
      break;
    case nc3Byte:
      status = Nc3Error::set_err(
              nc_get_vara_schar(the_file->id(), the_id, crnr, edgs, 
                                (signed char *)valp->base())
                                 );
      break;
    case nc3Char:
      status = Nc3Error::set_err(
              nc_get_vara_text(the_file->id(), the_id, crnr, edgs, 
                               (char *)valp->base())
                                 );
      break;
    case nc3NoType:
    default:
      return 0;
  }
  if (status != NC_NOERR)
    return 0;
  return valp;
}

int Nc3Var::dim_to_index(Nc3Dim *rdim)
{
  for (int i=0; i < num_dims() ; i++) {
    if (strcmp(get_dim(i)->name(),rdim->name()) == 0) {
      return i;
    }
  }
  // we should fail and gripe about it here....
  return -1;
}

void Nc3Var::set_rec(Nc3Dim *rdim, long slice)
{
  int i = dim_to_index(rdim);
  // we should fail and gripe about it here....
  if (slice >= get_dim(i)->size() && ! get_dim(i)->is_unlimited())
    return;  
  cur_rec[i] = slice;
  return;
} 

void Nc3Var::set_rec(long rec)
{
  // Since we can't ask for the record dimension here
  // just assume [0] is it.....
  set_rec(get_dim(0),rec);
  return;
} 

Nc3Values* Nc3Var::get_rec(void)
{
  return get_rec(get_dim(0), cur_rec[0]);
}

Nc3Values* Nc3Var::get_rec(long rec)
{
  return get_rec(get_dim(0), rec);
}

Nc3Values* Nc3Var::get_rec(Nc3Dim* rdim, long slice)
{
  int idx = dim_to_index(rdim);
  long size = num_dims();
  size_t* start = new size_t[size];
  long* startl = new long[size];
  for (int i=1; i < size ; i++) {
    start[i] = 0;
    startl[i] = 0;
  }
  start[idx] = slice;
  startl[idx] = slice;
  Nc3Bool result = set_cur(startl);
  if (! result ) {
    delete [] start;
    delete [] startl;
    return 0;
  }

  long* edgel = edges();
  size_t* edge = new size_t[size];
  for (int i=1; i < size ; i++) {
    edge[i] = edgel[i];
  }
  edge[idx] = 1;
  edgel[idx] = 1;
  Nc3Values* valp = get_space(rec_size(rdim));
  int status;
  switch (type()) {
    case nc3Float:
      status = Nc3Error::set_err(
              nc_get_vara_float(the_file->id(), the_id, start, edge, 
                                (float *)valp->base())
                                 );
      break;
    case nc3Double:
      status = Nc3Error::set_err(
              nc_get_vara_double(the_file->id(), the_id, start, edge, 
                                 (double *)valp->base())
                                 );
      break;
    case nc3Int:
      status = Nc3Error::set_err(
              nc_get_vara_int(the_file->id(), the_id, start, edge, 
                              (int *)valp->base())
                                 );
      break;
    case nc3Short:
      status = Nc3Error::set_err(
              nc_get_vara_short(the_file->id(), the_id, start, edge, 
                                (short *)valp->base())
                                 );
      break;
    case nc3Byte:
      status = Nc3Error::set_err(
              nc_get_vara_schar(the_file->id(), the_id, start, edge, 
                                (signed char *)valp->base())
                                 );
      break;
    case nc3Char:
      status = Nc3Error::set_err(
              nc_get_vara_text(the_file->id(), the_id, start, edge, 
                               (char *)valp->base())
                                 );
      break;
    case nc3NoType:
    default:
      delete [] start;
      delete [] startl;
      delete [] edge;
      delete [] edgel;
      return 0;
  }
  delete [] start;
  delete [] startl;
  delete [] edge;
  delete [] edgel;
  if (status != NC_NOERR) {
    delete valp;
    return 0;
  }
  return valp;
} 

// OLD MACRO DEFINITION - now explicitly expanded in the code
// #define Nc3Var_put_rec(TYPE)                                    
//   Nc3Bool Nc3Var::put_rec( const TYPE* vals)                    
//   {                                                             
//     return put_rec(get_dim(0), vals, cur_rec[0]);               
//   }                                                             
//                                                                 
//   Nc3Bool Nc3Var::put_rec( Nc3Dim *rdim, const TYPE* vals)      
//   {                                                             
//     int idx = dim_to_index(rdim);                               
//     return put_rec(rdim, vals, cur_rec[idx]);                   
//   }                                                             
//                                                                 
//   Nc3Bool Nc3Var::put_rec( const TYPE* vals,                    
//                            long rec)                            
//   {                                                             
//     return put_rec(get_dim(0), vals, rec);                      
//   }                                                             
//                                                                 
//   Nc3Bool Nc3Var::put_rec( Nc3Dim* rdim, const TYPE* vals,      
//                            long slice)                          
//   {                                                             
//     int idx = dim_to_index(rdim);                               
//     long size = num_dims();                                     
//     long* start = new long[size];                               
//     for (int i=1; i < size ; i++) start[i] = 0;                 
//     start[idx] = slice;                                         
//     Nc3Bool result = set_cur(start);                            
//     delete [] start;                                            
//     if (! result )                                              
//       return FALSE;                                             
//                                                                 
//     long* edge = edges();                                       
//     edge[idx] = 1;                                              
//     result = put(vals, edge);                                   
//     delete [] edge;                                             
//     return result;                                              
//   }

// Nc3Var_put_rec(ncbyte)
//   Nc3Var_put_rec(char)
//   Nc3Var_put_rec(short)
//   Nc3Var_put_rec(int)
//   Nc3Var_put_rec(long)
//   Nc3Var_put_rec(float)
//   Nc3Var_put_rec(double)

Nc3Bool Nc3Var::put_rec( const ncbyte* vals) {
  return put_rec(get_dim(0), vals, cur_rec[0]);
}
Nc3Bool Nc3Var::put_rec( Nc3Dim *rdim, const ncbyte* vals) {
  int idx = dim_to_index(rdim);
  return put_rec(rdim, vals, cur_rec[idx]);
}
Nc3Bool Nc3Var::put_rec( const ncbyte* vals, long rec) {
  return put_rec(get_dim(0), vals, rec);
}
Nc3Bool Nc3Var::put_rec( Nc3Dim* rdim, const ncbyte* vals, long slice) {
  int idx = dim_to_index(rdim);
  long size = num_dims();
  long* start = new long[size];
  for (int i=1; i < size ; i++) start[i] = 0;
  start[idx] = slice;
  Nc3Bool result = set_cur(start);
  delete [] start;
  if (! result ) return 0;
  long* edge = edges();
  edge[idx] = 1;
  result = put(vals, edge);
  delete [] edge;
  return result;
}

Nc3Bool Nc3Var::put_rec( const char* vals) {
  return put_rec(get_dim(0), vals, cur_rec[0]);
}
Nc3Bool Nc3Var::put_rec( Nc3Dim *rdim, const char* vals) {
  int idx = dim_to_index(rdim);
  return put_rec(rdim, vals, cur_rec[idx]);
}
Nc3Bool Nc3Var::put_rec( const char* vals, long rec) {
  return put_rec(get_dim(0), vals, rec);
}
Nc3Bool Nc3Var::put_rec( Nc3Dim* rdim, const char* vals, long slice) {
  int idx = dim_to_index(rdim);
  long size = num_dims();
  long* start = new long[size];
  for (int i=1; i < size ; i++) start[i] = 0;
  start[idx] = slice;
  Nc3Bool result = set_cur(start);
  delete [] start;
  if (! result ) return 0;
  long* edge = edges();
  edge[idx] = 1;
  result = put(vals, edge);
  delete [] edge;
  return result;
}

Nc3Bool Nc3Var::put_rec( const short* vals) {
  return put_rec(get_dim(0), vals, cur_rec[0]);
}
Nc3Bool Nc3Var::put_rec( Nc3Dim *rdim, const short* vals) {
  int idx = dim_to_index(rdim);
  return put_rec(rdim, vals, cur_rec[idx]);
}
Nc3Bool Nc3Var::put_rec( const short* vals, long rec) {
  return put_rec(get_dim(0), vals, rec);
}
Nc3Bool Nc3Var::put_rec( Nc3Dim* rdim, const short* vals, long slice) {
  int idx = dim_to_index(rdim);
  long size = num_dims();
  long* start = new long[size];
  for (int i=1; i < size ; i++) start[i] = 0;
  start[idx] = slice;
  Nc3Bool result = set_cur(start);
  delete [] start;
  if (! result ) return 0;
  long* edge = edges();
  edge[idx] = 1;
  result = put(vals, edge);
  delete [] edge;
  return result;
}

Nc3Bool Nc3Var::put_rec( const int* vals) {
  return put_rec(get_dim(0), vals, cur_rec[0]);
}
Nc3Bool Nc3Var::put_rec( Nc3Dim *rdim, const int* vals) {
  int idx = dim_to_index(rdim);
  return put_rec(rdim, vals, cur_rec[idx]);
}
Nc3Bool Nc3Var::put_rec( const int* vals, long rec) {
  return put_rec(get_dim(0), vals, rec);
}
Nc3Bool Nc3Var::put_rec( Nc3Dim* rdim, const int* vals, long slice) {
  int idx = dim_to_index(rdim);
  long size = num_dims();
  long* start = new long[size];
  for (int i=1; i < size ; i++) start[i] = 0;
  start[idx] = slice;
  Nc3Bool result = set_cur(start);
  delete [] start;
  if (! result ) return 0;
  long* edge = edges();
  edge[idx] = 1;
  result = put(vals, edge);
  delete [] edge;
  return result;
}

Nc3Bool Nc3Var::put_rec( const long* vals) {
  return put_rec(get_dim(0), vals, cur_rec[0]);
}
Nc3Bool Nc3Var::put_rec( Nc3Dim *rdim, const long* vals) {
  int idx = dim_to_index(rdim);
  return put_rec(rdim, vals, cur_rec[idx]);
}
Nc3Bool Nc3Var::put_rec( const long* vals, long rec) {
  return put_rec(get_dim(0), vals, rec);
}
Nc3Bool Nc3Var::put_rec( Nc3Dim* rdim, const long* vals, long slice) {
  int idx = dim_to_index(rdim);
  long size = num_dims();
  long* start = new long[size];
  for (int i=1; i < size ; i++) start[i] = 0;
  start[idx] = slice;
  Nc3Bool result = set_cur(start);
  delete [] start;
  if (! result ) return 0;
  long* edge = edges();
  edge[idx] = 1;
  result = put(vals, edge);
  delete [] edge;
  return result;
}

Nc3Bool Nc3Var::put_rec( const float* vals) {
  return put_rec(get_dim(0), vals, cur_rec[0]);
}
Nc3Bool Nc3Var::put_rec( Nc3Dim *rdim, const float* vals) {
  int idx = dim_to_index(rdim);
  return put_rec(rdim, vals, cur_rec[idx]);
}
Nc3Bool Nc3Var::put_rec( const float* vals, long rec) {
  return put_rec(get_dim(0), vals, rec);
}
Nc3Bool Nc3Var::put_rec( Nc3Dim* rdim, const float* vals, long slice) {
  int idx = dim_to_index(rdim);
  long size = num_dims();
  long* start = new long[size];
  for (int i=1; i < size ; i++) start[i] = 0;
  start[idx] = slice;
  Nc3Bool result = set_cur(start);
  delete [] start;
  if (! result ) return 0;
  long* edge = edges();
  edge[idx] = 1;
  result = put(vals, edge);
  delete [] edge;
  return result;
}

Nc3Bool Nc3Var::put_rec( const double* vals) {
  return put_rec(get_dim(0), vals, cur_rec[0]);
}
Nc3Bool Nc3Var::put_rec( Nc3Dim *rdim, const double* vals) {
  int idx = dim_to_index(rdim);
  return put_rec(rdim, vals, cur_rec[idx]);
}
Nc3Bool Nc3Var::put_rec( const double* vals, long rec) {
  return put_rec(get_dim(0), vals, rec);
}
Nc3Bool Nc3Var::put_rec( Nc3Dim* rdim, const double* vals, long slice) {
  int idx = dim_to_index(rdim);
  long size = num_dims();
  long* start = new long[size];
  for (int i=1; i < size ; i++) start[i] = 0;
  start[idx] = slice;
  Nc3Bool result = set_cur(start);
  delete [] start;
  if (! result ) return 0;
  long* edge = edges();
  edge[idx] = 1;
  result = put(vals, edge);
  delete [] edge;
  return result;
}

long Nc3Var::rec_size(void) {
  return rec_size(get_dim(0));
}

long Nc3Var::rec_size(Nc3Dim *rdim) {
  int idx = dim_to_index(rdim); 
  long size = 1;
  long* edge = edges();
  for( int i = 0 ; i<num_dims() ; i++) {
    if (i != idx) {
      size *= edge[i];
    }
  }
  delete [] edge;
  return size;
}

// OLD MACRO DEFINITION - now explicitly expanded in the code
// #define Nc3Var_get_index(TYPE)                                          
//   long Nc3Var::get_index(const TYPE* key)                               
//   {                                                                     
//    return get_index(get_dim(0), key);                                   
//    }                                                                    
//                                                                         
//   long Nc3Var::get_index(Nc3Dim *rdim, const TYPE* key)                 
//   {                                                                     
//    if (type() != Nc3TypeEnum(TYPE))                                     
//      return -1;                                                         
//    if (! the_file->data_mode())                                         
//      return -1;                                                         
//    int idx = dim_to_index(rdim);                                        
//    long maxrec = get_dim(idx)->size();                                  
//    long maxvals = rec_size(rdim);                                       
//    Nc3Values* val;                                                      
//    int validx;                                                          
//    for (long j=0; j<maxrec; j++) {                                      
//                                   val = get_rec(rdim,j);                
//                                   if (val == NULL) return -1;           
//                                   for (validx = 0; validx < maxvals; validx++) { 
//                                                                                 if (key[validx] != val->as_ ## TYPE(validx)) break; 
//                                                                                 } 
//                                   delete val;                           
//                                   if (validx == maxvals) return j;      
//                                   }                                     
//    return -1;                                                           
//    }


// Nc3Var_get_index(ncbyte)
//   Nc3Var_get_index(char)
//   Nc3Var_get_index(short)
//   Nc3Var_get_index(nclong)
//   Nc3Var_get_index(long)
//   Nc3Var_get_index(float)
//   Nc3Var_get_index(double)

long Nc3Var::get_index(const ncbyte* key) {
  return get_index(get_dim(0), key);
}
long Nc3Var::get_index(Nc3Dim *rdim, const ncbyte* key) {
  if (type() != nc3Byte) return -1;
  if (! the_file->data_mode()) return -1;
  int idx = dim_to_index(rdim);
  long maxrec = get_dim(idx)->size();
  long maxvals = rec_size(rdim);
  Nc3Values* val; int validx;
  for (long j=0; j<maxrec; j++) {
    val = get_rec(rdim,j);
    if (val == __null) return -1;
    for (validx = 0; validx < maxvals; validx++) {
      if (key[validx] != val->as_ncbyte(validx)) break;
    }
    delete val;
    if (validx == maxvals) return j;
  }
  return -1;
}

long Nc3Var::get_index(const char* key) {
  return get_index(get_dim(0), key);
}
long Nc3Var::get_index(Nc3Dim *rdim, const char* key) {
  if (type() != nc3Char) return -1;
  if (! the_file->data_mode()) return -1;
  int idx = dim_to_index(rdim);
  long maxrec = get_dim(idx)->size();
  long maxvals = rec_size(rdim);
  Nc3Values* val; int validx;
  for (long j=0; j<maxrec; j++) {
    val = get_rec(rdim,j);
    if (val == __null) return -1;
    for (validx = 0; validx < maxvals; validx++) {
      if (key[validx] != val->as_char(validx)) break;
    }
    delete val;
    if (validx == maxvals) return j;
  }
  return -1;
}

long Nc3Var::get_index(const short* key) {
  return get_index(get_dim(0), key);
}
long Nc3Var::get_index(Nc3Dim *rdim, const short* key) {
  if (type() != nc3Short) return -1;
  if (! the_file->data_mode()) return -1;
  int idx = dim_to_index(rdim);
  long maxrec = get_dim(idx)->size();
  long maxvals = rec_size(rdim);
  Nc3Values* val;
  int validx;
  for (long j=0; j<maxrec; j++) {
    val = get_rec(rdim,j);
    if (val == __null) return -1;
    for (validx = 0; validx < maxvals; validx++) {
      if (key[validx] != val->as_short(validx)) break;
    } delete val;
    if (validx == maxvals) return j;
  }
  return -1;
}

long Nc3Var::get_index(const nclong* key) {
  return get_index(get_dim(0), key);
}
long Nc3Var::get_index(Nc3Dim *rdim, const nclong* key) {
  if (type() != nc3Long) return -1;
  if (! the_file->data_mode()) return -1;
  int idx = dim_to_index(rdim);
  long maxrec = get_dim(idx)->size();
  long maxvals = rec_size(rdim);
  Nc3Values* val;
  int validx;
  for (long j=0; j<maxrec; j++) {
    val = get_rec(rdim,j);
    if (val == __null) return -1;
    for (validx = 0; validx < maxvals; validx++) {
      if (key[validx] != val->as_nclong(validx)) break;
    }
    delete val;
    if (validx == maxvals) return j;
  }
  return -1;
}

long Nc3Var::get_index(const long* key) {
  return get_index(get_dim(0), key);
}
long Nc3Var::get_index(Nc3Dim *rdim, const long* key) {
  if (type() != nc3Long) return -1;
  if (! the_file->data_mode()) return -1;
  int idx = dim_to_index(rdim);
  long maxrec = get_dim(idx)->size();
  long maxvals = rec_size(rdim);
  Nc3Values* val;
  int validx;
  for (long j=0; j<maxrec; j++) {
    val = get_rec(rdim,j);
    if (val == __null) return -1;
    for (validx = 0; validx < maxvals; validx++) {
      if (key[validx] != val->as_long(validx)) break;
    }
    delete val;
    if (validx == maxvals) return j;
  }
  return -1;
}

long Nc3Var::get_index(const float* key) {
  return get_index(get_dim(0), key);
}
long Nc3Var::get_index(Nc3Dim *rdim, const float* key) {
  if (type() != nc3Float) return -1;
  if (! the_file->data_mode()) return -1;
  int idx = dim_to_index(rdim);
  long maxrec = get_dim(idx)->size();
  long maxvals = rec_size(rdim);
  Nc3Values* val;
  int validx;
  for (long j=0; j<maxrec; j++) {
    val = get_rec(rdim,j);
    if (val == __null) return -1;
    for (validx = 0; validx < maxvals; validx++) {
      if (key[validx] != val->as_float(validx)) break;
    }
    delete val;
    if (validx == maxvals) return j;
  }
  return -1;
}

long Nc3Var::get_index(const double* key) {
  return get_index(get_dim(0), key);
}
long Nc3Var::get_index(Nc3Dim *rdim, const double* key) {
  if (type() != nc3Double) return -1;
  if (! the_file->data_mode()) return -1;
  int idx = dim_to_index(rdim);
  long maxrec = get_dim(idx)->size();
  long maxvals = rec_size(rdim);
  Nc3Values* val; int validx;
  for (long j=0; j<maxrec; j++) {
    val = get_rec(rdim,j);
    if (val == __null) return -1;
    for (validx = 0; validx < maxvals; validx++) {
      if (key[validx] != val->as_double(validx)) break;
    }
    delete val;
    if (validx == maxvals) return j;
  }
  return -1;
}
  
// OLD MACRO DEFINITION - now explicitly expanded in the code
// Macros below work for short, nclong, long, float, and double, but for ncbyte
// and char, we must use corresponding schar, uchar, or text C functions, so in 
// these cases macros are expanded manually.
// #define Nc3Var_put_array(TYPE)                                          
//   Nc3Bool Nc3Var::put( const TYPE* vals,                                
//                          long edge0,                                    
//                          long edge1,                                    
//                          long edge2,                                    
//                          long edge3,                                    
//                          long edge4)                                    
//   {                                                                     
//    /* no need to check type() vs. TYPE, invoked C function will do that */ 
//    if (! the_file->data_mode())                                         
//      return FALSE;                                                      
//    size_t count[5];                                                     
//    count[0] = edge0;                                                    
//    count[1] = edge1;                                                    
//    count[2] = edge2;                                                    
//    count[3] = edge3;                                                    
//    count[4] = edge4;                                                    
//    for (int i = 0; i < 5; i++) {                                        
//                                 if (count[i]) {                         
//                                                if (num_dims() < i)      
//                                                  return FALSE;          
//                                                } else                   
//                                   break;                                
//                                 }                                       
//    size_t start[5];                                                     
//    for (int j = 0; j < 5; j++) {                                        
//                                 start[j] = the_cur[j];                  
//                                 }                                       
//    return Nc3Error::set_err(                                            
// 			    makename2(nc_put_vara_,TYPE) (the_file->id(), the_id, start, count, vals) 
// 			    ) == NC_NOERR;                              
//    }

Nc3Bool Nc3Var::put( const ncbyte* vals,
                     long edge0,
                     long edge1,
                     long edge2,
                     long edge3,
                     long edge4)
{
  /* no need to check type() vs. TYPE, invoked C function will do that */
  if (! the_file->data_mode())
    return FALSE;
  size_t count[5];
  count[0] = edge0;
  count[1] = edge1;
  count[2] = edge2;
  count[3] = edge3;
  count[4] = edge4;
  for (int i = 0; i < 5; i++) {
    if (count[i]) {
      if (num_dims() < i)
        return FALSE;
    } else
      break;
  }
  size_t start[5];
  for (int j = 0; j < 5; j++) {
    start[j] = the_cur[j];
  }
  return Nc3Error::set_err(
          nc_put_vara_schar (the_file->id(), the_id, start, count, vals)
                           ) == NC_NOERR;
}

Nc3Bool Nc3Var::put( const char* vals,
		     long edge0,
		     long edge1,
		     long edge2,
		     long edge3,
		     long edge4)
{
  /* no need to check type() vs. TYPE, invoked C function will do that */
  if (! the_file->data_mode())
    return FALSE;
  size_t count[5];
  count[0] = edge0;
  count[1] = edge1;
  count[2] = edge2;
  count[3] = edge3;
  count[4] = edge4;
  for (int i = 0; i < 5; i++) {
    if (count[i]) {
      if (num_dims() < i)
        return FALSE;
    } else
      break;
  }
  size_t start[5];
  for (int j = 0; j < 5; j++) {
    start[j] = the_cur[j];
  }
  return Nc3Error::set_err(
          nc_put_vara_text (the_file->id(), the_id, start, count, vals)
                           ) == NC_NOERR;
}

// Nc3Var_put_array(short)
//   Nc3Var_put_array(int)
//   Nc3Var_put_array(long)
//   Nc3Var_put_array(float)
//   Nc3Var_put_array(double)

Nc3Bool Nc3Var::put( const short* vals, long edge0, long edge1, long edge2, long edge3, long edge4) {
  if (! the_file->data_mode()) return 0;
  size_t count[5];
  count[0] = edge0;
  count[1] = edge1;
  count[2] = edge2;
  count[3] = edge3;
  count[4] = edge4;
  for (int i = 0; i < 5; i++) {
    if (count[i]) {
      if (num_dims() < i) return 0;
    } else break;
  }
  size_t start[5];
  for (int j = 0; j < 5; j++) {
    start[j] = the_cur[j];
  }
  return Nc3Error::set_err( nc_put_vara_short (the_file->id(), the_id, start, count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::put( const int* vals, long edge0, long edge1, long edge2, long edge3, long edge4) {
  if (! the_file->data_mode()) return 0;
  size_t count[5];
  count[0] = edge0;
  count[1] = edge1;
  count[2] = edge2;
  count[3] = edge3;
  count[4] = edge4;
  for (int i = 0; i < 5; i++) {
    if (count[i]) {
      if (num_dims() < i) return 0;
    } else break;
  }
  size_t start[5];
  for (int j = 0; j < 5; j++) {
    start[j] = the_cur[j];
  }
  return Nc3Error::set_err( nc_put_vara_int (the_file->id(), the_id, start, count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::put( const long* vals, long edge0, long edge1, long edge2, long edge3, long edge4) {
  if (! the_file->data_mode()) return 0;
  size_t count[5];
  count[0] = edge0;
  count[1] = edge1;
  count[2] = edge2;
  count[3] = edge3;
  count[4] = edge4;
  for (int i = 0; i < 5; i++) {
    if (count[i]) { if (num_dims() < i) return 0;
    } else break;
  }
  size_t start[5];
  for (int j = 0; j < 5; j++) {
    start[j] = the_cur[j];
  }
  return Nc3Error::set_err( nc_put_vara_long (the_file->id(), the_id, start, count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::put( const float* vals, long edge0, long edge1, long edge2, long edge3, long edge4) {
  if (! the_file->data_mode()) return 0;
  size_t count[5];
  count[0] = edge0;
  count[1] = edge1;
  count[2] = edge2;
  count[3] = edge3;
  count[4] = edge4;
  for (int i = 0; i < 5; i++) {
    if (count[i]) { if (num_dims() < i) return 0;
    } else break;
  }
  size_t start[5];
  for (int j = 0; j < 5; j++) {
    start[j] = the_cur[j];
  }
  return Nc3Error::set_err( nc_put_vara_float (the_file->id(), the_id, start, count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::put( const double* vals, long edge0, long edge1, long edge2, long edge3, long edge4) {
  if (! the_file->data_mode()) return 0;
  size_t count[5];
  count[0] = edge0;
  count[1] = edge1;
  count[2] = edge2;
  count[3] = edge3;
  count[4] = edge4;
  for (int i = 0; i < 5; i++) { if (count[i]) {
      if (num_dims() < i) return 0;
    } else break;
  }
  size_t start[5];
  for (int j = 0; j < 5; j++) {
    start[j] = the_cur[j];
  }
  return Nc3Error::set_err( nc_put_vara_double (the_file->id(), the_id, start, count, vals) ) == NC_NOERR;
}

// OLD MACRO DEFINITION - now explicitly expanded in the code
// #define Nc3Var_put_nd_array(TYPE)                                       
//   Nc3Bool Nc3Var::put( const TYPE* vals, const long* count )            
//   {                                                                     
//     /* no need to check type() vs. TYPE, invoked C function will do that */ 
//     if (! the_file->data_mode())                                        
//       return FALSE;                                                     
//     size_t start[NC_MAX_DIMS];                                          
//     for (int i = 0; i < num_dims(); i++)                                
//       start[i] = the_cur[i];                                            
//     return Nc3Error::set_err(                                           
//             makename2(nc_put_vara_,TYPE) (the_file->id(), the_id, start, (const size_t *) count, vals) 
//                                                                                ) == NC_NOERR; 
//   }

Nc3Bool Nc3Var::put( const ncbyte* vals, const long* count )
{
  /* no need to check type() vs. TYPE, invoked C function will do that */
  if (! the_file->data_mode())
    return FALSE;
  size_t start[NC_MAX_DIMS];
  for (int i = 0; i < num_dims(); i++)
    start[i] = the_cur[i];
  return Nc3Error::set_err(
          nc_put_vara_schar (the_file->id(), the_id, start, (const size_t *)count, vals)
                           ) == NC_NOERR;
}

Nc3Bool Nc3Var::put( const char* vals, const long* count )
{
  /* no need to check type() vs. TYPE, invoked C function will do that */
  if (! the_file->data_mode())
    return FALSE;
  size_t start[NC_MAX_DIMS];
  for (int i = 0; i < num_dims(); i++)
    start[i] = the_cur[i];
  return Nc3Error::set_err(
          nc_put_vara_text (the_file->id(), the_id, start, (const size_t *)count, vals)
                           ) == NC_NOERR;
}

// Nc3Var_put_nd_array(short)
//   Nc3Var_put_nd_array(int)
//   Nc3Var_put_nd_array(long)
//   Nc3Var_put_nd_array(float)
//   Nc3Var_put_nd_array(double)

Nc3Bool Nc3Var::put( const short* vals, const long* count ) {
  if (! the_file->data_mode()) return 0;
  size_t start[1024];
  for (int i = 0; i < num_dims(); i++) start[i] = the_cur[i];
  return Nc3Error::set_err( nc_put_vara_short (the_file->id(), the_id, start, (const size_t *) count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::put( const int* vals, const long* count ) {
  if (! the_file->data_mode()) return 0;
  size_t start[1024];
  for (int i = 0; i < num_dims(); i++) start[i] = the_cur[i];
  return Nc3Error::set_err( nc_put_vara_int (the_file->id(), the_id, start, (const size_t *) count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::put( const long* vals, const long* count ) {
  if (! the_file->data_mode()) return 0;
  size_t start[1024];
  for (int i = 0; i < num_dims(); i++) start[i] = the_cur[i];
  return Nc3Error::set_err( nc_put_vara_long (the_file->id(), the_id, start, (const size_t *) count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::put( const float* vals, const long* count ) {
  if (! the_file->data_mode()) return 0;
  size_t start[1024];
  for (int i = 0; i < num_dims(); i++) start[i] = the_cur[i];
  return Nc3Error::set_err( nc_put_vara_float (the_file->id(), the_id, start, (const size_t *) count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::put( const double* vals, const long* count ) {
  if (! the_file->data_mode()) return 0;
  size_t start[1024];
  for (int i = 0; i < num_dims(); i++) start[i] = the_cur[i];
  return Nc3Error::set_err( nc_put_vara_double (the_file->id(), the_id, start, (const size_t *) count, vals) ) == NC_NOERR;
}

// OLD MACRO DEFINITION - now explicitly expanded in the code
// #define Nc3Var_get_array(TYPE)                                          
//   Nc3Bool Nc3Var::get( TYPE* vals,                                      
//                          long edge0,                                    
//                          long edge1,                                    
//                          long edge2,                                    
//                          long edge3,                                    
//                          long edge4) const                              
//   {                                                                     
//    if (! the_file->data_mode())                                         
//      return FALSE;                                                      
//    size_t count[5];                                                     
//    count[0] = edge0;                                                    
//    count[1] = edge1;                                                    
//    count[2] = edge2;                                                    
//    count[3] = edge3;                                                    
//    count[4] = edge4;                                                    
//    for (int i = 0; i < 5; i++) {                                        
//                                 if (count[i]) {                         
//                                                if (num_dims() < i)      
//                                                  return FALSE;          
//                                                } else                   
//                                   break;                                
//                                 }                                       
//    size_t start[5];                                                     
//    for (int j = 0; j < 5; j++) {                                        
//                                 start[j] = the_cur[j];                  
//                                 }                                       
//    return Nc3Error::set_err(                                            
// 			    makename2(nc_get_vara_,TYPE) (the_file->id(), the_id, start, count, vals) 
// 			    ) == NC_NOERR;                              
//    }

Nc3Bool Nc3Var::get( ncbyte* vals,
                     long edge0,
                     long edge1,
                     long edge2,
                     long edge3,
                     long edge4) const
{
  if (! the_file->data_mode())
    return FALSE;
  size_t count[5];
  count[0] = edge0;
  count[1] = edge1;
  count[2] = edge2;
  count[3] = edge3;
  count[4] = edge4;
  for (int i = 0; i < 5; i++) {
    if (count[i]) {
      if (num_dims() < i)
        return FALSE;
    } else
      break;
  }
  size_t start[5];
  for (int j = 0; j < 5; j++) {
    start[j] = the_cur[j];
  }
  return Nc3Error::set_err(
          nc_get_vara_schar (the_file->id(), the_id, start, count, vals)
                           ) == NC_NOERR;
}

Nc3Bool Nc3Var::get( char* vals,
                     long edge0,
                     long edge1,
                     long edge2,
                     long edge3,
                     long edge4) const
{
  if (! the_file->data_mode())
    return FALSE;
  size_t count[5];
  count[0] = edge0;
  count[1] = edge1;
  count[2] = edge2;
  count[3] = edge3;
  count[4] = edge4;
  for (int i = 0; i < 5; i++) {
    if (count[i]) {
      if (num_dims() < i)
        return FALSE;
    } else
      break;
  }
  size_t start[5];
  for (int j = 0; j < 5; j++) {
    start[j] = the_cur[j];
  }
  return Nc3Error::set_err(
          nc_get_vara_text (the_file->id(), the_id, start, count, vals)
                           ) == NC_NOERR;
}

// Nc3Var_get_array(short)
//   Nc3Var_get_array(int)
//   Nc3Var_get_array(long)
//   Nc3Var_get_array(float)
//   Nc3Var_get_array(double)

Nc3Bool Nc3Var::get( short* vals, long edge0, long edge1, long edge2, long edge3, long edge4) const {
  if (! the_file->data_mode()) return 0;
  size_t count[5];
  count[0] = edge0;
  count[1] = edge1;
  count[2] = edge2;
  count[3] = edge3;
  count[4] = edge4;
  for (int i = 0; i < 5; i++) {
    if (count[i]) {
      if (num_dims() < i) return 0;
    } else break;
  }
  size_t start[5];
  for (int j = 0; j < 5; j++) {
    start[j] = the_cur[j];
  }
  return Nc3Error::set_err( nc_get_vara_short (the_file->id(), the_id, start, count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::get( int* vals, long edge0, long edge1, long edge2, long edge3, long edge4) const {
  if (! the_file->data_mode()) return 0;
  size_t count[5];
  count[0] = edge0;
  count[1] = edge1;
  count[2] = edge2;
  count[3] = edge3;
  count[4] = edge4;
  for (int i = 0; i < 5; i++) { if (count[i]) {
      if (num_dims() < i) return 0;
    } else break;
  }
  size_t start[5];
  for (int j = 0; j < 5; j++) {
    start[j] = the_cur[j];
  }
  return Nc3Error::set_err( nc_get_vara_int (the_file->id(), the_id, start, count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::get( long* vals, long edge0, long edge1, long edge2, long edge3, long edge4) const {
  if (! the_file->data_mode()) return 0;
  size_t count[5];
  count[0] = edge0;
  count[1] = edge1;
  count[2] = edge2;
  count[3] = edge3;
  count[4] = edge4;
  for (int i = 0; i < 5; i++) { if (count[i]) {
      if (num_dims() < i) return 0;
    } else break;
  }
  size_t start[5];
  for (int j = 0; j < 5; j++) {
    start[j] = the_cur[j];
  }
  return Nc3Error::set_err( nc_get_vara_long (the_file->id(), the_id, start, count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::get( float* vals, long edge0, long edge1, long edge2, long edge3, long edge4) const {
  if (! the_file->data_mode()) return 0;
  size_t count[5];
  count[0] = edge0;
  count[1] = edge1;
  count[2] = edge2;
  count[3] = edge3;
  count[4] = edge4;
  for (int i = 0; i < 5; i++) {
    if (count[i]) { if (num_dims() < i) return 0;
    } else break;
  }
  size_t start[5];
  for (int j = 0; j < 5; j++) {
    start[j] = the_cur[j];
  }
  return Nc3Error::set_err( nc_get_vara_float (the_file->id(), the_id, start, count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::get( double* vals, long edge0, long edge1, long edge2, long edge3, long edge4) const {
  if (! the_file->data_mode()) return 0;
  size_t count[5];
  count[0] = edge0;
  count[1] = edge1;
  count[2] = edge2;
  count[3] = edge3;
  count[4] = edge4;
  for (int i = 0; i < 5; i++) {
    if (count[i]) { if (num_dims() < i) return 0;
    } else break;
  }
  size_t start[5];
  for (int j = 0; j < 5; j++) {
    start[j] = the_cur[j];
  }
  return Nc3Error::set_err( nc_get_vara_double (the_file->id(), the_id, start, count, vals) ) == NC_NOERR;
}

// OLD MACRO DEFINITION - now explicitly expanded in the code
// #define Nc3Var_get_nd_array(TYPE)                                    
//   Nc3Bool Nc3Var::get( TYPE* vals, const long* count ) const            
//   {                                                                     
//    if (! the_file->data_mode())                                         
//      return FALSE;                                                      
//    size_t start[NC_MAX_DIMS];                                           
//    for (int i = 0; i < num_dims(); i++)                                 
//      start[i] = the_cur[i];                                             
//    return Nc3Error::set_err(                                            
// 			    makename2(nc_get_vara_,TYPE) (the_file->id(), the_id, start,  (const size_t *) count, vals) 
// 			    ) == NC_NOERR;                              
//    }

Nc3Bool Nc3Var::get( ncbyte* vals, const long* count ) const
{
  if (! the_file->data_mode())
    return FALSE;
  size_t start[NC_MAX_DIMS];
  for (int i = 0; i < num_dims(); i++)
    start[i] = the_cur[i];
  return nc_get_vara_schar (the_file->id(), the_id, start,  (const size_t *) count, vals) == NC_NOERR;
}

Nc3Bool Nc3Var::get( char* vals, const long* count ) const
{
  if (! the_file->data_mode())
    return FALSE;
  size_t start[NC_MAX_DIMS];
  for (int i = 0; i < num_dims(); i++)
    start[i] = the_cur[i];
  return nc_get_vara_text (the_file->id(), the_id, start, (const size_t*) count, vals) == NC_NOERR;
}

// Nc3Var_get_nd_array(short)
//   Nc3Var_get_nd_array(int)
//   Nc3Var_get_nd_array(long)
//   Nc3Var_get_nd_array(float)
//   Nc3Var_get_nd_array(double)

Nc3Bool Nc3Var::get( short* vals, const long* count ) const {
  if (! the_file->data_mode()) return 0;
  size_t start[1024];
  for (int i = 0; i < num_dims(); i++) start[i] = the_cur[i];
  return Nc3Error::set_err( nc_get_vara_short (the_file->id(), the_id, start,
                                               (const size_t *) count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::get( int* vals, const long* count ) const {
  if (! the_file->data_mode()) return 0;
  size_t start[1024];
  for (int i = 0; i < num_dims(); i++) start[i] = the_cur[i];
  return Nc3Error::set_err( nc_get_vara_int (the_file->id(), the_id, start,
                                             (const size_t *) count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::get( long* vals, const long* count ) const {
  if (! the_file->data_mode()) return 0;
  size_t start[1024];
  for (int i = 0; i < num_dims(); i++) start[i] = the_cur[i];
  return Nc3Error::set_err( nc_get_vara_long (the_file->id(), the_id, start,
                                              (const size_t *) count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::get( float* vals, const long* count ) const {
  if (! the_file->data_mode()) return 0;
  size_t start[1024];
  for (int i = 0; i < num_dims(); i++) start[i] = the_cur[i];
  return Nc3Error::set_err( nc_get_vara_float (the_file->id(), the_id, start,
                                               (const size_t *) count, vals) ) == NC_NOERR;
}

Nc3Bool Nc3Var::get( double* vals, const long* count ) const {
  if (! the_file->data_mode()) return 0;
  size_t start[1024];
  for (int i = 0; i < num_dims(); i++) start[i] = the_cur[i];
  return Nc3Error::set_err( nc_get_vara_double (the_file->id(), the_id, start,
                                                (const size_t *) count, vals) ) == NC_NOERR;
}

// If no args, set cursor to all zeros.	 Else set initial elements of cursor
// to args provided, rest to zeros.
Nc3Bool Nc3Var::set_cur(long c0, long c1, long c2, long c3, long c4)
{
  long t[6];
  t[0] = c0;
  t[1] = c1;
  t[2] = c2;
  t[3] = c3;
  t[4] = c4;
  t[5] = -1;
  for(int j = 0; j < 6; j++) { // find how many parameters were used
    int i;
    if (t[j] == -1) {
      if (num_dims() < j)
        return FALSE;	// too many for variable's dimensionality
      for (i = 0; i < j; i++) {
        if (t[i] >= get_dim(i)->size() && ! get_dim(i)->is_unlimited())
          return FALSE;	// too big for dimension
        the_cur[i] = t[i];
      }
      for(i = j; i < num_dims(); i++)
        the_cur[i] = 0;
      return TRUE;
    }
  }
  return TRUE;
}

Nc3Bool Nc3Var::set_cur(long* cur)
{
  for(int i = 0; i < num_dims(); i++) {
    if (cur[i] >= get_dim(i)->size() && ! get_dim(i)->is_unlimited())
      return FALSE;
    the_cur[i] = cur[i];
  }
  return TRUE;
}

// OLD MACRO DEFINITION - now explicitly expanded in the code
// #define Nc3Var_add_scalar_att(TYPE)                                  
//   Nc3Bool Nc3Var::add_att(Nc3Token aname, TYPE val)                  
//   {                                                                     
//    if (! the_file->define_mode())                                    
//      return FALSE;                                                   
//    if (Nc3Error::set_err(                                            
//                          makename2(nc_put_att_,TYPE) (the_file->id(), the_id, aname, (nc_type) Nc3TypeEnum(TYPE), 
//                                                         1, &val)     
//                          ) != NC_NOERR)                              
//      return FALSE;                                                   
//    return TRUE;                                                      
//    }

Nc3Bool Nc3Var::add_att(Nc3Token aname, ncbyte val)
{
  if (! the_file->define_mode())
    return FALSE;
  if (nc_put_att_schar (the_file->id(), the_id, aname, (nc_type) Nc3TypeEnum(ncbyte),
                        1, &val) != NC_NOERR)
    return FALSE;
  return TRUE;
}

Nc3Bool Nc3Var::add_att(Nc3Token aname, char val)
{
  if (! the_file->define_mode())
    return FALSE;
  if (nc_put_att_text (the_file->id(), the_id, aname,
                       1, &val) != NC_NOERR)
    return FALSE;
  return TRUE;
}

Nc3Bool Nc3Var::add_att(Nc3Token aname, short val)
{
  if (! the_file->define_mode()) {
    return FALSE;
  }
  if (Nc3Error::set_err
      (nc_put_att_short(the_file->id(), the_id, aname, NC_SHORT, 1, &val)) != NC_NOERR) {
    return FALSE;
  }
  return TRUE;
}

Nc3Bool Nc3Var::add_att(Nc3Token aname, int val)
{
  if (! the_file->define_mode()) {
    return FALSE;
  }
  if (Nc3Error::set_err
      (nc_put_att_int(the_file->id(), the_id, aname, NC_INT, 1, &val)) != NC_NOERR) {
    return FALSE;
  }
  return TRUE;
}

Nc3Bool Nc3Var::add_att(Nc3Token aname, long val)
{
  if (! the_file->define_mode()) {
    return FALSE;
  }
  if (Nc3Error::set_err
      (nc_put_att_long(the_file->id(), the_id, aname, NC_LONG, 1, &val)) != NC_NOERR) {
    return FALSE;
  }
  return TRUE;
}

Nc3Bool Nc3Var::add_att(Nc3Token aname, long long val)
{
  if (! the_file->define_mode()) {
    return FALSE;
  }
  if (Nc3Error::set_err
      (nc_put_att_longlong(the_file->id(), the_id, aname, NC_INT64, 1, &val)) != NC_NOERR) {
    return FALSE;
  }
  return TRUE;
}

Nc3Bool Nc3Var::add_att(Nc3Token aname, double val)
{
  if (! the_file->define_mode()) {
    return FALSE;
  }
  if (Nc3Error::set_err
      (nc_put_att_double(the_file->id(), the_id, aname, NC_DOUBLE, 1, &val)) != NC_NOERR) {
    return FALSE;
  }
  return TRUE;
}

// Nc3Var_add_scalar_att(short)
//   Nc3Var_add_scalar_att(int)
//   Nc3Var_add_scalar_att(long)
//   Nc3Var_add_scalar_att(double)

Nc3Bool Nc3Var::add_att(Nc3Token aname, float val)
{
  if (! the_file->define_mode())
    return FALSE;
  float fval = (float) val;	// workaround for bug, val passed as double??
  if (nc_put_att_float(the_file->id(), the_id, aname, (nc_type) nc3Float,
                       1, &fval) != NC_NOERR)
    return FALSE;
  return TRUE;
}

Nc3Bool Nc3Var::add_att(Nc3Token aname, const char* val)
{
  if (! the_file->define_mode())
    return FALSE;
  if (nc_put_att_text(the_file->id(), the_id, aname,
                      strlen(val), val) != NC_NOERR)
    return FALSE;
  return TRUE;
}

// OLD MACRO DEFINITION - now explicitly expanded in the code
// #define Nc3Var_add_vector_att(TYPE)                                  
//   Nc3Bool Nc3Var::add_att(Nc3Token aname, int len, const TYPE* vals) 
//   {                                                                  
//     if (! the_file->define_mode())                                   
//       return FALSE;                                                  
//     if (Nc3Error::set_err(                                           
//                 makename2(nc_put_att_,TYPE) (the_file->id(), the_id, aname, (nc_type) Nc3TypeEnum(TYPE), 
//                                              len, vals)              
//                                                                                ) != NC_NOERR) 
//       return FALSE;                                                  
//     return TRUE;                                                     
//   }

Nc3Bool Nc3Var::add_att(Nc3Token aname, int len, const ncbyte* vals)
{
  if (! the_file->define_mode())
    return FALSE;
  if (Nc3Error::set_err(
              nc_put_att_schar (the_file->id(), the_id, aname, (nc_type) Nc3TypeEnum(ncbyte),
                                len, vals)
                        ) != NC_NOERR)
    return FALSE;
  return TRUE;
}

Nc3Bool Nc3Var::add_att(Nc3Token aname, int len, const char* vals)
{
  if (! the_file->define_mode())
    return FALSE;
  if (Nc3Error::set_err(
              nc_put_att_text (the_file->id(), the_id, aname,
                               len, vals)
                        ) != NC_NOERR)
    return FALSE;
  return TRUE;
}

// Nc3Var_add_vector_att(short)
//   Nc3Var_add_vector_att(int)
//   Nc3Var_add_vector_att(long)
//   Nc3Var_add_vector_att(float)
//   Nc3Var_add_vector_att(double)

Nc3Bool Nc3Var::add_att(Nc3Token aname, int len, const short* vals) {
  if (! the_file->define_mode()) return 0;
  if (Nc3Error::set_err( nc_put_att_short (the_file->id(), the_id, aname,
                                           (nc_type) nc3Short, len, vals) ) != NC_NOERR) return 0;
  return 1;
}

Nc3Bool Nc3Var::add_att(Nc3Token aname, int len, const int* vals) {
  if (! the_file->define_mode()) return 0;
  if (Nc3Error::set_err( nc_put_att_int (the_file->id(), the_id, aname,
                                         (nc_type) nc3Int, len, vals) ) != NC_NOERR) return 0;
  return 1;
}

Nc3Bool Nc3Var::add_att(Nc3Token aname, int len, const long* vals) {
  if (! the_file->define_mode()) return 0;
  if (Nc3Error::set_err( nc_put_att_long (the_file->id(), the_id, aname,
                                          (nc_type) nc3Long, len, vals) ) != NC_NOERR) return 0;
  return 1;
}

Nc3Bool Nc3Var::add_att(Nc3Token aname, int len, const long long* vals) {
  if (! the_file->define_mode()) return 0;
  if (Nc3Error::set_err( nc_put_att_longlong (the_file->id(), the_id, aname,
                                              (nc_type) nc3Long, len, vals) ) != NC_NOERR) return 0;
  return 1;
}

Nc3Bool Nc3Var::add_att(Nc3Token aname, int len, const float* vals) {
  if (! the_file->define_mode()) return 0;
  if (Nc3Error::set_err( nc_put_att_float (the_file->id(), the_id, aname,
                                           (nc_type) nc3Float, len, vals) ) != NC_NOERR) return 0;
  return 1;
}

Nc3Bool Nc3Var::add_att(Nc3Token aname, int len, const double* vals) {
  if (! the_file->define_mode()) return 0;
  if (Nc3Error::set_err( nc_put_att_double (the_file->id(), the_id, aname,
                                            (nc_type) nc3Double, len, vals) ) != NC_NOERR)
    return 0;
  return 1;
}

Nc3Bool Nc3Var::rename(Nc3Token newname)
{
  if (strlen(newname) > strlen(the_name)) {
    if (! the_file->define_mode())
      return FALSE;
  }
  Nc3Bool ret = Nc3Error::set_err(
          nc_rename_var(the_file->id(), the_id, newname)
                                  ) == NC_NOERR;
  if (ret) {
    delete [] the_name;
    the_name = new char [1 + strlen(newname)];
    strcpy(the_name, newname);
  }
  return ret;
}

int Nc3Var::id( void ) const
{
  return the_id;
}

Nc3Bool Nc3Var::sync(void)
{
  if (the_name) {
    delete [] the_name;
  }
  if (the_cur) {
    delete [] the_cur;
  }
  if (cur_rec) {
    delete [] cur_rec;
  }
  char nam[NC_MAX_NAME];
  if (the_file 
      && Nc3Error::set_err(
              nc_inq_varname(the_file->id(), the_id, nam)
                           ) == NC_NOERR) {
    the_name = new char[1 + strlen(nam)];
    strcpy(the_name, nam);
  } else {
    the_name = 0;
    return FALSE;
  }
  init_cur(); 
  return TRUE;
}


Nc3Var::Nc3Var(Nc3File* nc, int id)
        : Nc3TypedComponent(nc), the_id(id)
{
  char nam[NC_MAX_NAME];
  memset(nam, 0, sizeof(nam));
  if (the_file 
      && Nc3Error::set_err(
              nc_inq_varname(the_file->id(), the_id, nam)
                           ) == NC_NOERR) {
    the_name = new char[1 + strlen(nam)];
    strcpy(the_name, nam);
  } else {
    the_name = 0;
  }
  init_cur();
}

int Nc3Var::attnum( Nc3Token attrname ) const
{
  int num;
  for(num=0; num < num_atts(); num++) {
    char aname[NC_MAX_NAME];
    memset(aname, 0, sizeof(aname));
    Nc3Error::set_err(
            nc_inq_attname(the_file->id(), the_id, num, aname)
                      );
    if (strcmp(aname, attrname) == 0)
      break;
  }
  return num;			// num_atts() if no such attribute
}

Nc3Token Nc3Var::attname( int attnum ) const // caller must delete[]
{
  if (attnum < 0 || attnum >= num_atts())
    return 0;
  char aname[NC_MAX_NAME];
  memset(aname, 0, sizeof(aname));
  if (Nc3Error::set_err(
              nc_inq_attname(the_file->id(), the_id, attnum, aname)
                        ) != NC_NOERR)
    return 0;
  char* rname = new char[1 + strlen(aname)];
  strcpy(rname, aname);
  return rname;
}

void Nc3Var::init_cur( void )
{
  the_cur = new long[NC_MAX_DIMS]; // *** don't know num_dims() yet?
  cur_rec = new long[NC_MAX_DIMS]; // *** don't know num_dims() yet?
  for(int i = 0; i < NC_MAX_DIMS; i++) { 
    the_cur[i] = 0; cur_rec[i] = 0; }
}

///////////////////////////////////////////////////////////////////
// Get compression parameters
// returns 0 on success, -1 on failure

int Nc3Var::get_compression_parameters(bool& shuffleFilterEnabled,
                                       bool& deflateFilterEnabled,
                                       int& deflateLevel) const
  
{

  int fileId = the_file->id();
  int varId = the_id;
  int shuffle, control, level;

  if (nc_inq_var_deflate(fileId, varId, &shuffle, &control, &level) == NC_NOERR) {
    shuffleFilterEnabled = static_cast<bool> (shuffle);
    deflateFilterEnabled = static_cast<bool> (control);
    deflateLevel = level;
    return 0;
  } else {
    shuffleFilterEnabled = 0;
    deflateFilterEnabled = 0;
    deflateLevel = 0;
    return -1;
  }

}

Nc3Att::Nc3Att(Nc3File* nc, const Nc3Var* var, Nc3Token name)
        : Nc3TypedComponent(nc), the_variable(var)
{
  the_name = new char[1 + strlen(name)];
  strcpy(the_name, name);
}

Nc3Att::Nc3Att(Nc3File* nc, Nc3Token name)
        : Nc3TypedComponent(nc), the_variable(NULL)
{
  the_name = new char[1 + strlen(name)];
  strcpy(the_name, name);
}

Nc3Att::~Nc3Att( void )
{
  delete [] the_name;
}

Nc3Token Nc3Att::name( void ) const
{
  return the_name;
}

Nc3Type Nc3Att::type( void ) const
{
  nc_type typ;
  Nc3Error::set_err(
          nc_inq_atttype(the_file->id(), the_variable->id(), the_name, &typ)
                    );
  return (Nc3Type) typ;
}

long Nc3Att::num_vals( void ) const
{
  size_t len;
  Nc3Error::set_err(
          nc_inq_attlen(the_file->id(), the_variable->id(), the_name, &len)
                    );
  return len;
}

Nc3Bool Nc3Att::is_valid( void ) const
{
  int num;
  return the_file->is_valid() &&
    (the_variable->id() == NC_GLOBAL || the_variable->is_valid()) &&
    Nc3Error::set_err(
            nc_inq_attid(the_file->id(), the_variable->id(), the_name, &num)
                      ) == NC_NOERR;
}

Nc3Values* Nc3Att::values( void ) const
{
  Nc3Values* valp = get_space();
  int status;
  switch (type()) {
    case nc3Float:
      status = Nc3Error::set_err(
              nc_get_att_float(the_file->id(), the_variable->id(), the_name,
                               (float *)valp->base())
                                 );
      break;
    case nc3Double:
      status = Nc3Error::set_err(
              nc_get_att_double(the_file->id(), the_variable->id(), the_name,
                                (double *)valp->base())
                                 );
      break;
    case nc3Int:
      status = Nc3Error::set_err(
              nc_get_att_int(the_file->id(), the_variable->id(), the_name,
                             (int *)valp->base())
                                 );
      break;
    case nc3Short:
      status = Nc3Error::set_err(
              nc_get_att_short(the_file->id(), the_variable->id(), the_name,
                               (short *)valp->base())
                                 );
      break;
    case nc3Byte:
      status = Nc3Error::set_err(
              nc_get_att_schar(the_file->id(), the_variable->id(), the_name,
                               (signed char *)valp->base())
                                 );
      break;
    case nc3Char:
      status = Nc3Error::set_err(
              nc_get_att_text(the_file->id(), the_variable->id(), the_name,
                              (char *)valp->base())
                                 );
      break;
    case nc3NoType:
    default:
      return 0;
  }
  if (status != NC_NOERR) {
    delete valp;
    return 0;
  }
  return valp;
}

Nc3Bool Nc3Att::rename(Nc3Token newname)
{
  if (strlen(newname) > strlen(the_name)) {
    if (! the_file->define_mode())
      return FALSE;
  }
  return Nc3Error::set_err(
          nc_rename_att(the_file->id(), the_variable->id(),
                        the_name, newname)
                           ) == NC_NOERR;
}

Nc3Bool Nc3Att::remove( void )
{
  if (! the_file->define_mode())
    return FALSE;
  return Nc3Error::set_err(
          nc_del_att(the_file->id(), the_variable->id(), the_name)
                           ) == NC_NOERR;
}

Nc3Error::Nc3Error( Behavior b )
{
  the_old_state = ncopts;	// global variable in version 2 C interface
  the_old_err = ncerr;	// global variable in version 2 C interface
  ncopts = (int) b;
}

Nc3Error::~Nc3Error( void )
{
  ncopts = the_old_state;
  ncerr = the_old_err;
}

int Nc3Error::get_err( void )	// returns most recent error
{
  return ncerr;
}

int Nc3Error::set_err (int err)
{
  ncerr = err;
  // Check ncopts and handle appropriately
  if(err != NC_NOERR) {
    if(ncopts == verbose_nonfatal || ncopts == verbose_fatal) {
      std::cout << nc_strerror(err) << std::endl;
    }
    if(ncopts == silent_fatal || ncopts == verbose_fatal) {
      exit(ncopts);
    }
  }
  return err;
}

int Nc3Error::ncerr = NC_NOERR;
int Nc3Error::ncopts = Nc3Error::verbose_fatal ; // for backward compatibility
