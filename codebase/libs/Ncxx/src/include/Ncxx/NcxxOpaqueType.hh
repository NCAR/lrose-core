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
//  Ncxx C++ classes for NetCDF4
//
//  Copied from code by:
//
//    Lynton Appel, of the Culham Centre for Fusion Energy (CCFE)
//    in Oxfordshire, UK.
//    The netCDF-4 C++ API was developed for use in managing
//    fusion research data from CCFE's innovative MAST
//    (Mega Amp Spherical Tokamak) experiment.
// 
//  Offical NetCDF codebase is at:
//
//    https://github.com/Unidata/netcdf-cxx4
//
//  Modification for LROSE made by:
//
//    Mike Dixon, EOL, NCAR
//    P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//  The base code makes extensive use of exceptions.
//
//  December 2016
//
//////////////////////////////////////////////////////////////////////

#include <string>
#include <Ncxx/NcxxType.hh>
#include <netcdf.h>

#ifndef NcxxOpaqueTypeClass
#define NcxxOpaqueTypeClass


class NcxxGroup;  // forward declaration.

/*! Class represents a netCDF opaque type */
class NcxxOpaqueType : public NcxxType
{
public:
  
  /*! Constructor generates a \ref isNull "null object". */
  NcxxOpaqueType();
  
  /*! 
    Constructor.
    The opaque Type must already exist in the netCDF file. New netCDF opaque types #
    can be added using NcxxGroup::addNcxxOpaqueType();
    \param grp        The parent group where this type is defined.
    \param name       Name of new type.
  */
  NcxxOpaqueType(const NcxxGroup& grp, const std::string& name);
  
  /*! 
    Constructor.
    Constructs from the base type NcxxType object. Will throw an exception if the NcxxType is not the base of a Opaque type.
    \param ncType     A Nctype object.
  */
  NcxxOpaqueType(const NcxxType& ncType);
  
  /*! assignment operator */
  NcxxOpaqueType& operator=(const NcxxOpaqueType& rhs);
  
  /*! 
    Assignment operator.
    This assigns from the base type NcxxType object. Will throw an exception if the NcxxType is not the base of an Opaque type.
    */
  NcxxOpaqueType& operator=(const NcxxType& rhs);
  
  /*! The copy constructor.*/
  NcxxOpaqueType(const NcxxOpaqueType& rhs);
  
  /*!  destructor */
  ~NcxxOpaqueType(){;}
  
  /*! Returns the size of the opaque type in bytes. */
  size_t  getTypeSize() const;
  
};
  
#endif
