#include <NcUtils/NcxxType.hh>

#ifndef NcxxCharClass
#define NcxxCharClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Char type. */
  class NcxxChar : public NcxxType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcxxChar & rhs);
    
    ~NcxxChar();
    
    /*! Constructor */
    NcxxChar();
  };

  /*! A global instance  of the NcxxChar class within the netCDF namespace. */
  extern NcxxChar ncChar;

}
#endif
