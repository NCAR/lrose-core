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
#include <set>
#include <map>
#include <Ncxx/NcxxType.hh>
#include <Ncxx/NcxxEnumType.hh>
#include <Ncxx/NcxxGroupAtt.hh>
#include <Ncxx/NcxxErrStr.hh>



#ifndef NcxxGroupClass
#define NcxxGroupClass


// static int file_id;

class NcxxVar;          // forward declaration.
class NcxxDim;          // forward declaration.
class NcxxVlenType;     // forward declaration.
class NcxxCompoundType; // forward declaration.
class NcxxOpaqueType;   // forward declaration.

/*! Class represents a netCDF group. */
class NcxxGroup : public NcxxErrStr
{

public:

  /*!
    The enumeration list contains the options for selecting groups (used for returned set of NcxxGroup objects).
  */
  enum GroupLocation
    {
      ChildrenGrps,              //!< Select from the set of children in the current group.
      ParentsGrps,               //!< Select from set of parent groups (excludes the current group).
      ChildrenOfChildrenGrps,    //!< Select from set of all children of children in the current group.
      AllChildrenGrps,           //!< Select from set of all children of the current group and beneath.
      ParentsAndCurrentGrps,     //!< Select from set of parent groups(includes the current group).
      AllGrps                    //!< Select from set of parent groups, current groups and all the children beneath.
    };

  /*!
    The enumeration list contains the options for selecting groups.
  */
  enum Location
    {
      Current,            //!< Select from contents of current group.
      Parents,            //!< Select from contents of parents groups.
      Children,           //!< Select from contents of children groups.
      ParentsAndCurrent,  //!< Select from contents of current and parents groups.
      ChildrenAndCurrent, //!< Select from contents of current and child groups.
      All                 //!< Select from contents of current, parents and child groups.
    };


  /*! assignment operator  */
  NcxxGroup& operator=(const NcxxGroup& rhs);

  /*! Constructor generates a \ref isNull "null object". */
  NcxxGroup();

  //* constructor */
  NcxxGroup(int groupId);

  /*! The copy constructor. */
  NcxxGroup(const NcxxGroup& rhs);

  /*! destructor  */
  virtual ~NcxxGroup();

  /*! equivalence operator */
  bool operator==(const NcxxGroup& rhs) const;

  /*!  != operator */
  bool operator!=(const NcxxGroup& rhs) const;

  /*! comparator operator  */
  friend bool operator<(const NcxxGroup& lhs,const NcxxGroup& rhs);

  /*! comparator operator  */
  friend bool operator>(const NcxxGroup& lhs,const NcxxGroup& rhs);

  // /////////////
  // NcxxGroup-related methods
  // /////////////

  /*! Gets the group name. */
  /*!
    Method will throw an NcNullgrp exception if the group is null (ie
    NcxxGroup::isNull()=true).
    \param fullName
    If true then the full name is returned with subgroups separated by a forward
    slash "/" (default is false)
    \return
    The group name.
  */
  std::string getName(bool fullName=false) const;

  /*!
    Gets the parent group.  Method will throw an NcNullgrp exception if the
    group is null (ie NcxxGroup::isNull()=true).  If the current root is the
    parent group, then return a null group.
  */
  NcxxGroup getParentGroup() const ;

  /*!
    Gets the group id.
    Method will throw an NcNullgrp exception if the group is null (ie
    NcxxGroup::isNull()=true).
  */
  int  getId() const;

  /*!
    Gets the number of  NcxxGroup objects.
    Method will throw an NcNullgrp exception if the group is null (ie NcxxGroup::isNull()=true).
    \param location Enumeration type controlling the groups to search.
    \return         Number of groups.
  */
  int getGroupCount(NcxxGroup::GroupLocation location = ChildrenGrps) const;

  /*!
    Gets the collection of NcxxGroup objects.
    Method will throw an NcNullgrp exception if the group is null (ie NcxxGroup::isNull()=true).
    \param location Enumeration type controlling the groups to search.
    \return         A STL multimap object, containing pairs of <attribute name, NcxxGroup object> entities.
  */
  std::multimap<std::string,NcxxGroup>
    getGroups(NcxxGroup::GroupLocation location = ChildrenGrps) const;


