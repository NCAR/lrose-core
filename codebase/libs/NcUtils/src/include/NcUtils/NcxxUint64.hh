#include <NcUtils/NcxxType.hh>

#ifndef NcxxUint64Class
#define NcxxUint64Class

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Uint64 type.*/
  class NcxxUint64 : public NcxxType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcxxUint64 & rhs);
    
    /*! destructor */
    ~NcxxUint64();
    
    /*! Constructor */
    NcxxUint64();
  };

  /*! A global instance  of the NcxxUint64 class within the netCDF namespace. */
  extern NcxxUint64 ncUint64;

}
#endif
