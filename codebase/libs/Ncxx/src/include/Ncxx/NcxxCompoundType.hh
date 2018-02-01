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
#include <vector>
#include <Ncxx/NcxxType.hh>
#include <netcdf.h>

#ifndef NcxxCompoundTypeClass
#define NcxxCompoundTypeClass


class NcxxGroup;  // forward declaration.

/*! 
  Class represents a netCDF compound type
*/
class NcxxCompoundType : public NcxxType
{
public:

  /*! Constructor generates a \ref isNull "null object". */
  NcxxCompoundType();

  /*! 
    Constructor.
    The compound Type must already exist in the netCDF file. New netCDF compound types can be 
    added using NcxxGroup::addNcxxCompoundType();
    \param grp        The parent group where this type is defined.
    \param name       Name of new type.
  */
  NcxxCompoundType(const NcxxGroup& grp, const std::string& name);

  /*! 
    Constructor.
    Constructs from the base type NcxxType object. Will throw an exception if the NcxxType is not the base of a Compound type.
    \param ncType     A Nctype object.
  */
  NcxxCompoundType(const NcxxType& ncType);

  /*! assignment operator */
  NcxxCompoundType& operator=(const NcxxCompoundType& rhs);
      
  /*! 
    Assignment operator.
    This assigns from the base type NcxxType object. Will throw an exception if the NcxxType is not the base of a Compound type.
  */
  NcxxCompoundType& operator=(const NcxxType& rhs);
      
  /*! The copy constructor. */
  NcxxCompoundType(const NcxxCompoundType& rhs);
      
  /*! equivalence operator */
  bool operator==(const NcxxCompoundType & rhs);

  /*! destructor */
  ~NcxxCompoundType(){;}
      
      
  /*!  
    Adds a named field.
    \param memName       Name of new field.
    \param newMemberType The type of the new member.
    \param offset        Offset of this member in bytes, obtained by a call to offsetof. For example 
    the offset of a member "mem4" in structure struct1 is: offsetof(struct1,mem4).
  */
  void addMember(const std::string& memName, const NcxxType& newMemberType,size_t offset);

  /*!  
    Adds a named array field.
    \param memName       Name of new field.
    \param newMemberType The type of the new member.
    \param offset        Offset of this member in bytes, obtained by a call to offsetof. For example 
    the offset of a member "mem4" in structure struct1 is: offsetof(struct1,mem4).
    \param shape         The shape of the array field.
  */
  void addMember(const std::string& memName, const NcxxType& newMemberType, size_t offset, const std::vector<int>& shape);


  /*! Returns number of members in this NcxxCompoundType object. */
  size_t  getMemberCount() const;
      
  /*! Returns a NcxxType object for a single member. */
  NcxxType getMember(int memberIndex) const;

  /*! Returns name of member field. */
  std::string getMemberName(int memberIndex) const;

  /*! Returns index of named member field. */
  int getMemberIndex(const std::string& memberName) const;

  /*! Returns the offset of the member with given index. */
  size_t getMemberOffset(const int index) const;

  /*! 
    Returns the number of dimensions of a member with the given index. 
    \param Index of member (numbering starts at zero).
    \return The number of dimensions of the field. Non-array fields have 0 dimensions.
  */
  int getMemberDimCount(int memberIndex) const;
      
      
  /*! 
    Returns the shape of a given member. 
    \param Index of member (numbering starts at zero).
    \return The size of the dimensions of the field. Non-array fields have 0 dimensions.
  */
  std::vector<int> getMemberShape(int memberIndex) const;
      
      
private:
      
  // int myOffset;
      
};
  

#endif