  /*!
    Gets NcxxGroup objects with a given name.
    Method will throw an NcNullgrp exception if the group is null (ie NcxxGroup::isNull()=true).
    \param name     Name of group.
    \param location Enumeration type controlling the groups to search.
    \return         Set of NcxxGroup objects with given name.
  */
  std::set<NcxxGroup>
    getGroups(const std::string& name,
              NcxxGroup::GroupLocation location = ChildrenGrps) const;

  /*!
    Gets the named child NcxxGroup object.
    Method will throw an NcNullgrp exception if the group is null (ie NcxxGroup::isNull()=true).
    \param name  Group name.
    \param location   Enumeration type controlling the groups to search.
    \return      An NcxxGroup object. If there are multiple objects indentied with the same name,
    the object closest to the current group is returned. If no valid object is found ,
    a \ref NcxxGroup::isNull "null node" is returned.
  */
  NcxxGroup getGroup(const std::string& name,
                     NcxxGroup::GroupLocation location = ChildrenGrps) const;

  /*!
    Adds a new child netCDF group object.
    Method will throw an NcNullgrp exception if the group is null (ie NcxxGroup::isNull()=true).
    \param   name     Variable name.
    \return  NcxxGroup  The NcxxGroup object for this new netCDF group.
  */
  NcxxGroup addGroup(const std::string& name) const;


  /*! Returns true if this object is null (i.e. it has no contents); otherwise returns false. */
  bool isNull() const  {return nullObject;}

  /*! Returns true if this is the root group, otherwise returns false. */
  bool isRootGroup() const;

  // /////////////
  // NcxxVar-related accessors
  // /////////////

  /*!
    Gets the number of NcxxVar objects in this group.
    \param location Enumeration type controlling the groups to search.
    \return         Number of variables.
  */
  int getVarCount(NcxxGroup::Location location = Current) const;

  /*!
    Get the collection of NcxxVar objects.
    \param location Enumeration type controlling the groups to search.
    \return         A STL multimap object, containing pairs of <attribute name, NcxxVar object> entities.
  */
  std::multimap<std::string,NcxxVar>
    getVars(NcxxGroup::Location location = Current) const;

  /*!
    Gets all NcxxVar objects with a given name.
    \param name     Name of attribute
    \param location Enumeration type controlling the groups to search.
    \return         Set of NcxxVar objects.
  */
  std::set<NcxxVar> 
    getVars(const std::string& name,
            NcxxGroup::Location location = Current) const;

  /*!
    Gets the named NcxxVar object..
    \param name     Variable name.
    \param location Enumeration type controlling the groups to search.
    \return         A NcxxVar object. If there are multiple objects indentied with the
    same name, the object closest  to the current group is returned.
    If no valid object is found , a \ref NcxxVar::isNull "null node" is returned.
  */
  NcxxVar getVar(const std::string& name,
                 NcxxGroup::Location location = Current) const;

  /*!
    Adds a new netCDF scalar variable.
    The NcxxType must be non-null, and be defined in either the current group or a parent group.
    An NcNullType exception is thrown if the NcxxType object is invalid.
    \param    name     Variable name.
    \param   typeName  Type name.
    \return            The NcxxVar object for this new netCDF variable.
  */
  NcxxVar addVar(const std::string& name,
                 const NcxxType& ncType) const;

  /*!
    Adds a new netCDF variable.
    The NcxxType and NcxxDim objects must be non-null, and be defined in either the current group or a parent group.
    An NcNullType exception is thrown if the NcxxType object is invalid.
    An NcNullDim exception is thrown if the NcxxDim object is invalid.
    \param    name     Variable name.
    \param   typeName  Type name.
    \param   dimName   Dimension name.
    \return            The NcxxVar object for this new netCDF variable.
  */
  NcxxVar addVar(const std::string& name,
                 const std::string& typeName,
                 const std::string& dimName) const;

