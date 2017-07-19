// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
//////////////////////////////////////////////////////////////////////
//  Ncxx C++ classes for NetCDF4
//
//  Copied from code by:
//
//    Lynton Appel, of the Culham Centre for Fusion Energy (CCFE)
//    in Oxfordshire, UK.
//    The netCDF-4 C++ API was developed for use in managing
//    fusion research data from CCFE's innovative MAST
//    (Mega Amp Spherical Tokamak) experiment.
// 
//  Offical NetCDF codebase is at:
//
//    https://github.com/Unidata/netcdf-cxx4
//
//  Modification for LROSE made by:
//
//    Mike Dixon, EOL, NCAR
//    P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//  The base code makes extensive use of exceptions.
//  Additional methods have been added to return error conditions. 
//
//  December 2016
//
//////////////////////////////////////////////////////////////////////

#include <NcUtils/NcxxType.hh>
#include <NcUtils/NcxxException.hh>
#include <NcUtils/NcxxErrStr.hh>
#include <vector>
#include <string>
#include <typeinfo>

#ifndef NcxxAttClass
#define NcxxAttClass

namespace netCDF
{

  /*! Abstract base class represents inherited by ncVarAtt and ncGroupAtt. */
  class NcxxAtt : public NcxxErrStr
  {
  public:
    
    /*! destructor */
    virtual ~NcxxAtt()=0;

    /*! Constructor generates a \ref isNull "null object". */
    NcxxAtt ();
    
    /*! Constructor for non-null instances. */
    NcxxAtt(bool nullObject); 

    /*! The copy constructor. */
    NcxxAtt(const NcxxAtt& rhs);

    /*! Get the attribute name. */
    std::string getName() const {return myName;}

    /*! Gets attribute length. */
    size_t  getAttLength() const;

    /*! Returns the attribute type. */
    NcxxType  getType() const;

    /*! Gets parent group. */
    NcxxGroup  getParentGroup() const;
      
    /*! equivalence operator */
    bool operator== (const NcxxAtt& rhs) const;
      
    /*!  != operator */
    bool operator!=(const NcxxAtt& rhs) const;     

    /*! \overload
     */ 
    void getValues(char* dataValues) const;
    /*! \overload
     */ 
    void getValues(unsigned char* dataValues) const;
    /*! \overload
     */ 
    void getValues(signed char* dataValues) const;
    /*! \overload
     */ 
    void getValues(short* dataValues) const;
    /*! \overload
     */ 
    void getValues(int* dataValues) const;
    /*! \overload
     */ 
    void getValues(long* dataValues) const;
    /*! \overload
     */ 
    void getValues(float* dataValues) const;
    /*! \overload
     */ 
    void getValues(double* dataValues) const;
    /*! \overload
     */ 
    void getValues(unsigned short* dataValues) const;
    /*! \overload
     */ 
    void getValues(unsigned int* dataValues) const;
    /*! \overload
     */ 
    void getValues(long long* dataValues) const;
    /*! \overload
     */ 
    void getValues(unsigned long long* dataValues) const;
    /*! \overload
     */ 
    void getValues(char** dataValues) const;

    /*! \overload
      (The string variable does not need preallocating.)
     */ 
    void getValues(std::string& dataValues) const;

    /*! 
      Gets a netCDF attribute.
      The user must ensure that the variable "dataValues" has sufficient space to hold the attribute.
      \param  dataValues On return contains the value of the attribute. 
      If the type of data values differs from the netCDF variable type, type conversion will occur. 
      (However, no type conversion is carried out for variables using the user-defined data types:
      nc_Vlen, nc_Opaque, nc_Compound and nc_Enum.)
    */
    void getValues(void* dataValues) const;

    //////////////////////////////////////////
    // get values loaded into vector
    // vector is resized to hold attribute length
    // returns 0 on success, -1 on failure
    // On failure, getErrStr() holds error message.
    
    int getValues(vector<char *> &dataValues) const;
    int getValues(vector<char> &dataValues) const;
    int getValues(vector<unsigned char> &dataValues) const;
    int getValues(vector<short> &dataValues) const;
    int getValues(vector<unsigned short> &dataValues) const;
    int getValues(vector<int> &dataValues) const;
    int getValues(vector<unsigned int> &dataValues) const;
    int getValues(vector<long> &dataValues) const;
    int getValues(vector<long long> &dataValues) const;
    int getValues(vector<unsigned long long> &dataValues) const;
    int getValues(vector<float> &dataValues) const;
    int getValues(vector<double> &dataValues) const;

    /*! Returns true if this object is null (i.e. it has no contents); otherwise returns false. */
    bool isNull() const {return nullObject;}

    ///////////////////////////////////////////
    // get string representation of attribute
    
    std::string asString();
  
  protected:
    /*! assignment operator */
    NcxxAtt& operator= (const NcxxAtt& rhs);
      
    bool nullObject;

    std::string myName;
    
    int groupId;
      
    int varId;
    
  };
  
}

#endif
