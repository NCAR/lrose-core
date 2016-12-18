#include <NcUtils/NcxxType.hh>

#ifndef NcxxUshortClass
#define NcxxUshortClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Ushort type. */
  class NcxxUshort : public NcxxType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcxxUshort & rhs);
    
    /*! destructor */
    ~NcxxUshort();
    
    /*! Constructor */
    NcxxUshort();
  };

  // declare that the class instance ncUshort is known by all....
  extern NcxxUshort ncUshort;

}
#endif
