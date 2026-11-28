/**
 *
 * @file HdfFile.cc
 *
 * @class HdfFile
 *
 * Class controlling access to a GPM level 3 HDF5 file.
 */

#include <iostream>

#include <toolsa/Path.hh>
#include <rapmath/math_macros.h>
#include <cstdlib>
#include "HdfFile.hh"

HdfFile::HdfFile(const string &file_path,
		 const bool debug_flag, const bool verbose_flag) :
  _debug(debug_flag),
  _verbose(verbose_flag),
  _filePath(file_path)
{
}

HdfFile::~HdfFile()
{
  _file.close();
}

bool HdfFile::init()
{
  static const string methodName = "HdfFile::init(): ";
  
  if (_verbose)
    cerr << methodName << "Instantiating H5File object" << endl;
  
  try
  {
    /*
     * Turn off the auto-printing when failure occurs so that we can
     * handle the errors appropriately
     */
    Exception::dontPrint();

    H5std_string FILE_NAME(_filePath);
    
    /*
     * Open the specified file and the specified dataset in the file.
     */
    _file.openFile( FILE_NAME, H5F_ACC_RDONLY );
    
  }
  catch( FileIException &error )
  {
    error.printErrorStack();

    return false;
  }
  return true;
}

void HdfFile::getTimes( DateTime &beginTime, DateTime &endTime)
{
  //
  // Assumption is made that start and stop strings are stored in the 
  // the "FileHeader" attribute string following 'StartGranuleDateTime='
  // and 'StopGranuleDateTime=' with format yyyy-mm-ddThh:MM:ss.sss
  //
  static const string methodName = "HdfFile::getTimes(): ";

  Group fileGroup = _file.openGroup("/");

  hid_t groupId = fileGroup.getId();

  Attribute fileAttr = H5Aopen_name( groupId ,"FileHeader");

  DataType fileAttrDataType = fileAttr.getDataType();

  string  attStr;

  fileAttr.read(fileAttrDataType, attStr);

  string startIndicator("StartGranuleDateTime=");

  size_t index = attStr.rfind(startIndicator);

  string startStr = attStr.substr(index + (int)startIndicator.size(), 24);
  
  if (_verbose)
  {
    cerr << methodName << "Start time string: " << startStr << endl;
  }

  string endIndicator("StopGranuleDateTime=");

  index = attStr.rfind(endIndicator);

  string endStr = attStr.substr(index + (int)endIndicator.size(), 24);
  
  if (_verbose)
  {
    cerr << methodName << "End time string: " << endStr << endl;
  }

  beginTime.setFromW3c(startStr.c_str(), true);

  endTime.setFromW3c(endStr.c_str(), true);
}

