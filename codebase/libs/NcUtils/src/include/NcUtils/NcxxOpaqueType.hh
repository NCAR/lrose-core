#include <string>
#include <NcUtils/NcxxType.hh>
#include <netcdf.h>

#ifndef NcxxOpaqueTypeClass
#define NcxxOpaqueTypeClass


namespace netCDF
{
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
  
}

#endif
