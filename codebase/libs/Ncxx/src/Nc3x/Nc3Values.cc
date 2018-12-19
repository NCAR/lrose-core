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
 *   Purpose:	implementation of classes of typed arrays for netCDF
 *********************************************************************/

#include <iostream>
#include <string>
#include <cstring>
#include <Ncxx/Nc3Values.hh>

Nc3Values::Nc3Values( void ) : the_type(nc3NoType), the_number(0)
{}

Nc3Values::Nc3Values(Nc3Type type, long num)
	: the_type(type), the_number(num)
{}

Nc3Values::~Nc3Values( void )
{}

long Nc3Values::num( void )
{
    return the_number;
}    

std::ostream& operator<< (std::ostream& os, const Nc3Values& vals)
{
    return vals.print(os);
}

implement(Nc3Values,ncbyte)
implement(Nc3Values,char)
implement(Nc3Values,short)
implement(Nc3Values,int)
implement(Nc3Values,nclong)
implement(Nc3Values,long)
implement(Nc3Values,float)
implement(Nc3Values,double)

Ncbytes_for_one_implement(ncbyte)
Ncbytes_for_one_implement(char)
Ncbytes_for_one_implement(short)
Ncbytes_for_one_implement(int)
Ncbytes_for_one_implement(nclong)
Ncbytes_for_one_implement(long)
Ncbytes_for_one_implement(float)
Ncbytes_for_one_implement(double)

as_ncbyte_implement(short)
as_ncbyte_implement(int)
as_ncbyte_implement(nclong)
as_ncbyte_implement(long)
as_ncbyte_implement(int64_t)
as_ncbyte_implement(float)
as_ncbyte_implement(double)

inline ncbyte Nc3Values_char::as_ncbyte( long n ) const
{
    return the_values[n];
}

inline ncbyte Nc3Values_ncbyte::as_ncbyte( long n ) const
{
    return the_values[n];
}

as_char_implement(short)
as_char_implement(int)
as_char_implement(nclong)
as_char_implement(long)
as_char_implement(int64_t)
as_char_implement(float)
as_char_implement(double)

inline char Nc3Values_ncbyte::as_char( long n ) const
{
    return the_values[n] > CHAR_MAX ? nc3Bad_char : (char) the_values[n];
}

inline char Nc3Values_char::as_char( long n ) const
{
    return the_values[n];
}

as_short_implement(int)
as_short_implement(nclong)
as_short_implement(long)
as_short_implement(int64_t)
as_short_implement(float)
as_short_implement(double)

inline short Nc3Values_ncbyte::as_short( long n ) const
{
    return the_values[n];
}

inline short Nc3Values_char::as_short( long n ) const
{
    return the_values[n];
}

inline short Nc3Values_short::as_short( long n ) const
{
    return the_values[n];
}


as_int_implement(float)
as_int_implement(double)

inline int Nc3Values_ncbyte::as_int( long n ) const
{
    return the_values[n];
}

inline int Nc3Values_char::as_int( long n ) const
{
    return the_values[n];
}

inline int Nc3Values_short::as_int( long n ) const
{
    return the_values[n];
}

inline int Nc3Values_int::as_int( long n ) const
{
    return the_values[n];
}

inline int Nc3Values_nclong::as_int( long n ) const
{
    return the_values[n];
}

inline int Nc3Values_long::as_int( long n ) const
{
    return the_values[n];
}

inline int Nc3Values_int64_t::as_int( long n ) const
{
    return the_values[n];
}

as_nclong_implement(float)
as_nclong_implement(double)

inline nclong Nc3Values_ncbyte::as_nclong( long n ) const
{
    return the_values[n];
}

inline nclong Nc3Values_char::as_nclong( long n ) const
{
    return the_values[n];
}

inline nclong Nc3Values_short::as_nclong( long n ) const
{
    return the_values[n];
}

inline nclong Nc3Values_int::as_nclong( long n ) const
{
    return the_values[n];
}

inline nclong Nc3Values_nclong::as_nclong( long n ) const
{
    return the_values[n];
}

