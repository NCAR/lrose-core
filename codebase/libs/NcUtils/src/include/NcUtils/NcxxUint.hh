#include <NcUtils/NcxxType.hh>

#ifndef NcxxUintClass
#define NcxxUintClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Uint type. */
  class NcxxUint : public NcxxType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcxxUint & rhs);
    
    /*! destructor */
    ~NcxxUint();
    
    /*! Constructor */
    NcxxUint();
  };

  /*! A global instance  of the NcxxUint class within the netCDF namespace. */
  extern NcxxUint ncUint;

}
#endif
