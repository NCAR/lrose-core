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
//
//  December 2016
//
//////////////////////////////////////////////////////////////////////

#include <exception>
#include <string>
#include <typeinfo>
#include <map>
#include <vector>
#include <netcdf.h>
#include <Ncxx/NcxxVarAtt.hh>
#include <Ncxx/NcxxGroup.hh>
#include <Ncxx/NcxxByte.hh>
#include <Ncxx/NcxxUbyte.hh>
#include <Ncxx/NcxxChar.hh>
#include <Ncxx/NcxxShort.hh>
#include <Ncxx/NcxxUshort.hh>
#include <Ncxx/NcxxInt.hh>
#include <Ncxx/NcxxUint.hh>
#include <Ncxx/NcxxInt64.hh>
#include <Ncxx/NcxxUint64.hh>
#include <Ncxx/NcxxFloat.hh>
#include <Ncxx/NcxxDouble.hh>
#include <Ncxx/NcxxString.hh>
#include <Ncxx/NcxxErrStr.hh>

#ifndef NcxxVarClass
#define NcxxVarClass

class NcxxDim;    // forward declaration.
class NcxxType;   // forward declaration.

/*! Class represents a netCDF variable. */

class NcxxVar : public NcxxErrStr
{

public:

  /*!
    Used for chunking specifications (see NcxxVar::setChunking,
    NcxxVar::getChunkingParameters).
  */

  enum ChunkMode
    {
      /*!
        Chunked storage is used for this variable.
      */
      nc_CHUNKED    = NC_CHUNKED,
      /*! Contiguous storage is used for this variable. Variables with one or
        more unlimited dimensions cannot use contiguous storage. If contiguous
        storage is turned on, the chunkSizes parameter is ignored.
      */
      nc_CONTIGUOUS = NC_CONTIGUOUS
    };

  /*!
    Used to specifying the endianess of the data, (see NcxxVar::setEndianness,
    NcxxVar::getEndianness). By default this is NC_ENDIAN_NATIVE.
  */
  enum EndianMode
    {
      nc_ENDIAN_NATIVE = NC_ENDIAN_NATIVE, //!< Native endian.
      nc_ENDIAN_LITTLE = NC_ENDIAN_LITTLE, //!< Little endian.
      nc_ENDIAN_BIG    = NC_ENDIAN_BIG     //!< Big endian.
    };

  /*! Used for checksum specification (see NcxxVar::setChecksum, NcxxVar::getChecksum). */
  enum ChecksumMode
    {
      nc_NOCHECKSUM = NC_NOCHECKSUM, //!< No checksum (the default).
      nc_FLETCHER32 = NC_FLETCHER32  //!< Selects the Fletcher32 checksum filter.
    };

  /*! destructor */
  ~NcxxVar(){};

  /*! Constructor generates a \ref isNull "null object". */
  NcxxVar ();
  
  /*! Constructor for a variable .
    The variable must already exist in the netCDF file.
    New netCDF variables can be added using NcxxGroup::addNcxxVar();
    \param grp    Parent NcxxGroup object.
    \param varId  Id of the is NcxxVar object.
  */
  NcxxVar(const NcxxGroup& grp, const int& varId);

  /*! assignment operator  */
  NcxxVar& operator =(const NcxxVar& rhs);

  /*! equivalence operator */
  bool operator==(const NcxxVar& rhs) const;
  
  /*!  != operator */
  bool operator!=(const NcxxVar& rhs) const;

  /*! The copy constructor. */
  NcxxVar(const NcxxVar& ncVar);

  /*! Name of this NcxxVar object.*/
  std::string getName() const;

  /*! Gets parent group. */
  NcxxGroup  getParentGroup() const;

  /*! Returns the variable type. */
  NcxxType getType() const;

  /*! Rename the variable. */
  void rename( const std::string& newname ) const;
  
  /*! Get the variable id. */
  int getId() const;

  /*! Returns true if this object variable is not defined. */
  bool isNull() const  {return nullObject;}

  /*! comparator operator  */
  friend bool operator<(const NcxxVar& lhs,const NcxxVar& rhs);

  /*! comparator operator  */
  friend bool operator>(const NcxxVar& lhs,const NcxxVar& rhs);

  ////////////////////////////////////////
  // Information about Dimensions
  ////////////////////////////////////////

  /*! The the number of dimensions. */
  int getDimCount() const;

  /*! Gets the i'th NcxxDim object. */
  NcxxDim getDim(int i) const;

  /*! Gets the set of NcxxDim objects. */
  std::vector<NcxxDim> getDims() const;

  /////////////////////////////////
  // Information about Attributes
  /////////////////////////////////

  /*! Gets the number of attributes. */
  int getAttCount() const;

  /*! Gets attribute by name */
  NcxxVarAtt getAtt(const std::string& name) const;

