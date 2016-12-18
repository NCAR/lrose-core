#include <NcUtils/NcxxType.hh>

#ifndef NcxxStringClass
#define NcxxStringClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic String type. */
  class NcxxString : public NcxxType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcxxString & rhs);
    
    /*! destructor */
    ~NcxxString();
    
    /*! Constructor */
    NcxxString();
  };

  /*! A global instance  of the NcxxString class within the netCDF namespace. */
  extern NcxxString ncString;

}
#endif
