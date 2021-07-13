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
/////////////////////////////////////////////////////////////
// Hdf5xx.cc
//
// HDF5 utilities
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2016
//
///////////////////////////////////////////////////////////////

#include <Ncxx/Hdf5xx.hh>
#include <Ncxx/Ncxx.hh>
#include <Ncxx/ByteOrder.hh>
#include <cstring>
#include <cmath>

//////////////////////////////////////////////////
// get float val for a specific comp header member

int Hdf5xx::loadFloatVar(CompType compType,
                         char *buf,
                         const string &varName,
                         NcxxPort::fl64 &floatVal)
  
{
  
  clearErrStr();  
  
  bool isInt, isFloat, isString;
  NcxxPort::si64 intVal;
  string stringVal;
  
  if (loadCompVar(compType, buf, varName,
                  isInt, isFloat, isString,
                  intVal, floatVal, stringVal)) {
    return -1;
  }
  if (!isFloat) {
    _addErrStr("Incorrect type for comp variable: ", varName);
    _addErrStr("  Should be float type");
    if (isInt) {
      _addErrStr("  is int type instead");
    } else if (isString) {
      _addErrStr("  is string type instead");
    }
    return -1;
  }
  return 0;

}

//////////////////////////////////////////////////
// get int val for a specific comp header member

int Hdf5xx::loadIntVar(CompType compType,
                       char *buf,
                       const string &varName,
                       NcxxPort::si64 &intVal)

{

  clearErrStr();  

  bool isInt, isFloat, isString;
  NcxxPort::fl64 floatVal;
  string stringVal;

  if (loadCompVar(compType, buf, varName,
                  isInt, isFloat, isString,
                  intVal, floatVal, stringVal)) {
    return -1;
  }
  if (!isInt) {
    _addErrStr("Incorrect type for comp variable: ", varName);
    _addErrStr("  Should be int type");
    if (isFloat) {
      _addErrStr("  is float type instead");
    } else if (isString) {
      _addErrStr("  is string type instead");
    }
    return -1;
  }
  return 0;

}

//////////////////////////////////////////////////
// get string val for a specific comp header member

int Hdf5xx::loadStringVar(CompType compType,
                          char *buf,
                          const string &varName,
                          string &stringVal)

{

  clearErrStr();  

  bool isInt, isFloat, isString;
  NcxxPort::si64 intVal;
  NcxxPort::fl64 floatVal;

  if (loadCompVar(compType, buf, varName,
                  isInt, isFloat, isString,
                  intVal, floatVal, stringVal)) {
    return -1;
  }
  if (!isString) {
    _addErrStr("Incorrect type for comp variable: ", varName);
    _addErrStr("  Should be string type");
    if (isInt) {
      _addErrStr("  is int type instead");
    } else if (isFloat) {
      _addErrStr("  is float type instead");
    }
    return -1;
  }
  return 0;

}


/////////////////////////////////////////////////
// get val for a specific compound header member

int Hdf5xx::loadCompVar(CompType compType,
                        char *buf,
                        const string &varName,
                        bool &isInt,
                        bool &isFloat,
                        bool &isString,
                        NcxxPort::si64 &intVal,
                        NcxxPort::fl64 &floatVal,
                        string &stringVal)


{

  isInt = false;
  isFloat = false;
  isString = false;

  int nMembers = compType.getNmembers();
  int index = -1;
  try {
    index = compType.getMemberIndex(varName.c_str());
  }
  catch (const H5x::Exception &e) {
    _addErrStr("Cannot find comp variable: ", varName);
    return -1;
  }
  if (index >= nMembers) {
    _addErrStr("Bad index for comp variable: ", varName);
    _addErrInt("  index: ", index);
    _addErrInt("  mMembers: ", nMembers);
    return -1;
  }

  DataType dtype = compType.getMemberDataType(index);
  int offset = compType.getMemberOffset(index);
  H5T_class_t mclass = compType.getMemberClass(index);

  if (mclass == H5T_INTEGER) {

    isInt = true;

    IntType intType = compType.getMemberIntType(index);
    H5T_order_t order = intType.getOrder();
    H5T_sign_t sign = intType.getSign();
    size_t tsize = intType.getSize();

    if (sign == H5T_SGN_NONE) {

      // unsigned

      if (tsize == 1) {

        NcxxPort::ui08 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        intVal = (int) ival;

      } else if (tsize == 2) {

        NcxxPort::ui16 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap16(&ival, sizeof(ival), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap16(&ival, sizeof(ival), true);
          }
        }
        intVal = ival;

      } else if (tsize == 4) {

        NcxxPort::ui32 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap32(&ival, sizeof(ival), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap32(&ival, sizeof(ival), true);
          }
        }
        intVal = ival;

      } else if (tsize == 8) {

        NcxxPort::ui64 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap64(&ival, sizeof(ival), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap64(&ival, sizeof(ival), true);
          }
        }
        intVal = ival;

      }

    } else {

      // signed

      if (tsize == 1) {

        NcxxPort::si08 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        intVal = (int) ival;

      } else if (tsize == 2) {

        NcxxPort::si16 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap16(&ival, sizeof(ival), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap16(&ival, sizeof(ival), true);
          }
        }
        intVal = ival;

      } else if (tsize == 4) {

        NcxxPort::si32 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap32(&ival, sizeof(ival), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap32(&ival, sizeof(ival), true);
          }
        }
        intVal = ival;

      } else if (tsize == 8) {

        NcxxPort::si64 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap64(&ival, sizeof(ival), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap64(&ival, sizeof(ival), true);
          }
        }
        intVal = ival;

      }

    }

  } else if (mclass == H5T_FLOAT) {

    isFloat = true;

    FloatType flType = compType.getMemberFloatType(index);
    H5T_order_t order = flType.getOrder();
    size_t tsize = flType.getSize();

    if (tsize == 4) {

      NcxxPort::fl32 fval;
      memcpy(&fval, buf + offset, sizeof(fval));
      if (NcxxPort::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          NcxxPort::swap32(&fval, sizeof(fval), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          NcxxPort::swap32(&fval, sizeof(fval), true);
        }
      }
      floatVal = fval;

    } else if (tsize == 8) {

      NcxxPort::fl64 fval;
      memcpy(&fval, buf + offset, sizeof(fval));
      if (NcxxPort::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          NcxxPort::swap64(&fval, sizeof(fval), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          NcxxPort::swap64(&fval, sizeof(fval), true);
        }
      }
      floatVal = fval;

    }

  } else if (mclass == H5T_STRING) {

    isString = true;

    StrType strType = compType.getMemberStrType(index);
    int strLen = strType.getSize();
    char *str = new char[strLen + 1];
    memcpy(str, buf + offset, strLen);
    str[strLen] = '\0';
    stringVal = str;
    delete [] str;

  } else if (mclass == H5T_COMPOUND) {

    _addErrStr("Found nested compound type for variable: ", varName);
    _addErrStr("  Cannot deal with nested compound types");
    return -1;

  }

  return 0;

}

///////////////////////////////////////////////////////////////////
// load an attribute from an object, given the name
// returns 0 on success, -1 on failure

int Hdf5xx::loadAttribute(H5Object &obj,
                          const string &name,
                          const string &context,
                          DecodedAttr &decodedAttr)

