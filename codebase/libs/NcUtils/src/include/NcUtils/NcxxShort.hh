#include <NcUtils/NcxxType.hh>

#ifndef NcxxShortClass
#define NcxxShortClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Short type. */
  class NcxxShort : public NcxxType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcxxShort & rhs);
    
    /*! destructor */
    ~NcxxShort();
    
    /*! Constructor */
    NcxxShort();
  };

  /*! A global instance  of the NcxxShort class within the netCDF namespace. */
  extern NcxxShort ncShort;

}
#endif
