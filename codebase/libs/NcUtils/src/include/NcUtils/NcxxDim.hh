#include <string>
#include <netcdf.h>

#ifndef NcxxDimClass
#define NcxxDimClass


namespace netCDF
{
  class NcxxGroup;  // forward declaration.

  /*! Class represents a netCDF dimension */
  class NcxxDim   {

  public:

    /*! destructor*/
    ~NcxxDim(){};

    /*! Constructor generates a \ref isNull "null object". */
    NcxxDim ();

    /*!
      Constructor for a dimension .
      The dimension must already exist in the netCDF file. New netCDF variables can be added using NcxxGroup::addNcxxDim();
      \param grp    Parent NcxxGroup object.
      \param dimId  Id of the NcxxDim object.
    */
    NcxxDim(const NcxxGroup& grp, int dimId);

    /*! assignment operator  */
    NcxxDim& operator =(const NcxxDim &);

    /*! equivalence operator */
    bool operator==(const NcxxDim& rhs) const;

    /*!  != operator */
    bool operator!=(const NcxxDim& rhs) const;

    /*! The copy constructor. */
    NcxxDim(const NcxxDim& ncDim);

    /*! The name of this dimension.*/
    std::string getName() const;

    /*! The netCDF Id of this dimension. */
    int getId() const {return myId;};

    /*! Gets a  NcxxGroup object of the parent group. */
    NcxxGroup getParentGroup() const;

    /*! Returns true if this is an unlimited dimension */
    bool isUnlimited() const;

    /*! The size of the dimension; for unlimited, this is the number of records written so far. */
    size_t  getSize() const;

    /*!renames the dimension */
    void rename( const std::string& newName);

    /*! Returns true if this object is null (i.e. it has no contents); otherwise returns false. */
    bool isNull() const  {return nullObject;}

    /*! comparator operator  */
    friend bool operator<(const NcxxDim& lhs,const NcxxDim& rhs);

    /*! comparator operator  */
    friend bool operator>(const NcxxDim& lhs,const NcxxDim& rhs);

  private:

    bool nullObject;

    int myId;

    int groupId;

  };
  
}


#endif