{

  clearErrStr();
  decodedAttr.clear();

  Attribute *attr = NULL;
  try {
    attr = new Attribute(obj.openAttribute(name));
  }
  catch (const H5x::Exception &e) {
    _addErrStr("Hdf5xx::loadAttribute");
    _addErrStr("  Cannot find attribute, name: ", name);
    _addErrStr("  Context: ", context);
    if (attr) delete attr;
    return -1;
  }

  DataType dtype = attr->getDataType();
  H5T_class_t aclass = dtype.getClass();
  DataSpace dataspace = attr->getSpace();
  int ndims = dataspace.getSimpleExtentNdims();
  int npoints = dataspace.getSimpleExtentNpoints();
  vector<hsize_t> dims;
  dims.resize(ndims);
  if (ndims > 0) {
    dataspace.getSimpleExtentDims(dims.data());
  }

  if (aclass == H5T_INTEGER) {

    IntType intType = attr->getIntType();
    H5T_order_t order = intType.getOrder();
    H5T_sign_t sign = intType.getSign();
    size_t tsize = intType.getSize();
    NcxxPort::si64 lval = 0;

    if (sign == H5T_SGN_NONE) {

      // unsigned

      if (tsize == 1) {

        vector<NcxxPort::ui08> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        lval = (int) ivals[0];

      } else if (tsize == 2) {

        vector<NcxxPort::ui16> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::ui16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::ui16), true);
          }
        }
        lval = ivals[0];

      } else if (tsize == 4) {

        vector<NcxxPort::ui32> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::ui32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::ui32), true);
          }
        }
        lval = ivals[0];

      } else if (tsize == 8) {

        vector<NcxxPort::ui64> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::ui64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::ui64), true);
          }
        }
        lval = ivals[0];
      }

    } else {

      // signed

      if (tsize == 1) {

        vector<NcxxPort::si08> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        lval = ivals[0];

      } else if (tsize == 2) {

        vector<NcxxPort::si16> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::si16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::si16), true);
          }
        }
        lval = ivals[0];

      } else if (tsize == 4) {

        vector<NcxxPort::si32> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::si32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::si32), true);
          }
        }
        lval = ivals[0];

      } else if (tsize == 8) {

        vector<NcxxPort::si64> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::si64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::si64), true);
          }
        }
        lval = ivals[0];
      }

    }

    decodedAttr.setAsInt(lval);

  } else if (aclass == H5T_FLOAT) {

    FloatType flType = attr->getFloatType();
    H5T_order_t order = flType.getOrder();
    size_t tsize = flType.getSize();
    NcxxPort::fl64 dval = 0;

    if (tsize == 4) {

      vector<NcxxPort::fl32> fvals;
      fvals.resize(npoints);
      attr->read(dtype, fvals.data());
      if (NcxxPort::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          NcxxPort::swap32(fvals.data(), npoints * sizeof(NcxxPort::fl32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          NcxxPort::swap32(fvals.data(), npoints * sizeof(NcxxPort::fl32), true);
        }
      }
      dval = fvals[0];

    } else if (tsize == 8) {

      vector<NcxxPort::fl64> fvals;
      fvals.resize(npoints);
      attr->read(dtype, fvals.data());
      if (NcxxPort::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          NcxxPort::swap64(fvals.data(), npoints * sizeof(NcxxPort::fl64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          NcxxPort::swap64(fvals.data(), npoints * sizeof(NcxxPort::fl64), true);
        }
      }
      dval = fvals[0];

    }

    decodedAttr.setAsDouble(dval);

  } else if (aclass == H5T_STRING) {

    StrType strType = attr->getStrType();
    H5std_string sval;
    attr->read(dtype, sval);
    decodedAttr.setAsString(sval);
    
  }

  // clean up

  delete attr;

  return 0;

}

///////////////////////////////////////////////////////////////////
// load an array attribute from an object, given the name
// returns 0 on success, -1 on failure

int Hdf5xx::loadArrayAttribute(H5Object &obj,
                               const string &name,
                               const string &context,
                               ArrayAttr &arrayAttr)
  
{

  clearErrStr();
  arrayAttr.clear();

  Attribute *attr = NULL;
  try {
    attr = new Attribute(obj.openAttribute(name));
  }
  catch (const H5x::Exception &e) {
    _addErrStr("Hdf5xx::loadArrayAttribute");
    _addErrStr("  Cannot find attribute, name: ", name);
    _addErrStr("  Context: ", context);
    if (attr) delete attr;
    return -1;
  }

  DataType dtype = attr->getDataType();
  H5T_class_t aclass = dtype.getClass();
  DataSpace dataspace = attr->getSpace();
  int ndims = dataspace.getSimpleExtentNdims();
  size_t npoints = dataspace.getSimpleExtentNpoints();
  vector<hsize_t> dims;
  dims.resize(ndims);
  if (ndims > 0) {
    dataspace.getSimpleExtentDims(dims.data());
  }

  if (aclass == H5T_INTEGER) {

    IntType intType = attr->getIntType();
    H5T_order_t order = intType.getOrder();
    H5T_sign_t sign = intType.getSign();
    size_t tsize = intType.getSize();
    vector<NcxxPort::si64> lvals;
    lvals.resize(npoints);
    
    if (sign == H5T_SGN_NONE) {

      // unsigned
      
      if (tsize == 1) {

        vector<NcxxPort::ui08> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = (int) ivals.data()[ii];
        }

      } else if (tsize == 2) {
        
        vector<NcxxPort::ui16> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::ui16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::ui16), true);
          }
        }
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = ivals[ii];
        }

      } else if (tsize == 4) {

        vector<NcxxPort::ui32> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::ui32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::ui32), true);
          }
        }
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = ivals[ii];
        }

      } else if (tsize == 8) {

        vector<NcxxPort::ui64> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::ui64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::ui64), true);
          }
        }
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = ivals[ii];
        }
      }

    } else {

      // signed

      if (tsize == 1) {
        
        vector<NcxxPort::si08> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = (int) ivals[ii];
        }

      } else if (tsize == 2) {

        vector<NcxxPort::si16> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::si16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::si16), true);
          }
        }
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = ivals[ii];
        }

      } else if (tsize == 4) {

        vector<NcxxPort::si32> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::si32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::si32), true);
          }
        }
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = ivals[ii];
        }
        
      } else if (tsize == 8) {

        vector<NcxxPort::si64> ivals;
        ivals.resize(npoints);
        attr->read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::si64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::si64), true);
          }
        }
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = ivals[ii];
        }
      }

    }
    
    arrayAttr.setAsInts(lvals.data(), npoints);

  } else if (aclass == H5T_FLOAT) {

    FloatType flType = attr->getFloatType();
    H5T_order_t order = flType.getOrder();
    size_t tsize = flType.getSize();
    vector<NcxxPort::fl64> dvals;
    dvals.resize(npoints);

    if (tsize == 4) {
      
      vector<NcxxPort::fl32> fvals;
      fvals.resize(npoints);
      attr->read(dtype, fvals.data());
      if (NcxxPort::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          NcxxPort::swap32(fvals.data(), npoints * sizeof(NcxxPort::fl32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          NcxxPort::swap32(fvals.data(), npoints * sizeof(NcxxPort::fl32), true);
        }
      }
      for (size_t ii = 0; ii < npoints; ii++) {
        dvals[ii] = fvals[ii];
      }

    } else if (tsize == 8) {

      vector<NcxxPort::fl64> fvals;
      fvals.resize(npoints);
      attr->read(dtype, fvals.data());
      if (NcxxPort::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          NcxxPort::swap64(fvals.data(), npoints * sizeof(NcxxPort::fl64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          NcxxPort::swap64(fvals.data(), npoints * sizeof(NcxxPort::fl64), true);
        }
      }
      for (size_t ii = 0; ii < npoints; ii++) {
        dvals[ii] = fvals[ii];
      }

    }

    arrayAttr.setAsDoubles(dvals.data(), npoints);

  }

  // clean up

  delete attr;

  return 0;

}

///////////////////////////////////////////////////////////////////
// Enquire about the properties of a variable
// Returns 0 on success, -1 on failure
// On success, sets the following:
//    dims     - dimensions
//    units    - string
//    h5class  - H5T_INTEGER or H5T_FLOAT
//    h5sign   - H5T_SGN_NONE if unsigned integer, otherwise signed
//               does not apply to floats of course
//    h5order  - H5T_ORDER_LE or H5T_ORDER_BE
//    h5size   - length of data type in bytes

int Hdf5xx::getVarProps(Group &group,
                        const string &dsName,
                        vector<size_t> &dims,
                        string &units,
                        H5T_class_t &h5class,
                        H5T_sign_t &h5sign,
                        H5T_order_t &h5order,
                        size_t &h5size)
  
{
  
  string groupName = group.getObjName();
  string context(groupName);
  context += "-";
  context += dsName;

  if (!group.nameExists(dsName)) {
    return -1;
  }

  // get data space for this data set
  
  DataSet dset = group.openDataSet(dsName);
  DataSpace dspace = dset.getSpace();
  
  // set the units
  
  units = "";
  DecodedAttr unitsAtt;
  if (dset.attrExists("Units")) {
    if (loadAttribute(dset, "Units", context, unitsAtt) == 0) {
      units = unitsAtt.getAsString();
    }
  } else if (dset.attrExists("units")) {
    if (loadAttribute(dset, "units", context, unitsAtt) == 0) {
      units = unitsAtt.getAsString();
    }
  }

  // determine the dimensions

  dims.clear();
  int nDims = dspace.getSimpleExtentNdims();
  vector<hsize_t> hdims;
  hdims.resize(nDims);
  dspace.getSimpleExtentDims(hdims.data());
  dims.clear();
  for (size_t ii = 0; ii < hdims.size(); ii++) {
    dims.push_back(hdims[ii]);
  }

  // class, sign, order and size
  
  DataType dtype = dset.getDataType();
  h5class = dtype.getClass();
  
  if (h5class == H5T_INTEGER) {
    
    IntType intType = dset.getIntType();
    h5order = intType.getOrder();
    h5sign = intType.getSign();
    h5size = intType.getSize();
    
  } else if (h5class == H5T_FLOAT) {
    
    FloatType flType = dset.getFloatType();
    h5order = flType.getOrder();
    h5size = flType.getSize();

  } else {

    _addErrStr("Hdf5xx::getVarProps()");
    _addErrStr("  Data is not integer of float", dsName);
    _addErrStr("  Context: ", context);
    return -1;

  }

  return 0;

}

///////////////////////////////////////////////////////////////////
// Read data set into 32-bit int array
// Fills in dims, missingVal, vals, units (if available)
// returns 0 on success, -1 on failure

int Hdf5xx::readSi32Array(Group &group,
                          const string &dsName,
                          const string &context,
                          vector<size_t> &dims,
                          NcxxPort::si32 &missingVal,
                          vector<NcxxPort::si32> &vals,
                          string &units)
  
{

  if (!group.nameExists(dsName)) {
    return -1;
  }

  // get data space for this data set
  
  DataSet dset = group.openDataSet(dsName);
  DataSpace dspace = dset.getSpace();
  DataType dtype = dset.getDataType();
  H5T_class_t aclass = dtype.getClass();
  if (aclass != H5T_INTEGER) {
    _addErrStr("Hdf5xx::readSi32Array()");
    _addErrStr("  Data is not an integer type, name: ", dsName);
    _addErrStr("  Context: ", context);
    _addErrStr("  You need to read this as a float");
    return -1;
  }

  // set the missing value (_fillValue)

  missingVal = -9999.0;
  if (dset.attrExists("_fillValue")) {
    DecodedAttr fillValueAtt;
    if (loadAttribute(dset, "_fillValue", context, fillValueAtt) == 0) {
      missingVal = fillValueAtt.getAsInt();
    }
  } else if (dset.attrExists("_FillValue")) {
    DecodedAttr FillValueAtt;
    if (loadAttribute(dset, "_FillValue", context, FillValueAtt) == 0) {
      missingVal = FillValueAtt.getAsInt();
    }
  }
  
  // set the units
  
  units = "";
  DecodedAttr unitsAtt;
  if (dset.attrExists("Units")) {
    if (loadAttribute(dset, "Units", context, unitsAtt) == 0) {
      units = unitsAtt.getAsString();
    }
  } else if (dset.attrExists("units")) {
    if (loadAttribute(dset, "units", context, unitsAtt) == 0) {
      units = unitsAtt.getAsString();
    }
  }

  // determine the dimensions
  
  int nDims = dspace.getSimpleExtentNdims();
  vector<hsize_t> hdims;
  hdims.resize(nDims);
  dspace.getSimpleExtentDims(hdims.data());
  dims.clear();
  for (size_t ii = 0; ii < hdims.size(); ii++) {
    dims.push_back(hdims[ii]);
  }

  // allocate space for the data values
  
  hssize_t nPoints = dspace.getSimpleExtentNpoints();
  vals.resize(nPoints);

  // read in the data depending on the type
  
  IntType intType = dset.getIntType();
  H5T_order_t order = intType.getOrder();
  H5T_sign_t sign = intType.getSign();
  size_t tsize = intType.getSize();
  
  if (sign == H5T_SGN_NONE) {
    
    // unsigned
    
    if (tsize == 1) {
      
      vector<NcxxPort::ui08> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
      
    } else if (tsize == 2) {
      
      vector<NcxxPort::ui16> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::ui16), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::ui16), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
      
    } else if (tsize == 4) {
      
      vector<NcxxPort::ui32> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::ui32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::ui32), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
      
    } else if (tsize == 8) {
      
      vector<NcxxPort::ui64> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::ui64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::ui64), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
    }
    
  } else {
    
    // signed
    
    if (tsize == 1) {
      
      vector<NcxxPort::si08> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
      
    } else if (tsize == 2) {
      
      vector<NcxxPort::si16> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::si16), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::si16), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
      
    } else if (tsize == 4) {
      
      vector<NcxxPort::si32> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::si32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::si32), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
      
    } else if (tsize == 8) {
      
      vector<NcxxPort::si64> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::si64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::si64), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
    }
    
  }

  return 0;

}

