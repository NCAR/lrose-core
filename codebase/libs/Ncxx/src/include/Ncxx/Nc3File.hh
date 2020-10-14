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

#ifndef Nc3File_HH
#define Nc3File_HH

#include <Ncxx/Nc3Values.hh>    // arrays that know their element type
#include <sys/types.h>

typedef const char* Nc3Token;    // names for netCDF objects
typedef unsigned int Nc3Bool;    // many members return 0 on failure

class Nc3Dim;                    // dimensions
class Nc3Var;                    // variables
class Nc3Att;                    // attributes

/*
 * ***********************************************************************
 * A netCDF file.
 * ***********************************************************************
 */
class Nc3File
{
public:

  virtual ~Nc3File( void );

  enum FileMode {
    ReadOnly,	// file exists, open read-only
    Write,		// file exists, open for writing
    Replace,	// create new file, even if already exists
    New		// create new file, fail if already exists
  };

  enum FileFormat {
    Classic,         // netCDF classic format (i.e. version 1 format)
    Offset64Bits,    // netCDF 64-bit offset format
    Netcdf4,		// netCDF-4 using HDF5 format
    Netcdf4Classic,	// netCDF-4 using HDF5 format using only netCDF-3 calls
    BadFormat
  };

  Nc3File( const char * path, FileMode = ReadOnly ,
           size_t *bufrsizeptr = NULL,    // optional tuning parameters
           size_t initialsize = 0,
           FileFormat = Classic );

  Nc3Bool is_valid( void ) const;         // opened OK in ctr, still valid

  int num_dims( void ) const;            // number of dimensions
  int num_vars( void ) const;            // number of variables
  int num_atts( void ) const;            // number of (global) attributes

  Nc3Dim* get_dim( Nc3Token ) const;       // dimension by name
  Nc3Var* get_var( Nc3Token ) const;       // variable by name
  Nc3Att* get_att( Nc3Token ) const;       // global attribute by name

  Nc3Dim* get_dim( int ) const;           // n-th dimension
  Nc3Var* get_var( int ) const;           // n-th variable
  Nc3Att* get_att( int ) const;           // n-th global attribute
  Nc3Dim* rec_dim( void ) const;          // unlimited dimension, if any
    
  // Add new dimensions, variables, global attributes.
  // These put the file in "define" mode, so could be expensive.
  virtual Nc3Dim* add_dim( Nc3Token dimname, long dimsize );
  virtual Nc3Dim* add_dim( Nc3Token dimname );     // unlimited

  virtual Nc3Var* add_var( Nc3Token varname, Nc3Type type,       // scalar
                           const Nc3Dim* dim0=0,                // 1-dim
                           const Nc3Dim* dim1=0,                // 2-dim
                           const Nc3Dim* dim2=0,                // 3-dim
                           const Nc3Dim* dim3=0,                // 4-dim
                           const Nc3Dim* dim4=0 );              // 5-dim
  virtual Nc3Var* add_var( Nc3Token varname, Nc3Type type,       // n-dim
                           int ndims, const Nc3Dim** dims );

  Nc3Bool add_att( Nc3Token attname, char );             // scalar attributes
  Nc3Bool add_att( Nc3Token attname, ncbyte );
  Nc3Bool add_att( Nc3Token attname, short );
  Nc3Bool add_att( Nc3Token attname, long );
  Nc3Bool add_att( Nc3Token attname, int );
  Nc3Bool add_att( Nc3Token attname, long long );
  Nc3Bool add_att( Nc3Token attname, float );
  Nc3Bool add_att( Nc3Token attname, double );
  Nc3Bool add_att( Nc3Token attname, const char*);       // string attribute
  Nc3Bool add_att( Nc3Token attname, int, const char* ); // vector attributes
  Nc3Bool add_att( Nc3Token attname, int, const ncbyte* );
  Nc3Bool add_att( Nc3Token attname, int, const short* );
  Nc3Bool add_att( Nc3Token attname, int, const long* );
  Nc3Bool add_att( Nc3Token attname, int, const int* );
  Nc3Bool add_att( Nc3Token attname, int, const long long* );
  Nc3Bool add_att( Nc3Token attname, int, const float* );
  Nc3Bool add_att( Nc3Token attname, int, const double* );

