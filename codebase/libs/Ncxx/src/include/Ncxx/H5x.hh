/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

///////////////////////////////////////////////////////////////////////////////
// H5x classes are copies of H5 classes in HDF5 source distribution
// This code was copied into Ncxx because some LINUX distribtions
// do not properly support the C++ versions of HDF5.
// See libs/Ncxx/src/Hdf5/COPYING
///////////////////////////////////////////////////////////////////////////////

#ifndef __H5x_HH
#define __H5x_HH

#include <hdf5.h>
#include <string>

// These are defined in H5Opkg.h, which should not be included in the C++ API,
// so re-define them here for now.

/* Initial version of the object header format */
#define H5O_VERSION_1    1

/* Revised version - leaves out reserved bytes and alignment padding, and adds
 *      magic number as prefix and checksum as suffix for all chunks.
 */
#define H5O_VERSION_2    2

// start of namespace

namespace H5x {

#define H5std_string std::string

  ///////////////////////////////////////////////////////////////////////////
  // Classes forward declaration
  ///////////////////////////////////////////////////////////////////////////

  class Exception;
  class IdComponent;
  class H5Location;
  class H5Object;
  class PropList;
  class FileCreatPropList;
  class FileAccPropList;
  class LinkAccPropList;
  class DSetCreatPropList;
  class DSetMemXferPropList;
  class DTypePropList;
  class DataType;
  class DataSpace;
  class AtomType;
  class PredType;
  class IntType;
  class FloatType;
  class StrType;
  class EnumType;
  class CompType;
  class AbstractDs;
  class DataSet;
  class Group;
  class H5File;
  class Attribute;
  class H5Library;
  class ArrayType;
  class VarLenType;

  ///////////////////////////////////////////////////////////////////////  
  ///////////////////////////////////////////////////////////////////////  
  /*! \class Exception
    \brief Exception provides wrappers of HDF5 error handling functions.
  
    Many classes are derived from Exception for specific HDF5 C interfaces.
  */

  class H5_DLLCPP Exception {
  public:
  // Creates an exception with a function name where the failure occurs
  // and an optional detailed message
  Exception(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);

  // Returns a character string that describes the error specified by
  // a major error number.
  H5std_string getMajorString(hid_t err_major_id) const;

  // Returns a character string that describes the error specified by
  // a minor error number.
  H5std_string getMinorString(hid_t err_minor_id) const;

  // Returns the detailed message set at the time the exception is thrown
  H5std_string getDetailMsg() const;
  const char* getCDetailMsg() const;   // C string of detailed message
  H5std_string getFuncName() const;    // function name as a string object
  const char* getCFuncName() const;    // function name as a char string

  // Turns on the automatic error printing.
  static void setAutoPrint(H5E_auto2_t& func, void* client_data);

  // Turns off the automatic error printing.
  static void dontPrint();

  // Set the automatic error printing to the default.
  // i.e. use H5Eprint2() to print to stderr.
  static void defaultPrint();

  // Retrieves the current settings for the automatic error stack
  // traversal function and its data.
  static void getAutoPrint(H5E_auto2_t& func, void** client_data);

  // Clears the error stack for the current thread.
  static void clearErrorStack();

  // Walks the error stack for the current thread, calling the
  // specified function.
  static void walkErrorStack(H5E_direction_t direction,
                             H5E_walk2_t func, void* client_data);

  // Prints the error stack in a default manner.
  static void printErrorStack(FILE* stream = stderr,
                              hid_t err_stack = H5E_DEFAULT);
  // Deprecated in favor of printErrorStack.
  // Removed from code. -BMR, 2017/08/11 1.8.20 and 1.10.2
  // virtual void printError(FILE* stream = NULL) const;

  // Default constructor
  Exception();

  // copy constructor
  Exception(const Exception& orig);

  // virtual Destructor
  virtual ~Exception() throw();

  protected:
  // Default value for detail_message
  static const char DEFAULT_MSG[];

  private:
  H5std_string detail_message;
  H5std_string func_name;
  };

  class H5_DLLCPP FileIException : public Exception {
  public:
    FileIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
    FileIException();
    virtual ~FileIException() throw();
  };

  class H5_DLLCPP GroupIException : public Exception {
  public:
    GroupIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
    GroupIException();
    virtual ~GroupIException() throw();
  };

  class H5_DLLCPP DataSpaceIException : public Exception {
  public:
    DataSpaceIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
    DataSpaceIException();
    virtual ~DataSpaceIException() throw();
  };

  class H5_DLLCPP DataTypeIException : public Exception {
  public:
    DataTypeIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
    DataTypeIException();
    virtual ~DataTypeIException() throw();
  };

  class H5_DLLCPP ObjHeaderIException : public Exception {
  public:
    ObjHeaderIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
    ObjHeaderIException();
    virtual ~ObjHeaderIException() throw();
  };

  class H5_DLLCPP PropListIException : public Exception {
  public:
    PropListIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
    PropListIException();
    virtual ~PropListIException() throw();
  };

  class H5_DLLCPP DataSetIException : public Exception {
  public:
    DataSetIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
    DataSetIException();
    virtual ~DataSetIException() throw();
  };

  class H5_DLLCPP AttributeIException : public Exception {
  public:
    AttributeIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
    AttributeIException();
    virtual ~AttributeIException() throw();
  };

  class H5_DLLCPP ReferenceException : public Exception {
  public:
    ReferenceException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
    ReferenceException();
    virtual ~ReferenceException() throw();
  };

  class H5_DLLCPP LibraryIException : public Exception {
  public:
    LibraryIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
    LibraryIException();
    virtual ~LibraryIException() throw();
  };

  class H5_DLLCPP LocationException : public Exception {
  public:
    LocationException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
    LocationException();
    virtual ~LocationException() throw();
  };

  class H5_DLLCPP IdComponentException : public Exception {
  public:
    IdComponentException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
    IdComponentException();
    virtual ~IdComponentException() throw();

  }; // end of IdComponentException


  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class IdComponent
    \brief Class IdComponent provides wrappers of the C functions that
    operate on an HDF5 identifier.

    In most cases, the C library handles these operations and an application
    rarely needs them.
  */

  class H5_DLLCPP IdComponent {
  public:
  
  // Increment reference counter.
  void incRefCount(const hid_t obj_id) const;
  void incRefCount() const;

  // Decrement reference counter.
  void decRefCount(const hid_t obj_id) const;
  void decRefCount() const;

  // Get the reference counter to this identifier.
  int getCounter(const hid_t obj_id) const;
  int getCounter() const;

  // Returns an HDF5 object type, given the object id.
  static H5I_type_t getHDFObjType(const hid_t obj_id);

  // Returns an HDF5 object type of this object.
  H5I_type_t getHDFObjType() const;

  // Returns the number of members in a type.
  static hsize_t getNumMembers(H5I_type_t type);

  // Checks if the given ID is valid.
  static bool isValid(hid_t an_id);

  // Determines if an type exists.
  static bool typeExists(H5I_type_t type);

  // Assignment operator.
  IdComponent& operator=(const IdComponent& rhs);

  // Sets the identifier of this object to a new value.
  void setId(const hid_t new_id);

#ifndef DOXYGEN_SHOULD_SKIP_THIS

  // Gets the identifier of this object.
  virtual hid_t getId () const = 0;

  // Pure virtual function for there are various H5*close for the
  // subclasses.
  virtual void close() = 0;

  // Makes and returns the string "<class-name>::<func_name>";
  // <class-name> is returned by fromClass().
  H5std_string inMemFunc(const char* func_name) const;

  ///\brief Returns this class name.
  virtual H5std_string fromClass() const { return("IdComponent");}

#endif // DOXYGEN_SHOULD_SKIP_THIS

  // Destructor
  virtual ~IdComponent();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

  protected:
  // Default constructor.
  IdComponent();

  // Gets the name of the file, in which an HDF5 object belongs.
  H5std_string p_get_file_name() const;

  // Verifies that the given id is valid.
  static bool p_valid_id(const hid_t obj_id);

  // Sets the identifier of this object to a new value. - this one
  // doesn't increment reference count
  virtual void p_setId(const hid_t new_id) = 0;

  // This flag is used to decide whether H5dont_atexit should be called
  static bool H5dontAtexit_called;

  private:
  // This flag indicates whether H5Library::initH5cpp has been called
  // to register various terminating functions with atexit()
  static bool H5cppinit;

#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end class IdComponent

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class DataSpace
    \brief Class DataSpace inherits from IdComponent and provides wrappers for
    the HDF5's dataspaces.
  */
  //  Inheritance: IdComponent

  class H5_DLLCPP DataSpace : public IdComponent {
  public:
    ///\brief Default DataSpace objects
    static const DataSpace& ALL;

    // Creates a dataspace object given the space type
    DataSpace(H5S_class_t type = H5S_SCALAR);

    // Creates a simple dataspace
    DataSpace(int rank, const hsize_t * dims, const hsize_t * maxdims = NULL);

    // Creates a DataSpace object using an existing dataspace id.
    DataSpace(const hid_t space_id);

    // Copy constructor - same as the original DataSpace.
    DataSpace(const DataSpace& original);

    // Assignment operator
    DataSpace& operator=(const DataSpace& rhs);

    // Closes this dataspace.
    virtual void close();

    // Makes copy of an existing dataspace.
    void copy(const DataSpace& like_space);

    // Copies the extent of this dataspace.
    void extentCopy(const DataSpace& dest_space) const;
    // removed from 1.8.18 and 1.10.1
    //void extentCopy(DataSpace& dest_space) const;

    // Gets the bounding box containing the current selection.
    void getSelectBounds(hsize_t* start, hsize_t* end) const;

    // Gets the number of element points in the current selection.
    hssize_t getSelectElemNpoints() const;

    // Retrieves the list of element points currently selected.
    void getSelectElemPointlist(hsize_t startpoint, hsize_t numpoints, hsize_t *buf) const;

    // Gets the list of hyperslab blocks currently selected.
    void getSelectHyperBlocklist(hsize_t startblock, hsize_t numblocks, hsize_t *buf) const;

    // Get number of hyperslab blocks.
    hssize_t getSelectHyperNblocks() const;

    // Gets the number of elements in this dataspace selection.
    hssize_t getSelectNpoints() const;

    // Retrieves dataspace dimension size and maximum size.
    int getSimpleExtentDims(hsize_t *dims, hsize_t *maxdims = NULL) const;

    // Gets the dimensionality of this dataspace.
    int getSimpleExtentNdims() const;

    // Gets the number of elements in this dataspace.
    // 12/05/00 - changed return type to hssize_t from hsize_t - C API
    hssize_t getSimpleExtentNpoints() const;

    // Gets the current class of this dataspace.
    H5S_class_t getSimpleExtentType() const;

    // Determines if this dataspace is a simple one.
    bool isSimple() const;

    // Sets the offset of this simple dataspace.
    void offsetSimple(const hssize_t* offset) const;

    // Selects the entire dataspace.
    void selectAll() const;

    // Selects array elements to be included in the selection for
    // this dataspace.
    void selectElements(H5S_seloper_t op, const size_t num_elements, const hsize_t *coord) const;

    // Selects a hyperslab region to add to the current selected region.
    void selectHyperslab(H5S_seloper_t op, const hsize_t *count, const hsize_t *start, const hsize_t *stride = NULL, const hsize_t *block = NULL) const;

    // Resets the selection region to include no elements.
    void selectNone() const;

    // Verifies that the selection is within the extent of the dataspace.
    bool selectValid() const;

    // Removes the extent from this dataspace.
    void setExtentNone() const;

    // Sets or resets the size of this dataspace.
    void setExtentSimple(int rank, const hsize_t *current_size, const hsize_t *maximum_size = NULL) const;

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("DataSpace"); }

    // Gets the dataspace id.
    virtual hid_t getId() const;

    // Deletes the global constant
    static void deleteConstants();

    // Destructor: properly terminates access to this dataspace.
    virtual ~DataSpace();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

  protected:
    // Sets the dataspace id.
    virtual void p_setId(const hid_t new_id);

#endif // DOXYGEN_SHOULD_SKIP_THIS

  private:
    hid_t id;       // HDF5 dataspace id

#ifndef DOXYGEN_SHOULD_SKIP_THIS

    static DataSpace* ALL_;

    // Creates the global constant
    static DataSpace* getConstant();

    // Friend function to set DataSpace id.  For library use only.
    friend void f_DataSpace_setId(DataSpace *dspace, hid_t new_id);

#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of DataSpace

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class PropList
    \brief Class PropList inherits from IdComponent and provides wrappers for
    the HDF5 generic property list.
  */
  //  Inheritance: IdComponent

  class H5_DLLCPP PropList : public IdComponent {
  public:
    ///\brief Default property list
    static const PropList& DEFAULT;

    // Creates a property list of a given type or creates a copy of an
    // existing property list giving the property list id.
    PropList(const hid_t plist_id);

    // Make a copy of the given property list using assignment statement
    PropList& operator=(const PropList& rhs);

    // Compares this property list or class against the given list or class.
    bool operator==(const PropList& rhs) const;

    // Close this property list.
    virtual void close();

    // Close a property list class.
    void closeClass() const;

    // Makes a copy of the given property list.
    void copy(const PropList& like_plist);

    // Copies a property from this property list or class to another
    void copyProp(PropList& dest, const char* name) const;
    void copyProp(PropList& dest, const H5std_string& name) const;

    // Copies a property from one property list or property class to another
    void copyProp(PropList& dest, PropList& src, const char* name) const;
    void copyProp(PropList& dest, PropList& src, const H5std_string& name) const;

    // Gets the class of this property list, i.e. H5P_FILE_CREATE,
    // H5P_FILE_ACCESS, ...
    hid_t getClass() const;

    // Return the name of a generic property list class.
    H5std_string getClassName() const;

    // Returns the parent class of a generic property class.
    PropList getClassParent() const;

    // Returns the number of properties in this property list or class.
    size_t getNumProps() const;

    // Query the value of a property in a property list.
    void getProperty(const char* name, void* value) const;
    void getProperty(const H5std_string& name, void* value) const;
    H5std_string getProperty(const char* name) const;
    H5std_string getProperty(const H5std_string& name) const;