  /*!
    Adds a new netCDF variable.
    The NcxxType and NcxxDim objects must be non-null, and be defined in either the current group or a parent group.
    An NcNullType exception is thrown if the NcxxType object is invalid.
    An NcNullDim exception is thrown if the NcxxDim object is invalid.
    \param    name      Variable name.
    \param    ncType    NcxxType object.
    \param    ncDim     NcxxDim object.
    \return             The NcxxVar object for this new netCDF variable.
  */
  NcxxVar addVar(const std::string& name,
                 const NcxxType& ncType,
                 const NcxxDim& ncDim) const;

  /*!
    Adds a new netCDF multi-dimensional variable.
    The NcxxType and NcxxDim objects must be non-null, and be defined in either the current group or a parent group.
    An NcNullType exception is thrown if the NcxxType object is invalid.
    An NcNullDim exception is thrown if the NcxxDim object is invalid.
    \param   name     Variable name.
    \param   typeName Type name.
    \param   dimNames Vector of dimension names.
    \return           The NcxxVar object for this new netCDF variable.
  */
  NcxxVar addVar(const std::string& name,
                 const std::string& typeName,
                 const std::vector<std::string>& dimNames) const;


  /*!
    Adds a new multi-dimensional netCDF variable.
    The NcxxType and NcxxDim objects must be non-null, and be defined in either the current group or a parent group.
    An NcNullType exception is thrown if the NcxxType object is invalid.
    An NcNullDim exception is thrown if any of the the NcxxDim objects are invalid.
    \param    name        Variable name.
    \param    ncType      NcxxType object.
    \param    ncDimvector Vector of NcxxDim objects.
    \return               The NcxxVar object for this new netCDF variable.
  */
  NcxxVar addVar(const std::string& name,
                 const NcxxType& ncType,
                 const std::vector<NcxxDim>& ncDimVector) const;

  // /////////////
  // NcxxGroupAtt-related methods
  // /////////////

  /*!
    Gets the number of group attributes.
    \param location Enumeration type controlling the groups to search.
    \return         Number of attributes.
  */
  int getAttCount(NcxxGroup::Location location = Current) const;

  /*!
    Gets the collection of NcxxGroupAtt objects.
    \param location Enumeration type controlling the groups to search.
    \return         A STL multimap object, containing pairs of <attribute name, NcxxGroupAtt object> entities.
  */
  std::multimap<std::string, NcxxGroupAtt>
    getAtts(NcxxGroup::Location location = Current) const;

  /*!
    Gets all NcxxGroupAtt objects with a given name.
    \param name     Name of attribute
    \param location Enumeration type controlling the groups to search.
    \return         Set of NcxxGroupAtt objects.
  */
  std::set<NcxxGroupAtt>
    getAtts(const std::string& name,
            NcxxGroup::Location location = Current) const;

  /*!
    Gets the named NcxxGroupAtt object.
    \param name     Name of attribute
    \param location Enumeration type controlling the groups to search.
    \return         A NcxxGroupAtt object. If there are multiple objects indentied with the
    same name, the object closest  to the current group is returned.  If no valid object is found ,
    a \ref NcxxGroupAtt::isNull "null node" is returned.
  */
  NcxxGroupAtt getAtt(const std::string& name,
                      NcxxGroup::Location location = Current) const;