  enum FillMode {
    Fill = NC_FILL,                    // prefill (default)
    NoFill = NC_NOFILL,                // don't prefill
    Bad
  };

  Nc3Bool set_fill( FillMode = Fill );    // set fill-mode
  FillMode get_fill( void ) const;       // get fill-mode
  FileFormat get_format( void ) const;   // get format version

  Nc3Bool sync( void );                   // synchronize to disk
  Nc3Bool close( void );                  // to close earlier than dtr
  Nc3Bool abort( void );                  // back out of bad defines
    
  // Needed by other Nc classes, but users will not need them
  Nc3Bool define_mode( void ); // leaves in define mode, if possible
  Nc3Bool data_mode( void );   // leaves in data mode, if possible
  int id( void ) const;       // id used by C interface

protected:
  int the_id;
  int in_define_mode;
  FillMode the_fill_mode;
  Nc3Dim** dimensions;
  Nc3Var** variables;
  Nc3Var* globalv;             // "variable" for global attributes
};

/*
 * For backward compatibility.  We used to derive NcOldFile and NcNewFile
 * from Nc3File, but that was over-zealous inheritance.
 */
#define NcOldFile Nc3File
#define NcNewFile Nc3File
#define Clobber Replace
#define NoClobber New

/*
 * **********************************************************************
 * A netCDF dimension, with a name and a size.  These are only created
 * by Nc3File member functions, because they cannot exist independently
 * of an open netCDF file.
 * **********************************************************************
 */
class Nc3Dim
{
public:
  Nc3Token name( void ) const;
  long size( void ) const;
  Nc3Bool is_valid( void ) const;
  Nc3Bool is_unlimited( void ) const;
  Nc3Bool rename( Nc3Token newname );
  int id( void ) const;
  Nc3Bool sync( void );

private:
  Nc3File *the_file;		// not const because of rename
  int the_id;
  char *the_name;

  Nc3Dim(Nc3File*, int num);	// existing dimension
  Nc3Dim(Nc3File*, Nc3Token name, long sz); // defines a new dim
  virtual ~Nc3Dim( void );

  // to construct dimensions, since constructor is private
  friend class Nc3File;
};


/*
 * **********************************************************************
 * Abstract base class for a netCDF variable or attribute, both of which
 * have a name, a type, and associated values.  These only exist as
 * components of an open netCDF file.
 * **********************************************************************
 */
class Nc3TypedComponent
{
public:
  virtual ~Nc3TypedComponent( void ) {}
  virtual Nc3Token name( void ) const = 0;
  virtual Nc3Type type( void ) const = 0;
  virtual Nc3Bool is_valid( void ) const = 0;
  virtual long num_vals( void ) const = 0; 
  virtual Nc3Bool rename( Nc3Token newname ) = 0;
  virtual Nc3Values* values( void ) const = 0; // block of all values

  // The following member functions provide conversions from the value
  // type to a desired basic type.  If the value is out of range,
  // the default "fill-value" for the appropriate type is returned.

  virtual ncbyte as_ncbyte( long n ) const;    // nth value as an unsgnd char
  virtual char as_char( long n ) const;        // nth value as char
  virtual short as_short( long n ) const;      // nth value as short
  virtual int as_int( long n ) const;	       // nth value as int
  virtual int64_t as_int64( long n ) const;    // nth value as int64
  virtual int as_nclong( long n ) const;       // nth value as nclong (deprecated)
  virtual long as_long( long n ) const;        // nth value as long
  virtual float as_float( long n ) const;      // nth value as floating-point
  virtual double as_double( long n ) const;    // nth value as double
  virtual char* as_string( long n ) const;     // nth value as string

protected:
  Nc3File *the_file;
  Nc3TypedComponent( Nc3File* );
  virtual Nc3Values* get_space( long numVals = 0 ) const;  // to hold values
};