///////////////////////////////////////////////////////////////////
// Read data set into 16-bit int array
// Fills in dims, missingVal, vals, units (if available)
// returns 0 on success, -1 on failure

int Hdf5xx::readSi16Array(Group &group,
                          const string &dsName,
                          const string &context,
                          vector<size_t> &dims,
                          NcxxPort::si16 &missingVal,
                          vector<NcxxPort::si16> &vals,
                          string &units)
  
{

  if (!group.nameExists(dsName)) {
    return -1;
  }

  // get data space for this data set
  
  DataSet dset = group.openDataSet(dsName);
  DataSpace dspace = dset.getSpace();
  DataType dtype = dset.getDataType();
  H5T_class_t aclass = dtype.getClass();
  if (aclass != H5T_INTEGER) {
    _addErrStr("Hdf5xx::readSi16Array()");
    _addErrStr("  Data is not an integer type, name: ", dsName);
    _addErrStr("  Context: ", context);
    _addErrStr("  You need to read this as a float");
    return -1;
  }

  // set the missing value (_fillValue)

  missingVal = -9999.0;
  if (dset.attrExists("_fillValue")) {
    DecodedAttr fillValueAtt;
    if (loadAttribute(dset, "_fillValue", context, fillValueAtt) == 0) {
      missingVal = fillValueAtt.getAsInt();
    }
  } else if (dset.attrExists("_FillValue")) {
    DecodedAttr FillValueAtt;
    if (loadAttribute(dset, "_FillValue", context, FillValueAtt) == 0) {
      missingVal = FillValueAtt.getAsInt();
    }
  }
  
  // set the units
  
  units = "";
  DecodedAttr unitsAtt;
  if (dset.attrExists("Units")) {
    if (loadAttribute(dset, "Units", context, unitsAtt) == 0) {
      units = unitsAtt.getAsString();
    }
  } else if (dset.attrExists("units")) {
    if (loadAttribute(dset, "units", context, unitsAtt) == 0) {
      units = unitsAtt.getAsString();
    }
  }

  // determine the dimensions
  
  int nDims = dspace.getSimpleExtentNdims();
  vector<hsize_t> hdims;
  hdims.resize(nDims);
  dspace.getSimpleExtentDims(hdims.data());
  dims.clear();
  for (size_t ii = 0; ii < hdims.size(); ii++) {
    dims.push_back(hdims[ii]);
  }

  // allocate space for the data values
  
  hssize_t nPoints = dspace.getSimpleExtentNpoints();
  vals.resize(nPoints);

  // read in the data depending on the type
  
  IntType intType = dset.getIntType();
  H5T_order_t order = intType.getOrder();
  H5T_sign_t sign = intType.getSign();
  size_t tsize = intType.getSize();
  
  if (sign == H5T_SGN_NONE) {
    
    // unsigned
    
    if (tsize == 1) {
      
      vector<NcxxPort::ui08> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
      
    } else if (tsize == 2) {
      
      vector<NcxxPort::ui16> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::ui16), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::ui16), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
      
    } else if (tsize == 4) {
      
      vector<NcxxPort::ui32> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::ui32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::ui32), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
      
    } else if (tsize == 8) {
      
      vector<NcxxPort::ui64> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::ui64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::ui64), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
    }
    
  } else {
    
    // signed
    
    if (tsize == 1) {
      
      vector<NcxxPort::si08> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
      
    } else if (tsize == 2) {
      
      vector<NcxxPort::si16> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::si16), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::si16), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
      
    } else if (tsize == 4) {
      
      vector<NcxxPort::si32> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::si32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::si32), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
      
    } else if (tsize == 8) {
      
      vector<NcxxPort::si64> ivals;
      ivals.resize(nPoints);
      dset.read(ivals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::si64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::si64), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = ivals[ii];
      }
    }
    
  }

  return 0;

}

///////////////////////////////////////////////////////////////////
// Read data set into 32-bit float array
// Fills in dims, missingVal, vals, units (if available)
// returns 0 on success, -1 on failure