inline nclong Nc3Values_long::as_nclong( long n ) const
{
    return the_values[n];
}

inline nclong Nc3Values_int64_t::as_nclong( long n ) const
{
    return the_values[n];
}

as_long_implement(float)
as_long_implement(double)

inline long Nc3Values_ncbyte::as_long( long n ) const
{
    return the_values[n];
}

inline long Nc3Values_char::as_long( long n ) const
{
    return the_values[n];
}

inline long Nc3Values_short::as_long( long n ) const
{
    return the_values[n];
}

inline long Nc3Values_int::as_long( long n ) const
{
    return the_values[n];
}

inline long Nc3Values_nclong::as_long( long n ) const
{
    return the_values[n];
}

inline long Nc3Values_long::as_long( long n ) const
{
    return the_values[n];
}

inline long Nc3Values_int64_t::as_long( long n ) const
{
    return the_values[n];
}


as_int64_implement(float)
as_int64_implement(double)

inline int64_t Nc3Values_ncbyte::as_int64( long n ) const
{
    return the_values[n];
}

inline int64_t Nc3Values_char::as_int64( long n ) const
{
    return the_values[n];
}

inline int64_t Nc3Values_short::as_int64( long n ) const
{
    return the_values[n];
}

inline int64_t Nc3Values_int::as_int64( long n ) const
{
    return the_values[n];
}

inline int64_t Nc3Values_nclong::as_int64( long n ) const
{
    return the_values[n];
}

inline int64_t Nc3Values_long::as_int64( long n ) const
{
    return the_values[n];
}

inline int64_t Nc3Values_int64_t::as_int64( long n ) const
{
    return the_values[n];
}

as_float_implement(ncbyte)
as_float_implement(char)
as_float_implement(short)
as_float_implement(int)
as_float_implement(nclong)
as_float_implement(long)
as_float_implement(int64_t)
as_float_implement(float)
as_float_implement(double)

as_double_implement(ncbyte)
as_double_implement(char)
as_double_implement(short)
as_double_implement(int)
as_double_implement(nclong)
as_double_implement(long)
as_double_implement(int64_t)
as_double_implement(float)
as_double_implement(double)

as_string_implement(short)
as_string_implement(int)
as_string_implement(nclong)
as_string_implement(long)
as_string_implement(int64_t)
as_string_implement(float)
as_string_implement(double)

inline char* Nc3Values_ncbyte::as_string( long n ) const
{
    char* s = new char[the_number + 1];
    s[the_number] = '\0';
    strncpy(s, (const char*)the_values + n, (int)the_number);
    return s;
}

inline char* Nc3Values_char::as_string( long n ) const
{
    char* s = new char[the_number + 1];
    s[the_number] = '\0';
    strncpy(s, (const char*)the_values + n, (int)the_number);
    return s;
}

std::ostream& Nc3Values_short::print(std::ostream& os) const
{
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1] ;
    return os;
}

std::ostream& Nc3Values_int::print(std::ostream& os) const
{
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1] ;
    return os;
}

std::ostream& Nc3Values_nclong::print(std::ostream& os) const
{
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1] ;
    return os;
}

std::ostream& Nc3Values_long::print(std::ostream& os) const
{
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1] ;
    return os;
}

std::ostream& Nc3Values_ncbyte::print(std::ostream& os) const
{
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1] ;
    return os;
}

std::ostream& Nc3Values_char::print(std::ostream& os) const
{
    os << '"';
    long len = the_number;
    while (the_values[--len] == '\0') // don't output trailing null bytes
	;
    for(int i = 0; i <= len; i++)
	os << the_values[i] ;
    os << '"';
    
    return os;
}

std::ostream& Nc3Values_float::print(std::ostream& os) const
{
    std::streamsize save=os.precision();
    os.precision(7);
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1] ;
    os.precision(save);
    return os;
}

std::ostream& Nc3Values_double::print(std::ostream& os) const
{
    std::streamsize save=os.precision();
    os.precision(15);
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1];
    os.precision(save);
    return os;
}
