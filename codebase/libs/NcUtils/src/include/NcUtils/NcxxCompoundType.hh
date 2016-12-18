#include <string>
#include <vector>
#include <NcUtils/NcxxType.hh>
#include <netcdf.h>

#ifndef NcxxCompoundTypeClass
#define NcxxCompoundTypeClass


namespace netCDF
{
  class NcxxGroup;  // forward declaration.

  /*! 
    Class represents a netCDF compound type
  */
  class NcxxCompoundType : public NcxxType
  {
  public:

    /*! Constructor generates a \ref isNull "null object". */
    NcxxCompoundType();

    /*! 
      Constructor.
      The compound Type must already exist in the netCDF file. New netCDF compound types can be 
      added using NcxxGroup::addNcxxCompoundType();
      \param grp        The parent group where this type is defined.
      \param name       Name of new type.
    */
    NcxxCompoundType(const NcxxGroup& grp, const std::string& name);

    /*! 
      Constructor.
      Constructs from the base type NcxxType object. Will throw an exception if the NcxxType is not the base of a Compound type.
      \param ncType     A Nctype object.
    */
    NcxxCompoundType(const NcxxType& ncType);

    /*! assignment operator */
    NcxxCompoundType& operator=(const NcxxCompoundType& rhs);
      
    /*! 
      Assignment operator.
      This assigns from the base type NcxxType object. Will throw an exception if the NcxxType is not the base of a Compound type.
    */
    NcxxCompoundType& operator=(const NcxxType& rhs);
      
    /*! The copy constructor. */
    NcxxCompoundType(const NcxxCompoundType& rhs);
      
    /*! equivalence operator */
    bool operator==(const NcxxCompoundType & rhs);

    /*! destructor */
    ~NcxxCompoundType(){;}
      
      
    /*!  
      Adds a named field.
      \param memName       Name of new field.
      \param newMemberType The type of the new member.
      \param offset        Offset of this member in bytes, obtained by a call to offsetof. For example 
the offset of a member "mem4" in structure struct1 is: offsetof(struct1,mem4).
    */
    void addMember(const std::string& memName, const NcxxType& newMemberType,size_t offset);

    /*!  
      Adds a named array field.
      \param memName       Name of new field.
      \param newMemberType The type of the new member.
      \param offset        Offset of this member in bytes, obtained by a call to offsetof. For example 
                           the offset of a member "mem4" in structure struct1 is: offsetof(struct1,mem4).
      \param shape         The shape of the array field.
    */
    void addMember(const std::string& memName, const NcxxType& newMemberType, size_t offset, const std::vector<int>& shape);


    /*! Returns number of members in this NcxxCompoundType object. */
    size_t  getMemberCount() const;
      
    /*! Returns a NcxxType object for a single member. */
    NcxxType getMember(int memberIndex) const;

    /*! Returns name of member field. */
    std::string getMemberName(int memberIndex) const;

    /*! Returns index of named member field. */
    int getMemberIndex(const std::string& memberName) const;

    /*! Returns the offset of the member with given index. */
    size_t getMemberOffset(const int index) const;

    /*! 
      Returns the number of dimensions of a member with the given index. 
      \param Index of member (numbering starts at zero).
      \return The number of dimensions of the field. Non-array fields have 0 dimensions.
    */
    int getMemberDimCount(int memberIndex) const;
      
      
    /*! 
      Returns the shape of a given member. 
      \param Index of member (numbering starts at zero).
      \return The size of the dimensions of the field. Non-array fields have 0 dimensions.
    */
    std::vector<int> getMemberShape(int memberIndex) const;
      
      
  private:
      
    int myOffset;
      
  };
  
}


#endif