bool HdfFile::getDataset(const string datasetName, void** mdvData, 
			 fl32 &missingDataVal, string &units,
			 HdfDataType_t &dataType)
{
  static const string methodName = "HdfFile::getDataset(): ";

  units ="none";
  try 
  {
    if (_verbose)
    {
      cerr << "Getting dataset " <<  datasetName.c_str() << endl;
    }

    DataSet dataset = _file.openDataSet(datasetName);

    hid_t datasetId = dataset.getId();

    //
    // Get the missing data value from a dataset attribute.
    // The assumption is made that this attribute exists, is named
    // "CodeMissingValue", and is a string data type
    //
    Attribute attrMissingData = H5Aopen_name( datasetId ,"CodeMissingValue");

    DataType  dtypeMissing = attrMissingData.getDataType();

    hid_t missingTypeId = attrMissingData.getId();

    string  missingDataStr;

    attrMissingData.read(dtypeMissing, missingDataStr);

    missingDataVal = atof(missingDataStr.c_str());

    attrMissingData.close();

    if (_verbose)
    {
      cerr << methodName << "Missing: " << missingDataStr.c_str() << endl;
    }

    //
    // Get the Units string from a dataset attribute
    //
   
    hid_t  unitsAttr = H5Aopen_name( datasetId ,"Units");
    
    if (unitsAttr < 0)
    {
      if (_verbose)
      {
	cerr << methodName << "No units attribute found " << endl;
      }
    }
    else
    {
      //
      // The assumption is made that this attribute is labeled "Units"
      // and is string data type.
      //
      Attribute attrUnitsStr = H5Aopen_name( datasetId ,"Units");
      
      DataType dtypeUnits = attrUnitsStr.getDataType();
      
      attrUnitsStr.read( dtypeUnits, units);
      
      attrUnitsStr.close();
      
      if(_verbose)
      {
	cerr << methodName  << "Units: " << units.c_str() << endl;
      }
    }
    
    //
    // Get the class of the datatype that is used by the dataset.
    //
    H5T_class_t type_class = dataset.getTypeClass();
    
    //
    // Get class of datatype and print message if it's an integer.
    // The assumption is made that the dataset has data type 
    // int8, int16, or float32 based on knowledge of the 
    // contents gpm level 3 file
    // 
    if( type_class == H5T_INTEGER )
    {
      IntType intType = dataset.getIntType();

      int size = intType.getSize();

      if (size == 1)
      {
	dataType = HDF_DATA_INT8;
      }
      else if (size == 2)
      {
	dataType = HDF_DATA_INT16;
      }
    }
    else
    {
      dataType = HDF_DATA_FLOAT32;
    }

    //
    // Get dataspace of the dataset.
    //
    DataSpace dataspace = dataset.getSpace();

    //
    // Get the class of the datatype that is used by the dataset.
    //
    DataType dtype = dataset.getDataType();

    //
    // Get the number of dimensions in the dataspace.
    //
    dataspace.getSimpleExtentNdims();
    
    //
    // Get the dimension size of each dimension in the dataspace and
    // display them.
    //
    hsize_t dims_out[2];
  
    dataspace.getSimpleExtentDims( dims_out, NULL);
   
    int nx = dims_out[0];

    int ny = dims_out[1];
    
    if (dataType == HDF_DATA_INT8)
    {
      //
      // Create space for 8 bit integer dataset
      // 
      ui08 *hdata = new ui08[nx*ny];
      
      dataset.read( hdata, dtype, dataspace);

      *mdvData = new ui08[ny*nx];
      
      if (*mdvData == NULL)
      {
	cerr << "ERROR: " << methodName << "Memory allocation failed dataset " <<  datasetName.c_str() << endl;
      }

      for (int j = 0; j < ny; j++)
      {
	for (int i = 0; i < nx; i++)
	{
	  ( (ui08*) *mdvData)[j*nx + i] = hdata[i*ny + j];
	}
      }
      delete [] hdata;
    }
    else if (dataType == HDF_DATA_INT16)
    {
      //
      // Create space for 16 bit integer dataset
      // 
      si16 *hdata = new si16[nx*ny];
      
      dataset.read( hdata, dtype, dataspace);

      *mdvData = new si16[ny*nx];
      
      if (*mdvData == NULL)
      {
	cerr  << "ERROR: " << methodName << "Memory allocation failed dataset " <<  datasetName.c_str() << endl;
      }

      for (int j = 0; j < ny; j++)
      {
	for (int i = 0; i < nx; i++)
	{
	  ( (si16*) *mdvData)[j*nx + i] = hdata[i*ny + j];
	}
      }
      delete [] hdata;
    }
    else
    {
      //
      // Create space 32 bit floating point for dataset
      // 
      fl32 *hdata = new fl32[nx*ny];
      
      dataset.read( hdata, dtype, dataspace);

      *mdvData = new fl32[ny*nx];
      
      if (*mdvData == NULL)
	cerr << "ERROR: " << methodName << " Memory allocation failed dataset " <<  datasetName.c_str() << endl;
      
      for (int j = 0; j < ny; j++)
      {
	for (int i = 0; i < nx; i++)
	{
	  ( (fl32*) *mdvData)[j*nx + i] = hdata[i*ny + j];
	}
      }
      delete [] hdata;
    }

    if (_verbose)
    {
      cerr << methodName << "Successfully retrieved dataset " <<  datasetName.c_str() << endl;    
    }
  } // end of try block
  
  catch( FileIException &error )
  {
    error.printErrorStack();
    return false;
  }
  catch( DataSetIException &error )
  {
    error.printErrorStack();
    return false;
  }
  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException &error )
  {
    error.printErrorStack();
    return false;
  }
  // catch failure caused by the DataSpace operations
  catch( DataTypeIException &error )
  {
    error.printErrorStack();
    return false;
  }
  catch ( AttributeIException &error)
  { 
    error.printErrorStack();
    return false;
  }

  return true;
}
