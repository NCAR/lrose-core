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
 *   Purpose:	interface for classes of typed arrays for netCDF
 *********************************************************************/

#ifndef Nc3Values_hh
#define Nc3Values_hh

#include <iostream>
#include <sstream>
#include <limits.h>
#include <netcdf.h>
#include <sys/types.h>

// Documentation warned this might change and now it has, for
// consistency with C interface 
typedef signed char ncbyte;

#define NC3_UNSPECIFIED ((nc_type)0)

// C++ interface dates from before netcdf-3, still uses some netcdf-2 names
#ifdef NO_NETCDF_2
#define NC3_LONG NC_INT
#define NC3_FILL_LONG NC_FILL_INT
typedef int nclong;
#define	NC3_FATAL	1
#define	NC3_VERBOSE	2
#endif

enum Nc3Type 
  {
    nc3NoType = NC3_UNSPECIFIED, 
    nc3Byte = NC_BYTE, 
    nc3Char = NC_CHAR, 
    nc3Short = NC_SHORT, 
    nc3Int = NC_INT,
    nc3Long = NC_LONG,		// deprecated, someday want to use for 64-bit ints
    nc3Float = NC_FLOAT, 
    nc3Double = NC_DOUBLE,
    nc3Int64 = NC_INT64
  };

#define nc3Bad_ncbyte nc3Bad_byte
static const ncbyte nc3Bad_byte = NC_FILL_BYTE;
static const char nc3Bad_char = NC_FILL_CHAR;
static const short nc3Bad_short = NC_FILL_SHORT;
static const nclong nc3Bad_nclong = FILL_LONG; // deprecated
static const int nc3Bad_int = NC_FILL_INT;
static const long nc3Bad_long = FILL_LONG; // deprecated
static const float nc3Bad_float = NC_FILL_FLOAT;
static const double nc3Bad_double = NC_FILL_DOUBLE;

// macros to glue tokens together to form new names (used to be in generic.h)
#define name2(a,b) a ## b
#define declare(clas,t)        name2(clas,declare)(t)
#define implement(clas,t)      name2(clas,implement)(t)
// This is the same as the name2 macro, but we need to define our own
// version since rescanning something generated with the name2 macro
// won't necessarily cause name2 to be expanded again.
#define makename2(z, y)		makename2_x(z, y)
#define makename2_x(z, y)		z##y

#define Nc3Val(TYPE) makename2(Nc3Values_,TYPE)

#define Nc3Valuesdeclare(TYPE)                                  \
  class Nc3Val(TYPE) : public Nc3Values                         \
  {                                                             \
  public:                                                       \
    Nc3Val(TYPE)( void );                                       \
    Nc3Val(TYPE)(long num);                                     \
    Nc3Val(TYPE)(long num, const TYPE* vals);                   \
    Nc3Val(TYPE)(const Nc3Val(TYPE)&);                          \
    virtual Nc3Val(TYPE)& operator=(const Nc3Val(TYPE)&);       \
    virtual ~Nc3Val(TYPE)( void );                              \
    virtual void* base( void ) const;                           \
    virtual int bytes_for_one( void ) const;                    \
    virtual ncbyte as_ncbyte( long n ) const;                   \
    virtual char as_char( long n ) const;                       \
    virtual short as_short( long n ) const;                     \
    virtual int as_int( long n ) const;                         \
    virtual int as_nclong( long n ) const;                      \
    virtual long as_long( long n ) const;                       \
    virtual int64_t as_int64( long n ) const;                   \
    virtual float as_float( long n ) const;                     \
    virtual double as_double( long n ) const;                   \
    virtual char* as_string( long n ) const;                    \
    virtual int invalid( void ) const;                          \
  private:                                                      \
    TYPE* the_values;                                           \
    std::ostream& print(std::ostream&) const;                   \
  };

