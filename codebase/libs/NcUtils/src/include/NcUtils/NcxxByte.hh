#include <NcUtils/NcxxType.hh>

#ifndef NcxxByteClass
#define NcxxByteClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Byte type. */
  class NcxxByte : public NcxxType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcxxByte & rhs);

    /*! storage size */
    int sizeoff();

    ~NcxxByte();
    
    /*! Constructor */
    NcxxByte();
  };

  /*! A global instance  of the NcxxByte class within the netCDF namespace. */
  extern NcxxByte ncByte;

}
#endif