    // Set a property's value in a property list.
    void setProperty(const char* name, const char* charptr) const;
    void setProperty(const char* name, const void* value) const;
    void setProperty(const char* name, const H5std_string& strg) const;
    void setProperty(const H5std_string& name, const void* value) const;
    void setProperty(const H5std_string& name, const H5std_string& strg) const;
    // Deprecated after 1.10.1, missing const
    void setProperty(const char* name, void* value) const;
    void setProperty(const char* name, H5std_string& strg) const;
    void setProperty(const H5std_string& name, void* value) const;
    void setProperty(const H5std_string& name, H5std_string& strg) const;

    // Query the size of a property in a property list or class.
    size_t getPropSize(const char *name) const;
    size_t getPropSize(const H5std_string& name) const;

    // Determines whether a property list is a certain class.
    bool isAClass(const PropList& prop_class) const;

    /// Query the existence of a property in a property object.
    bool propExist(const char* name) const;
    bool propExist(const H5std_string& name) const;

    // Removes a property from a property list.
    void removeProp(const char *name) const;
    void removeProp(const H5std_string& name) const;

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("PropList"); }

    // Default constructor: creates a stub PropList object.
    PropList();

    // Copy constructor: same as the original PropList.
    PropList(const PropList& original);

    // Gets the property list id.
    virtual hid_t getId() const;

    // Destructor: properly terminates access to this property list.
    virtual ~PropList();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

    // Deletes the PropList global constant
    static void deleteConstants();

  protected:
    hid_t id;    // HDF5 property list id

    // Sets the property list id.
    virtual void p_setId(const hid_t new_id);

  private:
    static PropList* DEFAULT_;

    // Dynamically allocates the PropList global constant
    static PropList* getConstant();

    // Friend function to set PropList id.  For library use only.
    friend void f_PropList_setId(PropList* plist, hid_t new_id);

#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of PropList

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class FileAccPropList
    \brief Class FileAccPropList inherits from PropList and provides
    wrappers for the HDF5 file access property list.
  */
  //  Inheritance: PropList -> IdComponent

  class H5_DLLCPP FileAccPropList : public PropList {
  public:
    ///\brief Default file access property list.
    static const FileAccPropList& DEFAULT;

    // Creates a file access property list.
    FileAccPropList();

    // Modifies this property list to use the H5FD_STDIO driver
    void setStdio() const;

    // Set file driver for this property list
    void setDriver(hid_t new_driver_id, const void *new_driver_info) const;

    // Returns a low-level file driver identifier.
    hid_t getDriver() const;

    // Sets offset for family driver.
    void setFamilyOffset(hsize_t offset) const;

    // Gets offset for family driver.
    hsize_t getFamilyOffset() const;

    // Modifies this file access property list to use the sec2 driver.
    void setSec2() const;

    // Modifies this file access property list to use the H5FD_CORE
    // driver.
    void setCore (size_t increment, hbool_t backing_store) const;

    // Queries H5FD_CORE driver properties.
    void getCore (size_t& increment, hbool_t& backing_store) const;

    // Sets this file access properties list to the family driver.
    void setFamily(hsize_t memb_size, const FileAccPropList& memb_plist) const;

    // Returns information about the family file access property list.
    void getFamily(hsize_t& memb_size, FileAccPropList& memb_plist) const;
    FileAccPropList getFamily(hsize_t& memb_size) const;

    // Emulates the old split file driver,
    void setSplit(const FileAccPropList& meta_plist,
                  const FileAccPropList& raw_plist,
                  const char* meta_ext = ".meta",
                  const char* raw_ext = ".raw") const;
    void setSplit(const FileAccPropList& meta_plist,
                  const FileAccPropList& raw_plist,
                  const H5std_string& meta_ext = ".meta",
                  const H5std_string& raw_ext = ".raw") const;

    // Sets the maximum size of the data sieve buffer.
    void setSieveBufSize(size_t bufsize) const;

    // Returns the current settings for the data sieve buffer size
    // property
    size_t getSieveBufSize() const;

    // Sets the minimum size of metadata block allocations.
    void setMetaBlockSize(hsize_t &block_size) const;

    // Returns the current metadata block size setting.
    hsize_t getMetaBlockSize() const;

    // Modifies this file access property list to use the logging driver.
    void setLog(const char *logfile, unsigned flags, size_t buf_size) const;
    void setLog(const H5std_string& logfile, unsigned flags, size_t buf_size) const;

    // Sets alignment properties of this file access property list
    void setAlignment(hsize_t threshold = 1, hsize_t alignment = 1) const;

    // Retrieves the current settings for alignment properties from
    // this property list.
    void getAlignment(hsize_t& threshold, hsize_t& alignment) const;

    // Sets data type for multi driver.
    void setMultiType(H5FD_mem_t dtype) const;

    // Returns the data type property for MULTI driver.
    H5FD_mem_t getMultiType() const;

    // Sets the meta data cache and raw data chunk cache parameters.
    void setCache(int mdc_nelmts, size_t rdcc_nelmts, size_t rdcc_nbytes, double rdcc_w0) const;

    // Queries the meta data cache and raw data chunk cache parameters.
    void getCache(int& mdc_nelmts, size_t& rdcc_nelmts, size_t& rdcc_nbytes, double& rdcc_w0) const;

    // Sets the degree for the file close behavior.
    void setFcloseDegree(H5F_close_degree_t degree) const;

    // Returns the degree for the file close behavior.
    H5F_close_degree_t getFcloseDegree() const;

    // Sets file access property list to use the H5FD_DIRECT driver.
    void setFileAccDirect(size_t boundary, size_t block_size, size_t cbuf_size) const;

    // Retrieves information about the direct file access property list.
    void getFileAccDirect(size_t &boundary, size_t &block_size, size_t &cbuf_size) const;

    // Sets garbage collecting references flag.
    void setGcReferences(unsigned gc_ref = 0) const;

    // Returns garbage collecting references setting.
    unsigned getGcReferences() const;

    // Sets bounds on versions of library format to be used when creating
    // or writing objects.
    void setLibverBounds(H5F_libver_t libver_low, H5F_libver_t libver_high) const;

    // Gets the current settings for the library version format bounds.
    void getLibverBounds(H5F_libver_t& libver_low, H5F_libver_t& libver_high) const;

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("FileAccPropList"); }

    // Copy constructor: same as the original FileAccPropList.
    FileAccPropList(const FileAccPropList& original);

    // Creates a copy of an existing file access property list
    // using the property list id.
    FileAccPropList (const hid_t plist_id);

    // Noop destructor
    virtual ~FileAccPropList();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

    // Deletes the global constant, should only be used by the library
    static void deleteConstants();

  private:
    static FileAccPropList* DEFAULT_;

    // Creates the global constant, should only be used by the library
    static FileAccPropList* getConstant();

#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of FileAccPropList

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class FileCreatPropList
    \brief Class FileCreatPropList inherits from PropList and provides
    wrappers for the HDF5 file create property list.
  */
  //  Inheritance: PropList -> IdComponent

  class H5_DLLCPP FileCreatPropList : public PropList {
  public:
    ///\brief Default file creation property list.
    static const FileCreatPropList& DEFAULT;

    // Creates a file create property list.
    FileCreatPropList();

#ifndef H5_NO_DEPRECATED_SYMBOLS
    // Retrieves version information for various parts of a file.
    void getVersion(unsigned& super, unsigned& freelist, unsigned& stab, unsigned& shhdr) const;
#endif /* H5_NO_DEPRECATED_SYMBOLS */

    // Sets the userblock size field of a file creation property list.
    void setUserblock(hsize_t size) const;

    // Gets the size of a user block in this file creation property list.
    hsize_t getUserblock() const;

    // Retrieves the size-of address and size quantities stored in a
    // file according to this file creation property list.
    void getSizes(size_t& sizeof_addr, size_t& sizeof_size) const;

    // Sets file size-of addresses and sizes.
    void setSizes(size_t sizeof_addr = 4, size_t sizeof_size = 4) const;

    // Retrieves the size of the symbol table B-tree 1/2 rank and the
    // symbol table leaf node 1/2 size.
    void getSymk(unsigned& int_nodes_k, unsigned& leaf_nodes_k) const;

    // Sets the size of parameters used to control the symbol table nodes.
    void setSymk(unsigned int_nodes_k, unsigned leaf_nodes_k) const;

    // Returns the 1/2 rank of an indexed storage B-tree.
    unsigned getIstorek() const;

    // Sets the size of parameter used to control the B-trees for
    // indexing chunked datasets.
    void setIstorek(unsigned ik) const;

#ifdef HDF5_V10
    // Sets the strategy and the threshold value that the library will
    // will employ in managing file space.
    void setFileSpaceStrategy(H5F_fspace_strategy_t strategy, hbool_t persist, hsize_t threshold) const;

    // Returns the strategy that the library uses in managing file space.
    void getFileSpaceStrategy(H5F_fspace_strategy_t& strategy, hbool_t& persist, hsize_t& threshold) const;

    // Sets the file space page size for paged aggregation.
    void setFileSpacePagesize(hsize_t fsp_psize) const;

    // Returns the threshold value that the library uses in tracking free
    // space sections.
    hsize_t getFileSpacePagesize() const;
#endif

    ///\brief Returns this class name.
    virtual H5std_string fromClass() const { return("FileCreatPropList"); }

    // Copy constructor: same as the original FileCreatPropList.
    FileCreatPropList(const FileCreatPropList& orig);

    // Creates a copy of an existing file create property list
    // using the property list id.
    FileCreatPropList(const hid_t plist_id);

    // Noop destructor
    virtual ~FileCreatPropList();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

    // Deletes the global constant, should only be used by the library
    static void deleteConstants();

  private:
    static FileCreatPropList* DEFAULT_;

    // Creates the global constant, should only be used by the library
    static FileCreatPropList* getConstant();

#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of FileCreatPropList

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class ObjCreatPropList
    \brief Class ObjCreatPropList inherits from PropList and provides
    wrappers for the HDF5 object create property list.
  */
  //  Inheritance: PropList -> IdComponent

  class H5_DLLCPP ObjCreatPropList : public PropList {
  public:
    ///\brief Default object creation property list.
    static const ObjCreatPropList& DEFAULT;

    // Creates a object creation property list.
    ObjCreatPropList();

    // Sets attribute storage phase change thresholds.
    void setAttrPhaseChange(unsigned max_compact = 8, unsigned min_dense = 6) const;

    // Gets attribute storage phase change thresholds.
    void getAttrPhaseChange(unsigned& max_compact, unsigned& min_dense) const;

    // Sets tracking and indexing of attribute creation order.
    void setAttrCrtOrder(unsigned crt_order_flags) const;

    // Gets tracking and indexing settings for attribute creation order.
    unsigned getAttrCrtOrder() const;


    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("ObjCreatPropList"); }

    // Copy constructor: same as the original ObjCreatPropList.
    ObjCreatPropList(const ObjCreatPropList& original);

    // Creates a copy of an existing object creation property list
    // using the property list id.
    ObjCreatPropList (const hid_t plist_id);

    // Noop destructor
    virtual ~ObjCreatPropList();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

    // Deletes the global constant, should only be used by the library
    static void deleteConstants();

  private:
    static ObjCreatPropList* DEFAULT_;

    // Creates the global constant, should only be used by the library
    static ObjCreatPropList* getConstant();

#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of ObjCreatPropList

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class DSetCreatPropList
    \brief Class DSetCreatPropList inherits from ObjCreatPropList and provides
    wrappers for the HDF5 dataset creation property functions.
  */
  //  Inheritance: ObjCreatPropList -> PropList -> IdComponent

  class H5_DLLCPP DSetCreatPropList : public ObjCreatPropList {
  public:
    ///\brief Default dataset creation property list.
    static const DSetCreatPropList& DEFAULT;

    // Creates a dataset creation property list.
    DSetCreatPropList();

    // Queries whether all the filters set in this property list are
    // available currently.
    bool allFiltersAvail() const;

    // Get space allocation time for this property.
    H5D_alloc_time_t getAllocTime() const;

    // Set space allocation time for dataset during creation.
    void setAllocTime(H5D_alloc_time_t alloc_time) const;

    // Retrieves the size of the chunks used to store a chunked layout dataset.
    int getChunk(int max_ndims, hsize_t* dim) const;

    // Sets the size of the chunks used to store a chunked layout dataset.
    void setChunk(int ndims, const hsize_t* dim) const;

    // Returns information about an external file.
    void getExternal(unsigned idx, size_t name_size, char* name, off_t& offset, hsize_t& size) const;

    // Returns the number of external files for a dataset.
    int getExternalCount() const;

    // Gets fill value writing time.
    H5D_fill_time_t getFillTime() const;

    // Sets fill value writing time for dataset.
    void setFillTime(H5D_fill_time_t fill_time) const;

    // Retrieves a dataset fill value.
    void getFillValue(const DataType& fvalue_type, void* value) const;

    // Sets a dataset fill value.
    void setFillValue(const DataType& fvalue_type, const void* value) const;

    // Returns information about a filter in a pipeline.
    H5Z_filter_t getFilter(int filter_number, unsigned int& flags, size_t& cd_nelmts, unsigned int* cd_values, size_t namelen, char name[], unsigned int &filter_config) const;

    // Returns information about a filter in a pipeline given the filter id.
    void getFilterById(H5Z_filter_t filter_id, unsigned int &flags, size_t &cd_nelmts, unsigned int* cd_values, size_t namelen, char name[], unsigned int &filter_config) const;

    // Gets the layout of the raw data storage of the data that uses this
    // property list.
    H5D_layout_t getLayout() const;

    // Sets the type of storage used to store the raw data for the
    // dataset that uses this property list.
    void setLayout(H5D_layout_t layout) const;

    // Returns the number of filters in the pipeline.
    int getNfilters() const;

    // Checks if fill value has been defined for this property.
    H5D_fill_value_t isFillValueDefined() const;

    // Modifies the specified filter.
    void modifyFilter(H5Z_filter_t filter_id, unsigned int flags, size_t cd_nelmts, const unsigned int cd_values[]) const;