#define Nc3TypeEnum(TYPE) makename2(_nc__,TYPE)
#define _nc__ncbyte nc3Byte
#define _nc__char nc3Char
#define _nc__short nc3Short
#define _nc__int nc3Int
#define _nc__nclong nc3Long
#define _nc__long nc3Long
#define _nc__float nc3Float
#define _nc__double nc3Double
#define Nc3Valuesimplement(TYPE)                                        \
  Nc3Val(TYPE)::Nc3Val(TYPE)( void )                                    \
               : Nc3Values(Nc3TypeEnum(TYPE), 0), the_values(0)         \
  {}                                                                    \
                                                                        \
    Nc3Val(TYPE)::Nc3Val(TYPE)(long num, const TYPE* vals)              \
                 : Nc3Values(Nc3TypeEnum(TYPE), num)                    \
    {                                                                   \
     the_values = new TYPE[num];                                        \
     for(int i = 0; i < num; i++)                                       \
       the_values[i] = vals[i];                                         \
     }                                                                  \
                                                                        \
      Nc3Val(TYPE)::Nc3Val(TYPE)(long num)                              \
                   : Nc3Values(Nc3TypeEnum(TYPE), num), the_values(new TYPE[num]) \
      {}                                                                \
                                                                        \
        Nc3Val(TYPE)::Nc3Val(TYPE)(const Nc3Val(TYPE)& v) :             \
                     Nc3Values(v)                                       \
        {                                                               \
         delete[] the_values;                                           \
         the_values = new TYPE[v.the_number];                           \
         for(int i = 0; i < v.the_number; i++)                          \
           the_values[i] = v.the_values[i];                             \
         }                                                              \
                                                                        \
          Nc3Val(TYPE)& Nc3Val(TYPE)::operator=(const Nc3Val(TYPE)& v)  \
          {                                                             \
           if ( &v != this) {                                           \
                             Nc3Values::operator=(v);                   \
                             delete[] the_values;                       \
                             the_values = new TYPE[v.the_number];       \
                             for(int i = 0; i < v.the_number; i++)      \
                               the_values[i] = v.the_values[i];         \
                             }                                          \
           return *this;                                                \
           }                                                            \
                                                                        \
            void* Nc3Val(TYPE)::base( void ) const                      \
            {                                                           \
             return the_values;                                         \
             }                                                          \
                                                                        \
              Nc3Val(TYPE)::~Nc3Val(TYPE)( void )                       \
              {                                                         \
               delete[] the_values;                                     \
               }                                                        \
                                                                        \
                int Nc3Val(TYPE)::invalid( void ) const                 \
                {                                                       \
                 for(int i=0;i<the_number;i++)                          \
                   if (the_values[i] == makename2(nc3Bad_,TYPE)) return 1; \
                 return 0;                                              \
                 }                                                      \


#define Ncbytes_for_one_implement(TYPE)         \
  int Nc3Val(TYPE)::bytes_for_one( void ) const \
  {                                             \
    return sizeof(TYPE);                        \
  }

#define as_ncbyte_implement(TYPE)                       \
  ncbyte Nc3Val(TYPE)::as_ncbyte( long n ) const        \
  {                                                     \
    if (the_values[n] < 0 || the_values[n] > UCHAR_MAX) \
      return nc3Bad_byte;                               \
    return (ncbyte) the_values[n];                      \
  }

#define as_char_implement(TYPE)                                 \
  char Nc3Val(TYPE)::as_char( long n ) const                    \
  {                                                             \
    if (the_values[n] < CHAR_MIN || the_values[n] > CHAR_MAX)   \
      return nc3Bad_char;                                       \
    return (char) the_values[n];                                \
  }

#define as_short_implement(TYPE)                                \
  short Nc3Val(TYPE)::as_short( long n ) const                  \
  {                                                             \
    if (the_values[n] < SHRT_MIN || the_values[n] > SHRT_MAX)   \
      return nc3Bad_short;                                      \
    return (short) the_values[n];                               \
  }

