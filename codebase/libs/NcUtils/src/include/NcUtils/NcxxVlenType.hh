#include <string>
#include <NcUtils/NcxxType.hh>
#include <netcdf.h>

#ifndef NcxxVlenTypeClass
#define NcxxVlenTypeClass


namespace netCDF
{
  class NcxxGroup;  // forward declaration.

  /*! Class represents a netCDF VLEN type */
  class NcxxVlenType : public NcxxType
  {
  public:

    /*! Constructor generates a \ref isNull "null object". */
    NcxxVlenType();

    /*! 
      Constructor.
      The vlen Type must already exist in the netCDF file. New netCDF vlen types can be added 
      using NcxxGroup::addNcxxVlenType();
      \param grp        The parent group where this type is defined.
      \param name       Name of new type.
    */
    NcxxVlenType(const NcxxGroup& grp, const std::string& name);

    /*! 
      Constructor.
      Constructs from the base type NcxxType object. Will throw an exception if the NcxxType is not the base of a Vlen type.
      \param ncType     A Nctype object.
    */
    NcxxVlenType(const NcxxType& ncType);

    /*! assignment operator */
    NcxxVlenType& operator=(const NcxxVlenType& rhs);
      
    /*! 
      Assignment operator.
      This assigns from the base type NcxxType object. Will throw an exception if the NcxxType is not the base of a Vlen type.
    */
    NcxxVlenType& operator=(const NcxxType& rhs);
      
    /*! The copy constructor. */
    NcxxVlenType(const NcxxVlenType& rhs);
      
    ~NcxxVlenType(){;}
      
    /*! Returns the base type. */
    NcxxType  getBaseType() const;
      
  };
  
}

#endif