int Hdf5xx::readFl32Array(Group &group,
                          const string &dsName,
                          const string &context,
                          vector<size_t> &dims,
                          NcxxPort::fl32 &missingVal,
                          vector<NcxxPort::fl32> &vals,
                          string &units)
  
{

  if (!group.nameExists(dsName)) {
    return -1;
  }

  // get data space for this data set
  
  DataSet dset = group.openDataSet(dsName);
  DataSpace dspace = dset.getSpace();

  // set the missing value (_fillValue)

  missingVal = -9999.0;
  if (dset.attrExists("_fillValue")) {
    DecodedAttr fillValueAtt;
    if (loadAttribute(dset, "_fillValue", context, fillValueAtt) == 0) {
      missingVal = fillValueAtt.getAsDouble();
    }
  } else if (dset.attrExists("_FillValue")) {
    DecodedAttr FillValueAtt;
    if (loadAttribute(dset, "_FillValue", context, FillValueAtt) == 0) {
      missingVal = FillValueAtt.getAsDouble();
    }
  }
  
  // set the units
  
  units = "";
  DecodedAttr unitsAtt;
  if (dset.attrExists("Units")) {
    if (loadAttribute(dset, "Units", context, unitsAtt) == 0) {
      units = unitsAtt.getAsString();
    }
  } else if (dset.attrExists("units")) {
    if (loadAttribute(dset, "units", context, unitsAtt) == 0) {
      units = unitsAtt.getAsString();
    }
  }

  // determine the dimensions
  
  int nDims = dspace.getSimpleExtentNdims();
  vector<hsize_t> hdims;
  hdims.resize(nDims);
  dspace.getSimpleExtentDims(hdims.data());
  dims.clear();
  for (size_t ii = 0; ii < hdims.size(); ii++) {
    dims.push_back(hdims[ii]);
  }

  // allocate space for the data values
  
  hssize_t nPoints = dspace.getSimpleExtentNpoints();
  vals.resize(nPoints);

  // read in the data depending on the type
  
  DataType dtype = dset.getDataType();
  H5T_class_t aclass = dtype.getClass();
  
  if (aclass == H5T_INTEGER) {
    
    IntType intType = dset.getIntType();
    H5T_order_t order = intType.getOrder();
    H5T_sign_t sign = intType.getSign();
    size_t tsize = intType.getSize();
    
    if (sign == H5T_SGN_NONE) {
      
      // unsigned
      
      if (tsize == 1) {
        
        vector<NcxxPort::ui08> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }
        
      } else if (tsize == 2) {
        
        vector<NcxxPort::ui16> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::ui16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::ui16), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }

      } else if (tsize == 4) {
        
        vector<NcxxPort::ui32> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::ui32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::ui32), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }

      } else if (tsize == 8) {

        vector<NcxxPort::ui64> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::ui64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::ui64), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }
      }

    } else {

      // signed

      if (tsize == 1) {

        vector<NcxxPort::si08> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }

      } else if (tsize == 2) {

        vector<NcxxPort::si16> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::si16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::si16), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }

      } else if (tsize == 4) {

        vector<NcxxPort::si32> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::si32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::si32), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }

      } else if (tsize == 8) {

        vector<NcxxPort::si64> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::si64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::si64), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }
      }

    }

  } else if (aclass == H5T_FLOAT) {

    FloatType flType = dset.getFloatType();
    H5T_order_t order = flType.getOrder();
    size_t tsize = flType.getSize();
    
    if (tsize == 4) {

      vector<NcxxPort::fl32> fvals;
      fvals.resize(nPoints);
      dset.read(fvals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap32(fvals.data(), nPoints * sizeof(NcxxPort::fl32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap32(fvals.data(), nPoints * sizeof(NcxxPort::fl32), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = fvals[ii];
      }

    } else if (tsize == 8) {
      
      vector<NcxxPort::fl64> fvals;
      fvals.resize(nPoints);
      dset.read(fvals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap64(fvals.data(), nPoints * sizeof(NcxxPort::fl64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap64(fvals.data(), nPoints * sizeof(NcxxPort::fl64), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = fvals[ii];
      }

    }

  } else {

    return -1;

  }
  
  return 0;

}

///////////////////////////////////////////////////////////////////
// Read data set into 64-bit float array
// Fills in dims, missingVal, vals, units (if available)

int Hdf5xx::readFl64Array(Group &group,
                          const string &dsName,
                          const string &context,
                          vector<size_t> &dims,
                          NcxxPort::fl64 &missingVal,
                          vector<NcxxPort::fl64> &vals,
                          string &units)
  
{

  if (!group.nameExists(dsName)) {
    return -1;
  }

  // get data space for this data set
  
  DataSet dset = group.openDataSet(dsName);
  DataSpace dspace = dset.getSpace();

  // set the missing value (_fillValue)

  missingVal = -9999.0;
  if (dset.attrExists("_fillValue")) {
    DecodedAttr fillValueAtt;
    if (loadAttribute(dset, "_fillValue", context, fillValueAtt) == 0) {
      missingVal = fillValueAtt.getAsDouble();
    }
  } else if (dset.attrExists("_FillValue")) {
    DecodedAttr FillValueAtt;
    if (loadAttribute(dset, "_FillValue", context, FillValueAtt) == 0) {
      missingVal = FillValueAtt.getAsDouble();
    }
  }
  
  // set the units
  
  units = "";
  DecodedAttr unitsAtt;
  if (dset.attrExists("Units")) {
    if (loadAttribute(dset, "Units", context, unitsAtt) == 0) {
      units = unitsAtt.getAsString();
    }
  } else if (dset.attrExists("units")) {
    if (loadAttribute(dset, "units", context, unitsAtt) == 0) {
      units = unitsAtt.getAsString();
    }
  }

  // determine the dimensions
  
  int nDims = dspace.getSimpleExtentNdims();
  vector<hsize_t> hdims;
  hdims.resize(nDims);
  dspace.getSimpleExtentDims(hdims.data());
  dims.clear();
  for (size_t ii = 0; ii < hdims.size(); ii++) {
    dims.push_back(hdims[ii]);
  }

  // allocate space for the data values
  
  hssize_t nPoints = dspace.getSimpleExtentNpoints();
  vals.resize(nPoints);

  // read in the data depending on the type
  
  DataType dtype = dset.getDataType();
  H5T_class_t aclass = dtype.getClass();
  
  if (aclass == H5T_INTEGER) {
    
    IntType intType = dset.getIntType();
    H5T_order_t order = intType.getOrder();
    H5T_sign_t sign = intType.getSign();
    size_t tsize = intType.getSize();
    
    if (sign == H5T_SGN_NONE) {
      
      // unsigned
      
      if (tsize == 1) {
        
        vector<NcxxPort::ui08> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }
        
      } else if (tsize == 2) {
        
        vector<NcxxPort::ui16> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::ui16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::ui16), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }

      } else if (tsize == 4) {
        
        vector<NcxxPort::ui32> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::ui32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::ui32), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }

      } else if (tsize == 8) {

        vector<NcxxPort::ui64> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::ui64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::ui64), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }
      }

    } else {

      // signed

      if (tsize == 1) {

        vector<NcxxPort::si08> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }

      } else if (tsize == 2) {

        vector<NcxxPort::si16> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::si16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals.data(), nPoints * sizeof(NcxxPort::si16), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }

      } else if (tsize == 4) {

        vector<NcxxPort::si32> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::si32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals.data(), nPoints * sizeof(NcxxPort::si32), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }

      } else if (tsize == 8) {

        vector<NcxxPort::si64> ivals;
        ivals.resize(nPoints);
        dset.read(ivals.data(), dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::si64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals.data(), nPoints * sizeof(NcxxPort::si64), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          vals[ii] = ivals[ii];
        }
      }

    }

  } else if (aclass == H5T_FLOAT) {

    FloatType flType = dset.getFloatType();
    H5T_order_t order = flType.getOrder();
    size_t tsize = flType.getSize();
    
    if (tsize == 4) {

      vector<NcxxPort::fl32> fvals;
      fvals.resize(nPoints);
      dset.read(fvals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap32(fvals.data(), nPoints * sizeof(NcxxPort::fl32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap32(fvals.data(), nPoints * sizeof(NcxxPort::fl32), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = fvals[ii];
      }

    } else if (tsize == 8) {
      
      vector<NcxxPort::fl64> fvals;
      fvals.resize(nPoints);
      dset.read(fvals.data(), dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap64(fvals.data(), nPoints * sizeof(NcxxPort::fl64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap64(fvals.data(), nPoints * sizeof(NcxxPort::fl64), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        vals[ii] = fvals[ii];
      }

    }

  } else {

    return -1;

  }
  
  return 0;

}

/////////////////////////////////////////
// add a string attribute to an object
// returns the attribute

Attribute Hdf5xx::addAttr(H5Object &obj,
                          const string &name,
                          const string &val)
{

  // Create dataspace for attribute
  DataSpace attrDataspace = DataSpace(H5S_SCALAR);
  
  // Create string datatype for attribute
  StrType strDatatype(PredType::C_S1, val.size() + 1);
  strDatatype.setCset(H5T_CSET_ASCII);
  strDatatype.setStrpad(H5T_STR_NULLTERM);

  // Set up write buffer for attribute
  const H5std_string strWritebuf(val);
  
  // Create attribute and write to it
  Attribute att = obj.createAttribute(name, strDatatype, attrDataspace);
  att.write(strDatatype, strWritebuf); 

  return att;

}

/////////////////////////////////////////
// add a 64-bit int attribute to an object
// returns the attribute

Attribute Hdf5xx::addAttr(H5Object &obj,
                          const string &name,
                          NcxxPort::si64 val)
{

  // Create dataspace for attribute
  DataSpace attrDataspace = DataSpace(H5S_SCALAR);
  
  // Create datatype for attribute
  IntType intDatatype(H5x::PredType::STD_I64LE);
  if (ByteOrder::hostIsBigEndian()) {
    intDatatype = H5x::PredType::STD_I64BE;
  }

  // Create attribute and write to it
  Attribute att = obj.createAttribute(name, intDatatype, attrDataspace);
  att.write(intDatatype, &val); 

  return att;

}

/////////////////////////////////////////
// add a 64-bit float attribute to an object
// returns the attribute

Attribute Hdf5xx::addAttr(H5Object &obj,
                          const string &name,
                          NcxxPort::fl64 val)
{

  // Create dataspace for attribute
  DataSpace attrDataspace = DataSpace(H5S_SCALAR);
  
  // Create datatype for attribute
  FloatType floatDatatype(H5x::PredType::IEEE_F64LE);
  if (ByteOrder::hostIsBigEndian()) {
    floatDatatype = H5x::PredType::IEEE_F64BE;
  }

  // Create attribute and write to it
  Attribute att = obj.createAttribute(name, floatDatatype, attrDataspace);
  att.write(floatDatatype, &val); 

  return att;

}

////////////////////////////////////////////////////
// add a 64-bit float array attribute to an object
// returns the attribute

Attribute Hdf5xx::addAttr(H5Object &obj,
                          const string &name,
                          const vector<NcxxPort::fl64> &vals)
{

  // Create dataspace for attribute
  hsize_t dim = vals.size();
  DataSpace attrDataspace = DataSpace(1, &dim);
  
  // Create datatype for attribute
  FloatType floatDatatype(H5x::PredType::IEEE_F64LE);
  if (ByteOrder::hostIsBigEndian()) {
    floatDatatype = H5x::PredType::IEEE_F64BE;
  }

  // Create attribute and write to it
  Attribute att = obj.createAttribute(name, floatDatatype, attrDataspace);
  att.write(floatDatatype, &vals[0]); 

  return att;

}

///////////////////////////////////////////////////////////////////
// Print HDF5 group, and recursively any contained groups

void Hdf5xx::printGroup(Group &group, const string grname,
                        ostream &out,
                        bool printRays, bool printData)
  
{

  size_t nGroup = group.getNumObjs();
  size_t nAttr = group.getNumAttrs();
  out << "=============================================" << endl;
  out << "  class: " << group.fromClass() << endl;
  out << "  group name: " << grname << endl;
  out << "  n objects in group: " << nGroup << endl;
  out << "  n attrs in group: " << nAttr << endl;

  printAttributes(group, out);

  for (size_t ii = 0; ii < nGroup; ii++) {
    
    H5std_string typeName;
    H5G_obj_t objType = group.getObjTypeByIdx(ii, typeName);
    H5std_string name = group.getObjnameByIdx(ii);
    out << "  index, objType, typeName, name: " << ii << ", " << objType << ", " 
        << typeName << ", " << name << endl;
    
  } // ii

  for (size_t ii = 0; ii < nGroup; ii++) {
    
    H5std_string typeName;
    H5G_obj_t objType = group.getObjTypeByIdx(ii, typeName);
    H5std_string name = group.getObjnameByIdx(ii);
    
    if (objType == H5G_GROUP) {
      out << "  ====>> found group, name: " << name << endl;
      Group gr(group.openGroup(name));
      printGroup(gr, name, out, printRays, printData);
    } else if (objType == H5G_DATASET) {
      out << "  ====>> found dataSet, name: " << name << endl;
      DataSet ds(group.openDataSet(name));
      printDataSet(ds, name, out, printRays, printData);
    }

  } // ii

}

///////////////////////////////////////////////////////////////////
// Print HDF5 data set

void Hdf5xx::printDataSet(DataSet &ds, const string dsname,
                          ostream &out,
                          bool printRays, bool printData)
  
{

  out << "---------------------------------------------" << endl;
  out << "  class: " << ds.fromClass() << endl;
  out << "  dataset name: " << dsname << endl;

  size_t nAttr = ds.getNumAttrs();
  out << "  n attrs in dataSet: " << nAttr << endl;

  printAttributes(ds, out);

  out << "  storageSize: " << ds.getStorageSize() << endl;
  out << "  inMemDataSize: " << ds.getInMemDataSize() << endl;
  out << "  id: " << ds.getId() << endl;

  DataType dtype = ds.getDataType();
  H5T_class_t aclass = dtype.getClass();
  DataSpace dataspace = ds.getSpace();
  int ndims = dataspace.getSimpleExtentNdims();
  int npoints = dataspace.getSimpleExtentNpoints();
  out << "    ndims: " << ndims << endl;
  out << "    npoints: " << npoints << endl;
  if (ndims == 0 && npoints == 1) {
    out << "    ds is scalar" << endl;
  }
  if (ndims > 0) {
    vector<hsize_t> dims;
    dims.resize(ndims);
    dataspace.getSimpleExtentDims(dims.data());
    for (int ii = 0; ii < ndims; ii++) {
      out << "        dim[" << ii << "]: " << dims[ii] << endl;
    }
  }

  if (aclass == H5T_INTEGER) {

    out << "    Ds type: INTEGER" << endl;
    IntType intType = ds.getIntType();
    H5T_order_t order = intType.getOrder();
    if (order == H5T_ORDER_BE) {
      out << "    order: BE" << endl;
    } else {
      out << "    order: LE" << endl;
    }
    H5T_sign_t sign = intType.getSign();
    if (sign == H5T_SGN_NONE) {
      out << "    type: unsigned" << endl;
    } else {
      out << "    type: signed" << endl;
    }
    size_t tsize = intType.getSize();
    out << "    data elem size: " << tsize << endl;

    vector<NcxxPort::si64> lvals;
    lvals.resize(npoints);

    if (sign == H5T_SGN_NONE) {
      
      // unsigned

      if (tsize == 1) {
        
        vector<NcxxPort::ui08> ivals;
        ivals.resize(npoints);
        ds.read(ivals.data(), dtype);
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (NcxxPort::si64) ivals[ii];
        }

      } else if (tsize == 2) {

        vector<NcxxPort::ui16> ivals;
        ivals.resize(npoints);
        ds.read(ivals.data(), dtype);
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::ui16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::ui16), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (NcxxPort::si64) ivals[ii];
        }

      } else if (tsize == 4) {

        vector<NcxxPort::ui32> ivals;
        ivals.resize(npoints);
        ds.read(ivals.data(), dtype);
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::ui32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::ui32), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (NcxxPort::si64) ivals[ii];
        }

      } else if (tsize == 8) {

        vector<NcxxPort::ui64> ivals;
        ivals.resize(npoints);
        ds.read(ivals.data(), dtype);
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::ui64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::ui64), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (NcxxPort::si64) ivals[ii];
        }
      }

    } else {

      // signed

      if (tsize == 1) {

        vector<NcxxPort::si08> ivals;
        ivals.resize(npoints);
        ds.read(ivals.data(), dtype);
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (NcxxPort::si64) ivals[ii];
        }

      } else if (tsize == 2) {

        vector<NcxxPort::si16> ivals;
        ivals.resize(npoints);
        ds.read(ivals.data(), dtype);
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::si16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::si16), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (NcxxPort::si64) ivals[ii];
        }
        
      } else if (tsize == 4) {

        vector<NcxxPort::si32> ivals;
        ivals.resize(npoints);
        ds.read(ivals.data(), dtype);
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::si32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::si32), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (NcxxPort::si64) ivals[ii];
        }

      } else { 

        // tsize == 8

        vector<NcxxPort::si64> ivals;
        ivals.resize(npoints);
        ds.read(ivals.data(), dtype);
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::si64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::si64), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (NcxxPort::si64) ivals[ii];
        }
      }

    }

    if (printData) {
      _printDataVals(out, npoints, lvals.data());
    }

  } else if (aclass == H5T_FLOAT) {

    out << "    Ds type: FLOAT" << endl;
    FloatType flType = ds.getFloatType();
    H5T_order_t order = flType.getOrder();
    if (order == H5T_ORDER_BE) {
      out << "    order: BE" << endl;
    } else {
      out << "    order: LE" << endl;
    }
    size_t tsize = flType.getSize();
    out << "    data elem size: " << tsize << endl;

    vector<NcxxPort::fl64> dvals;
    dvals.resize(npoints);
    
    if (tsize == 4) {
      
      vector<NcxxPort::fl32> fvals;
      fvals.resize(npoints);
      ds.read(fvals.data(), dtype);
      if (NcxxPort::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          NcxxPort::swap32(fvals.data(), npoints * sizeof(NcxxPort::fl32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          NcxxPort::swap32(fvals.data(), npoints * sizeof(NcxxPort::fl32), true);
        }
      }
      for (int ii = 0; ii < npoints; ii++) {
        dvals[ii] = fvals[ii];
      }
      
    } else { 

      // tsize == 8
      vector<NcxxPort::fl64> fvals;
      fvals.resize(npoints);
      ds.read(fvals.data(), dtype);
      if (NcxxPort::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          NcxxPort::swap64(fvals.data(), npoints * sizeof(NcxxPort::fl64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          NcxxPort::swap64(fvals.data(), npoints * sizeof(NcxxPort::fl64), true);
        }
      }
      for (int ii = 0; ii < npoints; ii++) {
        dvals[ii] = fvals[ii];
      }

    }

    if (printData) {
      _printDataVals(out, npoints, dvals.data());
    }

  } else if (aclass == H5T_STRING) {

    out << "    Ds type: STRING" << endl;
    StrType strType = ds.getStrType();
    H5std_string sval;
    ds.read(sval, dtype);
    out << "      sval: " << sval << endl;

  } else if (aclass == H5T_COMPOUND) {

    out << "    Ds type: COMPOUND" << endl;
    
    CompType compType = ds.getCompType();
    int nMembers = compType.getNmembers();
    int msize = compType.getSize();
    out << "    nMembers: " << nMembers << endl;
    out << "    msize: " << msize << endl;
    
    char *buf = new char[npoints * msize]; 
    ds.read(buf, dtype);
    for (int ii = 0; ii < npoints; ii++) {
      int offset = ii * msize;
      if (printRays) {
        printCompoundType(compType, ii, buf + offset, out);
      }
    }
    delete[] buf;
    
  } else if (aclass == H5T_TIME) {
    out << "    Ds type: TIME" << endl;
  } else if (aclass == H5T_BITFIELD) {
    out << "    Ds type: BITFIELD" << endl;
  } else if (aclass == H5T_OPAQUE) {
    out << "    Ds type: OPAQUE" << endl;
  } else if (aclass == H5T_REFERENCE) {
    out << "    Ds type: REFERENCE" << endl;
  } else if (aclass == H5T_ENUM) {
    out << "    Ds type: ENUM" << endl;
  } else if (aclass == H5T_VLEN) {
    out << "    Ds type: VLEN" << endl;
  } else if (aclass == H5T_ARRAY) {
    out << "    Ds type: ARRAY" << endl;
  }

  out << "---------------------------------------------" << endl;

}

//////////////////////
// print compound type

void Hdf5xx::printCompoundType(CompType &compType,
                               int ipoint,
                               char *buf,
                               ostream &out)
  
{
  
  int nMembers = compType.getNmembers();

  out << "--- Compound type --------" << endl;
  out << "      ipoint: " << ipoint << endl;
  
  for (int ii = 0; ii < nMembers; ii++) {
    
    H5std_string name = compType.getMemberName(ii);
    DataType dtype = compType.getMemberDataType(ii);
    int offset = compType.getMemberOffset(ii);
    H5T_class_t mclass = compType.getMemberClass(ii);
    
    out << "    memb[" << ii << "]: " << ii << endl;
    out << "      name: " << name << endl;
    out << "      offset: " << offset << endl;
    
    if (mclass == H5T_INTEGER) {
      
      out << "    Member type: INTEGER" << endl;
      IntType intType = compType.getMemberIntType(ii);
      H5T_order_t order = intType.getOrder();
      if (order == H5T_ORDER_BE) {
        out << "    order: BE" << endl;
      } else {
        out << "    order: LE" << endl;
      }
      H5T_sign_t sign = intType.getSign();
      if (sign == H5T_SGN_NONE) {
        out << "    type: unsigned" << endl;
      } else {
        out << "    type: signed" << endl;
      }
      size_t tsize = intType.getSize();
      out << "    data elem size: " << tsize << endl;
      
      if (sign == H5T_SGN_NONE) {
        
        // unsigned
        
        if (tsize == 1) {
          
          NcxxPort::ui08 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          out << "      val: " << (int) ival << endl;
          
        } else if (tsize == 2) {
          
          NcxxPort::ui16 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          if (NcxxPort::hostIsBigEndian()) {
            if (order == H5T_ORDER_LE) {
              NcxxPort::swap16(&ival, sizeof(ival), true);
            }
          } else {
            if (order == H5T_ORDER_BE) {
              NcxxPort::swap16(&ival, sizeof(ival), true);
            }
          }
          out << "      val: " << ival << endl;
          
        } else if (tsize == 4) {
          
          NcxxPort::ui32 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          if (NcxxPort::hostIsBigEndian()) {
            if (order == H5T_ORDER_LE) {
              NcxxPort::swap32(&ival, sizeof(ival), true);
            }
          } else {
            if (order == H5T_ORDER_BE) {
              NcxxPort::swap32(&ival, sizeof(ival), true);
            }
          }
          out << "      val: " << ival << endl;
          
        } else if (tsize == 8) {
          
          NcxxPort::ui64 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          if (NcxxPort::hostIsBigEndian()) {
            if (order == H5T_ORDER_LE) {
              NcxxPort::swap64(&ival, sizeof(ival), true);
            }
          } else {
            if (order == H5T_ORDER_BE) {
              NcxxPort::swap64(&ival, sizeof(ival), true);
            }
          }
          out << "      val: " << ival << endl;
          
        }
        
      } else {
        
        // signed

        if (tsize == 1) {
          
          NcxxPort::si08 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          out << "      val: " << (int) ival << endl;
          
        } else if (tsize == 2) {
          
          NcxxPort::si16 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          if (NcxxPort::hostIsBigEndian()) {
            if (order == H5T_ORDER_LE) {
              NcxxPort::swap16(&ival, sizeof(ival), true);
            }
          } else {
            if (order == H5T_ORDER_BE) {
              NcxxPort::swap16(&ival, sizeof(ival), true);
            }
          }
          out << "      val: " << ival << endl;
          
        } else if (tsize == 4) {
          
          NcxxPort::si32 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          if (NcxxPort::hostIsBigEndian()) {
            if (order == H5T_ORDER_LE) {
              NcxxPort::swap32(&ival, sizeof(ival), true);
            }
          } else {
            if (order == H5T_ORDER_BE) {
              NcxxPort::swap32(&ival, sizeof(ival), true);
            }
          }
          out << "      val: " << ival << endl;
          
        } else if (tsize == 8) {
          
          NcxxPort::si64 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          if (NcxxPort::hostIsBigEndian()) {
            if (order == H5T_ORDER_LE) {
              NcxxPort::swap64(&ival, sizeof(ival), true);
            }
          } else {
            if (order == H5T_ORDER_BE) {
              NcxxPort::swap64(&ival, sizeof(ival), true);
            }
          }
          out << "      val: " << ival << endl;
          
        }
        
      }
      
    } else if (mclass == H5T_FLOAT) {
      
      out << "    Member type: FLOAT" << endl;
      FloatType flType = compType.getMemberFloatType(ii);
      H5T_order_t order = flType.getOrder();
      if (order == H5T_ORDER_BE) {
        out << "    order: BE" << endl;
      } else {
        out << "    order: LE" << endl;
      }
      size_t tsize = flType.getSize();
      out << "    data elem size: " << tsize << endl;
      
      if (tsize == 4) {
        
        NcxxPort::fl32 fval;
        memcpy(&fval, buf + offset, sizeof(fval));
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap32(&fval, sizeof(fval), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap32(&fval, sizeof(fval), true);
          }
        }
        out << "      val: " << fval << endl;
          
      } else if (tsize == 8) {
        
        NcxxPort::fl64 fval;
        memcpy(&fval, buf + offset, sizeof(fval));
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap64(&fval, sizeof(fval), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap64(&fval, sizeof(fval), true);
          }
        }
        out << "      val: " << fval << endl;
          
      }
      
    } else if (mclass == H5T_STRING) {
      
      out << "    Member type: STRING" << endl;
      StrType strType = compType.getMemberStrType(ii);
      int strLen = strType.getSize();
      char *str = new char[strLen + 1];
      memcpy(str, buf + offset, strLen);
      str[strLen] = '\0';
      H5std_string sval(str);
      out << "      sval: " << sval << endl;
      delete [] str;    
    } else if (mclass == H5T_COMPOUND) {
      
      out << "    Member type: COMPOUND" << endl;
      CompType cType = compType.getMemberCompType(ii);
      printCompoundType(cType, ipoint, buf + offset, out);
      
    } else if (mclass == H5T_TIME) {
      out << "    Member type: TIME" << endl;
    } else if (mclass == H5T_BITFIELD) {
      out << "    Member type: BITFIELD" << endl;
    } else if (mclass == H5T_OPAQUE) {
      out << "    Member type: OPAQUE" << endl;
    } else if (mclass == H5T_REFERENCE) {
      out << "    Member type: REFERENCE" << endl;
    } else if (mclass == H5T_ENUM) {
      out << "    Member type: ENUM" << endl;
    } else if (mclass == H5T_VLEN) {
      out << "    Member type: VLEN" << endl;
    } else if (mclass == H5T_ARRAY) {
      out << "    Member type: ARRAY" << endl;
    }
    
    out << "--------------------------" << endl;
    
  } // ii

}
    