  /*! Gets the set of attributes. */
  std::map<std::string,NcxxVarAtt> getAtts() const;

  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    size_t len, 
                    const char** dataValues) const;

  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const std::string& dataValues) const;

  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type, 
                    size_t len, 
                    const unsigned char* dataValues) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type, 
                    size_t len, 
                    const signed char* dataValues) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type, 
                    short datumValue) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type, 
                    int datumValue) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type, 
                    long datumValue) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type, 
                    float datumValue) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type, 
                    double datumValue) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type, 
                    unsigned short datumValue) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type,
                    unsigned int datumValue) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type,
                    unsigned long long datumValue) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type,
                    long long datumValue) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type,
                    size_t len, 
                    const short* dataValues) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type,
                    size_t len, 
                    const int* dataValues) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type,
                    size_t len, 
                    const long* dataValues) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type,
                    size_t len, 
                    const float* dataValues) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type,
                    size_t len, 
                    const double* dataValues) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type,
                    size_t len, 
                    const unsigned short* dataValues) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type,
                    size_t len, 
                    const unsigned int* dataValues) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type,
                    size_t len, 
                    const unsigned long long* dataValues) const;
  /*! \overload
   */
  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type,
                    size_t len, 
                    const long long* dataValues) const;

  /*!

    Creates a new variable attribute or if already exisiting replaces it.
    If you are writing a _Fill_Value_ attribute, and will tell the HDF5 layer to use
    the specified fill value for that variable.

    \par
    Although it's possible to create attributes of all types, text and double
    attributes are adequate for most purposes.

    \param name        Name of attribute.
    \param type        The attribute type.
    \param len         The length of the attribute (number of Nctype repeats).
    \param dataValues  Data Values to put into the new attribute.

    If the type of data values differs from the netCDF variable type, type
    conversion will occur.  (However, no type conversion is carried out for
    variables using the user-defined data types: nc_Vlen, nc_Opaque, nc_Compound
    and nc_Enum.)
    
    \return            The NcxxVarAtt object for this new netCDF attribute.
  */

  NcxxVarAtt putAtt(const std::string& name, 
                    const NcxxType& type,
                    size_t len, 
                    const void* dataValues) const;

  ////////////////////
  // Chunking details
  ////////////////////

  /*! Sets chunking parameters.
    \param chunkMode   Enumeration type. 
    Allowable parameters are: "nc_CONTIGUOUS", "nc_CHUNKED"
    \param chunksizes  Shape of chunking, used if ChunkMode=nc_CHUNKED.
  */
  void setChunking(ChunkMode chunkMode, 
                   std::vector<size_t>& chunksizes) const;

  /*! Gets the chunking parameters
    \param chunkMode   On return contains either: "nc_CONTIGUOUS" or "nc_CHUNKED"
    \param chunksizes  On return contains shape of chunking, used if ChunkMode=nc_CHUNKED.
  */
  void getChunkingParameters(ChunkMode& chunkMode, 
                             std::vector<size_t>& chunkSizes) const;


  ////////////////////
  // Fill details
  ////////////////////

  // Sets the fill parameters
  /*!
    \overload
  */
  void setFill(bool fillMode,void* fillValue=NULL) const;

  /*!
    This is an overloaded member function, provided for convenience.
    It differs from the above function in what argument(s) it accepts.
    The function can be used for any type, including user-defined types.

    \param fillMode   Setting to true, turns on fill mode.
    \param fillValue  Pointer to fill value.

    Must be the same type as the variable. Ignored if fillMode=.false.
  */
  void setFill(bool fillMode,const void* fillValue=NULL) const;

  /*! Sets the fill parameters
    \param fillMode   Setting to true, turns on fill mode.
    \param fillValue  Fill value for the variable.
    Must be the same type as the variable. Ignored if fillMode=.false.
  */
  template<class T>
    void setFill(bool fillMode, T fillValue) const
  {
    ncxxCheck(nc_def_var_fill(groupId,myId,static_cast<int>
                              (!fillMode),&fillValue),__FILE__,__LINE__);
  }

  /*!
    This is an overloaded member function, provided for convenience.
    It differs from the above function in what argument(s) it accepts.
    The function can be used for any type, including user-defined types.
    
    \param fillMode   On return set to true  if fill mode is enabled.
    \param fillValue  On return containts a pointer to fill value.

    Must be the same type as the variable. Ignored if fillMode=.false.
  */
  void getFillModeParameters(bool& fillMode, 
                             void* fillValue=NULL) const;


  /*! Gets the fill parameters
    \param On return set to true  if fill mode is enabled.
    \param On return  is set to the fill value.
  */
  template <class T> void getFillModeParameters(bool& fillMode,T& fillValue) const{
    int fillModeInt;
    ncxxCheck(nc_inq_var_fill(groupId,myId,&fillModeInt,&fillValue),
              __FILE__,__LINE__);
    fillMode= static_cast<bool> (fillModeInt == 0);
  }

  ///////////////////////////
  // Compression details
  ///////////////////////////

  /*! Sets the compression parameters
    \param enableShuffleFilter Set to true to turn on shuffle filter.
    \param enableDeflateFilter Set to true to turn on deflate filter.
    \param deflateLevel        The deflate level, must be 0 and 9.
  */
  void setCompression(bool enableShuffleFilter,
                      bool enableDeflateFilter, 
                      int deflateLevel) const;

  /*! Gets the compression parameters
    \param enableShuffleFilter  On return set to true if the shuffle filter is enabled.
    \param enableDeflateFilter  On return set to true if the deflate filter is enabled.
    \param deflateLevel         On return set to the deflate level.
  */
  void getCompressionParameters(bool& shuffleFilterEnabled,
                                bool& deflateFilterEnabled, 
                                int& deflateLevel) const;

  ////////////////////////
  // Endianness details
  ////////////////////////

  /*! Sets the endianness of the variable.
    \param Endianness enumeration type.
    Allowable parameters are: "nc_ENDIAN_NATIVE" (the default),
    "nc_ENDIAN_LITTLE", "nc_ENDIAN_BIG"
  */
  void setEndianness(EndianMode endianMode) const;

  /*! Gets the endianness of the variable.
    \return Endianness
    enumeration type.
    Allowable parameters are:
    "nc_ENDIAN_NATIVE" (the default),
    "nc_ENDIAN_LITTLE",
    "nc_ENDIAN_BIG"
  */
  EndianMode getEndianness() const;

  ////////////////////
  // Checksum details
  ////////////////////

  /*! Sets the checksum parameters of a variable.
    \param ChecksumMode
    Enumeration type.
    Allowable parameters are: "nc_NOCHECKSUM", "nc_FLETCHER32".
  */
  void setChecksum(ChecksumMode checksumMode) const;

  /*! Gets the checksum parameters of the variable.
    \return ChecksumMode Enumeration type. 
    Allowable parameters are: "nc_NOCHECKSUM", "nc_FLETCHER32".
  */
  ChecksumMode getChecksum() const;

  ////////////////////
  //  data  reading
  ////////////////////

  /////////////////////////////////////////////////////////////////////////////
  // Reads the entire data into the netCDF variable.

  /*!
    Reads the entire data from an netCDF variable.  This is the simplest
    interface to use for reading the value of a scalar variable or when all the
    values of a multidimensional variable can be read at once. The values are
    read into consecutive locations with the last dimension varying fastest.

    Take care when using the simplest forms of this interface with record
    variables when you don't specify how many records are to be read. If you try
    to read all the values of a record variable into an array but there are more
    records in the file than you assume, more data will be read than you expect,
    which may cause a segmentation violation.

    \param dataValues
    Pointer to the location into which the data value is read.
    If the type of data value differs from the netCDF variable type, type
    conversion will occur.  (However, no type conversion is carried out for
    variables using the user-defined data types: nc_Vlen, nc_Opaque, nc_Compound
    and nc_Enum.)
  */

  /*! \overload
   */
  void getVal(char** dataValues) const;
  /*! \overload
   */
  void getVal(char* dataValues) const;
  /*! \overload
   */
  void getVal(unsigned char* dataValues) const;
  /*! \overload
   */
  void getVal(signed char* dataValues) const;
  /*! \overload
   */
  void getVal(short* dataValues) const;
  /*! \overload
   */
  void getVal(int* dataValues) const;
  /*! \overload
   */
  void getVal(long* dataValues) const;
  /*! \overload
   */
  void getVal(float* dataValues) const;
  /*! \overload
   */
  void getVal(double* dataValues) const;
  /*! \overload
   */
  void getVal(unsigned short* dataValues) const;
  /*! \overload
   */
  void getVal(unsigned int* dataValues) const;
  /*! \overload
   */
  void getVal(unsigned long long* dataValues) const;
  /*! \overload
   */
  void getVal(long long* dataValues) const;
  /*!
    This is an overloaded member function, provided for convenience.
    It differs from the above function in what argument(s) it accepts.
    In addition, no data conversion is carried out. This means that
    the type of the data in memory must match the type of the variable.
  */
  void getVal(void* dataValues) const;

  /////////////////////////////////////////////////////////////////////////////
  // Reads a single datum value from a variable of an open netCDF dataset.
  /*! Reads a single datum value from a variable of an open netCDF dataset.
    The value is converted from the external data type of the variable, if necessary.

    \param index
    Vector specifying the index of the data value to be read.  The indices are
    relative to 0, so for example, the first data value of a two-dimensional
    variable would have index (0,0). The elements of index must correspond to
    the variable's dimensions.  Hence, if the variable is a record variable, the
    first index is the record number.

    \param datumValue
    Pointer to the location into which the data value is read. If the type of data
    value differs from the netCDF variable type, type conversion will occur.
    (However, no type conversion is carried out for variables using the
    user-defined data types: nc_Vlen, nc_Opaque, nc_Compound and nc_Enum.)
  */

  /*! \overload
   */
  void getVal(const std::vector<size_t>& index,
              char** datumValue) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& index,
              char* datumValue) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& index,
              unsigned char* datumValue) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& index,
              signed char* datumValue) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& index,
              short* datumValue) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& index,
              int* datumValue) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& index,
              long* datumValue) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& index,
              float* datumValue) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& index,
              double* datumValue) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& index,
              unsigned short* datumValue) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& index,
              unsigned int* datumValue) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& index,
              unsigned long long* datumValue) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& index,
              long long* datumValue) const;
  /*!
    This is an overloaded member function, provided for convenience.
    It differs from the above function in what argument(s) it accepts.
    In addition, no data conversion is carried out. This means that
    the type of the data in memory must match the type of the variable.
  */
  void getVal(const std::vector<size_t>& index,
              void* datumValue) const;

  /////////////////////////////////////////////////////////////////////////////
  // Reads an array of values from a netCDF variable of an open netCDF dataset.

  /*! 
    Reads an array of values from a netCDF variable of an open netCDF dataset.
    The array is specified by giving a corner and a vector of edge lengths.  The
    values are read into consecutive locations with the last dimension varying
    fastest.

    \param start
    Vector specifying the index in the variable where the first of
    the data values will be read.  The indices are relative to 0, so for
    example, the first data value of a variable would have index (0, 0, ... ,
    0).  The length of start must be the same as the number of dimensions of the
    specified variable.  The elements of start correspond, in order, to the
    variable's dimensions. Hence, if the variable is a record variable, the
    first index would correspond to the starting record number for reading the
    data values.

    \param count
    Vector specifying the edge lengths along each dimension of the
    block of data values to be read.  To read a single value, for example,
    specify count as (1, 1, ... , 1). The length of count is the number of
    dimensions of the specified variable. The elements of count correspond, in
    order, to the variable's dimensions.  Hence, if the variable is a record
    variable, the first element of count corresponds to a count of the number of
    records to read.  Note: setting any element of the count array to zero
    causes the function to exit without error, and without doing anything.

    \param dataValues
    Pointer to the location into which the data value is
    read. If the type of data value differs from the netCDF variable type, type
    conversion will occur.  (However, no type conversion is carried out for
    variables using the user-defined data types: nc_Vlen, nc_Opaque, nc_Compound
    and nc_Enum.)

  */

  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              char** dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              char* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              unsigned char* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,
              signed char* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              short* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              int* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              long* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count, 
              float* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              double* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              unsigned short* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              unsigned int* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              unsigned long long* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,
              long long* dataValues) const;

  /*!
    This is an overloaded member function, provided for convenience.
    It differs from the above function in what argument(s) it accepts.
    In addition, no data conversion is carried out. This means that
    the type of the data in memory must match the type of the variable.
  */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,
              void* dataValues) const;
  
  /////////////////////////////////////////////////////////////////////////////
  // Reads a subsampled (strided) array section of values from a netCDF variable.

  /*!
    Reads a subsampled (strided) array section of values from a netCDF variable.
    The subsampled array section is specified by giving a corner, a vector of
    edge lengths, and a stride vector.  The values are read with the last
    dimension of the netCDF variable varying fastest.

    \param start
    Vector specifying the index in the variable where the first of
    the data values will be read.  The indices are relative to 0, so for
    example, the first data value of a variable would have index (0, 0, ... ,
    0).  The length of start must be the same as the number of dimensions of the
    specified variable.  The elements of start correspond, in order, to the
    variable's dimensions. Hence, if the variable is a record variable, the
    first index would correspond to the starting record number for reading the
    data values.

    \param
    count Vector specifying the edge lengths along each dimension of the
    block of data values to be read.  To read a single value, for example,
    specify count as (1, 1, ... , 1). The length of count is the number of
    dimensions of the specified variable. The elements of count correspond, in
    order, to the variable's dimensions.  Hence, if the variable is a record
    variable, the first element of count corresponds to a count of the number of
    records to read.  Note: setting any element of the count array to zero
    causes the function to exit without error, and without doing anything.

    \param stride
    Vector specifying the interval between selected indices. The
    elements of the stride vector correspond, in order, to the variable's
    dimensions. A value of 1 accesses adjacent values of the netCDF variable in
    the corresponding dimension; a value of 2 accesses every other value of the
    netCDF variable in the corresponding dimension; and so on. A NULL stride
    argument is treated as (1, 1, ... , 1).

    \param dataValues
    Pointer to the location into which the data value is
    read. If the type of data value differs from the netCDF variable type, type
    conversion will occur.  (However, no type conversion is carried out for
    variables using the user-defined data types: nc_Vlen, nc_Opaque, nc_Compound
    and nc_Enum.)
  */
  
  
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              const std::vector<ptrdiff_t>& stride,
              char** dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              const std::vector<ptrdiff_t>& stride,
              char* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              const std::vector<ptrdiff_t>& stride, 
              unsigned char* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              const std::vector<ptrdiff_t>& stride,
              signed char* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              const std::vector<ptrdiff_t>& stride,
              short* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              const std::vector<ptrdiff_t>& stride,
              int* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              const std::vector<ptrdiff_t>& stride,
              long* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count, 
              const std::vector<ptrdiff_t>& stride,
              float* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              const std::vector<ptrdiff_t>& stride,
              double* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              const std::vector<ptrdiff_t>& stride,
              unsigned short* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              const std::vector<ptrdiff_t>& stride,
              unsigned int* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              const std::vector<ptrdiff_t>& stride,
              unsigned long long* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              const std::vector<ptrdiff_t>& stride,
              long long* dataValues) const;
  /*!
    This is an overloaded member function, provided for convenience.
    It differs from the above function in what argument(s) it accepts.
    In addition, no data conversion is carried out. This means that
    the type of the data in memory must match the type of the variable.
  */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count, 
              const std::vector<ptrdiff_t>& stride,
              void* dataValues) const;
  
  //////////////////////

  // Reads a mapped array section of values from a netCDF variable.
  /*!

    Reads a mapped array section of values from a netCDF variable.  The mapped
    array section is specified by giving a corner, a vector of edge lengths, a
    stride vector, and an index mapping vector. The index mapping vector is a
    vector of integers that specifies the mapping between the dimensions of a
    netCDF variable and the in-memory structure of the internal data array. No
    assumptions are made about the ordering or length of the dimensions of the
    data array.

    \param start
    Vector specifying the index in the variable where the first of
    the data values will be read.  The indices are relative to 0, so for
    example, the first data value of a variable would have index (0, 0, ... ,
    0).  The length of start must be the same as the number of dimensions of the
    specified variable.  The elements of start correspond, in order, to the
    variable's dimensions. Hence, if the variable is a record variable, the
    first index would correspond to the starting record number for reading the
    data values.

    \param count
    Vector specifying the edge lengths along each dimension of the
    block of data values to be read.  To read a single value, for example,
    specify count as (1, 1, ... , 1). The length of count is the number of
    dimensions of the specified variable. The elements of count correspond, in
    order, to the variable's dimensions.  Hence, if the variable is a record
    variable, the first element of count corresponds to a count of the number of
    records to read.  Note: setting any element of the count array to zero
    causes the function to exit without error, and without doing anything.

    \param stride
    Vector specifying the interval between selected indices. The
    elements of the stride vector correspond, in order, to the variable's
    dimensions. A value of 1 accesses adjacent values of the netCDF variable in
    the corresponding dimension; a value of 2 accesses every other value of the
    netCDF variable in the corresponding dimension; and so on. A NULL stride
    argument is treated as (1, 1, ... , 1).

    \param imap
    Vector of integers that specifies the mapping between the
    dimensions of a netCDF variable and the in-memory structure of the internal
    data array. imap[0] gives the distance between elements of the internal
    array corresponding to the most slowly varying dimension of the netCDF
    variable. imap[n-1] (where n is the rank of the netCDF variable) gives the
    distance between elements of the internal array corresponding to the most
    rapidly varying dimension of the netCDF variable. Intervening imap elements
    correspond to other dimensions of the netCDF variable in the obvious way.
    Distances between elements are specified in type-independent units of
    elements (the distance between internal elements that occupy adjacent memory
    locations is 1 and not the element's byte-length as in netCDF 2).

    \param
    dataValues Pointer to the location into which the data value is
    read. If the type of data value differs from the netCDF variable type, type
    conversion will occur.  (However, no type conversion is carried out for
    variables using the user-defined data types: nc_Vlen, nc_Opaque, nc_Compound
    and nc_Enum.)

  */
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,
              const std::vector<ptrdiff_t>& stride,
              const std::vector<ptrdiff_t>& imap,
              char** dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,  
              const std::vector<ptrdiff_t>& stride, 
              const std::vector<ptrdiff_t>& imap, 
              char* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,  
              const std::vector<ptrdiff_t>& stride, 
              const std::vector<ptrdiff_t>& imap, 
              unsigned char* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,  
              const std::vector<ptrdiff_t>& stride, 
              const std::vector<ptrdiff_t>& imap, 
              signed char* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,  
              const std::vector<ptrdiff_t>& stride, 
              const std::vector<ptrdiff_t>& imap, 
              short* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,  
              const std::vector<ptrdiff_t>& stride, 
              const std::vector<ptrdiff_t>& imap, 
              int* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,  
              const std::vector<ptrdiff_t>& stride, 
              const std::vector<ptrdiff_t>& imap, 
              long* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,  
              const std::vector<ptrdiff_t>& stride, 
              const std::vector<ptrdiff_t>& imap, 
              float* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,  
              const std::vector<ptrdiff_t>& stride, 
              const std::vector<ptrdiff_t>& imap, 
              double* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,  
              const std::vector<ptrdiff_t>& stride, 
              const std::vector<ptrdiff_t>& imap, 
              unsigned short* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,  
              const std::vector<ptrdiff_t>& stride, 
              const std::vector<ptrdiff_t>& imap, 
              unsigned int* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,  
              const std::vector<ptrdiff_t>& stride, 
              const std::vector<ptrdiff_t>& imap, 
              unsigned long long* dataValues) const;
  /*! \overload
   */
  void getVal(const std::vector<size_t>& start, 
              const std::vector<size_t>& count,  
              const std::vector<ptrdiff_t>& stride, 
              const std::vector<ptrdiff_t>& imap, 
              long long* dataValues) const;

  /*!
    This is an overloaded member function, provided for convenience.
    It differs from the above function in what argument(s) it accepts.
    In addition, no data conversion is carried out. This means that
    the type of the data in memory must match the type of the variable.
  */
  void getVal(const std::vector<size_t>& start,
              const std::vector<size_t>& count,
              const std::vector<ptrdiff_t>& stride,
              const std::vector<ptrdiff_t>& imap,
              void* dataValues) const;

  ////////////////////
  //  data writing
  ////////////////////

  // Write a scalar into the netCDF variable.
  
  // Write string scalar
  
  void putStringScalar(const string &dataVal) const;

  // Write char scalar
  void putVal(char dataVal) const;

  // Write an unsigned char scalar
  
  void putVal(unsigned char dataVal) const;

  // Write a signed char scalar
  
  void putVal(signed char dataVal) const;

  // Write a short scalar.
  
  void putVal(short dataVal) const;

  // Write an unsigned short scalar.
  
  void putVal(unsigned short dataVal) const;

  // Write in int scalar
  
  void putVal(int dataVal) const;

  // Write an unsigned int scalar
  
  void putVal(unsigned int dataVal) const;

  // Write a long scalar
  
  void putVal(long dataVal) const;

  // Write a long long scalar
  
  void putVal(long long dataVal) const;

  // Write an unsigned long long scalar
  
  void putVal(unsigned long long dataVal) const;

  // Write a float scalar
  
  void putVal(float dataVal) const;

  // Write a double scalar
  
  void putVal(double dataVal) const;

  //////////////////////////////////////////////////////////////////////////////
  // Writes the entire data into the netCDF variable.

  /*!

    Writes the entire data into the netCDF variable.  This is the simplest
    interface to use for writing a value in a scalar variable or whenever all
    the values of a multidimensional variable can all be written at once. The
    values to be written are associated with the netCDF variable by assuming
    that the last dimension of the netCDF variable varies fastest in the C
    interface.

    Take care when using the simplest forms of this interface with record
    variables when you don't specify how many records are to be written. If you
    try to write all the values of a record variable into a netCDF file that has
    no record data yet (hence has 0 records), nothing will be
    written. Similarly, if you try to write all of a record variable but there
    are more records in the file than you assume, more data may be written to
    the file than you supply, which may result in a segmentation violation.

    \param
    dataValues The data values. The order in which the data will be
    written to the netCDF variable is with the last dimension of the specified
    variable varying fastest. If the type of data values differs from the netCDF
    variable type, type conversion will occur.  (However, no type conversion is
    carried out for variables using the user-defined data types: nc_Vlen,
    nc_Opaque, nc_Compound and nc_Enum.)

  */

  /*! \overload
   */
  void putVal(const char** dataValues) const;
  /*!  \overload
   */
  void putVal(const char* dataValues) const;
  /*!  \overload
   */
  void putVal(const unsigned char* dataValues) const;
  /*!  \overload
   */
  void putVal(const signed char* dataValues) const;
  /*!  \overload
   */
  void putVal(const short* dataValues) const;
  /*!  \overload
   */
  void putVal(const int* dataValues) const;
  /*!  \overload
   */
  void putVal(const long* dataValues) const;
  /*!  \overload
   */
  void putVal(const float* dataValues) const;
  /*!  \overload
   */
  void putVal(const double* dataValues) const;
  /*!  \overload
   */
  void putVal(const unsigned short* dataValues) const;
  /*!  \overload
   */
  void putVal(const unsigned int* dataValues) const;
  /*!  \overload
   */
  void putVal(const unsigned long long* dataValues) const;
  /*!  \overload
   */
  void putVal(const long long* dataValues) const;
  /*!
    This is an overloaded member function, provided for convenience.
    It differs from the above function in what argument(s) it accepts.
    In addition, no data conversion is carried out. This means that
    the type of the data in memory must match the type of the variable.
  */
  void putVal(const void* dataValues) const;

  //////////////////////////////////////////////////////////////////////////////
  // Writes a single datum into the netCDF variable.
  /*!

    Writes a single datum into the netCDF variable.

    \param index
    Vector specifying the index where the data values will be written. The
    indices are relative to 0, so for example, the first data value of a
    two-dimensional variable would have index (0,0). The elements of index must
    correspond to the variable's dimensions.  Hence, if the variable uses the
    unlimited dimension, the first index would correspond to the unlimited
    dimension.

    \param datumValue
    The data value. If the type of data values differs from the netCDF variable
    type, type conversion will occur.  (However, no type conversion is carried
    out for variables using the user-defined data types: nc_Vlen, nc_Opaque,
    nc_Compound and nc_Enum.)

  */

  /*! \overload
   */
  void putVal(const std::vector<size_t>& index, 
              const char** datumValue) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& index, 
              const std::string& datumValue) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& index, 
              const unsigned char* datumValue) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& index, 
              const signed char* datumValue) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& index, 
              const short datumValue) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& index, 
              const int datumValue) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& index, 
              const long datumValue) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& index, 
              const float datumValue) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& index, 
              const double datumValue) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& index, 
              const unsigned short datumValue) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& index, 
              const unsigned int datumValue) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& index, 
              const unsigned long long datumValue) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& index, 
              const long long datumValue) const;
  /*!
    This is an overloaded member function, provided for convenience.
    It differs from the above function in what argument(s) it accepts.
    In addition, no data conversion is carried out. This means that
    the type of the data in memory must match the type of the variable.
  */
  void putVal(const std::vector<size_t>& index, 
              const void* datumValue) const;


  //////////////////////////////////////////////////////////////////////////////
  // Writes an array of values into the netCDF variable.
  /*!

    Writes an array of values into the netCDF variable.  The portion of the
    netCDF variable to write is specified by giving a corner and a vector of
    edge lengths that refer to an array section of the netCDF variable. The
    values to be written are associated with the netCDF variable by assuming
    that the last dimension of the netCDF variable varies fastest.

    \param startp
    Vector specifying the index where the first data values will
    be written.  The indices are relative to 0, so for example, the first data
    value of a variable would have index (0, 0, ... , 0). The elements of start
    correspond, in order, to the variable's dimensions. Hence, if the variable
    is a record variable, the first index corresponds to the starting record
    number for writing the data values.

    \param countp
    Vector specifying the number of indices selected along each
    dimension.  To write a single value, for example, specify count as (1, 1,
    ... , 1). The elements of count correspond, in order, to the variable's
    dimensions. Hence, if the variable is a record variable, the first element
    of count corresponds to a count of the number of records to write. Note:
    setting any element of the count array to zero causes the function to exit
    without error, and without doing anything.

    \param dataValues
    The data values. The order in which the data will be
    written to the netCDF variable is with the last dimension of the specified
    variable varying fastest. If the type of data values differs from the netCDF
    variable type, type conversion will occur. (However, no type conversion is
    carried out for variables using the user-defined data types: nc_Vlen,
    nc_Opaque, nc_Compound and nc_Enum.)

  */
  /*! \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const char** dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const char* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const unsigned char* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const signed char* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const short* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const int* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const long* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const float* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const double* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const unsigned short* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const unsigned int* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const unsigned long long* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const long long* dataValues) const;
  /*!
    This is an overloaded member function, provided for convenience.
    It differs from the above function in what argument(s) it accepts.
    In addition, no data conversion is carried out. This means that
    the type of the data in memory must match the type of the variable.
  */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const void* dataValues) const;
  
  
  //////////////////////////////////////////////////////////////////////////////
  // Writes a set of subsampled array values into the netCDF variable.
  /*!
    Writes an array of values into the netCDF variable.  The subsampled array
    section is specified by giving a corner, a vector of counts, and a stride
    vector.

    \param startp
    Vector specifying the index where the first data values will
    be written.  The indices are relative to 0, so for example, the first data
    value of a variable would have index (0, 0, ... , 0). The elements of start
    correspond, in order, to the variable's dimensions. Hence, if the variable
    is a record variable, the first index corresponds to the starting record
    number for writing the data values.

    \param countp
    Vector specifying the number of indices selected along each
    dimension.  To write a single value, for example, specify count as (1, 1,
    ... , 1). The elements of count correspond, in order, to the variable's
    dimensions. Hence, if the variable is a record variable, the first element
    of count corresponds to a count of the number of records to write. Note:
    setting any element of the count array to zero causes the function to exit
    without error, and without doing anything.

    \param stridep
    A vector of ptrdiff_t integers that specifies the sampling
    interval along each dimension of the netCDF variable.  The elements of the
    stride vector correspond, in order, to the netCDF variable's dimensions
    (stride[0] gives the sampling interval along the most slowly varying
    dimension of the netCDF variable). Sampling intervals are specified in
    type-independent units of elements (a value of 1 selects consecutive
    elements of the netCDF variable along the corresponding dimension, a value
    of 2 selects every other element, etc.). A NULL stride argument is treated
    as (1, 1, ... , 1).

    \param dataValues
    The data values. The order in which the data will be
    written to the netCDF variable is with the last dimension of the specified
    variable varying fastest. If the type of data values differs from the netCDF
    variable type, type conversion will occur.  (However, no type conversion is
    carried out for variables using the user-defined data types: nc_Vlen,
    nc_Opaque, nc_Compound and nc_Enum.
  */

  /*! \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const char** dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const char* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const unsigned char* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const signed char* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const short* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const int* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const long* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const float* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const double* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const unsigned short* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const unsigned int* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const unsigned long long* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const long long* dataValues) const;
  /*!
    This is an overloaded member function, provided for convenience.
    It differs from the above function in what argument(s) it accepts.
    In addition, no data conversion is carried out. This means that
    the type of the data in memory must match the type of the variable.
  */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const void* dataValues) const;

  //////////////////////////////////////////////////////////////////////////////
  // Writes a mapped array section of values into the netCDF variable.
  /*!

    Writes a mapped array section of values into the netCDF variable.  The
    mapped array section is specified by giving a corner, a vector of counts, a
    stride vector, and an index mapping vector.  The index mapping vector is a
    vector of integers that specifies the mapping between the dimensions of a
    netCDF variable and the in-memory structure of the internal data array.  No
    assumptions are made about the ordering or length of the dimensions of the
    data array.

    \param countp
    Vector specifying the number of indices selected along each
    dimension.  To write a single value, for example, specify count as (1, 1,
    ... , 1). The elements of count correspond, in order, to the variable's
    dimensions. Hence, if the variable is a record variable, the first element
    of count corresponds to a count of the number of records to write. Note:
    setting any element of the count array to zero causes the function to exit
    without error, and without doing anything.

    \param stridep
    A vector of ptrdiff_t integers that specifies the sampling
    interval along each dimension of the netCDF variable.  The elements of the
    stride vector correspond, in order, to the netCDF variable's dimensions
    (stride[0] gives the sampling interval along the most slowly varying
    dimension of the netCDF variable). Sampling intervals are specified in
    type-independent units of elements (a value of 1 selects consecutive
    elements of the netCDF variable along the corresponding dimension, a value
    of 2 selects every other element, etc.). A NULL stride argument is treated
    as (1, 1, ... , 1).

    \param imap
    Vector specifies the mapping between the dimensions of a netCDF
    variable and the in-memory structure of the internal data array.  The
    elements of the index mapping vector correspond, in order, to the netCDF
    variable's dimensions (imap[0] gives the distance between elements of the
    internal array corresponding to the most slowly varying dimension of the
    netCDF variable). Distances between elements are specified in
    type-independent units of elements (the distance between internal elements
    that occupy adjacent memory locations is 1 and not the element's byte-length
    as in netCDF 2). A NULL argument means the memory-resident values have the
    same structure as the associated netCDF variable.

    \param dataValues
    The data values. The order in which the data will be
    written to the netCDF variable is with the last dimension of the specified
    variable varying fastest. If the type of data values differs from the netCDF
    variable type, type conversion will occur.  (However, no type conversion is
    carried out for variables using the user-defined data types: nc_Vlen,
    nc_Opaque, nc_Compound and nc_Enum.)
  */

  /*! \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const std::vector<ptrdiff_t>& imapp, 
              const char** dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const std::vector<ptrdiff_t>& imapp, 
              const char* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const std::vector<ptrdiff_t>& imapp, 
              const unsigned char* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const std::vector<ptrdiff_t>& imapp, 
              const signed char* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const std::vector<ptrdiff_t>& imapp, 
              const short* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const std::vector<ptrdiff_t>& imapp, 
              const int* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const std::vector<ptrdiff_t>& imapp, 
              const long* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const std::vector<ptrdiff_t>& imapp, 
              const float* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const std::vector<ptrdiff_t>& imapp, 
              const double* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const std::vector<ptrdiff_t>& imapp, 
              const unsigned short* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const std::vector<ptrdiff_t>& imapp, 
              const unsigned int* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const std::vector<ptrdiff_t>& imapp, 
              const unsigned long long* dataValues) const;
  /*!  \overload
   */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const std::vector<ptrdiff_t>& imapp, 
              const long long* dataValues) const;
  /*!
    This is an overloaded member function, provided for convenience.
    It differs from the above function in what argument(s) it accepts.
    In addition, no data conversion is carried out. This means that
    the type of the data in memory must match the type of the variable.
  */
  void putVal(const std::vector<size_t>& startp, 
              const std::vector<size_t>& countp, 
              const std::vector<ptrdiff_t>& stridep, 
              const std::vector<ptrdiff_t>& imapp, 
              const void* dataValues) const;

  //////////////////////////////////////////////////////////////////////////
  /// add scalar attribute of various types
  /// throws NcxxException on error
    
  void addScalarAttr(const string &name, const string &val);
  void addScalarAttr(const string &name, signed char val);
  void addScalarAttr(const string &name, unsigned char val);
  void addScalarAttr(const string &name, short val);
  void addScalarAttr(const string &name, int val);
  void addScalarAttr(const string &name, int64_t val);
  void addScalarAttr(const string &name, float val);
  void addScalarAttr(const string &name, double val);

  /// get the total number of values in a variable
  /// this is the product of the dimension sizes
  /// and is 1 for a scalar (i.e. no dimensions)
    
  int64_t numVals();
  
  ///////////////////////////////////////////////////////////////////////////
  /// write a scalar variables
  /// throws NcxxException on error
  
  void write(double val);
  void write(float val);
  void write(int val);

  /// write a 1-D vector variable
  /// number of elements specified in dimension
  /// throws NcxxException on error

  void write(const NcxxDim &dim, const void *data);
  
  /// write a 1-D vector variable
  /// number of elements specified in arguments
  /// throws NcxxException on error
    
  void write(const NcxxDim &dim,
             size_t count, 
             const void *data);

  ///////////////////////////////////////////////////////////////////////////
  /// write a string variable
  /// throws NcxxException on error
  
  void writeStrings(const void *str);
  
  ////////////////////////////////////////
  // set default fill value, based on type
  // throws NcxxException on error
    
  void setDefaultFillValue();
      
  ////////////////////////////////////////
  // set meta fill value, based on type
  // throws NcxxException on error
  
  void setMetaFillValue();

  ////////////////////////////////////////
  // convert var type string
    
  string varTypeToStr() const;

  ////////////////////////////////////////
  // create the description string
  
  string getDesc() const;
      
private:

  bool nullObject;

  int myId;

  int groupId;

};

#endif
