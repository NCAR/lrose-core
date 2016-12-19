#include <string>
#include <NcUtils/NcxxGroup.hh>
#include <netcdf.h>

#ifndef NcxxFileClass
#define NcxxFileClass


//!  C++ API for netCDF4.
namespace netCDF
{

  /*!
    Class represents a netCDF root group.
    The Ncfile class is the same as the NcxxGroup class with the additional functionality for opening
    and closing files.
   */
  class NcxxFile : public NcxxGroup
   {
   public:

      enum FileMode
	 {
	    read,	//!< File exists, open read-only.
	    write,      //!< File exists, open for writing.
	    replace,	//!< Create new file, even if already exists.
	    newFile	//!< Create new file, fail if already exists.
	 };

      enum FileFormat
         {
	    classic,    //!< Classic format, classic data model
	    classic64,  //!< 64-bit offset format, classic data model
	    nc4,        //!< (default) netCDF-4/HDF5 format, enhanced data model
	    nc4classic  //!< netCDF-4/HDF5 format, classic data model
         };


      /*! Constructor generates a \ref isNull "null object". */
      NcxxFile();

      /*!
	Opens a netCDF file.
	\param filePath    Name of netCDF optional path.
	\param fMode       The file mode:
	                    - 'read'    File exists, open for read-only.
	                    - 'write'   File exists, open for writing.
	                    - 'replace' Create new file, even it already exists.
	                    - 'newFile' Create new file, fail it exists already.
      */
      NcxxFile(const std::string& filePath, FileMode fMode);
      /*!
      Opens a netCDF file.
      \param filePath    Name of netCDF optional path.
      \param fMode       The file mode:
                          - 'read'    File exists, open for read-only.
                          - 'write'   File exists, open for writing.
                          - 'replace' Create new file, even it already exists.
                          - 'newFile' Create new file, fail it exists already.
      */
      void open(const std::string& filePath, FileMode fMode);

      /*!
	Creates a netCDF file of a specified format.
	\param filePath    Name of netCDF optional path.
	\param fMode       The file mode:
	                    - 'replace' Create new file, even it already exists.
	                    - 'newFile' Create new file, fail it exists already.
      */
      NcxxFile(const std::string& filePath, FileMode fMode, FileFormat fFormat);
      /*!
      Creates a netCDF file of a specified format.
      \param filePath    Name of netCDF optional path.
      \param fMode       The file mode:
                          - 'replace' Create new file, even it already exists.
                          - 'newFile' Create new file, fail it exists already.
      */
      void open(const std::string& filePath, FileMode fMode, FileFormat fFormat);
     
     // open file for reading
     // Returns 0 on success, -1 on failure
     
     int openRead(const string &path);
  
     /// open file for writing
     /// set the netcdf format, before a write
     /// format options are:
     ///   classic - classic format (i.e. version 1 format)
     ///   classic64 - 64-bit offset format
     ///   nc4 - using HDF5 format
     ///   nc4classic - netCDF-4 using HDF5 but only netCDF-3 calls
     /// Returns 0 on success, -1 on failure

     int openWrite(const string &path,
                   NcxxFile::FileFormat format);

      //! Close a file before destructor call
      void close();

      /*! destructor */
      virtual ~NcxxFile(); //closes file and releases all resources

      //! Synchronize an open netcdf dataset to disk
      void sync();

      //! Leave define mode, used for classic model
      void enddef();

     ///////////////////////////////////////////
     // add string global attribute
     // Returns 0 on success, -1 on failure
     
     int addGlobAttr(const string &name, const string &val);

     ///////////////////////////////////////////
     // add int global attribute
     // Returns 0 on success, -1 on failure
     
     int addGlobAttr(const string &name, int val);

     ///////////////////////////////////////////
     // add float global attribute
     // Returns 0 on success, -1 on failure
     
     int addGlobAttr(const string &name, float val);

     ///////////////////////////////////////////
     // read a global attribute
     // Returns 0 on success, -1 on failure
     
     int readGlobAttr(const string &name, string &val);
     int readGlobAttr(const string &name, int &val);
     int readGlobAttr(const string &name, float &val);
     int readGlobAttr(const string &name, double &val);

     ///////////////////////////////////////////
     // add a dimension
     // Returns 0 on success, -1 on failure
     // Side effect: dim arg is updated
     
     int addDim(NcxxDim &dim, const string &name, int size);

     ///////////////////////////////////////////
     // read a dimension
     // Returns 0 on success, -1 on failure
     // Side effect: dim arg is set
     
     int readDim(const string &name, NcxxDim &dim);

     //////////////////////////////////////////////
     // Add scalar var
     // Returns 0 on success, -1 on failure
     // Side effect: var is set
     
     int addVar(NcxxVar &var,
                const string &name, 
                const string &standardName,
                const string &longName,
                NcxxType ncType, 
                const string &units = "");

     ///////////////////////////////////////
     // Add 1-D array var
     // Returns 0 on success, -1 on failure
     // Side effect: var is set
     
     int addVar(NcxxVar &var, 
                const string &name, 
                const string &standardName,
                const string &longName,
                NcxxType ncType, 
                NcxxDim &dim, 
                const string &units = "");
     
     ///////////////////////////////////////
     // Add 2-D array var
     // Returns 0 on success, -1 on failure
     // Side effect: var is set
     
     int addVar(NcxxVar &var, 
                const string &name,
                const string &standardName,
                const string &longName,
                NcxxType ncType,
                NcxxDim &dim0,
                NcxxDim &dim1,
                const string &units = "");

     ///////////////////////////////////////
     // Add var in multiple-dimensions
     // Returns 0 on success, -1 on failure
     // Side effect: var is set
     
     int addVar(NcxxVar &var, 
                const string &name,
                const string &standardName,
                const string &longName,
                NcxxType ncType,
                vector<NcxxDim> &dims,
                const string &units = "");
     
   private:

     string _pathInUse;
     FileMode _mode;
     FileFormat _format;

     /* Do not allow definition of NcxxFile involving copying any NcxxFile or NcxxGroup.
        Because the destructor closes the file and releases al resources such 
        an action could leave NcxxFile objects in an invalid state */
     NcxxFile& operator =(const NcxxGroup & rhs);
     NcxxFile& operator =(const NcxxFile & rhs);
     NcxxFile(const NcxxGroup& rhs);
     NcxxFile(const NcxxFile& rhs);
     
     
  };

}


#endif
