#include <NcUtils/NcxxType.hh>

#ifndef NcxxDoubleClass
#define NcxxDoubleClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Double type. */
  class NcxxDouble : public NcxxType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcxxDouble & rhs);
    
    /*!  destructor */
    ~NcxxDouble();
    
    /*! Constructor */
    NcxxDouble();
  };

  /*! A global instance  of the NcxxDouble class within the netCDF namespace. */
  extern NcxxDouble ncDouble;

}
#endif