/////////////////////////////////////////////////////////
// print Data field values

void Hdf5xx::_printDataVals(ostream &out,
                            int nPoints,
                            NcxxPort::fl64 *vals) const
  
{

  out << "================== Data ===================" << endl;
  int count = 1;
  NcxxPort::fl64 prevVal = vals[0];
  string outStr;
  for (int ii = 1; ii < nPoints; ii++) {
    NcxxPort::fl64 dval = vals[ii];
    if (dval != prevVal) {
      _printPacked(prevVal, count, outStr);
      if (outStr.size() > 75) {
        out << outStr << endl;
        outStr.clear();
      }
      prevVal = dval;
      count = 1;
    } else {
      count++;
    }
  } // ii
  _printPacked(prevVal, count, outStr);
  out << outStr << endl;
  out << "===========================================" << endl;

}

void Hdf5xx::_printDataVals(ostream &out,
                            int nPoints,
                            NcxxPort::si64 *vals) const
  
{

  out << "================== Data ===================" << endl;
  int count = 1;
  NcxxPort::si64 prevVal = vals[0];
  string outStr;
  for (int ii = 1; ii < nPoints; ii++) {
    NcxxPort::si64 ival = vals[ii];
    if (ival != prevVal) {
      _printPacked(prevVal, count, outStr);
      if (outStr.size() > 75) {
        out << outStr << endl;
        outStr.clear();
      }
      prevVal = ival;
      count = 1;
    } else {
      count++;
    }
  } // ii
  _printPacked(prevVal, count, outStr);
  out << outStr << endl;
  out << "===========================================" << endl;

}

