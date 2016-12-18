#include <NcUtils/NcxxAtt.hh>
#include <netcdf.h>

#ifndef NcxxGroupAttClass
#define NcxxGroupAttClass

namespace netCDF
{
  class NcxxGroup;  // forward declaration.

  /*! Class represents a netCDF group attribute */
  class NcxxGroupAtt : public NcxxAtt
  {
  public:
    
    /*! assignment operator */
    NcxxGroupAtt& operator= (const NcxxGroupAtt& rhs);
   
    /*! Constructor generates a \ref isNull "null object". */
    NcxxGroupAtt ();
    
    /*! The copy constructor. */
    NcxxGroupAtt(const NcxxGroupAtt& rhs) ;
      
    /*! 
      Constructor for an existing global attribute.
      \param  grp        Parent Group object.
      \param  index      The index (id) of the attribute.
    */
    NcxxGroupAtt(const NcxxGroup& grp, const int index);
    
    /*! equivalence operator */
    bool operator== (const NcxxGroupAtt& rhs);
      
    /*! comparator operator */
    friend bool operator<(const NcxxGroupAtt& lhs,const NcxxGroupAtt& rhs);
    
    /*! comparator operator */
    friend bool operator>(const NcxxGroupAtt& lhs,const NcxxGroupAtt& rhs);
    
  };
  
}

#endif