#define NCINT_MIN INT_MIN
#define NCINT_MAX INT_MAX
#define as_int_implement(TYPE)                                  \
  int Nc3Val(TYPE)::as_int( long n ) const                      \
  {                                                             \
    if (the_values[n] < NCINT_MIN || the_values[n] > NCINT_MAX) \
      return nc3Bad_int;                                        \
    return (int) the_values[n];                                 \
  }

#define NCINT64_MIN LLONG_MIN
#define NCINT64_MAX LLONG_MAX
#define as_int64_implement(TYPE)                                      \
  int64_t Nc3Val(TYPE)::as_int64( long n ) const                      \
  {                                                                   \
    if (the_values[n] < NCINT64_MIN || the_values[n] > NCINT64_MAX)   \
      return nc3Bad_int;                                              \
    return (int64_t) the_values[n];                                   \
  }

#define NCLONG_MIN INT_MIN
#define NCLONG_MAX INT_MAX
#define as_nclong_implement(TYPE)                                       \
  nclong Nc3Val(TYPE)::as_nclong( long n ) const                        \
  {                                                                     \
    if (the_values[n] < NCLONG_MIN || the_values[n] > NCLONG_MAX)       \
      return nc3Bad_nclong;                                             \
    return (nclong) the_values[n];                                      \
  }

#define as_long_implement(TYPE)                                 \
  long Nc3Val(TYPE)::as_long( long n ) const                    \
  {                                                             \
    if (the_values[n] < LONG_MIN || the_values[n] > LONG_MAX)   \
      return nc3Bad_long;                                       \
    return (long) the_values[n];                                \
  }

#define as_float_implement(TYPE)                        \
  inline float Nc3Val(TYPE)::as_float( long n ) const   \
  {                                                     \
    return (float) the_values[n];                       \
  }

#define as_double_implement(TYPE)                       \
  inline double Nc3Val(TYPE)::as_double( long n ) const \
  {                                                     \
    return (double) the_values[n];                      \
  }

#define as_string_implement(TYPE)               \
  char* Nc3Val(TYPE)::as_string( long n ) const \
  {                                             \
    char* s = new char[32];                     \
    std::ostringstream ostr;                    \
    ostr << the_values[n];                      \
    ostr.str().copy(s, std::string::npos);      \
    s[ostr.str().length()] = 0;                 \
    return s;                                   \
  }

class Nc3Values			// ABC for value blocks
{
public:
  Nc3Values( void );
  Nc3Values(Nc3Type, long);
  virtual ~Nc3Values( void );
  virtual long num( void );
  virtual std::ostream& print(std::ostream&) const = 0;
  virtual void* base( void ) const = 0;
  virtual int bytes_for_one( void ) const = 0;

  // The following member functions provide conversions from the value
  // type to a desired basic type.  If the value is out of range, the
  // default "fill-value" for the appropriate type is returned.
  virtual ncbyte as_ncbyte( long n ) const = 0; // nth value as a byte
  virtual char as_char( long n ) const = 0;     // nth value as char
  virtual short as_short( long n ) const = 0;   // nth value as short
  virtual int    as_int( long n ) const = 0;    // nth value as int
  virtual int64_t as_int64( long n ) const = 0;    // nth value as int64
  virtual int    as_nclong( long n ) const = 0; // nth value as nclong
  virtual long as_long( long n ) const = 0;     // nth value as long
  virtual float as_float( long n ) const = 0;   // nth value as floating-point
  virtual double as_double( long n ) const = 0; // nth value as double
  virtual char* as_string( long n ) const = 0;  // value as string
    
protected:
  Nc3Type the_type;
  long the_number;
  friend std::ostream& operator<< (std::ostream&, const Nc3Values&);
};

declare(Nc3Values,ncbyte)
  declare(Nc3Values,char)
  declare(Nc3Values,short)
  declare(Nc3Values,int)
  declare(Nc3Values,nclong)
  declare(Nc3Values,long)
  declare(Nc3Values,int64_t)
  declare(Nc3Values,float)
  declare(Nc3Values,double)

#endif
