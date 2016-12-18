#include <string>
#include <NcUtils/NcxxType.hh>
#include <netcdf.h>
#include <NcUtils/NcxxCheck.hh>

#ifndef NcxxEnumTypeClass
#define NcxxEnumTypeClass


namespace netCDF
{
  class NcxxGroup;  // forward declaration.

  /*! Class represents a netCDF enum type */
  class NcxxEnumType : public NcxxType
    {
    public:
      
      /*! List of NetCDF-4 Enumeration types.*/
      enum ncEnumType	{
	nc_BYTE     = NC_BYTE, 	//!< signed 1 byte integer
	nc_SHORT    = NC_SHORT, 	//!< signed 2 byte integer
	nc_INT      = NC_INT,	//!< signed 4 byte integer
	nc_UBYTE    = NC_UBYTE,	//!< unsigned 1 byte int
	nc_USHORT   = NC_USHORT,	//!< unsigned 2-byte int
	nc_UINT     = NC_UINT,	//!< unsigned 4-byte int
	nc_INT64    = NC_INT64,	//!< signed 8-byte int
	nc_UINT64   = NC_UINT64	//!< unsigned 8-byte int
      };
      
      /*! Constructor generates a \ref isNull "null object". */
      NcxxEnumType();

      /*! 
	Constructor.
	The enum Type must already exist in the netCDF file. New netCDF enum types can 
	be added using NcxxGroup::addNcxxEnumType();
	\param grp        The parent group where this type is defined.
	\param name       Name of new type.
      */
      NcxxEnumType(const NcxxGroup& grp, const std::string& name);

      /*! 
	Constructor.
	Constructs from the base type NcxxType object. Will throw an exception if the NcxxType is not the base of an Enum type.
	\param ncType     A Nctype object.
      */
      NcxxEnumType(const NcxxType& ncType);

      /*! assignment operator */
      NcxxEnumType& operator=(const NcxxEnumType& rhs);
      
      /*! 
	Assignment operator.
       This assigns from the base type NcxxType object. Will throw an exception if the NcxxType is not the base of an Enum type.
      */
      NcxxEnumType& operator=(const NcxxType& rhs);
      
      /*! The copy constructor. */
      NcxxEnumType(const NcxxEnumType& rhs);
      
      /*! Destructor */
      ~NcxxEnumType(){}
      
      
      /*! 
	Adds a new member to this NcxxEnumType type.
	\param name         Name for this new Enum memebr.
	\param memberValue  Member value, must be of the correct NcxxType.
      */
      template <class T> void addMember(const std::string& name, T memberValue)
      {
	ncxxCheck(nc_insert_enum(groupId, myId, name.c_str(), (void*) &memberValue),__FILE__,__LINE__);
      }

      /*! Returns number of members in this NcxxEnumType object. */
      size_t  getMemberCount() const;
      
      /*! Returns the member name for the given zero-based index. */
      std::string  getMemberNameFromIndex(int index) const;

      /*! Returns the member name for the given NcxxEnumType value. */
      template <class T>  std::string  getMemberNameFromValue(const T memberValue) const {
	char charName[NC_MAX_NAME+1];
	ncxxCheck(nc_inq_enum_ident(groupId,myId,static_cast<long long>(memberValue),charName),__FILE__,__LINE__);
	return std::string(charName);
      }
	
      /*! 
	Returns the value of a member with the given zero-based index.
	\param name         Name for this new Enum member.
	\param memberValue  Member value, returned by this routine.
      */
      template <class T> void getMemberValue(int index, T& memberValue) const
	{
	  char* charName=NULL;
	  ncxxCheck(nc_inq_enum_member(groupId,myId,index,charName,&memberValue),__FILE__,__LINE__);
	}

      /*! Returns the base type. */
      NcxxType  getBaseType() const;
      
  };
  
}

#endif
