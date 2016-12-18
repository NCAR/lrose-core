#include <NcUtils/NcxxType.hh>

#ifndef NcxxIntClass
#define NcxxIntClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Int type. */
  class NcxxInt : public NcxxType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcxxInt & rhs);
    
    /*!  destructor */
    ~NcxxInt();
    
    /*! Constructor */
    NcxxInt();
  };

  /*! A global instance  of the NcxxInt class within the netCDF namespace. */
  extern NcxxInt ncInt;

}
#endif
