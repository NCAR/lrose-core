#include <NcUtils/NcxxType.hh>

#ifndef NcxxFloatClass
#define NcxxFloatClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Float type. */
  class NcxxFloat : public NcxxType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcxxFloat & rhs);
    
    /*!  destructor */
    ~NcxxFloat();
    
    /*! Constructor */
    NcxxFloat();
  };

  /*! A global instance  of the NcxxFloat class within the netCDF namespace. */
  extern NcxxFloat ncFloat;

}
#endif