/*
 * **********************************************************************
 * netCDF variables.  In addition to a name and a type, these also have
 * a shape, given by a list of dimensions
 * **********************************************************************
 */
class Nc3Var : public Nc3TypedComponent
{
public:
  virtual ~Nc3Var( void );
  Nc3Token name( void ) const;
  Nc3Type type( void ) const;
  Nc3Bool is_valid( void ) const;
  int num_dims( void ) const;         // dimensionality of variable
  Nc3Dim* get_dim( int ) const;        // n-th dimension
  long* edges( void ) const;          // dimension sizes
  int num_atts( void ) const;         // number of attributes
  Nc3Att* get_att( Nc3Token ) const;    // attribute by name
  Nc3Att* get_att( int ) const;        // n-th attribute
  long num_vals( void ) const;        // product of dimension sizes
  Nc3Values* values( void ) const;     // all values
    
  // Put scalar or 1, ..., 5 dimensional arrays by providing enough
  // arguments.  Arguments are edge lengths, and their number must not
  // exceed variable's dimensionality.  Start corner is [0,0,..., 0] by
  // default, but may be reset using the set_cur() member.  FALSE is
  // returned if type of values does not match type for variable.
  Nc3Bool put( const ncbyte* vals,
               long c0=0, long c1=0, long c2=0, long c3=0, long c4=0 );
  Nc3Bool put( const char* vals,
               long c0=0, long c1=0, long c2=0, long c3=0, long c4=0 );
  Nc3Bool put( const short* vals,
               long c0=0, long c1=0, long c2=0, long c3=0, long c4=0 );
  Nc3Bool put( const int* vals,
               long c0=0, long c1=0, long c2=0, long c3=0, long c4=0 );
  Nc3Bool put( const long* vals,
               long c0=0, long c1=0, long c2=0, long c3=0, long c4=0 );
  Nc3Bool put( const float* vals,
               long c0=0, long c1=0, long c2=0, long c3=0, long c4=0 );
  Nc3Bool put( const double* vals,
               long c0=0, long c1=0, long c2=0, long c3=0, long c4=0 );

  // Put n-dimensional arrays, starting at [0, 0, ..., 0] by default,
  // may be reset with set_cur().
  Nc3Bool put( const ncbyte* vals, const long* counts );
  Nc3Bool put( const char* vals, const long* counts );
  Nc3Bool put( const short* vals, const long* counts );
  Nc3Bool put( const int* vals, const long* counts );
  Nc3Bool put( const long* vals, const long* counts );
  Nc3Bool put( const float* vals, const long* counts );
  Nc3Bool put( const double* vals, const long* counts );

  // Get scalar or 1, ..., 5 dimensional arrays by providing enough
  // arguments.  Arguments are edge lengths, and their number must not
  // exceed variable's dimensionality.  Start corner is [0,0,..., 0] by
  // default, but may be reset using the set_cur() member.
  Nc3Bool get( ncbyte* vals, long c0=0, long c1=0,
               long c2=0, long c3=0, long c4=0 ) const;
  Nc3Bool get( char* vals, long c0=0, long c1=0,
               long c2=0, long c3=0, long c4=0 ) const;
  Nc3Bool get( short* vals, long c0=0, long c1=0,
               long c2=0, long c3=0, long c4=0 ) const;
  Nc3Bool get( int* vals, long c0=0, long c1=0,
               long c2=0, long c3=0, long c4=0 ) const;
  Nc3Bool get( long* vals, long c0=0, long c1=0,
               long c2=0, long c3=0, long c4=0 ) const;
  Nc3Bool get( float* vals, long c0=0, long c1=0,
               long c2=0, long c3=0, long c4=0 ) const;
  Nc3Bool get( double* vals, long c0=0, long c1=0,
               long c2=0, long c3=0, long c4=0 ) const; 