  /*!

    Creates a new NetCDF group attribute or if already exisiting replaces it.
    If you are writing a _Fill_Value_ attribute, and will tell the HDF5 layer to
    use the specified fill value for that variable.

    Although it's possible to create attributes of all types, text and double
    attributes are adequate for most purposes.

    \param name Name of attribute.
    \param type The attribute type.
    \param len  The length of the attribute (number of Nctype repeats).

    \param dataValues
    Data Values to put into the new attribute.  If
    the type of data values differs from the netCDF variable type, type
    conversion will occur.  (However, no type conversion is carried out for
    variables using the user-defined data types: nc_Vlen, nc_Opaque, nc_Compound
    and nc_Enum.)

    \return The NcxxGroupAtt object for this new netCDF attribute.

  */

  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name, 
                      size_t len,
                      const char** dataValues) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name, 
                      const std::string& dataValues) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      short datumValue) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      int datumValue) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      long datumValue) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      float datumValue) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      double datumValue) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      unsigned short datumValue) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      unsigned int datumValue) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      unsigned long long datumValue) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      long long datumValue) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      size_t len,
                      const unsigned char* dataValues) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      size_t len,
                      const signed char* dataValues) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      size_t len,
                      const short* dataValues) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      size_t len,
                      const int* dataValues) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      size_t len, 
                      const long* dataValues) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      size_t len,
                      const float* dataValues) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      size_t len,
                      const double* dataValues) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      size_t len,
                      const unsigned short* dataValues) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      size_t len,
                      const unsigned int* dataValues) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      size_t len,
                      const unsigned long long* dataValues) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      size_t len,
                      const long long* dataValues) const ;
  /*! \overload
   */
  NcxxGroupAtt putAtt(const std::string& name,
                      const NcxxType& type,
                      size_t len,
                      const void* dataValues) const ;



  // /////////////
  // NcxxDim-related methods
  // /////////////

  /*!
    Gets the number of NcxxDim objects.
    \param location Enumeration type controlling the groups to search.
    \return         Number of dimensions.
  */
  int getDimCount(NcxxGroup::Location location = Current) const;

  /*!
    Gets the collection of NcxxDim objects.
    \param location Enumeration type controlling the groups to search.
    \return         A STL multimap object, containing pairs of <attribute name, NcxxDim object> entities.
  */
  std::multimap<std::string,NcxxDim>
    getDims(NcxxGroup::Location location = Current) const;

  /*!
    Gets NcxxDim objects with a given name.
    \param name     Name of dimension.
    \param location Enumeration type controlling the groups to search.
    \return         Set of NcxxDim objects with given name.
  */
  std::set<NcxxDim>
    getDims(const std::string& name,
            NcxxGroup::Location location = Current) const;

  /*!
    Gets the named NcxxDim object.
    \param name       Name of dimension.
    \param location   Enumeration type controlling the groups to search.
    \return           An NcxxDim object. If there are multiple objects indentied with the same name,
    the object closest to the current group is returned. If no valid object is found , a \ref NcxxDim::isNull "null node" is returned.
  */
  NcxxDim getDim(const std::string&
                 name,NcxxGroup::Location location = Current) const;

  /*!
    Adds a new netCDF dimension.
    \param The name of new dimension.
    \param Length of dimension; that is, number of values for this dimension as an index to variables
    that use it.
    \return   The NcxxDim object for this new netCDF dimension.
  */
  NcxxDim addDim(const std::string& name,
                 size_t dimSize) const;

  /*!
    Adds a new unlimited netCDF dimension.
    \param The name of new dimension.
    \return   The NcxxDim object for this new netCDF dimension.
  */
  NcxxDim addDim(const std::string& name) const;

  // /////////////
  // NcxxType-related methods
  // /////////////

  /*!
    Gets the number of type objects.
    \param location Enumeration type controlling the groups to search.
    \return         Number of types.
  */
  int getTypeCount(NcxxGroup::Location location = Current) const;


  /*!
    Gets the number of type objects with a given enumeration type.
    \param enumType The enumeration value of the object type.
    \param location Enumeration type controlling the groups to search.
    \return         Number of types of the given enumeration type.
  */
  int getTypeCount(NcxxType::ncxxType enumType,
                   NcxxGroup::Location location = Current) const;


  /*!
    Gets the collection of NcxxType objects.
    \param location Enumeration type controlling the groups to search.
    \return         A STL multimap object, on return contains pairs of <Type name, NcxxType object> entities.
    For atomic types, the type returned is the CDL name.
  */
  std::multimap<std::string,NcxxType> 
    getTypes(NcxxGroup::Location location = Current) const;


  /*!
    Gets the collection of NcxxType objects with a given name.
    \param name     Name of type. For atomic types, the CDL name is expected. This is consistent with the
    string returned from NcxxType::getName().
    \param location Enumeration type controlling the groups to search.
    \return         Set of  NcxxType objects.
  */
  std::set<NcxxType> getTypes(const std::string& name,
                              NcxxGroup::Location location = Current) const;

  /*!
    Gets the collection of NcxxType objects with a given data type.
    \param enumType Enumeration type specifying the data type.
    \param location Enumeration type controlling the groups to search.
    \return         Set of Nctype objects.
  */
  std::set<NcxxType> getTypes(NcxxType::ncxxType enumType,
                              NcxxGroup::Location location = Current) const;


  /*!
    Gets the collection of NcxxType objects with a given name and data type.
    \param name     Name of type. For atomic types, the CDL name is expected. This is consistent with the
    string returned from NcxxType::getName().
    \param enumType Enumeration type specifying the data type.
    \param location Enumeration type controlling the groups to search.
    \return         Set of Nctype objects.
  */
  std::set<NcxxType> getTypes(const std::string& name,
                              NcxxType::ncxxType enumType,
                              NcxxGroup::Location location = Current) const;


  /*!
    Gets the NcxxType object with a given name.
    \param name     Name of type. For atomic types, the CDL name is expected. This is consistent with the
    string returned from NcxxType::getName().
    \param location Enumeration type controlling the groups to search.
    \return         NcxxType object. If there are multiple objects indentied with the same name,
    the object closest to the current group is returned.  If no valid object is found , a \ref NcxxType::isNull "null node" is returned.

  */
  NcxxType getType(const std::string& name,
                   NcxxGroup::Location location = Current) const;


  /*!
    Adds a new netCDF enum type.
    \param name        Name of type. For atomic types, the CDL name is expected. This is consistent with the
    string returned from NcxxType::getName().
    \param enumType    The enumeration value of the object type.
    \return            The NcxxEnumType object for this new netCDF enum type.
  */
  NcxxEnumType addEnumType(const std::string& name,
                           NcxxEnumType::ncEnumType basetype) const;


  /*!
    Adds a new netCDF Vlen type.
    \param name        Name of type.
    \param basetype    A NcxxType object to be used for the basetype.
    \return            The NcxxVlenType object for this new netCDF vlen type.
  */
  NcxxVlenType addVlenType(const std::string& name,
                           NcxxType& basetype) const;


  /*!
    Adds a new netCDF Opaque type.
    \param name     Name of type.
    \param size     The size of the new type in bytes.
    \return         The NcxxOpaqueType object for this new netCDF opaque type..
  */
  NcxxOpaqueType addOpaqueType(const std::string& name, 
                               size_t size) const;


  /*!
    Adds a new netCDF UserDefined type.
    \param name     Name of type.
    \param size     The size of the new type in bytes.
    \return         The new NcxxCompoundType object for this new netCDF userDefined type.
  */

  NcxxCompoundType addCompoundType(const std::string& name,
                                   size_t size) const;


  /*!
    Gets a collection of  coordinate variables.
    Coordinate variable have  an NcxxDim and NcxxVar object with the same name defined in the same group.
    \par
    The method returns STL map object containing a coordinate variables in the current group  and optionally
    in the parent and child groups. It is expected that within each group, the names of dimensions are unique and
    the the names of variables are unique. However, if this is not the case, this method will still work correctly.

    \param location Enumeration type controlling the groups to search.
    \return         The NcxxVar dimension variable. If no valid object is found , a \ref NcxxVar::isNull "null node" is returned.
  */
  std::map<std::string,NcxxGroup>
    getCoordVars(NcxxGroup::Location location = Current) const;

  /*!
    Gets the NcxxDim and NcxxVar object pair for a named coordinate variable.
    Coordinate variable have  an NcxxDim and NcxxVar object with the same name defined in the same group.
    \par
    The method returns two objects for the named coordinate variable. The method searches first in the current
    group and optionally in the parent and child group and returns the first instance found.
    \param location Enumeration type controlling the groups to search.
    \return         The set of names of dimension variables.
  */
  void getCoordVar(std::string& coordVarName,
                   NcxxDim& ncDim,
                   NcxxVar& ncVar,
                   NcxxGroup::Location location = Current) const;

  ///////////////////////////////////////////
  // add string global attribute
  // Throws NcxxException on failure
     
  void addGlobAttr(const string &name, const string &val);

  ///////////////////////////////////////////
  // add int global attribute
  // Throws NcxxException on failure
     
  void addGlobAttr(const string &name, int val);

  ///////////////////////////////////////////
  // add float global attribute
  // Throws NcxxException on failure
     
  void addGlobAttr(const string &name, float val);

  ///////////////////////////////////////////
  // add double global attribute
  // Throws NcxxException on failure
     
  void addGlobAttr(const string &name, double val);

  ///////////////////////////////////////////
  // read a global attribute
  // Throws NcxxException on failure
     
  void readGlobAttr(const string &name, string &val);
  void readGlobAttr(const string &name, int &val);
  void readGlobAttr(const string &name, float &val);
  void readGlobAttr(const string &name, double &val);

  //////////////////////////////////////////////
  // When adding variables, you have the option
  // to use the 'proposed_standard_name' attribute
  // instead of 'standard_name'.

  void setUsedProposedStandardName(bool val) {
    _useProposedStandardName = val;
  }

  //////////////////////////////////////////////
  // Add scalar var
  // Throws NcxxException on failure
  // Side effect: var is set
     
  NcxxVar addVar(const string &name, 
                 const string &standardName,
                 const string &longName,
                 NcxxType ncType, 
                 const string &units = "",
                 bool isMetadata = false);

  ///////////////////////////////////////
  // Add 1-D array var
  // Throws NcxxException on failure
  // Side effect: var is set
     
  NcxxVar addVar(const string &name, 
                 const string &standardName,
                 const string &longName,
                 NcxxType ncType, 
                 NcxxDim &dim, 
                 const string &units = "",
                 bool isMetadata = false);
  
  ///////////////////////////////////////
  // Add 2-D array var
  // Throws NcxxException on failure
  // Side effect: var is set
     
  NcxxVar addVar(const string &name,
                 const string &standardName,
                 const string &longName,
                 NcxxType ncType,
                 NcxxDim &dim0,
                 NcxxDim &dim1,
                 const string &units = "",
                 bool isMetadata = false);

  ///////////////////////////////////////
  // Add 3-D array var
  // Throws NcxxException on failure
  // Side effect: var is set
     
  NcxxVar addVar(const string &name,
                 const string &standardName,
                 const string &longName,
                 NcxxType ncType,
                 NcxxDim &dim0,
                 NcxxDim &dim1,
                 NcxxDim &dim2,
                 const string &units = "",
                 bool isMetadata = false);

  ///////////////////////////////////////
  // Add var in multiple-dimensions
  // Throws NcxxException on failure
  // Side effect: var is set
     
  NcxxVar addVar(const string &name,
                 const string &standardName,
                 const string &longName,
                 NcxxType ncType,
                 vector<NcxxDim> &dims,
                 const string &units = "",
                 bool isMetadata = false);
     
  ///////////////////////////////////////////////
  // read variable based on type, set and val
  // returns var used
  // throws NcxxException on failure
  
  NcxxVar readIntVar(const string &name,
                     int &val,
                     int missingVal,
                     bool required = true);
  
  NcxxVar readFloatVar(const string &name,
                       float &val,
                       float missingVal,
                       bool required = true);
  
  NcxxVar readDoubleVar(const string &name,
                        double &val,
                        double missingVal,
                        bool required = true);
      
  ///////////////////////////////////
  // read a scalar char string variable
  // throws NcxxException on failure
  // returns var used
    
  NcxxVar readCharStringVar(const string &name,
                            string &val);

  ///////////////////////////////////
  // read a scalar char string variable
  // throws NcxxException on failure
  // returns var used

  NcxxVar readScalarStringVar(const string &name,
                              string &val);

protected:

  /*! assignment operator  */
  /* NcxxGroup& operator=(const NcxxGroup& rhs); */

  bool nullObject;

  int myId;

  // option to use the 'proposed_standard_name' attribute instead
  // of 'standard_name'.

  bool _useProposedStandardName;

};

#endif
