#include <NcUtils/NcxxType.hh>

#ifndef NcxxInt64Class
#define NcxxInt64Class

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Int64 type. */
  class NcxxInt64 : public NcxxType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcxxInt64 & rhs);
    
    /*!  destructor */
    ~NcxxInt64();
    
    /*! Constructor */
    NcxxInt64();
  };

  /*! A global instance  of the NcxxInt64 class within the netCDF namespace. */
  extern NcxxInt64 ncInt64;

}
#endif