  // Get n-dimensional arrays, starting at [0, 0, ..., 0] by default,
  // may be reset with set_cur().
  Nc3Bool get( ncbyte* vals, const long* counts ) const;
  Nc3Bool get( char* vals, const long* counts ) const;
  Nc3Bool get( short* vals, const long* counts ) const;
  Nc3Bool get( int* vals, const long* counts ) const;
  Nc3Bool get( long* vals, const long* counts ) const;
  Nc3Bool get( float* vals, const long* counts ) const;
  Nc3Bool get( double* vals, const long* counts ) const;

  Nc3Bool set_cur(long c0=-1, long c1=-1, long c2=-1,
                  long c3=-1, long c4=-1);
  Nc3Bool set_cur(long* cur);

  // these put file in define mode, so could be expensive
  Nc3Bool add_att( Nc3Token, char );             // add scalar attributes
  Nc3Bool add_att( Nc3Token, ncbyte );
  Nc3Bool add_att( Nc3Token, short );
  Nc3Bool add_att( Nc3Token, int );
  Nc3Bool add_att( Nc3Token, long );
  Nc3Bool add_att( Nc3Token, long long);
  Nc3Bool add_att( Nc3Token, float );
  Nc3Bool add_att( Nc3Token, double );
  Nc3Bool add_att( Nc3Token, const char* );      // string attribute
  Nc3Bool add_att( Nc3Token, int, const char* ); // vector attributes
  Nc3Bool add_att( Nc3Token, int, const ncbyte* );
  Nc3Bool add_att( Nc3Token, int, const short* );
  Nc3Bool add_att( Nc3Token, int, const int* );
  Nc3Bool add_att( Nc3Token, int, const long long* );
  Nc3Bool add_att( Nc3Token, int, const long* );
  Nc3Bool add_att( Nc3Token, int, const float* );
  Nc3Bool add_att( Nc3Token, int, const double* );

  Nc3Bool rename( Nc3Token newname );

  long rec_size ( void );             // number of values per record
  long rec_size ( Nc3Dim* );           // number of values per dimension slice

  // Though following are intended for record variables, they also work
  // for other variables, using first dimension as record dimension.

  // Get a record's worth of data
  Nc3Values *get_rec(void);	        // get current record
  Nc3Values *get_rec(long rec);        // get specified record
  Nc3Values *get_rec(Nc3Dim* d);        // get current dimension slice
  Nc3Values *get_rec(Nc3Dim* d, long slice); // get specified dimension slice

  // Put a record's worth of data in current record
  Nc3Bool put_rec( const ncbyte* vals );
  Nc3Bool put_rec( const char* vals );
  Nc3Bool put_rec( const short* vals );
  Nc3Bool put_rec( const int* vals );
  Nc3Bool put_rec( const long* vals );
  Nc3Bool put_rec( const float* vals );
  Nc3Bool put_rec( const double* vals );

  // Put a dimension slice worth of data in current dimension slice
  Nc3Bool put_rec( Nc3Dim* d, const ncbyte* vals );
  Nc3Bool put_rec( Nc3Dim* d, const char* vals );
  Nc3Bool put_rec( Nc3Dim* d, const short* vals );
  Nc3Bool put_rec( Nc3Dim* d, const int* vals );
  Nc3Bool put_rec( Nc3Dim* d, const long* vals );
  Nc3Bool put_rec( Nc3Dim* d, const float* vals );
  Nc3Bool put_rec( Nc3Dim* d, const double* vals );

  // Put a record's worth of data in specified record
  Nc3Bool put_rec( const ncbyte* vals, long rec );
  Nc3Bool put_rec( const char* vals, long rec );
  Nc3Bool put_rec( const short* vals, long rec );
  Nc3Bool put_rec( const int* vals, long rec );
  Nc3Bool put_rec( const long* vals, long rec );
  Nc3Bool put_rec( const float* vals, long rec );
  Nc3Bool put_rec( const double* vals, long rec );