/////////////////////////////////////////////////////////////////
// print in packed format, using count for identical data values

void Hdf5xx::_printPacked(NcxxPort::fl64 val,
                          int count,
                          string &outStr) const

{
  
  char text[1024];
  if (count > 1) {
    sprintf(text, "%d*", count);
    outStr += text;
  }
  if (val == Ncxx::missingMetaDouble) {
    outStr += "MISS";
  } else {
    if (fabs(val) > 0.01) {
      sprintf(text, "%.3f", val);
      outStr += text;
    } else if (val == 0.0) {
      outStr += "0.0";
    } else {
      sprintf(text, "%.3e", val);
      outStr += text;
    }
  }
  outStr += " ";
}

void Hdf5xx::_printPacked(NcxxPort::si64 val,
                          int count,
                          string &outStr) const

{
  
  char text[1024];
  if (count > 1) {
    sprintf(text, "%d*", count);
    outStr += text;
  }
  if (val == Ncxx::missingMetaInt) {
    outStr += "MISS";
  } else {
    sprintf(text, "%lld", (long long) val);
    outStr += text;
  }
  outStr += " ";
}

///////////////////////////////////////////////////////////////////
// print all attributes in an object

void Hdf5xx::printAttributes(H5Object &obj, ostream &out)

{

  for (int ii = 0; ii < obj.getNumAttrs(); ii++) {
    hid_t attrId = H5Aopen_idx(obj.getId(), ii);
    Attribute attr(attrId);
    printAttribute(attr, out);
  } // ii

}

