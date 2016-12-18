#include <NcUtils/NcxxAtt.hh>
#include <netcdf.h>

#ifndef NcxxVarAttClass
#define NcxxVarAttClass

namespace netCDF
{
  class NcxxGroup;  // forward declaration.
  class NcxxVar;    // forward declaration.

  /*! Class represents a netCDF attribute local to a netCDF variable. */
  class NcxxVarAtt : public NcxxAtt
  {
  public:
    
    /*! assignment operator */
    NcxxVarAtt& operator= (const NcxxVarAtt& rhs);
      
    /*! Constructor generates a \ref isNull "null object". */
    NcxxVarAtt ();

    /*! The copy constructor. */
    NcxxVarAtt(const NcxxVarAtt& rhs) ;
      
    /*! 
      Constructor for an existing local attribute.
      \param  grp        Parent Group object.
      \param  NcxxVar      Parent NcxxVar object.
      \param  index      The index (id) of the attribute.
    */
    NcxxVarAtt(const NcxxGroup& grp, const NcxxVar& ncVar, const int index);
    
    /*! Returns the NcxxVar parent object. */
    NcxxVar getParentVar() const;

    /*! comparator operator */
    friend bool operator<(const NcxxVarAtt& lhs,const NcxxVarAtt& rhs);
    
    /*! comparator operator  */
    friend bool operator>(const NcxxVarAtt& lhs,const NcxxVarAtt& rhs);
    
  };
  
}

#endif