  // Put a dimension slice worth of data in specified dimension slice
  Nc3Bool put_rec( Nc3Dim* d, const ncbyte* vals, long slice );
  Nc3Bool put_rec( Nc3Dim* d, const char* vals, long slice );
  Nc3Bool put_rec( Nc3Dim* d, const short* vals, long slice );
  Nc3Bool put_rec( Nc3Dim* d, const int* vals, long slice );
  Nc3Bool put_rec( Nc3Dim* d, const long* vals, long slice );
  Nc3Bool put_rec( Nc3Dim* d, const float* vals, long slice );
  Nc3Bool put_rec( Nc3Dim* d, const double* vals, long slice );

  // Get first record index corresponding to specified key value(s)
  long get_index( const ncbyte* vals );
  long get_index( const char* vals );
  long get_index( const short* vals );
  long get_index( const int* vals );
  long get_index( const long* vals );
  long get_index( const float* vals );
  long get_index( const double* vals );

  // Get first index of specified dimension corresponding to key values
  long get_index( Nc3Dim* d, const ncbyte* vals );
  long get_index( Nc3Dim* d, const char* vals );
  long get_index( Nc3Dim* d, const short* vals );
  long get_index( Nc3Dim* d, const int* vals );
  long get_index( Nc3Dim* d, const long* vals );
  long get_index( Nc3Dim* d, const float* vals );
  long get_index( Nc3Dim* d, const double* vals );

  // Set current record
  void set_rec ( long rec );
  // Set current dimension slice
  void set_rec ( Nc3Dim* d, long slice );

  int id( void ) const;               // rarely needed, C interface id
  Nc3Bool sync( void );
    
  int get_compression_parameters(bool& shuffleFilterEnabled,
                                 bool& deflateFilterEnabled,
                                 int& deflateLevel) const;
  

private:
  int dim_to_index(Nc3Dim* rdim);
  int the_id;
  long* the_cur;
  char* the_name;
  long* cur_rec;

  // private constructors because only an Nc3File creates these
  Nc3Var( void );
  Nc3Var(Nc3File*, int);

  int attnum( Nc3Token attname ) const;
  Nc3Token attname( int attnum ) const;
  void init_cur( void );

  // to make variables, since constructor is private
  friend class Nc3File;
};


/*
 * **********************************************************************
 * netCDF attributes.  In addition to a name and a type, these are each
 * associated with a specific variable, or are global to the file.
 * **********************************************************************
 */
class Nc3Att : public Nc3TypedComponent
{
public:          
  virtual ~Nc3Att( void );
  Nc3Token name( void ) const;
  Nc3Type type( void ) const;
  Nc3Bool is_valid( void ) const;
  long num_vals( void ) const; 
  Nc3Values* values( void ) const;
  Nc3Bool rename( Nc3Token newname );
  Nc3Bool remove( void );

private:
  const Nc3Var* the_variable;
  char* the_name;
  // protected constructors because only Nc3Vars and Nc3Files create
  // attributes
  Nc3Att( Nc3File*, const Nc3Var*, Nc3Token);
  Nc3Att( Nc3File*, Nc3Token); // global attribute
    
  // To make attributes, since constructor is private
  friend class Nc3File;
  friend Nc3Att* Nc3Var::get_att( Nc3Token ) const;
};


/*
 * **********************************************************************
 * To control error handling.  Declaring an Nc3Error object temporarily
 * changes the error-handling behavior until the object is destroyed, at
 * which time the previous error-handling behavior is restored.
 * **********************************************************************
 */
class Nc3Error {
public:
  enum Behavior {
    silent_nonfatal = 0,
    silent_fatal = 1,
    verbose_nonfatal = 2,
    verbose_fatal = 3   
  };

  // constructor saves previous error state, sets new state
  Nc3Error( Behavior b = verbose_fatal );

  // destructor restores previous error state
  virtual ~Nc3Error( void );

  int get_err( void );                 // returns most recent error number
  const char* get_errmsg( void ) {return nc_strerror(get_err());}
  static int set_err( int err );

private:
  int the_old_state;
  int the_old_err;
  static int ncopts;
  static int ncerr;
};

#endif                          /* NETCDF_HH */