///////////////////////////////////////////////////////////////////
// print details of one attribute

void Hdf5xx::printAttribute(Attribute &attr, ostream &out)

{

  out << "------------ Attribute -------------" << endl;
  out << "  name: " << attr.getName() << endl;
  DataType dtype = attr.getDataType();
  H5T_class_t aclass = dtype.getClass();
  DataSpace dataspace = attr.getSpace();
  int ndims = dataspace.getSimpleExtentNdims();
  int npoints = dataspace.getSimpleExtentNpoints();
  out << "    ndims: " << ndims << endl;
  out << "    npoints: " << npoints << endl;
  if (ndims == 0 && npoints == 1) {
    out << "    attr is scalar" << endl;
  }
  if (ndims > 0) {
    vector<hsize_t> dims;
    dims.resize(ndims);
    dataspace.getSimpleExtentDims(dims.data());
    for (int ii = 0; ii < ndims; ii++) {
      out << "        dim[" << ii << "]: " << dims[ii] << endl;
    }
  }

  if (aclass == H5T_INTEGER) {

    out << "    attr type: INTEGER" << endl;
    IntType intType = attr.getIntType();
    H5T_order_t order = intType.getOrder();
    if (order == H5T_ORDER_BE) {
      out << "    order: BE" << endl;
    } else {
      out << "    order: LE" << endl;
    }
    H5T_sign_t sign = intType.getSign();
    if (sign == H5T_SGN_NONE) {
      out << "    type: unsigned" << endl;
    } else {
      out << "    type: signed" << endl;
    }
    size_t tsize = intType.getSize();
    out << "    data elem size: " << tsize << endl;

    if (sign == H5T_SGN_NONE) {

      // unsigned

      if (tsize == 1) {

        vector<NcxxPort::ui08> ivals;
        ivals.resize(npoints);
        attr.read(dtype, ivals.data());
        for (int ii = 0; ii < npoints; ii++) {
          out << "      ival[" << ii << "]: " << (int) ivals[ii] << endl;
        }

      } else if (tsize == 2) {
        
        vector<NcxxPort::ui16> ivals;
        ivals.resize(npoints);
        attr.read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::ui16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::ui16), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          out << "        ival[" << ii << "]: " << ivals[ii] << endl;
        }

      } else if (tsize == 4) {

        vector<NcxxPort::ui32> ivals;
        ivals.resize(npoints);
        attr.read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::ui32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::ui32), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          out << "        ival[" << ii << "]: " << ivals[ii] << endl;
        }

      } else if (tsize == 8) {

        vector<NcxxPort::ui64> ivals;
        ivals.resize(npoints);
        attr.read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::ui64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::ui64), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          out << "        ival[" << ii << "]: " << ivals[ii] << endl;
        }
      }

    } else {

      // signed

      if (tsize == 1) {

        vector<NcxxPort::si08> ivals;
        ivals.resize(npoints);
        attr.read(dtype, ivals.data());
        for (int ii = 0; ii < npoints; ii++) {
          out << "        ival[" << ii << "]: " << (int) ivals[ii] << endl;
        }

      } else if (tsize == 2) {

        vector<NcxxPort::si16> ivals;
        ivals.resize(npoints);
        attr.read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::si16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap16(ivals.data(), npoints * sizeof(NcxxPort::si16), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          out << "        ival[" << ii << "]: " << ivals[ii] << endl;
        }
        
      } else if (tsize == 4) {

        vector<NcxxPort::si32> ivals;
        ivals.resize(npoints);
        attr.read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::si32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap32(ivals.data(), npoints * sizeof(NcxxPort::si32), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          out << "        ival[" << ii << "]: " << ivals[ii] << endl;
        }

      } else if (tsize == 8) {

        vector<NcxxPort::si64> ivals;
        ivals.resize(npoints);
        attr.read(dtype, ivals.data());
        if (NcxxPort::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::si64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            NcxxPort::swap64(ivals.data(), npoints * sizeof(NcxxPort::si64), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          out << "        ival[" << ii << "]: " << ivals[ii] << endl;
        }

      }

    }

  } else if (aclass == H5T_FLOAT) {

    out << "    Attr type: FLOAT" << endl;
    FloatType flType = attr.getFloatType();
    H5T_order_t order = flType.getOrder();
    if (order == H5T_ORDER_BE) {
      out << "    order: BE" << endl;
    } else {
      out << "    order: LE" << endl;
    }
    size_t tsize = flType.getSize();
    out << "    data elem size: " << tsize << endl;

    if (tsize == 4) {

      vector<NcxxPort::fl32> fvals;
      fvals.resize(npoints);
      attr.read(dtype, fvals.data());
      if (NcxxPort::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          NcxxPort::swap32(fvals.data(), npoints * sizeof(NcxxPort::fl32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          NcxxPort::swap32(fvals.data(), npoints * sizeof(NcxxPort::fl32), true);
        }
      }
      for (int ii = 0; ii < npoints; ii++) {
        out << "      fval[" << ii << "]: " << fvals[ii] << endl;
      }

    } else if (tsize == 8) {

      vector<NcxxPort::fl64> fvals;
      fvals.resize(npoints);
      attr.read(dtype, fvals.data());
      if (NcxxPort::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          NcxxPort::swap64(fvals.data(), npoints * sizeof(NcxxPort::fl64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          NcxxPort::swap64(fvals.data(), npoints * sizeof(NcxxPort::fl64), true);
        }
      }
      for (int ii = 0; ii < npoints; ii++) {
        out << "      fval[" << ii << "]: " << fvals[ii] << endl;
      }

    }

  } else if (aclass == H5T_STRING) {

    out << "    Attr type: STRING" << endl;
    StrType strType = attr.getStrType();
    H5std_string sval;
    attr.read(dtype, sval);
    out << "      sval: " << sval << endl;

  } else if (aclass == H5T_TIME) {
    out << "    Attr type: TIME" << endl;
  } else if (aclass == H5T_BITFIELD) {
    out << "    Attr type: BITFIELD" << endl;
  } else if (aclass == H5T_OPAQUE) {
    out << "    Attr type: OPAQUE" << endl;
  } else if (aclass == H5T_COMPOUND) {
    out << "    Attr type: COMPOUND" << endl;
  } else if (aclass == H5T_REFERENCE) {
    out << "    Attr type: REFERENCE" << endl;
  } else if (aclass == H5T_ENUM) {
    out << "    Attr type: ENUM" << endl;
  } else if (aclass == H5T_VLEN) {
    out << "    Attr type: VLEN" << endl;
  } else if (aclass == H5T_ARRAY) {
    out << "    Attr type: ARRAY" << endl;
  }

  out << "------------------------------------" << endl;

}

///////////////////////////////
// clear the error string

void Hdf5xx::clearErrStr()
{
  _errStr.clear();
}

///////////////////////////////////////////////
// add labelled integer value to error string,
// with optional following carriage return.

void Hdf5xx::_addErrInt(string label, int iarg, bool cr)
{
  _errStr += label;
  char str[32];
  sprintf(str, "%d", iarg);
  _errStr += str;
  if (cr) {
    _errStr += "\n";
  }
}

///////////////////////////////////////////////
// add labelled double value to error string,
// with optional following carriage return.
// Default format is %g.

void Hdf5xx::_addErrDbl(string label, double darg,
                        string format, bool cr)

{
  _errStr += label;
  char str[128];
  sprintf(str, format.c_str(), darg);
  _errStr += str;
  if (cr) {
    _errStr += "\n";
  }
}

////////////////////////////////////////
// add labelled string to error string
// with optional following carriage return.

void Hdf5xx::_addErrStr(string label, string strarg, bool cr)

{
  _errStr += label;
  _errStr += strarg;
  if (cr) {
    _errStr += "\n";
  }
}

/////////////////////////////////////////
// private class for decoding attributes
  
Hdf5xx::DecodedAttr::DecodedAttr()
{
  clear();
}

void Hdf5xx::DecodedAttr::clear() 
{
  _name.clear();
  _intVal = 0;
  _doubleVal = 0.0;
  _stringVal.clear();
  _isInt = false;
  _isDouble = false;
  _isString = false;
}

void Hdf5xx::DecodedAttr::setAsString(const string &val) 
{ 
  _stringVal = val;
  _isString = true;
}

void Hdf5xx::DecodedAttr::setAsInt(NcxxPort::si64 val) 
{
  _intVal = val;
  _isInt = true;
}

void Hdf5xx::DecodedAttr::setAsDouble(double val) 
{
  _doubleVal = val;
  _isDouble = true;
}

NcxxPort::fl64 Hdf5xx::DecodedAttr::getAsDouble() 
{
  if (_isDouble) return _doubleVal;
  if (_isInt) return (double) _intVal;
  if (_isString) {
    if (sscanf(_stringVal.c_str(), "%lg", &_doubleVal) == 1) {
      return _doubleVal;
    }
  }
  return -9.0e33;
}

NcxxPort::si64 Hdf5xx::DecodedAttr::getAsInt() 
{
  if (_isInt) return _intVal;
  if (_isDouble) return (NcxxPort::si64) _doubleVal;
  if (_isString) {
    long long int ival;
    if (sscanf(_stringVal.c_str(), "%lld", &ival) == 1) {
      _intVal = ival;
      return _intVal;
    }
  }
  return -9999;
}

string Hdf5xx::DecodedAttr::getAsString() 
{
  if (_isString) return _stringVal;
  return "not-set";
}

///////////////////////////////////////////////
// private class for decoding array attributes
  
Hdf5xx::ArrayAttr::ArrayAttr()
{
  _intVals = NULL;
  _doubleVals = NULL;
  _len = 0;
  clear();
}

void Hdf5xx::ArrayAttr::clear() 
{
  _name.clear();
  _isInt = false;
  _isDouble = false;
  if (_intVals) {
    delete[] _intVals;
    _intVals = NULL;
  }
  if (_doubleVals) {
    delete[] _doubleVals;
    _doubleVals = NULL;
  }
  _len = 0;
}

void Hdf5xx::ArrayAttr::setAsInts(const NcxxPort::si64 *vals,
                                  size_t len) 
{
  if (_intVals) {
    delete[] _intVals;
    _intVals = NULL;
  }
  _len = len;
  _intVals = new NcxxPort::si64[_len];
  memcpy(_intVals, vals, _len * sizeof(NcxxPort::si64));
  _isInt = true;
}

void Hdf5xx::ArrayAttr::setAsDoubles(const NcxxPort::fl64 *vals, 
                                     size_t len)
{
  if (_doubleVals) {
    delete[] _doubleVals;
    _doubleVals = NULL;
  }
  _len = len;
  _doubleVals = new NcxxPort::fl64[_len];
  memcpy(_doubleVals, vals, _len * sizeof(NcxxPort::fl64));
  _isDouble = true;
}

const NcxxPort::fl64 *Hdf5xx::ArrayAttr::getAsDoubles() 
{
  if (_isDouble) return _doubleVals;
  if (_doubleVals == NULL) {
    _doubleVals = new NcxxPort::fl64[_len];
    for (size_t ii = 0; ii < _len; ii++) {
      _doubleVals[ii] = (NcxxPort::fl64) _intVals[ii];
    }
  }
  return _doubleVals;
}

const NcxxPort::si64 *Hdf5xx::ArrayAttr::getAsInts() 
{
  if (_isInt) return _intVals;
  if (_intVals == NULL) {
    _intVals = new NcxxPort::si64[_len];
    for (size_t ii = 0; ii < _len; ii++) {
      _intVals[ii] = (NcxxPort::si64) floor(_doubleVals[ii] + 0.5);
    }
  }
  return _intVals;
}