    // Remove one or all filters from the filter pipeline.
    void removeFilter(H5Z_filter_t filter_id) const;

    // Sets compression method and compression level.
    void setDeflate(int level) const;

    // Adds an external file to the list of external files.
    void setExternal(const char* name, off_t offset, hsize_t size) const;

    // Adds a filter to the filter pipeline.
    void setFilter(H5Z_filter_t filter, unsigned int flags = 0, size_t cd_nelmts = 0, const unsigned int cd_values[] = NULL) const;

    // Sets Fletcher32 checksum of EDC for this property list.
    void setFletcher32() const;

    // Sets method of the shuffle filter.
    void setShuffle() const;

    // Sets SZIP compression method.
    void setSzip(unsigned int options_mask, unsigned int pixels_per_block) const;

    // Sets N-bit compression method.
    void setNbit() const;

#ifdef HDF5_V10
    // Maps elements of a virtual dataset to elements of the source dataset.
    void setVirtual(const DataSpace& vspace, const char *src_fname, const char *src_dsname, const DataSpace& sspace) const;
    void setVirtual(const DataSpace& vspace, const H5std_string src_fname, const H5std_string src_dsname, const DataSpace& sspace) const;
#endif

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("DSetCreatPropList"); }

    // Copy constructor - same as the original DSetCreatPropList.
    DSetCreatPropList(const DSetCreatPropList& orig);

    // Creates a copy of an existing dataset creation property list
    // using the property list id.
    DSetCreatPropList(const hid_t plist_id);

    // Noop destructor.
    virtual ~DSetCreatPropList();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

    // Deletes the global constant, should only be used by the library
    static void deleteConstants();

  private:
    static DSetCreatPropList* DEFAULT_;

    // Creates the global constant, should only be used by the library
    static DSetCreatPropList* getConstant();

#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of DSetCreatPropList

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class DSetMemXferPropList
    \brief Class DSetCreatPropList inherits from PropList and provides
    wrappers for the HDF5 dataset memory and transfer property list.
  */
  //  Inheritance: PropList -> IdComponent

  class H5_DLLCPP DSetMemXferPropList : public PropList {
  public:
    ///\brief Default dataset memory and transfer property list.
    static const DSetMemXferPropList& DEFAULT;

    // Creates a dataset memory and transfer property list.
    DSetMemXferPropList();

    // Creates a dataset transform property list.
    DSetMemXferPropList(const char* expression);

    // Sets type conversion and background buffers.
    void setBuffer(size_t size, void* tconv, void* bkg) const;

    // Reads buffer settings.
    size_t getBuffer(void** tconv, void** bkg) const;

    // Sets B-tree split ratios for a dataset transfer property list.
    void setBtreeRatios(double left, double middle, double right) const;

    // Gets B-tree split ratios for a dataset transfer property list.
    void getBtreeRatios(double& left, double& middle, double& right) const;

    // Sets data transform expression.
    void setDataTransform(const char* expression) const;
    void setDataTransform(const H5std_string& expression) const;

    // Gets data transform expression.
    ssize_t getDataTransform(char* exp, size_t buf_size=0) const;
    H5std_string getDataTransform() const;

    // Sets the dataset transfer property list status to TRUE or FALSE.
    void setPreserve(bool status) const;

    // Checks status of the dataset transfer property list.
    bool getPreserve() const;

    // Sets an exception handling callback for datatype conversion.
    void setTypeConvCB(H5T_conv_except_func_t op, void *user_data) const;

    // Gets the exception handling callback for datatype conversion.
    void getTypeConvCB(H5T_conv_except_func_t *op, void **user_data) const;

    // Sets the memory manager for variable-length datatype
    // allocation in H5Dread and H5Treclaim.
    void setVlenMemManager(H5MM_allocate_t alloc, void* alloc_info,
                           H5MM_free_t free, void* free_info) const;

    // alloc and free are set to NULL, indicating that system
    // malloc and free are to be used.
    void setVlenMemManager() const;

    // Gets the memory manager for variable-length datatype
    // allocation in H5Dread and H5Treclaim.
    void getVlenMemManager(H5MM_allocate_t& alloc, void** alloc_info,
                           H5MM_free_t& free, void** free_info) const;

    // Sets the size of a contiguous block reserved for small data.
    void setSmallDataBlockSize(hsize_t size) const;

    // Returns the current small data block size setting.
    hsize_t getSmallDataBlockSize() const;

    // Sets number of I/O vectors to be read/written in hyperslab I/O.
    void setHyperVectorSize(size_t vector_size) const;

    // Returns the number of I/O vectors to be read/written in
    // hyperslab I/O.
    size_t getHyperVectorSize() const;

    // Enables or disables error-detecting for a dataset reading
    // process.
    void setEDCCheck(H5Z_EDC_t check) const;

    // Determines whether error-detection is enabled for dataset reads.
    H5Z_EDC_t getEDCCheck() const;

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("DSetMemXferPropList"); }

    // Copy constructor - same as the original DSetMemXferPropList.
    DSetMemXferPropList(const DSetMemXferPropList& orig);

    // Creates a copy of an existing dataset memory and transfer
    // property list using the property list id.
    DSetMemXferPropList(const hid_t plist_id);

    // Noop destructor
    virtual ~DSetMemXferPropList();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

    // Deletes the global constant, should only be used by the library
    static void deleteConstants();

  private:
    static DSetMemXferPropList* DEFAULT_;

    // Creates the global constant, should only be used by the library
    static DSetMemXferPropList* getConstant();

#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of DSetMemXferPropList

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class LinkCreatPropList
    \brief Class LinkCreatPropList inherits from PropList and provides
    wrappers for the HDF5 link creation property list.
  */
  // Inheritance: PropList -> IdComponent

  class H5_DLLCPP LinkCreatPropList : public PropList {
  public:
    ///\brief Default link creation property list.
    static const LinkCreatPropList& DEFAULT;

    // Creates a link creation property list.
    LinkCreatPropList();

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("LinkCreatPropList"); }

    // Copy constructor: same as the original LinkCreatPropList.
    LinkCreatPropList(const LinkCreatPropList& original);

    // Creates a copy of an existing link creation property list
    // using the property list id.
    LinkCreatPropList (const hid_t plist_id);

    // Specifies in property list whether to create missing
    // intermediate groups
    void setCreateIntermediateGroup(bool crt_intmd_group) const;

    // Determines whether property is set to enable creating missing
    // intermediate groups
    bool getCreateIntermediateGroup() const;

    // Sets the character encoding of the string.
    void setCharEncoding(H5T_cset_t encoding) const;

    // Gets the character encoding of the string.
    H5T_cset_t getCharEncoding() const;

    // Noop destructor
    virtual ~LinkCreatPropList();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

    // Deletes the global constant, should only be used by the library
    static void deleteConstants();

  private:
    static LinkCreatPropList* DEFAULT_;

    // Creates the global constant, should only be used by the library
    static LinkCreatPropList* getConstant();

#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of LinkCreatPropList

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class LinkAccPropList
    \brief Class LinkAccPropList inherits from PropList and provides
    wrappers for the HDF5 link access property list.
  */
  // Inheritance: PropList -> IdComponent

  class H5_DLLCPP LinkAccPropList : public PropList {
  public:
    ///\brief Default link access property list.
    static const LinkAccPropList& DEFAULT;

    // Creates a link access property list.
    LinkAccPropList();

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("LinkAccPropList"); }

    // Copy constructor: same as the original LinkAccPropList.
    LinkAccPropList(const LinkAccPropList& original);

    // Creates a copy of an existing link access property list
    // using the property list id.
    LinkAccPropList (const hid_t plist_id);

    // Sets the number of soft or user-defined links that can be
    // traversed before a failure occurs.
    void setNumLinks(size_t nlinks) const;

    // Gets the number of soft or user-defined link traversals allowed
    size_t getNumLinks() const;

    // Noop destructor
    virtual ~LinkAccPropList();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

    // Deletes the global constant, should only be used by the library
    static void deleteConstants();

  private:
    static LinkAccPropList* DEFAULT_;

    // Creates the global constant, should only be used by the library
    static LinkAccPropList* getConstant();

#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of LinkAccPropList

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class DSetAccPropList
    \brief Class DSetAccPropList inherits from LinkAccPropList and provides
    wrappers for the HDF5 dataset access property functions.
  */
  //  Inheritance: LinkAccPropList -> PropList -> IdComponent

  class H5_DLLCPP DSetAccPropList : public LinkAccPropList {
  public:
    ///\brief Default dataset creation property list.
    static const DSetAccPropList& DEFAULT;

    // Creates a dataset creation property list.
    DSetAccPropList();

    // Sets the raw data chunk cache parameters.
    void setChunkCache(size_t rdcc_nslots, size_t rdcc_nbytes, double rdcc_w0) const;

    // Retrieves the raw data chunk cache parameters.
    void getChunkCache(size_t &rdcc_nslots, size_t &rdcc_nbytes, double &rdcc_w0) const;

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("DSetAccPropList"); }

    // Copy constructor - same as the original DSetAccPropList.
    DSetAccPropList(const DSetAccPropList& orig);

    // Creates a copy of an existing dataset creation property list
    // using the property list id.
    DSetAccPropList(const hid_t plist_id);

    // Noop destructor.
    virtual ~DSetAccPropList();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

    // Deletes the global constant, should only be used by the library
    static void deleteConstants();

  private:
    static DSetAccPropList* DEFAULT_;

    // Creates the global constant, should only be used by the library
    static DSetAccPropList* getConstant();

#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of DSetAccPropList

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class H5Location
    \brief H5Location is an abstract base class, added in version 1.8.12.

    It provides a collection of wrappers for the C functions that take a
    location identifier to specify the HDF5 object.  The location identifier
    can be either file, group, dataset, attribute, or named datatype.
    Wrappers for H5A functions stay in H5Object.
  */
  // Inheritance: IdComponent

