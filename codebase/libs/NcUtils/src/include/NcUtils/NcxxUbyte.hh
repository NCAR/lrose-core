#include <NcUtils/NcxxType.hh>

#ifndef NcxxUbyteClass
#define NcxxUbyteClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Ubyte type. */
  class NcxxUbyte : public NcxxType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcxxUbyte & rhs);
    
    /*! destructor */
    ~NcxxUbyte();
    
    /*! Constructor */
    NcxxUbyte();
  };

  /*! A global instance  of the NcxxUbyte class within the netCDF namespace. */
  extern NcxxUbyte ncUbyte;

}
#endif