  class H5_DLLCPP H5Location : public IdComponent {
  public:
    // Checks if a link of a given name exists in a location
    bool nameExists(const char* name, const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    bool nameExists(const H5std_string& name, const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;

    // Checks if a link of a given name exists in a location
    // Deprecated in favor of nameExists for better name.
    bool exists(const char* name, const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    bool exists(const H5std_string& name, const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;

    // Flushes all buffers associated with this location to disk.
    void flush(H5F_scope_t scope) const;

    // Gets the name of the file, specified by this location.
    H5std_string getFileName() const;

#ifndef H5_NO_DEPRECATED_SYMBOLS
    // Retrieves the type of object that an object reference points to.
    H5G_obj_t getObjType(void *ref, H5R_type_t ref_type = H5R_OBJECT) const;
#endif /* H5_NO_DEPRECATED_SYMBOLS */

    // Retrieves the type of object that an object reference points to.
    H5O_type_t getRefObjType(void *ref, H5R_type_t ref_type = H5R_OBJECT) const;
    // Note: getRefObjType deprecates getObjType, but getObjType's name is
    // misleading, so getRefObjType is used in the new function instead.

    // Sets the comment for an HDF5 object specified by its name.
    void setComment(const char* name, const char* comment) const;
    void setComment(const H5std_string& name, const H5std_string& comment) const;
    void setComment(const char* comment) const;
    void setComment(const H5std_string& comment) const;

    // Retrieves comment for the HDF5 object specified by its name.
    ssize_t getComment(const char* name, size_t buf_size, char* comment) const;
    H5std_string getComment(const char* name, size_t buf_size=0) const;
    H5std_string getComment(const H5std_string& name, size_t buf_size=0) const;

    // Removes the comment for the HDF5 object specified by its name.
    void removeComment(const char* name) const;
    void removeComment(const H5std_string& name) const;

    // Creates a reference to a named object or to a dataset region
    // in this object.
    void reference(void* ref, const char* name, 
                   H5R_type_t ref_type = H5R_OBJECT) const;
    void reference(void* ref, const H5std_string& name,
                   H5R_type_t ref_type = H5R_OBJECT) const;
    void reference(void* ref, const char* name, const DataSpace& dataspace,
                   H5R_type_t ref_type = H5R_DATASET_REGION) const;
    void reference(void* ref, const H5std_string& name, const DataSpace& dataspace,
                   H5R_type_t ref_type = H5R_DATASET_REGION) const;

#ifdef HDF5_V10
    // Open a referenced object whose location is specified by either
    // a file, an HDF5 object, or an attribute.
    void dereference(const H5Location& loc, const void* ref, H5R_type_t ref_type = H5R_OBJECT, const PropList& plist = PropList::DEFAULT);
    // Removed in 1.10.1, because H5Location is baseclass
    //void dereference(const Attribute& attr, const void* ref, H5R_type_t ref_type = H5R_OBJECT, const PropList& plist = PropList::DEFAULT);
#endif


    // Retrieves a dataspace with the region pointed to selected.
    DataSpace getRegion(void *ref, H5R_type_t ref_type = H5R_DATASET_REGION) const;

    // Create a new group with using link create property list.
    Group createGroup(const char* name, const LinkCreatPropList& lcpl) const;
    Group createGroup(const H5std_string& name, const LinkCreatPropList& lcpl) const;

    // From CommonFG
    // Creates a new group at this location which can be a file
    // or another group.
    Group createGroup(const char* name, size_t size_hint = 0) const;
    Group createGroup(const H5std_string& name, size_t size_hint = 0) const;

    // Opens an existing group in a location which can be a file
    // or another group.
    Group openGroup(const char* name) const;
    Group openGroup(const H5std_string& name) const;

    // Creates a new dataset in this location.
    DataSet createDataSet(const char* name, const DataType& data_type, const DataSpace& data_space, const DSetCreatPropList& create_plist = DSetCreatPropList::DEFAULT, const DSetAccPropList& dapl = DSetAccPropList::DEFAULT, const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT) const;
    DataSet createDataSet(const H5std_string& name, const DataType& data_type, const DataSpace& data_space, const DSetCreatPropList& create_plist = DSetCreatPropList::DEFAULT, const DSetAccPropList& dapl = DSetAccPropList::DEFAULT, const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT) const;

    // Deprecated to add LinkCreatPropList and DSetAccPropList - 1.10.3
    // DataSet createDataSet(const char* name, const DataType& data_type, const DataSpace& data_space, const DSetCreatPropList& create_plist = DSetCreatPropList::DEFAULT) const;
    // DataSet createDataSet(const H5std_string& name, const DataType& data_type, const DataSpace& data_space, const DSetCreatPropList& create_plist = DSetCreatPropList::DEFAULT) const;

    // Opens an existing dataset at this location.
    // DSetAccPropList is added - 1.10.3
    DataSet openDataSet(const char* name, const DSetAccPropList& dapl = DSetAccPropList::DEFAULT) const;
    DataSet openDataSet(const H5std_string& name, const DSetAccPropList& dapl = DSetAccPropList::DEFAULT) const;

#ifdef HDF5_V10
    H5L_info2_t getLinkInfo(const char* link_name, const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    H5L_info2_t getLinkInfo(const H5std_string& link_name, const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;

    // Returns the value of a symbolic link.
    H5std_string getLinkval(const char* link_name, size_t size=0) const;
    H5std_string getLinkval(const H5std_string& link_name, size_t size=0) const;
#endif

    // Returns the number of objects in this group.
    // Deprecated - moved to H5::Group in 1.10.2.
    hsize_t getNumObjs() const;

    // Retrieves the name of an object in this group, given the
    // object's index.
    H5std_string getObjnameByIdx(hsize_t idx) const;
    ssize_t getObjnameByIdx(hsize_t idx, char* name, size_t size) const;
    ssize_t getObjnameByIdx(hsize_t idx, H5std_string& name, size_t size) const;

#ifdef HDF5_V10
    // Retrieves the type of an object in this file or group, given the
    // object's name
    H5O_type_t childObjType(const H5std_string& objname) const;
    H5O_type_t childObjType(const char* objname) const;
    H5O_type_t childObjType(hsize_t index, H5_index_t index_type=H5_INDEX_NAME, H5_iter_order_t order=H5_ITER_INC, const char* objname=".") const;

    // Returns the object header version of an object in this file or group,
    // given the object's name.
    unsigned childObjVersion(const char* objname) const;
    unsigned childObjVersion(const H5std_string& objname) const;

#endif

#ifdef HDF5_V10
    // Retrieves information about an HDF5 object.
    void getObjinfo(H5O_info2_t& objinfo, unsigned fields = H5O_INFO_BASIC) const;

    // Retrieves information about an HDF5 object, given its name.
    void getObjinfo(const char* name, H5O_info2_t& objinfo,
                    unsigned fields = H5O_INFO_BASIC,
                    const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    void getObjinfo(const H5std_string& name, H5O_info2_t& objinfo,
                    unsigned fields = H5O_INFO_BASIC,
                    const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;

    // Retrieves information about an HDF5 object, given its index.
    void getObjinfo(const char* grp_name, H5_index_t idx_type,
                    H5_iter_order_t order, hsize_t idx, H5O_info2_t& objinfo,
                    unsigned fields = H5O_INFO_BASIC,
                    const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    void getObjinfo(const H5std_string& grp_name, H5_index_t idx_type,
                    H5_iter_order_t order, hsize_t idx, H5O_info2_t& objinfo,
                    unsigned fields = H5O_INFO_BASIC,
                    const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;

    // Retrieves native native information about an HDF5 object.
    void getNativeObjinfo(H5O_native_info_t& objinfo, unsigned fields = H5O_NATIVE_INFO_HDR) const;

    // Retrieves native information about an HDF5 object, given its name.
    void getNativeObjinfo(const char* name, H5O_native_info_t& objinfo,
                          unsigned fields = H5O_NATIVE_INFO_HDR,
                          const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    void getNativeObjinfo(const H5std_string& name, H5O_native_info_t& objinfo,
                          unsigned fields = H5O_NATIVE_INFO_HDR,
                          const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;

    // Retrieves native information about an HDF5 object, given its index.
    void getNativeObjinfo(const char* grp_name, H5_index_t idx_type,
                          H5_iter_order_t order, hsize_t idx, H5O_native_info_t& objinfo,
                          unsigned fields = H5O_NATIVE_INFO_HDR,
                          const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    void getNativeObjinfo(const H5std_string& grp_name, H5_index_t idx_type,
                          H5_iter_order_t order, hsize_t idx, H5O_native_info_t& objinfo,
                          unsigned fields = H5O_NATIVE_INFO_HDR,
                          const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
#endif

#ifndef H5_NO_DEPRECATED_SYMBOLS
    // Returns the type of an object in this group, given the
    // object's index.
    H5G_obj_t getObjTypeByIdx(hsize_t idx) const;
    H5G_obj_t getObjTypeByIdx(hsize_t idx, char* type_name) const;
    H5G_obj_t getObjTypeByIdx(hsize_t idx, H5std_string& type_name) const;

    // Returns information about an HDF5 object, given by its name,
    // at this location. - Deprecated
    void getObjinfo(const char* name, hbool_t follow_link, H5G_stat_t& statbuf) const;
    void getObjinfo(const H5std_string& name, hbool_t follow_link, H5G_stat_t& statbuf) const;
    void getObjinfo(const char* name, H5G_stat_t& statbuf) const;
    void getObjinfo(const H5std_string& name, H5G_stat_t& statbuf) const;

    // Iterates over the elements of this group - not implemented in
    // C++ style yet.
    int iterateElems(const char* name, int *idx, H5G_iterate_t op, void *op_data);
    int iterateElems(const H5std_string& name, int *idx, H5G_iterate_t op, void *op_data);
#endif /* H5_NO_DEPRECATED_SYMBOLS */

    // Creates a soft link from link_name to target_name.
    void link(const char *target_name, const char *link_name,
              const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT,
              const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    void link(const H5std_string& target_name,
              const H5std_string& link_name,
              const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT,
              const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;

    // Creates a hard link from new_name to curr_name.
    void link(const char *curr_name,
              const Group& new_loc, const char *new_name,
              const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT,
              const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    void link(const H5std_string& curr_name,
              const Group& new_loc, const H5std_string& new_name,
              const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT,
              const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;

    // Creates a hard link from new_name to curr_name in same location.
    void link(const char *curr_name,
              const hid_t same_loc, const char *new_name,
              const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT,
              const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    void link(const H5std_string& curr_name,
              const hid_t same_loc, const H5std_string& new_name,
              const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT,
              const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;

    // Creates a link of the specified type from new_name to current_name;
    // both names are interpreted relative to the specified location id.
    // Deprecated due to inadequate functionality.
    void link(H5L_type_t link_type, const char* curr_name, const char* new_name) const;
    void link(H5L_type_t link_type, const H5std_string& curr_name, const H5std_string& new_name) const;

    // Removes the specified link from this location.
    void unlink(const char *link_name,
                const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    void unlink(const H5std_string& link_name,
                const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;

    // Mounts the file 'child' onto this location.
    void mount(const char* name, const H5File& child, const PropList& plist) const;
    void mount(const H5std_string& name, const H5File& child, const PropList& plist) const;

    // Unmounts the file named 'name' from this parent location.
    void unmount(const char* name) const;
    void unmount(const H5std_string& name) const;

    // Copies a link from a group to another.
    void copyLink(const char *src_name,
                  const Group& dst, const char *dst_name,
                  const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT,
                  const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    void copyLink(const H5std_string& src_name,
                  const Group& dst, const H5std_string& dst_name,
                  const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT,
                  const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;

    // Makes a copy of a link in the same group.
    void copyLink(const char *src_name, const char *dst_name,
                  const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT,
                  const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    void copyLink(const H5std_string& src_name,
                  const H5std_string& dst_name,
                  const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT,
                  const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;

    // Renames a link in this group and moves to a new location.
    void moveLink(const char* src_name,
                  const Group& dst, const char* dst_name,
                  const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT,
                  const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    void moveLink(const H5std_string& src_name,
                  const Group& dst, const H5std_string& dst_name,
                  const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT,
                  const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;

    // Renames a link in this group.
    void moveLink(const char* src_name, const char* dst_name,
                  const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT,
                  const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;
    void moveLink(const H5std_string& src_name,
                  const H5std_string& dst_name,
                  const LinkCreatPropList& lcpl = LinkCreatPropList::DEFAULT,
                  const LinkAccPropList& lapl = LinkAccPropList::DEFAULT) const;

    // Renames an object at this location.
    // Deprecated due to inadequate functionality.
    void move(const char* src, const char* dst) const;
    void move(const H5std_string& src, const H5std_string& dst) const;

    // end From CommonFG

    /// For subclasses, H5File and Group, to throw appropriate exception.
    virtual void throwException(const H5std_string& func_name, const H5std_string& msg) const;

    // Default constructor
    H5Location();

  protected:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    // *** Deprecation warning ***
    // The following two constructors are no longer appropriate after the
    // data member "id" had been moved to the sub-classes.
    // The copy constructor is a noop and is removed in 1.8.15 and the
    // other will be removed from 1.10 release, and then from 1.8 if its
    // removal does not raise any problems in two 1.10 releases.

    // Creates a copy of an existing object giving the location id.
    // H5Location(const hid_t loc_id);

    // Creates a reference to an HDF5 object or a dataset region.
    void p_reference(void* ref, const char* name, hid_t space_id, H5R_type_t ref_type) const;

#ifdef HDF5_V10
    // Dereferences a ref into an HDF5 id.
    hid_t p_dereference(hid_t loc_id, const void* ref, H5R_type_t ref_type, const PropList& plist, const char* from_func);
#endif

#ifndef H5_NO_DEPRECATED_SYMBOLS
    // Retrieves the type of object that an object reference points to.
    H5G_obj_t p_get_obj_type(void *ref, H5R_type_t ref_type) const;
#endif /* H5_NO_DEPRECATED_SYMBOLS */

    // Retrieves the type of object that an object reference points to.
    H5O_type_t p_get_ref_obj_type(void *ref, H5R_type_t ref_type) const;

    // Sets the identifier of this object to a new value. - this one
    // doesn't increment reference count
    //virtual void p_setId(const hid_t new_id);

#endif // DOXYGEN_SHOULD_SKIP_THIS

    // Noop destructor.
    virtual ~H5Location();

  }; // end of H5Location

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class H5Object
    \brief Class H5Object is a bridge between H5Location and DataSet, DataType,
    and Group.

    Modification:
    Sept 18, 2012: Added class H5Location in between IdComponent and
    H5Object.  An H5File now inherits from H5Location.  All HDF5
    wrappers in H5Object are moved up to H5Location.  H5Object
    is left mostly empty for future wrappers that are only for
    group, dataset, and named datatype.  Note that the reason for
    adding H5Location instead of simply moving H5File to be under
    H5Object is H5File is not an HDF5 object, and renaming H5Object
    to H5Location will risk breaking user applications.
    -BMR
    Apr 2, 2014: Added wrapper getObjName for H5Iget_name 
    Sep 21, 2016: Rearranging classes (HDFFV-9920) moved H5A wrappers back
    into H5Object.  This way, C functions that takes attribute id
    can be in H5Location and those that cannot take attribute id
    can be in H5Object.
  */
  // Inheritance: H5Location -> IdComponent


  // Define the operator function pointer for H5Aiterate().
  typedef void (*attr_operator_t)(H5Object& loc,
                                  const H5std_string attr_name,
                                  void *operator_data);

#ifdef HDF5_V10
  // Define the operator function pointer for H5Ovisit3().
  typedef int (*visit_operator_t)(H5Object& obj,
                                  const H5std_string attr_name,
                                  const H5O_info2_t *oinfo,
                                  void *operator_data);
#endif

  // User data for attribute iteration
  class UserData4Aiterate {
  public:
    attr_operator_t op;
    void* opData;
    H5Object* location; // Consider changing to H5Location
  };

  // User data for visit iteration
#ifdef HDF5_V10
  class UserData4Visit {
  public:
    visit_operator_t op;
    void* opData;
    H5Object* obj;
  };
#endif

  class H5_DLLCPP H5Object : public H5Location {
  public:
    // Creates an attribute for the specified object
    // PropList is currently not used, so always be default.
    Attribute createAttribute(const char* name, const DataType& type, const DataSpace& space, const PropList& create_plist = PropList::DEFAULT) const;
    Attribute createAttribute(const H5std_string& name, const DataType& type, const DataSpace& space, const PropList& create_plist = PropList::DEFAULT) const;

    // Given its name, opens the attribute that belongs to an object at
    // this location.
    Attribute openAttribute(const char* name) const;
    Attribute openAttribute(const H5std_string& name) const;

    // Given its index, opens the attribute that belongs to an object at
    // this location.
    Attribute openAttribute(const unsigned int idx) const;

    // Iterate user's function over the attributes of this object.
    int iterateAttrs(attr_operator_t user_op, unsigned* idx = NULL, void* op_data = NULL);

#ifdef HDF5_V10
    // Recursively visit elements reachable from this object.
    void visit(H5_index_t idx_type, H5_iter_order_t order, visit_operator_t user_op, void *op_data, unsigned int fields);

    // Returns the object header version of an object
    unsigned objVersion() const;
#endif

    // Determines the number of attributes belong to this object.
    int getNumAttrs() const;

    // Checks whether the named attribute exists for this object.
    bool attrExists(const char* name) const;
    bool attrExists(const H5std_string& name) const;

    // Renames the named attribute to a new name.
    void renameAttr(const char* oldname, const char* newname) const;
    void renameAttr(const H5std_string& oldname, const H5std_string& newname) const;

    // Removes the named attribute from this object.
    void removeAttr(const char* name) const;
    void removeAttr(const H5std_string& name) const;

    // Returns an identifier.
    virtual hid_t getId() const = 0;

    // Gets the name of this HDF5 object, i.e., Group, DataSet, or
    // DataType.
    ssize_t getObjName(char *obj_name, size_t buf_size = 0) const;
    ssize_t getObjName(H5std_string& obj_name, size_t len = 0) const;
    H5std_string getObjName() const;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

  protected:
    // Default constructor
    H5Object();

    // Sets the identifier of this object to a new value. - this one
    // doesn't increment reference count
    virtual void p_setId(const hid_t new_id) = 0;

    // Noop destructor.
    virtual ~H5Object();

#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of H5Object

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class AbstractDs
    \brief AbstractDs is an abstract base class, inherited by Attribute
    and DataSet.

    It provides a collection of services that are common to both Attribute
    and DataSet.
  */

  class H5_DLLCPP AbstractDs {
  public:
  // Gets a copy the datatype of that this abstract dataset uses.
  // Note that this datatype is a generic one and can only be accessed
  // via generic member functions, i.e., member functions belong
  // to DataType.  To get specific datatype, i.e. EnumType, FloatType,
  // etc..., use the specific functions, that follow, instead.
  DataType getDataType() const;

  // Gets a copy of the specific datatype of this abstract dataset.
  ArrayType getArrayType() const;
  CompType getCompType() const;
  EnumType getEnumType() const;
  IntType getIntType() const;
  FloatType getFloatType() const;
  StrType getStrType() const;
  VarLenType getVarLenType() const;

  ///\brief Gets the size in memory of this abstract dataset.
  virtual size_t getInMemDataSize() const = 0;

  ///\brief Gets the dataspace of this abstract dataset - pure virtual.
  virtual DataSpace getSpace() const = 0;

  // Gets the class of the datatype that is used by this abstract
  // dataset.
  H5T_class_t getTypeClass() const;

  ///\brief Returns the amount of storage size required - pure virtual.
  virtual hsize_t getStorageSize() const = 0;

  // Returns this class name - pure virtual.
  virtual H5std_string fromClass() const = 0;

  // Destructor
  virtual ~AbstractDs();

  protected:
  // Default constructor
  AbstractDs();

  private:
  // This member function is implemented by DataSet and Attribute - pure virtual.
  virtual hid_t p_get_type() const = 0;

  }; // end of AbstractDs

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class Attribute
    \brief Class Attribute operates on HDF5 attributes.

    An attribute has many characteristics similar to a dataset, thus both
    Attribute and DataSet are derivatives of AbstractDs.  Attribute also
    inherits from H5Location because an attribute can be used to specify
    a location.
  */
  //  Inheritance: multiple H5Location/AbstractDs -> IdComponent

  class H5_DLLCPP Attribute : public AbstractDs, public H5Location {
  public:

    // Copy constructor: same as the original Attribute.
    Attribute(const Attribute& original);

    // Default constructor
    Attribute();

    // Creates a copy of an existing attribute using the attribute id
    Attribute(const hid_t attr_id);

    // Closes this attribute.
    virtual void close();

    // Gets the name of this attribute.
    ssize_t getName(char* attr_name, size_t buf_size = 0) const;
    H5std_string getName(size_t len) const;
    H5std_string getName() const;
    ssize_t getName(H5std_string& attr_name, size_t len = 0) const;
    // The overloaded function below is replaced by the one above and it
    // is kept for backward compatibility purpose.
    ssize_t getName(size_t buf_size, H5std_string& attr_name) const;

    // Gets a copy of the dataspace for this attribute.
    virtual DataSpace getSpace() const;

    // Returns the amount of storage size required for this attribute.
    virtual hsize_t getStorageSize() const;

    // Returns the in memory size of this attribute's data.
    virtual size_t getInMemDataSize() const;

    // Reads data from this attribute.
    void read(const DataType& mem_type, void *buf) const;
    void read(const DataType& mem_type, H5std_string& strg) const;

    // Writes data to this attribute.
    void write(const DataType& mem_type, const void *buf) const;
    void write(const DataType& mem_type, const H5std_string& strg) const;

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("Attribute"); }

    // Gets the attribute id.
    virtual hid_t getId() const;

    // Destructor: properly terminates access to this attribute.
    virtual ~Attribute();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  protected:
    // Sets the attribute id.
    virtual void p_setId(const hid_t new_id);
#endif // DOXYGEN_SHOULD_SKIP_THIS

  private:
    hid_t id;        // HDF5 attribute id

    // This function contains the common code that is used by
    // getTypeClass and various API functions getXxxType
    // defined in AbstractDs for generic datatype and specific
    // sub-types
    virtual hid_t p_get_type() const;

    // Reads variable or fixed len strings from this attribute.
    void p_read_variable_len(const DataType& mem_type, H5std_string& strg) const;
    void p_read_fixed_len(const DataType& mem_type, H5std_string& strg) const;

    // Friend function to set Attribute id.  For library use only.
    friend void f_Attribute_setId(Attribute* attr, hid_t new_id);

  }; // end of Attribute

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class DataType
    \brief Class DataType provides generic operations on HDF5 datatypes.

    DataType inherits from H5Object because a named datatype is an HDF5
    object and is a base class of ArrayType, AtomType, CompType, EnumType,
    and VarLenType.
  */
  //  Inheritance: DataType -> H5Object -> H5Location -> IdComponent

  class H5_DLLCPP DataType : public H5Object {
  public:
    // Creates a datatype given its class and size
    DataType(const H5T_class_t type_class, size_t size);

    // Copy constructor - same as the original DataType.
    DataType(const DataType& original);

    // Creates a copy of a predefined type
    DataType(const PredType& pred_type);

    // Constructors to open a generic named datatype at a given location.
    DataType(const H5Location& loc, const char* name);
    DataType(const H5Location& loc, const H5std_string& name);

#ifdef HDF5_V10
    // Creates a datatype by way of dereference.
    DataType(const H5Location& loc, const void* ref, H5R_type_t ref_type = H5R_OBJECT, const PropList& plist = PropList::DEFAULT);
    //        DataType(const Attribute& attr, const void* ref, H5R_type_t ref_type = H5R_OBJECT, const PropList& plist = PropList::DEFAULT);
#endif

    // Closes this datatype.
    virtual void close();

    // Copies an existing datatype to this datatype object.
    void copy(const DataType& like_type);

    // Copies the datatype of dset to this datatype object.
    void copy(const DataSet& dset);

    // Returns a DataType instance by decoding the binary object
    // description of this datatype.
    virtual DataType* decode() const;

    // Creates a binary object description of this datatype.
    void encode();

    // Returns the datatype class identifier.
    H5T_class_t getClass() const;

    // Commits a transient datatype to a file; this datatype becomes
    // a named datatype which can be accessed from the location.
    void commit(const H5Location& loc, const char* name);
    void commit(const H5Location& loc, const H5std_string& name);

    // These two overloaded functions are kept for backward compatibility
    // only; they missed the const - removed from 1.8.18 and 1.10.1
    //void commit(H5Location& loc, const char* name);
    //void commit(H5Location& loc, const H5std_string& name);

    // Determines whether this datatype is a named datatype or
    // a transient datatype.
    bool committed() const;

    // Finds a conversion function that can handle the conversion
    // this datatype to the given datatype, dest.
    H5T_conv_t find(const DataType& dest, H5T_cdata_t **pcdata) const;

    // Converts data from between specified datatypes.
    void convert(const DataType& dest, size_t nelmts, void *buf, void *background, const PropList& plist=PropList::DEFAULT) const;

    // Assignment operator
    DataType& operator=(const DataType& rhs);

    // Determines whether two datatypes are the same.
    bool operator==(const DataType& compared_type) const;

    // Determines whether two datatypes are not the same.
    bool operator!=(const DataType& compared_type) const;

    // Locks a datatype.
    void lock() const;

    // Returns the size of a datatype.
    size_t getSize() const;

    // Returns the base datatype from which a datatype is derived.
    // Note: not quite right for specific types yet???
    DataType getSuper() const;

    // Registers a conversion function.
    void registerFunc(H5T_pers_t pers, const char* name, const DataType& dest, H5T_conv_t func) const;
    void registerFunc(H5T_pers_t pers, const H5std_string& name, const DataType& dest, H5T_conv_t func) const;

    // Removes a conversion function from all conversion paths.
    void unregister(H5T_pers_t pers, const char* name, const DataType& dest, H5T_conv_t func) const;
    void unregister(H5T_pers_t pers, const H5std_string& name, const DataType& dest, H5T_conv_t func) const;

    // Tags an opaque datatype.
    void setTag(const char* tag) const;
    void setTag(const H5std_string& tag) const;

    // Gets the tag associated with an opaque datatype.
    H5std_string getTag() const;

    // Checks whether this datatype contains (or is) a certain type class.
    bool detectClass(H5T_class_t cls) const;
    static bool detectClass(const PredType& pred_type, H5T_class_t cls);

    // Checks whether this datatype is a variable-length string.
    bool isVariableStr() const;

    // Returns a copy of the creation property list of a datatype.
    PropList getCreatePlist() const;

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("DataType"); }

    // Creates a copy of an existing DataType using its id
    DataType(const hid_t type_id);

    // Default constructor
    DataType();

    // Determines whether this datatype has a binary object description.
    bool hasBinaryDesc() const;

    // Gets the datatype id.
    virtual hid_t getId() const;

    // Destructor: properly terminates access to this datatype.
    virtual ~DataType();

  protected:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    hid_t id;    // HDF5 datatype id

    // Returns an id of a type by decoding the binary object
    // description of this datatype.
    hid_t p_decode() const;

    // Sets the datatype id.
    virtual void p_setId(const hid_t new_id);

    // Opens a datatype and returns the id.
    hid_t p_opentype(const H5Location& loc, const char* dtype_name) const;

#endif // DOXYGEN_SHOULD_SKIP_THIS

  private:
    // Buffer for binary object description of this datatype, allocated
    // in DataType::encode and used in DataType::decode
    unsigned char *encoded_buf;
    size_t buf_size;

    // Friend function to set DataType id.  For library use only.
    friend void f_DataType_setId(DataType* dtype, hid_t new_id);

    void p_commit(hid_t loc_id, const char* name);

  }; // end of DataType

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class AtomType
    \brief AtomType is a base class, inherited by IntType, FloatType,
    StrType, and PredType.

    AtomType provides operations on HDF5 atomic datatypes.  It also inherits
    from DataType.
  */
  // Inheritance: DataType -> H5Object -> H5Location -> IdComponent

  class H5_DLLCPP AtomType : public DataType {
  public:
    // Returns the byte order of an atomic datatype.
    H5T_order_t getOrder() const;
    H5T_order_t getOrder(H5std_string& order_string) const;

    // Sets the byte ordering of an atomic datatype.
    void setOrder(H5T_order_t order) const;

    // Retrieves the bit offset of the first significant bit.
    // 12/05/00 - changed return type to int from size_t - C API
    int getOffset() const;

    // Sets the bit offset of the first significant bit.
    void setOffset(size_t offset) const;

    // Retrieves the padding type of the least and most-significant bit padding.
    void getPad(H5T_pad_t& lsb, H5T_pad_t& msb) const;

    // Sets the least and most-significant bits padding types
    void setPad(H5T_pad_t lsb, H5T_pad_t msb) const;

    // Returns the precision of an atomic datatype.
    size_t getPrecision() const;

    // Sets the precision of an atomic datatype.
    void setPrecision(size_t precision) const;

    // Sets the total size for an atomic datatype.
    void setSize(size_t size) const;

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("AtomType"); }

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    // Copy constructor: same as the original AtomType.
    AtomType(const AtomType& original);

    // Noop destructor
    virtual ~AtomType();
#endif // DOXYGEN_SHOULD_SKIP_THIS

  protected:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    // Default constructor
    AtomType();

    // Constructor that takes an existing id
    AtomType(const hid_t existing_id);
#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of AtomType

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class PredType
    \brief Class PredType holds the definition of all the HDF5 predefined
    datatypes.

    These types can only be made copy of, not created by H5Tcreate or
    closed by H5Tclose.  They are treated as constants.
  */
  //  Inheritance: AtomType -> DataType -> H5Object -> H5Location -> IdComponent

  class H5_DLLCPP PredType : public AtomType {
  public:
    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("PredType"); }

    // Makes a copy of the predefined type and stores the new
    // id in the left hand side object.
    PredType& operator=(const PredType& rhs);

    // Copy constructor: same as the original PredType.
    PredType(const PredType& original);

    // Noop destructor
    virtual ~PredType();

    /*! \brief This dummy function do not inherit from DataType - it will
      throw a DataTypeIException if invoked.
    */
    void commit(H5Location& loc, const H5std_string& name);
    /*! \brief This dummy function do not inherit from DataType - it will
      throw a DataTypeIException if invoked.
    */
    void commit(H5Location& loc, const char* name);
    /*! \brief This dummy function do not inherit from DataType - it will
      throw a DataTypeIException if invoked.
    */
    bool committed();

    ///\brief PredType constants
    static const PredType& STD_I8BE;
    static const PredType& STD_I8LE;
    static const PredType& STD_I16BE;
    static const PredType& STD_I16LE;
    static const PredType& STD_I32BE;
    static const PredType& STD_I32LE;
    static const PredType& STD_I64BE;
    static const PredType& STD_I64LE;
    static const PredType& STD_U8BE;
    static const PredType& STD_U8LE;
    static const PredType& STD_U16BE;
    static const PredType& STD_U16LE;
    static const PredType& STD_U32BE;
    static const PredType& STD_U32LE;
    static const PredType& STD_U64BE;
    static const PredType& STD_U64LE;
    static const PredType& STD_B8BE;
    static const PredType& STD_B8LE;
    static const PredType& STD_B16BE;
    static const PredType& STD_B16LE;
    static const PredType& STD_B32BE;
    static const PredType& STD_B32LE;
    static const PredType& STD_B64BE;
    static const PredType& STD_B64LE;
    static const PredType& STD_REF_OBJ;
    static const PredType& STD_REF_DSETREG;

    static const PredType& C_S1;
    static const PredType& FORTRAN_S1;

    static const PredType& IEEE_F32BE;
    static const PredType& IEEE_F32LE;
    static const PredType& IEEE_F64BE;
    static const PredType& IEEE_F64LE;

    static const PredType& UNIX_D32BE;
    static const PredType& UNIX_D32LE;
    static const PredType& UNIX_D64BE;
    static const PredType& UNIX_D64LE;

    static const PredType& INTEL_I8;
    static const PredType& INTEL_I16;
    static const PredType& INTEL_I32;
    static const PredType& INTEL_I64;
    static const PredType& INTEL_U8;
    static const PredType& INTEL_U16;
    static const PredType& INTEL_U32;
    static const PredType& INTEL_U64;
    static const PredType& INTEL_B8;
    static const PredType& INTEL_B16;
    static const PredType& INTEL_B32;
    static const PredType& INTEL_B64;
    static const PredType& INTEL_F32;
    static const PredType& INTEL_F64;

    static const PredType& ALPHA_I8;
    static const PredType& ALPHA_I16;
    static const PredType& ALPHA_I32;
    static const PredType& ALPHA_I64;
    static const PredType& ALPHA_U8;
    static const PredType& ALPHA_U16;
    static const PredType& ALPHA_U32;
    static const PredType& ALPHA_U64;
    static const PredType& ALPHA_B8;
    static const PredType& ALPHA_B16;
    static const PredType& ALPHA_B32;
    static const PredType& ALPHA_B64;
    static const PredType& ALPHA_F32;
    static const PredType& ALPHA_F64;

    static const PredType& MIPS_I8;
    static const PredType& MIPS_I16;
    static const PredType& MIPS_I32;
    static const PredType& MIPS_I64;
    static const PredType& MIPS_U8;
    static const PredType& MIPS_U16;
    static const PredType& MIPS_U32;
    static const PredType& MIPS_U64;
    static const PredType& MIPS_B8;
    static const PredType& MIPS_B16;
    static const PredType& MIPS_B32;
    static const PredType& MIPS_B64;
    static const PredType& MIPS_F32;
    static const PredType& MIPS_F64;

    static const PredType& NATIVE_CHAR;
    static const PredType& NATIVE_SCHAR;
    static const PredType& NATIVE_UCHAR;
    static const PredType& NATIVE_SHORT;
    static const PredType& NATIVE_USHORT;
    static const PredType& NATIVE_INT;
    static const PredType& NATIVE_UINT;
    static const PredType& NATIVE_LONG;
    static const PredType& NATIVE_ULONG;
    static const PredType& NATIVE_LLONG;
    static const PredType& NATIVE_ULLONG;
    static const PredType& NATIVE_FLOAT;
    static const PredType& NATIVE_DOUBLE;
    static const PredType& NATIVE_LDOUBLE;
    static const PredType& NATIVE_B8;
    static const PredType& NATIVE_B16;
    static const PredType& NATIVE_B32;
    static const PredType& NATIVE_B64;
    static const PredType& NATIVE_OPAQUE;
    static const PredType& NATIVE_HSIZE;
    static const PredType& NATIVE_HSSIZE;
    static const PredType& NATIVE_HERR;
    static const PredType& NATIVE_HBOOL;

    static const PredType& NATIVE_INT8;
    static const PredType& NATIVE_UINT8;
    static const PredType& NATIVE_INT16;
    static const PredType& NATIVE_UINT16;
    static const PredType& NATIVE_INT32;
    static const PredType& NATIVE_UINT32;
    static const PredType& NATIVE_INT64;
    static const PredType& NATIVE_UINT64;

    // LEAST types
#if H5_SIZEOF_INT_LEAST8_T != 0
    static const PredType& NATIVE_INT_LEAST8;
#endif /* H5_SIZEOF_INT_LEAST8_T */
#if H5_SIZEOF_UINT_LEAST8_T != 0
    static const PredType& NATIVE_UINT_LEAST8;
#endif /* H5_SIZEOF_UINT_LEAST8_T */

#if H5_SIZEOF_INT_LEAST16_T != 0
    static const PredType& NATIVE_INT_LEAST16;
#endif /* H5_SIZEOF_INT_LEAST16_T */
#if H5_SIZEOF_UINT_LEAST16_T != 0
    static const PredType& NATIVE_UINT_LEAST16;
#endif /* H5_SIZEOF_UINT_LEAST16_T */

#if H5_SIZEOF_INT_LEAST32_T != 0
    static const PredType& NATIVE_INT_LEAST32;
#endif /* H5_SIZEOF_INT_LEAST32_T */
#if H5_SIZEOF_UINT_LEAST32_T != 0
    static const PredType& NATIVE_UINT_LEAST32;
#endif /* H5_SIZEOF_UINT_LEAST32_T */

#if H5_SIZEOF_INT_LEAST64_T != 0
    static const PredType& NATIVE_INT_LEAST64;
#endif /* H5_SIZEOF_INT_LEAST64_T */
#if H5_SIZEOF_UINT_LEAST64_T != 0
    static const PredType& NATIVE_UINT_LEAST64;
#endif /* H5_SIZEOF_UINT_LEAST64_T */

    // FAST types
#if H5_SIZEOF_INT_FAST8_T != 0
    static const PredType& NATIVE_INT_FAST8;
#endif /* H5_SIZEOF_INT_FAST8_T */
#if H5_SIZEOF_UINT_FAST8_T != 0
    static const PredType& NATIVE_UINT_FAST8;
#endif /* H5_SIZEOF_UINT_FAST8_T */

#if H5_SIZEOF_INT_FAST16_T != 0
    static const PredType& NATIVE_INT_FAST16;
#endif /* H5_SIZEOF_INT_FAST16_T */
#if H5_SIZEOF_UINT_FAST16_T != 0
    static const PredType& NATIVE_UINT_FAST16;
#endif /* H5_SIZEOF_UINT_FAST16_T */

#if H5_SIZEOF_INT_FAST32_T != 0
    static const PredType& NATIVE_INT_FAST32;
#endif /* H5_SIZEOF_INT_FAST32_T */
#if H5_SIZEOF_UINT_FAST32_T != 0
    static const PredType& NATIVE_UINT_FAST32;
#endif /* H5_SIZEOF_UINT_FAST32_T */

#if H5_SIZEOF_INT_FAST64_T != 0
    static const PredType& NATIVE_INT_FAST64;
#endif /* H5_SIZEOF_INT_FAST64_T */
#if H5_SIZEOF_UINT_FAST64_T != 0
    static const PredType& NATIVE_UINT_FAST64;
#endif /* H5_SIZEOF_UINT_FAST64_T */

#ifndef DOXYGEN_SHOULD_SKIP_THIS

    // Deletes the PredType global constants
    static void deleteConstants();

    // Dummy constant
    static const PredType& PREDTYPE_CONST; // dummy constant

  protected:
    // Default constructor
    PredType();

    // Creates a pre-defined type using an HDF5 pre-defined constant
    PredType(const hid_t predtype_id);  // used by the library only

  private:
    // Activates the creation of the PredType global constants
    static PredType* getPredTypes();

    // Dynamically allocates PredType global constants
    static void makePredTypes();

    // Dummy constant
    static PredType* PREDTYPE_CONST_;

    // Declaration of pointers to constants
    static PredType* STD_I8BE_;
    static PredType* STD_I8LE_;
    static PredType* STD_I16BE_;
    static PredType* STD_I16LE_;
    static PredType* STD_I32BE_;
    static PredType* STD_I32LE_;
    static PredType* STD_I64BE_;
    static PredType* STD_I64LE_;
    static PredType* STD_U8BE_;
    static PredType* STD_U8LE_;
    static PredType* STD_U16BE_;
    static PredType* STD_U16LE_;
    static PredType* STD_U32BE_;
    static PredType* STD_U32LE_;
    static PredType* STD_U64BE_;
    static PredType* STD_U64LE_;
    static PredType* STD_B8BE_;
    static PredType* STD_B8LE_;
    static PredType* STD_B16BE_;
    static PredType* STD_B16LE_;
    static PredType* STD_B32BE_;
    static PredType* STD_B32LE_;
    static PredType* STD_B64BE_;
    static PredType* STD_B64LE_;
    static PredType* STD_REF_OBJ_;
    static PredType* STD_REF_DSETREG_;

    static PredType* C_S1_;
    static PredType* FORTRAN_S1_;

    static PredType* IEEE_F32BE_;
    static PredType* IEEE_F32LE_;
    static PredType* IEEE_F64BE_;
    static PredType* IEEE_F64LE_;

    static PredType* UNIX_D32BE_;
    static PredType* UNIX_D32LE_;
    static PredType* UNIX_D64BE_;
    static PredType* UNIX_D64LE_;

    static PredType* INTEL_I8_;
    static PredType* INTEL_I16_;
    static PredType* INTEL_I32_;
    static PredType* INTEL_I64_;
    static PredType* INTEL_U8_;
    static PredType* INTEL_U16_;
    static PredType* INTEL_U32_;
    static PredType* INTEL_U64_;
    static PredType* INTEL_B8_;
    static PredType* INTEL_B16_;
    static PredType* INTEL_B32_;
    static PredType* INTEL_B64_;
    static PredType* INTEL_F32_;
    static PredType* INTEL_F64_;

    static PredType* ALPHA_I8_;
    static PredType* ALPHA_I16_;
    static PredType* ALPHA_I32_;
    static PredType* ALPHA_I64_;
    static PredType* ALPHA_U8_;
    static PredType* ALPHA_U16_;
    static PredType* ALPHA_U32_;
    static PredType* ALPHA_U64_;
    static PredType* ALPHA_B8_;
    static PredType* ALPHA_B16_;
    static PredType* ALPHA_B32_;
    static PredType* ALPHA_B64_;
    static PredType* ALPHA_F32_;
    static PredType* ALPHA_F64_;

    static PredType* MIPS_I8_;
    static PredType* MIPS_I16_;
    static PredType* MIPS_I32_;
    static PredType* MIPS_I64_;
    static PredType* MIPS_U8_;
    static PredType* MIPS_U16_;
    static PredType* MIPS_U32_;
    static PredType* MIPS_U64_;
    static PredType* MIPS_B8_;
    static PredType* MIPS_B16_;
    static PredType* MIPS_B32_;
    static PredType* MIPS_B64_;
    static PredType* MIPS_F32_;
    static PredType* MIPS_F64_;

    static PredType* NATIVE_CHAR_;
    static PredType* NATIVE_SCHAR_;
    static PredType* NATIVE_UCHAR_;
    static PredType* NATIVE_SHORT_;
    static PredType* NATIVE_USHORT_;
    static PredType* NATIVE_INT_;
    static PredType* NATIVE_UINT_;
    static PredType* NATIVE_LONG_;
    static PredType* NATIVE_ULONG_;
    static PredType* NATIVE_LLONG_;
    static PredType* NATIVE_ULLONG_;
    static PredType* NATIVE_FLOAT_;
    static PredType* NATIVE_DOUBLE_;
    static PredType* NATIVE_LDOUBLE_;
    static PredType* NATIVE_B8_;
    static PredType* NATIVE_B16_;
    static PredType* NATIVE_B32_;
    static PredType* NATIVE_B64_;
    static PredType* NATIVE_OPAQUE_;
    static PredType* NATIVE_HSIZE_;
    static PredType* NATIVE_HSSIZE_;
    static PredType* NATIVE_HERR_;
    static PredType* NATIVE_HBOOL_;

    static PredType* NATIVE_INT8_;
    static PredType* NATIVE_UINT8_;
    static PredType* NATIVE_INT16_;
    static PredType* NATIVE_UINT16_;
    static PredType* NATIVE_INT32_;
    static PredType* NATIVE_UINT32_;
    static PredType* NATIVE_INT64_;
    static PredType* NATIVE_UINT64_;

    // LEAST types
#if H5_SIZEOF_INT_LEAST8_T != 0
    static PredType* NATIVE_INT_LEAST8_;
#endif /* H5_SIZEOF_INT_LEAST8_T */
#if H5_SIZEOF_UINT_LEAST8_T != 0
    static PredType* NATIVE_UINT_LEAST8_;
#endif /* H5_SIZEOF_UINT_LEAST8_T */

#if H5_SIZEOF_INT_LEAST16_T != 0
    static PredType* NATIVE_INT_LEAST16_;
#endif /* H5_SIZEOF_INT_LEAST16_T */
#if H5_SIZEOF_UINT_LEAST16_T != 0
    static PredType* NATIVE_UINT_LEAST16_;
#endif /* H5_SIZEOF_UINT_LEAST16_T */

#if H5_SIZEOF_INT_LEAST32_T != 0
    static PredType* NATIVE_INT_LEAST32_;
#endif /* H5_SIZEOF_INT_LEAST32_T */
#if H5_SIZEOF_UINT_LEAST32_T != 0
    static PredType* NATIVE_UINT_LEAST32_;
#endif /* H5_SIZEOF_UINT_LEAST32_T */

#if H5_SIZEOF_INT_LEAST64_T != 0
    static PredType* NATIVE_INT_LEAST64_;
#endif /* H5_SIZEOF_INT_LEAST64_T */
#if H5_SIZEOF_UINT_LEAST64_T != 0
    static PredType* NATIVE_UINT_LEAST64_;
#endif /* H5_SIZEOF_UINT_LEAST64_T */

    // FAST types
#if H5_SIZEOF_INT_FAST8_T != 0
    static PredType* NATIVE_INT_FAST8_;
#endif /* H5_SIZEOF_INT_FAST8_T */
#if H5_SIZEOF_UINT_FAST8_T != 0
    static PredType* NATIVE_UINT_FAST8_;
#endif /* H5_SIZEOF_UINT_FAST8_T */

#if H5_SIZEOF_INT_FAST16_T != 0
    static PredType* NATIVE_INT_FAST16_;
#endif /* H5_SIZEOF_INT_FAST16_T */
#if H5_SIZEOF_UINT_FAST16_T != 0
    static PredType* NATIVE_UINT_FAST16_;
#endif /* H5_SIZEOF_UINT_FAST16_T */

#if H5_SIZEOF_INT_FAST32_T != 0
    static PredType* NATIVE_INT_FAST32_;
#endif /* H5_SIZEOF_INT_FAST32_T */
#if H5_SIZEOF_UINT_FAST32_T != 0
    static PredType* NATIVE_UINT_FAST32_;
#endif /* H5_SIZEOF_UINT_FAST32_T */

#if H5_SIZEOF_INT_FAST64_T != 0
    static PredType* NATIVE_INT_FAST64_;
#endif /* H5_SIZEOF_INT_FAST64_T */
#if H5_SIZEOF_UINT_FAST64_T != 0
    static PredType* NATIVE_UINT_FAST64_;
#endif /* H5_SIZEOF_UINT_FAST64_T */
    // End of Declaration of pointers

#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of PredType

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class EnumType
    \brief EnumType is a derivative of a DataType and operates on HDF5
    enum datatypes.
  */
  //  Inheritance: DataType -> H5Object -> H5Location -> IdComponent

  class H5_DLLCPP EnumType : public DataType {

  public:
    // Creates an empty enumeration datatype based on a native signed
    // integer type, whose size is given by size.
    EnumType(size_t size);

    // Gets the enum datatype of the specified dataset
    EnumType(const DataSet& dataset);  // H5Dget_type

    // Creates a new enum datatype based on an integer datatype
    EnumType(const IntType& data_type);  // H5Tenum_create

    // Constructors that open an enum datatype, given a location.
    EnumType(const H5Location& loc, const char* name);
    EnumType(const H5Location& loc, const H5std_string& name);

    // Returns an EnumType object via DataType* by decoding the
    // binary object description of this type.
    virtual DataType* decode() const;

    // Returns the number of members in this enumeration datatype.
    int getNmembers () const;

    // Returns the index of a member in this enumeration data type.
    int getMemberIndex(const char* name) const;
    int getMemberIndex(const H5std_string& name) const;

    // Returns the value of an enumeration datatype member
    void getMemberValue(unsigned memb_no, void *value) const;

    // Inserts a new member to this enumeration type.
    void insert(const char* name, void *value) const;
    void insert(const H5std_string& name, void *value) const;

    // Returns the symbol name corresponding to a specified member
    // of this enumeration datatype.
    H5std_string nameOf(void *value, size_t size) const;

    // Returns the value corresponding to a specified member of this
    // enumeration datatype.
    void valueOf(const char* name, void *value) const;
    void valueOf(const H5std_string& name, void *value) const;

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("EnumType"); }

    // Default constructor
    EnumType();

    // Creates an enumeration datatype using an existing id
    EnumType(const hid_t existing_id);

    // Copy constructor: same as the original EnumType.
    EnumType(const EnumType& original);

    virtual ~EnumType();

  }; // end of EnumType

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class IntType
    \brief IntType is a derivative of a DataType and operates on HDF5
    integer datatype.
  */
  //  Inheritance: AtomType -> DataType -> H5Object -> H5Location -> IdComponent

  class H5_DLLCPP IntType : public AtomType {
  public:
    // Creates an integer type using a predefined type
    IntType(const PredType& pred_type);

    // Gets the integer datatype of the specified dataset
    IntType(const DataSet& dataset);

    // Constructors that open an HDF5 integer datatype, given a location.
    IntType(const H5Location& loc, const char* name);
    IntType(const H5Location& loc, const H5std_string& name);

    // Returns an IntType object via DataType* by decoding the
    // binary object description of this type.
    virtual DataType* decode() const;

    // Retrieves the sign type for an integer type
    H5T_sign_t getSign() const;

    // Sets the sign proprety for an integer type.
    void setSign(H5T_sign_t sign) const;

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("IntType"); }

    // Default constructor
    IntType();

    // Creates a integer datatype using an existing id
    IntType(const hid_t existing_id);

    // Copy constructor: same as the original IntType.
    IntType(const IntType& original);

    // Noop destructor.
    virtual ~IntType();

  }; // end of IntType

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class FloatType
    \brief FloatType is a derivative of a DataType and operates on HDF5
    floating point datatype.
  */
  //  Inheritance: AtomType -> DataType -> H5Object -> H5Location -> IdComponent

  class H5_DLLCPP FloatType : public AtomType {
  public:
    // Creates a floating-point type using a predefined type.
    FloatType(const PredType& pred_type);

    // Gets the floating-point datatype of the specified dataset.
    FloatType(const DataSet& dataset);

    // Constructors that open an HDF5 float datatype, given a location.
    FloatType(const H5Location& loc, const char* name);
    FloatType(const H5Location& loc, const H5std_string& name);

    // Returns an FloatType object via DataType* by decoding the
    // binary object description of this type.
    virtual DataType* decode() const;

    // Retrieves the exponent bias of a floating-point type.
    size_t getEbias() const;

    // Sets the exponent bias of a floating-point type.
    void setEbias(size_t ebias) const;

    // Retrieves floating point datatype bit field information.
    void getFields(size_t& spos, size_t& epos, size_t& esize, size_t& mpos, size_t& msize) const;

    // Sets locations and sizes of floating point bit fields.
    void setFields(size_t spos, size_t epos, size_t esize, size_t mpos, size_t msize) const;

    // Retrieves the internal padding type for unused bits in floating-point datatypes.
    H5T_pad_t getInpad(H5std_string& pad_string) const;

    // Fills unused internal floating point bits.
    void setInpad(H5T_pad_t inpad) const;

    // Retrieves mantissa normalization of a floating-point datatype.
    H5T_norm_t getNorm(H5std_string& norm_string) const;

    // Sets the mantissa normalization of a floating-point datatype.
    void setNorm(H5T_norm_t norm) const;

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("FloatType"); }

    // Default constructor
    FloatType();

    // Creates a floating-point datatype using an existing id.
    FloatType(const hid_t existing_id);

    // Copy constructor: same as the original FloatType.
    FloatType(const FloatType& original);

    // Noop destructor.
    virtual ~FloatType();

  }; // end of FloatType

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class StrType
    \brief StrType is a derivative of a DataType and operates on HDF5
    string datatype.
  */
  //  Inheritance: AtomType -> DataType -> H5Object -> H5Location -> IdComponent

  class H5_DLLCPP StrType : public AtomType {
  public:
    // Creates a string type using a predefined type
    StrType(const PredType& pred_type);

    // Creates a string type with specified length - may be obsolete
    StrType(const PredType& pred_type, const size_t& size);

    // Creates a string type with specified length
    StrType(const int dummy, const size_t& size);

    // Gets the string datatype of the specified dataset
    StrType(const DataSet& dataset);

    // Constructors that open an HDF5 string datatype, given a location.
    StrType(const H5Location& loc, const char* name);
    StrType(const H5Location& loc, const H5std_string& name);

    // Returns an StrType object via DataType* by decoding the
    // binary object description of this type.
    virtual DataType* decode() const;

    // Retrieves the character set type of this string datatype.
    H5T_cset_t getCset() const;

    // Sets character set to be used.
    void setCset(H5T_cset_t cset) const;

    // Retrieves the string padding method for this string datatype.
    H5T_str_t getStrpad() const;

    // Defines the storage mechanism for character strings.
    void setStrpad(H5T_str_t strpad) const;

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("StrType"); }

    // default constructor
    StrType();

    // Creates a string datatype using an existing id
    StrType(const hid_t existing_id);

    // Copy constructor: same as the original StrType.
    StrType(const StrType& original);

    // Noop destructor.
    virtual ~StrType();

  }; // end of StrType

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class CompType
    \brief CompType is a derivative of a DataType and operates on HDF5
    compound datatypes.
  */
  //  Inheritance: DataType -> H5Object -> H5Location -> IdComponent

  class H5_DLLCPP CompType : public DataType {
  public:
    // Default constructor
    CompType();

    // Creates a compound datatype using an existing id
    CompType(const hid_t existing_id);

    // Creates a new compound datatype, given the type's size
    CompType(size_t size); // H5Tcreate

    // Gets the compound datatype of the specified dataset
    CompType(const DataSet& dataset);  // H5Dget_type

    // Copy constructor - same as the original CompType.
    CompType(const CompType& original);

    // Constructors that open a compound datatype, given a location.
    CompType(const H5Location& loc, const char* name);
    CompType(const H5Location& loc, const H5std_string& name);

    // Returns a CompType object via DataType* by decoding the binary
    // object description of this type.
    virtual DataType* decode() const;

    // Returns the type class of the specified member of this compound
    // datatype.  It provides to the user a way of knowing what type
    // to create another datatype of the same class
    H5T_class_t getMemberClass(unsigned member_num) const;

    // Returns the index of a member in this compound data type.
    int getMemberIndex(const char* name) const;
    int getMemberIndex(const H5std_string& name) const;

    // Returns the offset of a member of this compound datatype.
    size_t getMemberOffset(unsigned memb_no) const;

    // Returns the name of a member of this compound datatype.
    H5std_string getMemberName(unsigned member_num) const;

    // Returns the generic datatype of the specified member in
    // this compound datatype.
    DataType getMemberDataType(unsigned member_num) const;

    // Returns the array datatype of the specified member in
    // this compound datatype.
    ArrayType getMemberArrayType(unsigned member_num) const;

    // Returns the compound datatype of the specified member in
    // this compound datatype.
    CompType getMemberCompType(unsigned member_num) const;

    // Returns the enumeration datatype of the specified member in
    // this compound datatype.
    EnumType getMemberEnumType(unsigned member_num) const;

    // Returns the integer datatype of the specified member in
    // this compound datatype.
    IntType getMemberIntType(unsigned member_num) const;

    // Returns the floating-point datatype of the specified member in
    // this compound datatype.
    FloatType getMemberFloatType(unsigned member_num) const;

    // Returns the string datatype of the specified member in
    // this compound datatype.
    StrType getMemberStrType(unsigned member_num) const;

    // Returns the variable length datatype of the specified member in
    // this compound datatype.
    VarLenType getMemberVarLenType(unsigned member_num) const;

    // Returns the number of members in this compound datatype.
    int getNmembers() const;

    // Adds a new member to this compound datatype.
    void insertMember(const H5std_string& name, size_t offset, const DataType& new_member) const;

    // Recursively removes padding from within this compound datatype.
    void pack() const;

    // Sets the total size for this compound datatype.
    void setSize(size_t size) const;

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("CompType"); }

    // Noop destructor.
    virtual ~CompType();

  private:
    // Contains common code that is used by the member functions
    // getMemberXxxType
    hid_t p_get_member_type(unsigned member_num) const;

  }; // end of CompType

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class ArrayType
    \brief Class ArrayType inherits from DataType and provides wrappers for
    the HDF5's Array Datatypes.
  */
  // Inheritance: DataType -> H5Object -> H5Location -> IdComponent

  class H5_DLLCPP ArrayType : public DataType {
  public:
    // Constructor that creates a new array data type based on the
    // specified base type.
    ArrayType(const DataType& base_type, int ndims, const hsize_t* dims);

    // Assignment operator
    ArrayType& operator=(const ArrayType& rhs);

    // Constructors that open an array datatype, given a location.
    ArrayType(const H5Location& loc, const char* name);
    ArrayType(const H5Location& loc, const H5std_string& name);

    // Returns an ArrayType object via DataType* by decoding the
    // binary object description of this type.
    virtual DataType* decode() const;

    // Returns the number of dimensions of this array datatype.
    int getArrayNDims() const;
    //int getArrayNDims(); // removed 1.8.18 and 1.10.1

    // Returns the sizes of dimensions of this array datatype.
    int getArrayDims(hsize_t* dims) const;
    //int getArrayDims(hsize_t* dims); // removed 1.8.18 and 1.10.1

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("ArrayType"); }

    // Copy constructor: same as the original ArrayType.
    ArrayType(const ArrayType& original);

    // Constructor that takes an existing id
    ArrayType(const hid_t existing_id);

    // Noop destructor
    virtual ~ArrayType();

    // Default constructor
    ArrayType();

  }; // end of ArrayType

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class VarLenType
    \brief VarLenType is a derivative of a DataType and operates on HDF5
    Variable-length Datatypes.
  */
  //  Inheritance: DataType -> H5Object -> H5Location -> IdComponent

  class H5_DLLCPP VarLenType : public DataType {
  public:
    // Constructor that creates a variable-length datatype based
    // on the specified base type.
    VarLenType(const DataType& base_type);

    // Deprecated - will be removed after 1.10.2
    VarLenType(const DataType* base_type);

    // Returns an VarLenType object via DataType* by decoding the
    // binary object description of this type.
    virtual DataType* decode() const;

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("VarLenType"); }

    // Copy constructor: same as the original VarLenType.
    VarLenType(const VarLenType& original);

    // Constructor that takes an existing id
    VarLenType(const hid_t existing_id);

    // Constructors that open a variable-length datatype, given a location.
    VarLenType(const H5Location& loc, const char* name);
    VarLenType(const H5Location& loc, const H5std_string& name);

    // Noop destructor
    virtual ~VarLenType();

    // Default constructor
    VarLenType();

  }; // end of VarLenType

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class DataSet
    \brief Class DataSet operates on HDF5 datasets.

    An datasets has many characteristics similar to an attribute, thus both
    Attribute and DataSet are derivatives of AbstractDs.  DataSet also
    inherits from H5Object because a dataset is an HDF5 object.
  */
  //  Inheritance: multiple H5Object/AbstractDs -> H5Location -> IdComponent

  class H5_DLLCPP DataSet : public H5Object, public AbstractDs {
  public:

    // Close this dataset.
    virtual void close();

    // Extends the dataset with unlimited dimension.
    void extend(const hsize_t* size) const;

    // Fills a selection in memory with a value
    void fillMemBuf(const void *fill, const DataType& fill_type, void *buf, const DataType& buf_type, const DataSpace& space) const;
    //void fillMemBuf(const void *fill, DataType& fill_type, void *buf, DataType& buf_type, DataSpace& space); // removed from 1.8.18 and 1.10.1

    // Fills a selection in memory with zero
    void fillMemBuf(void *buf, const DataType& buf_type, const DataSpace& space) const;
    //void fillMemBuf(void *buf, DataType& buf_type, DataSpace& space); // removed from 1.8.18 and 1.10.1

    // Gets the creation property list of this dataset.
    DSetCreatPropList getCreatePlist() const;

    // Gets the access property list of this dataset.
    DSetAccPropList getAccessPlist() const;

    // Returns the address of this dataset in the file.
    haddr_t getOffset() const;

    // Gets the dataspace of this dataset.
    virtual DataSpace getSpace() const;

    // Determines whether space has been allocated for a dataset.
    void getSpaceStatus(H5D_space_status_t& status) const;

    // Returns the amount of storage size required for this dataset.
    virtual hsize_t getStorageSize() const;

    // Returns the in memory size of this attribute's data.
    virtual size_t getInMemDataSize() const;

    // Returns the number of bytes required to store VL data.
    hsize_t getVlenBufSize(const DataType& type, const DataSpace& space) const;
    //hsize_t getVlenBufSize(DataType& type, DataSpace& space) const; // removed from 1.8.18 and 1.10.1

#ifdef HDF5_V10
    // Reclaims VL datatype memory buffers.
    static void vlenReclaim(const DataType& type, const DataSpace& space, const DSetMemXferPropList& xfer_plist, void* buf);
    static void vlenReclaim(void *buf, const DataType& type, const DataSpace& space = DataSpace::ALL, const DSetMemXferPropList& xfer_plist = DSetMemXferPropList::DEFAULT);
#endif

    // Reads the data of this dataset and stores it in the provided buffer.
    // The memory and file dataspaces and the transferring property list
    // can be defaults.
    void read(void* buf, const DataType& mem_type, const DataSpace& mem_space = DataSpace::ALL, const DataSpace& file_space = DataSpace::ALL, const DSetMemXferPropList& xfer_plist = DSetMemXferPropList::DEFAULT) const;
    void read(H5std_string& buf, const DataType& mem_type, const DataSpace& mem_space = DataSpace::ALL, const DataSpace& file_space = DataSpace::ALL, const DSetMemXferPropList& xfer_plist = DSetMemXferPropList::DEFAULT) const;

    // Writes the buffered data to this dataset.
    // The memory and file dataspaces and the transferring property list
    // can be defaults.
    void write(const void* buf, const DataType& mem_type, const DataSpace& mem_space = DataSpace::ALL, const DataSpace& file_space = DataSpace::ALL, const DSetMemXferPropList& xfer_plist = DSetMemXferPropList::DEFAULT) const;
    void write(const H5std_string& buf, const DataType& mem_type, const DataSpace& mem_space = DataSpace::ALL, const DataSpace& file_space = DataSpace::ALL, const DSetMemXferPropList& xfer_plist = DSetMemXferPropList::DEFAULT) const;

    // Iterates the selected elements in the specified dataspace - not implemented in C++ style yet
    int iterateElems(void* buf, const DataType& type, const DataSpace& space, H5D_operator_t op, void* op_data = NULL);

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("DataSet"); }

#ifdef HDF5_V10
    // Creates a dataset by way of dereference.
    DataSet(const H5Location& loc, const void* ref, H5R_type_t ref_type = H5R_OBJECT, const PropList& plist = PropList::DEFAULT);
    DataSet(const Attribute& attr, const void* ref, H5R_type_t ref_type = H5R_OBJECT, const PropList& plist = PropList::DEFAULT);
#endif

    // Default constructor.
    DataSet();

    // Copy constructor - same as the original DataSet.
    DataSet(const DataSet& original);

    // Creates a copy of an existing DataSet using its id.
    DataSet(const hid_t existing_id);

    // Gets the dataset id.
    virtual hid_t getId() const;

    // Destructor: properly terminates access to this dataset.
    virtual ~DataSet();

  protected:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    // Sets the dataset id.
    virtual void p_setId(const hid_t new_id);
#endif // DOXYGEN_SHOULD_SKIP_THIS

  private:
    hid_t id;       // HDF5 dataset id

    // This function contains the common code that is used by
    // getTypeClass and various API functions getXxxType
    // defined in AbstractDs for generic datatype and specific
    // sub-types
    virtual hid_t p_get_type() const;

    // Reads variable or fixed len strings from this dataset.
    void p_read_fixed_len(const hid_t mem_type_id, const hid_t mem_space_id, const hid_t file_space_id, const hid_t xfer_plist_id, H5std_string& strg) const;
    void p_read_variable_len(const hid_t mem_type_id, const hid_t mem_space_id, const hid_t file_space_id, const hid_t xfer_plist_id, H5std_string& strg) const;

    // Friend function to set DataSet id.  For library use only.
    friend void f_DataSet_setId(DataSet* dset, hid_t new_id);

  }; // end of DataSet

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class CommonFG
    \brief \a CommonFG is an abstract base class of H5Group.
  */
  /* Note: This class is being deprecated gradually. */

  class H5_DLLCPP CommonFG {
  public:
  // Opens a generic named datatype in this location.
  DataType openDataType(const char* name) const;
  DataType openDataType(const H5std_string& name) const;

  // Opens a named array datatype in this location.
  ArrayType openArrayType(const char* name) const;
  ArrayType openArrayType(const H5std_string& name) const;

  // Opens a named compound datatype in this location.
  CompType openCompType(const char* name) const;
  CompType openCompType(const H5std_string& name) const;

  // Opens a named enumeration datatype in this location.
  EnumType openEnumType(const char* name) const;
  EnumType openEnumType(const H5std_string& name) const;

  // Opens a named integer datatype in this location.
  IntType openIntType(const char* name) const;
  IntType openIntType(const H5std_string& name) const;

  // Opens a named floating-point datatype in this location.
  FloatType openFloatType(const char* name) const;
  FloatType openFloatType(const H5std_string& name) const;

  // Opens a named string datatype in this location.
  StrType openStrType(const char* name) const;
  StrType openStrType(const H5std_string& name) const;

  // Opens a named variable length datatype in this location.
  VarLenType openVarLenType(const char* name) const;
  VarLenType openVarLenType(const H5std_string& name) const;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  /// For subclasses, H5File and Group, to return the correct
  /// object id, i.e. file or group id.
  virtual hid_t getLocId() const = 0;


  /// For subclasses, H5File and Group, to throw appropriate exception.
  virtual void throwException(const H5std_string& func_name, const H5std_string& msg) const = 0;

  // Default constructor.
  CommonFG();

  // Noop destructor.
  virtual ~CommonFG();

  protected:
  virtual void p_setId(const hid_t new_id) = 0;

#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of CommonFG

  /***************************************************************************
                                Design Note
                                ===========

September 2017:

        This class used to be base class of H5File as well, until the
        restructure that moved H5File to be subclass of H5Group.
  */
  // C++ informative line for the emacs editor: -*- C++ -*-
  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
   * Copyright by The HDF Group.                                               *
   * Copyright by the Board of Trustees of the University of Illinois.         *
   * All rights reserved.                                                      *
   *                                                                           *
   * This file is part of HDF5.  The full HDF5 copyright notice, including     *
   * terms governing use, modification, and redistribution, is contained in    *
   * the COPYING file, which can be found at the root of the source code       *
   * distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.  *
   * If you do not have access to either file, you may request a copy from     *
   * help@hdfgroup.org.                                                        *
   * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class Group
    \brief Class Group represents an HDF5 group.
  */
  //  Inheritance: CommonFG/H5Object -> H5Location -> IdComponent

  class H5_DLLCPP Group : public H5Object, public CommonFG {
  public:
    // Close this group.
    virtual void close();

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("Group"); }

    // Throw group exception.
    virtual void throwException(const H5std_string& func_name, const H5std_string& msg) const;

    // for CommonFG to get the file id.
    virtual hid_t getLocId() const;

#ifdef HDF5_V10
    // Creates a group by way of dereference.
    Group(const H5Location& loc, const void* ref, H5R_type_t ref_type = H5R_OBJECT, const PropList& plist = PropList::DEFAULT);
    // Removed in 1.10.1, because H5Location is baseclass
    //        Group(const Attribute& attr, const void* ref, H5R_type_t ref_type = H5R_OBJECT, const PropList& plist = PropList::DEFAULT);
#endif

    // Returns the number of objects in this group.
    hsize_t getNumObjs() const;

    // Opens an object within a group or a file, i.e., root group.
    hid_t getObjId(const char* name, const PropList& plist = PropList::DEFAULT) const;
    hid_t getObjId(const H5std_string& name, const PropList& plist = PropList::DEFAULT) const;

    // Closes an object opened by getObjId().
    void closeObjId(hid_t obj_id) const;

    // default constructor
    Group();

    // Copy constructor: same as the original Group.
    Group(const Group& original);

    // Gets the group id.
    virtual hid_t getId() const;

    // Destructor
    virtual ~Group();

    // Creates a copy of an existing group using its id.
    Group(const hid_t group_id);

  protected:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    // Sets the group id.
    virtual void p_setId(const hid_t new_id);
#endif // DOXYGEN_SHOULD_SKIP_THIS

  private:
    hid_t id;    // HDF5 group id

  }; // end of Group

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class H5File
    \brief Class H5File represents an HDF5 file and inherits from class Group
    as file is a root group.
  */
  //  Inheritance: Group -> CommonFG/H5Object -> H5Location -> IdComponent

  class H5_DLLCPP H5File : public Group {
  public:
    // Creates or opens an HDF5 file.
    H5File(const char* name, unsigned int flags,
           const FileCreatPropList& create_plist = FileCreatPropList::DEFAULT,
           const FileAccPropList& access_plist = FileAccPropList::DEFAULT);
    H5File(const H5std_string& name, unsigned int flags,
           const FileCreatPropList& create_plist = FileCreatPropList::DEFAULT,
           const FileAccPropList& access_plist = FileAccPropList::DEFAULT);

    // Open the file
    void openFile(const H5std_string& name, unsigned int flags,
                  const FileAccPropList& access_plist = FileAccPropList::DEFAULT);
    void openFile(const char* name, unsigned int flags,
                  const FileAccPropList& access_plist = FileAccPropList::DEFAULT);

    // Close this file.
    virtual void close();

    // Gets a copy of the access property list of this file.
    FileAccPropList getAccessPlist() const;

    // Gets a copy of the creation property list of this file.
    FileCreatPropList getCreatePlist() const;

#ifdef HDF5_V10
    // Gets general information about this file.
    void getFileInfo(H5F_info2_t& file_info) const;
#endif

    // Returns the amount of free space in the file.
    hssize_t getFreeSpace() const;

    // Returns the number of opened object IDs (files, datasets, groups
    // and datatypes) in the same file.
    ssize_t getObjCount(unsigned types = H5F_OBJ_ALL) const;

    // Retrieves a list of opened object IDs (files, datasets, groups
    // and datatypes) in the same file.
    void getObjIDs(unsigned types, size_t max_objs, hid_t *oid_list) const;

    // Returns the pointer to the file handle of the low-level file driver.
    void getVFDHandle(void **file_handle) const;
    void getVFDHandle(const FileAccPropList& fapl, void **file_handle) const;
    //void getVFDHandle(FileAccPropList& fapl, void **file_handle) const; // removed from 1.8.18 and 1.10.1

    // Returns the file size of the HDF5 file.
    hsize_t getFileSize() const;

#ifdef HDF5_V10
    // Returns the 'file number' of the HDF5 file.
    unsigned long getFileNum() const;
#endif

    // Determines if a file, specified by its name, is in HDF5 format
    static bool isHdf5(const char* name);
    static bool isHdf5(const H5std_string& name);

    // Determines if a file, specified by its name, can be accessed as HDF5
    static bool isAccessible(const char* name, const FileAccPropList& access_plist = FileAccPropList::DEFAULT);
    static bool isAccessible(const H5std_string& name, const FileAccPropList& access_plist = FileAccPropList::DEFAULT);

    // Reopens this file.
    void reOpen();  // added for better name

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    void reopen();  // obsolete in favor of reOpen()

    // Creates an H5File using an existing file id.  Not recommended
    // in applications.
    H5File(hid_t existing_id);

#endif // DOXYGEN_SHOULD_SKIP_THIS

    ///\brief Returns this class name.
    virtual H5std_string fromClass () const { return("H5File"); }

    // Throw file exception.
    virtual void throwException(const H5std_string& func_name, const H5std_string& msg) const;

    // For CommonFG to get the file id.
    virtual hid_t getLocId() const;

    // Default constructor
    H5File();

    // Copy constructor: same as the original H5File.
    H5File(const H5File& original);

    // Gets the HDF5 file id.
    virtual hid_t getId() const;

    // H5File destructor.
    virtual ~H5File();

  protected:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    // Sets the HDF5 file id.
    virtual void p_setId(const hid_t new_id);
#endif // DOXYGEN_SHOULD_SKIP_THIS

  private:
    hid_t id;  // HDF5 file id

    // This function is private and contains common code between the
    // constructors taking a string or a char*
    void p_get_file(const char* name, unsigned int flags, const FileCreatPropList& create_plist, const FileAccPropList& access_plist);

  }; // end of H5File

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  /*! \class H5Library
    \brief Class H5Library operates the HDF5 library globably.

    It is not necessary to construct an instance of H5Library to use the
    methods.
  */

  class H5_DLLCPP H5Library {
  public:
  // Initializes the HDF5 library.
  static void open();

  // Flushes all data to disk, closes files, and cleans up memory.
  static void close();

  // Instructs library not to install atexit cleanup routine
  static void dontAtExit();

  // Returns the HDF library release number.
  static void getLibVersion(unsigned& majnum, unsigned& minnum, unsigned& relnum);

  // Verifies that the arguments match the version numbers compiled
  // into the library
  static void checkVersion(unsigned majnum, unsigned minnum, unsigned relnum);

  // Walks through all the garbage collection routines for the library,
  // which are supposed to free any unused memory they have allocated.
  static void garbageCollect();

  // Sets limits on the different kinds of free lists.
  static void setFreeListLimits(int reg_global_lim, int reg_list_lim, int
                                arr_global_lim, int arr_list_lim, int blk_global_lim, int blk_list_lim);

  // Initializes C++ library and registers terminating functions at exit.
  // Only for the library functions, not for user-defined functions.
  static void initH5cpp(void);

  // Sends request for terminating the HDF5 library.
  static void termH5cpp(void);

#ifndef DOXYGEN_SHOULD_SKIP_THIS

  private:

  // Default constructor - no instance ever created from outsiders
  H5Library();

  // Destructor
  ~H5Library();
#endif // DOXYGEN_SHOULD_SKIP_THIS

  }; // end of H5Library

  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////

} // namespace Hdf5x

#endif // __Hdfx_HH
