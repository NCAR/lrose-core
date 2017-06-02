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
// GAMIC HDF% file data for radar radial data
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2013
//
///////////////////////////////////////////////////////////////

#include <Ncxx/Hdf5xx.hh>
#include <Radx/ByteOrder.hh>
#include <cstring>
#include <cmath>

//////////////////////////////////////////////////
// get float val for a specific comp header member

int Hdf5xx::loadFloatVar(CompType compType,
                            char *buf,
                            const string &varName,
                            Radx::fl64 &floatVal)

{

  clearErrStr();  

  bool isInt, isFloat, isString;
  Radx::si64 intVal;
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
                          Radx::si64 &intVal)

{
  
  clearErrStr();  

  bool isInt, isFloat, isString;
  Radx::fl64 floatVal;
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
  Radx::si64 intVal;
  Radx::fl64 floatVal;
  
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
                           Radx::si64 &intVal,
                           Radx::fl64 &floatVal,
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
  catch (const H5::Exception &e) {
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
        
        Radx::ui08 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        intVal = (int) ival;
        
      } else if (tsize == 2) {
        
        Radx::ui16 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(&ival, sizeof(ival), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(&ival, sizeof(ival), true);
          }
        }
        intVal = ival;
        
      } else if (tsize == 4) {
        
        Radx::ui32 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(&ival, sizeof(ival), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(&ival, sizeof(ival), true);
          }
        }
        intVal = ival;
        
      } else if (tsize == 8) {
        
        Radx::ui64 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(&ival, sizeof(ival), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(&ival, sizeof(ival), true);
          }
        }
        intVal = ival;
        
      }
      
    } else {
      
      // signed
      
      if (tsize == 1) {
        
        Radx::si08 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        intVal = (int) ival;
        
      } else if (tsize == 2) {
        
        Radx::si16 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(&ival, sizeof(ival), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(&ival, sizeof(ival), true);
          }
        }
        intVal = ival;
        
      } else if (tsize == 4) {
        
        Radx::si32 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(&ival, sizeof(ival), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(&ival, sizeof(ival), true);
          }
        }
        intVal = ival;
        
      } else if (tsize == 8) {
        
        Radx::si64 ival;
        memcpy(&ival, buf + offset, sizeof(ival));
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(&ival, sizeof(ival), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(&ival, sizeof(ival), true);
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
      
      Radx::fl32 fval;
      memcpy(&fval, buf + offset, sizeof(fval));
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap32(&fval, sizeof(fval), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap32(&fval, sizeof(fval), true);
        }
      }
      floatVal = fval;
      
    } else if (tsize == 8) {
      
      Radx::fl64 fval;
      memcpy(&fval, buf + offset, sizeof(fval));
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap64(&fval, sizeof(fval), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap64(&fval, sizeof(fval), true);
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

int Hdf5xx::loadAttribute(H5Location &obj,
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
  catch (const H5::Exception &e) {
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
  hsize_t *dims = NULL;
  if (ndims > 0) {
    dims = new hsize_t[ndims];
    dataspace.getSimpleExtentDims(dims);
  }

  if (aclass == H5T_INTEGER) {

    IntType intType = attr->getIntType();
    H5T_order_t order = intType.getOrder();
    H5T_sign_t sign = intType.getSign();
    size_t tsize = intType.getSize();
    Radx::si64 lval = 0;
    
    if (sign == H5T_SGN_NONE) {

      // unsigned
      
      if (tsize == 1) {

        Radx::ui08 *ivals = new Radx::ui08[npoints];
        attr->read(dtype, ivals);
        lval = (int) ivals[0];
        delete[] ivals;

      } else if (tsize == 2) {

        Radx::ui16 *ivals = new Radx::ui16[npoints];
        attr->read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::ui16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::ui16), true);
          }
        }
        lval = ivals[0];
        delete[] ivals;

      } else if (tsize == 4) {

        Radx::ui32 *ivals = new Radx::ui32[npoints];
        attr->read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::ui32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::ui32), true);
          }
        }
        lval = ivals[0];
        delete[] ivals;

      } else if (tsize == 8) {

        Radx::ui64 *ivals = new Radx::ui64[npoints];
        attr->read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::ui64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::ui64), true);
          }
        }
        lval = ivals[0];
        delete[] ivals;
      }

    } else {

      // signed

      if (tsize == 1) {

        Radx::si08 *ivals = new Radx::si08[npoints];
        attr->read(dtype, ivals);
        lval = ivals[0];
        delete[] ivals;

      } else if (tsize == 2) {

        Radx::si16 *ivals = new Radx::si16[npoints];
        attr->read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::si16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::si16), true);
          }
        }
        lval = ivals[0];
        delete[] ivals;

      } else if (tsize == 4) {

        Radx::si32 *ivals = new Radx::si32[npoints];
        attr->read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::si32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::si32), true);
          }
        }
        lval = ivals[0];
        delete[] ivals;

      } else if (tsize == 8) {

        Radx::si64 *ivals = new Radx::si64[npoints];
        attr->read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::si64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::si64), true);
          }
        }
        lval = ivals[0];
        delete[] ivals;
      }

    }

    decodedAttr.setAsInt(lval);

  } else if (aclass == H5T_FLOAT) {

    FloatType flType = attr->getFloatType();
    H5T_order_t order = flType.getOrder();
    size_t tsize = flType.getSize();
    Radx::fl64 dval = 0;

    if (tsize == 4) {
      
      Radx::fl32 *fvals = new Radx::fl32[npoints];
      attr->read(dtype, fvals);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap32(fvals, npoints * sizeof(Radx::fl32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap32(fvals, npoints * sizeof(Radx::fl32), true);
        }
      }
      dval = fvals[0];
      delete[] fvals;

    } else if (tsize == 8) {

      Radx::fl64 *fvals = new Radx::fl64[npoints];
      attr->read(dtype, fvals);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap64(fvals, npoints * sizeof(Radx::fl64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap64(fvals, npoints * sizeof(Radx::fl64), true);
        }
      }
      dval = fvals[0];
      delete[] fvals;

    }

    decodedAttr.setAsDouble(dval);

  } else if (aclass == H5T_STRING) {

    StrType strType = attr->getStrType();
    H5std_string sval;
    attr->read(dtype, sval);
    decodedAttr.setAsString(sval);
    
  }

  // clean up

  if (attr) delete attr;
  if (dims) delete[] dims;

  return 0;

}

///////////////////////////////////////////////////////////////////
// load an array attribute from an object, given the name
// returns 0 on success, -1 on failure

int Hdf5xx::loadArrayAttribute(H5Location &obj,
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
  catch (const H5::Exception &e) {
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
  hsize_t *dims = NULL;
  if (ndims > 0) {
    dims = new hsize_t[ndims];
    dataspace.getSimpleExtentDims(dims);
  }

  if (aclass == H5T_INTEGER) {

    IntType intType = attr->getIntType();
    H5T_order_t order = intType.getOrder();
    H5T_sign_t sign = intType.getSign();
    size_t tsize = intType.getSize();
    Radx::si64 *lvals = new Radx::si64[npoints];
    
    if (sign == H5T_SGN_NONE) {

      // unsigned
      
      if (tsize == 1) {
        
        Radx::ui08 *ivals = new Radx::ui08[npoints];
        attr->read(dtype, ivals);
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = (int) ivals[ii];
        }
        delete[] ivals;

      } else if (tsize == 2) {

        Radx::ui16 *ivals = new Radx::ui16[npoints];
        attr->read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::ui16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::ui16), true);
          }
        }
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = ivals[ii];
        }
        delete[] ivals;

      } else if (tsize == 4) {

        Radx::ui32 *ivals = new Radx::ui32[npoints];
        attr->read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::ui32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::ui32), true);
          }
        }
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = ivals[ii];
        }
        delete[] ivals;

      } else if (tsize == 8) {

        Radx::ui64 *ivals = new Radx::ui64[npoints];
        attr->read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::ui64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::ui64), true);
          }
        }
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = ivals[ii];
        }
        delete[] ivals;
      }

    } else {

      // signed

      if (tsize == 1) {
        
        Radx::si08 *ivals = new Radx::si08[npoints];
        attr->read(dtype, ivals);
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = (int) ivals[ii];
        }
        delete[] ivals;

      } else if (tsize == 2) {

        Radx::si16 *ivals = new Radx::si16[npoints];
        attr->read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::si16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::si16), true);
          }
        }
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = ivals[ii];
        }
        delete[] ivals;

      } else if (tsize == 4) {

        Radx::si32 *ivals = new Radx::si32[npoints];
        attr->read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::si32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::si32), true);
          }
        }
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = ivals[ii];
        }
        delete[] ivals;

      } else if (tsize == 8) {

        Radx::si64 *ivals = new Radx::si64[npoints];
        attr->read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::si64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::si64), true);
          }
        }
        for (size_t ii = 0; ii < npoints; ii++) {
          lvals[ii] = ivals[ii];
        }
        delete[] ivals;
      }

    }

    arrayAttr.setAsInts(lvals, npoints);
    delete[] lvals;

  } else if (aclass == H5T_FLOAT) {

    FloatType flType = attr->getFloatType();
    H5T_order_t order = flType.getOrder();
    size_t tsize = flType.getSize();
    Radx::fl64 *dvals = new Radx::fl64[npoints];

    if (tsize == 4) {
      
      Radx::fl32 *fvals = new Radx::fl32[npoints];
      attr->read(dtype, fvals);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap32(fvals, npoints * sizeof(Radx::fl32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap32(fvals, npoints * sizeof(Radx::fl32), true);
        }
      }
      for (size_t ii = 0; ii < npoints; ii++) {
        dvals[ii] = fvals[ii];
      }
      delete[] fvals;

    } else if (tsize == 8) {

      Radx::fl64 *fvals = new Radx::fl64[npoints];
      attr->read(dtype, fvals);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap64(fvals, npoints * sizeof(Radx::fl64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap64(fvals, npoints * sizeof(Radx::fl64), true);
        }
      }
      for (size_t ii = 0; ii < npoints; ii++) {
        dvals[ii] = fvals[ii];
      }
      delete[] fvals;

    }

    arrayAttr.setAsDoubles(dvals, npoints);
    delete[] dvals;

  }

  // clean up

  if (attr) delete attr;
  if (dims) delete[] dims;

  return 0;

}

///////////////////////////////////////////////////////////////////
// append to vector of attribute names
// called by obj.iterateAttrs()

void Hdf5xx::appendAttrNames(H5Location &obj,
                                const H5std_string attr_name,
                                void *operator_data)
  
{
  vector<string> *names = (vector<string> *) operator_data;
  names->push_back(attr_name);
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

/////////////////////////////////////////
// add a string attribute to an object
// returns the attribute

Attribute Hdf5xx::addAttr(H5Location &loc,
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
  Attribute att = loc.createAttribute(name, strDatatype, attrDataspace);
  att.write(strDatatype, strWritebuf); 

  return att;

}

/////////////////////////////////////////
// add a 64-bit int attribute to an object
// returns the attribute

Attribute Hdf5xx::addAttr(H5Location &loc,
                             const string &name,
                             Radx::si64 val)
{

  // Create dataspace for attribute
  DataSpace attrDataspace = DataSpace(H5S_SCALAR);
  
  // Create datatype for attribute
  IntType intDatatype(H5::PredType::STD_I64LE);
  if (ByteOrder::hostIsBigEndian()) {
    intDatatype = H5::PredType::STD_I64BE;
  }

  // Create attribute and write to it
  Attribute att = loc.createAttribute(name, intDatatype, attrDataspace);
  att.write(intDatatype, &val); 

  return att;

}

/////////////////////////////////////////
// add a 64-bit float attribute to an object
// returns the attribute

Attribute Hdf5xx::addAttr(H5Location &loc,
                             const string &name,
                             Radx::fl64 val)
{

  // Create dataspace for attribute
  DataSpace attrDataspace = DataSpace(H5S_SCALAR);
  
  // Create datatype for attribute
  FloatType floatDatatype(H5::PredType::IEEE_F64LE);
  if (ByteOrder::hostIsBigEndian()) {
    floatDatatype = H5::PredType::IEEE_F64BE;
  }

  // Create attribute and write to it
  Attribute att = loc.createAttribute(name, floatDatatype, attrDataspace);
  att.write(floatDatatype, &val); 

  return att;

}

////////////////////////////////////////////////////
// add a 64-bit float array attribute to an object
// returns the attribute

Attribute Hdf5xx::addAttr(H5Location &loc,
                             const string &name,
                             const RadxArray<Radx::fl64> &vals)
{

  // Create dataspace for attribute
  hsize_t dim = vals.size();
  DataSpace attrDataspace = DataSpace(1, &dim);
  
  // Create datatype for attribute
  FloatType floatDatatype(H5::PredType::IEEE_F64LE);
  if (ByteOrder::hostIsBigEndian()) {
    floatDatatype = H5::PredType::IEEE_F64BE;
  }

  // Create attribute and write to it
  Attribute att = loc.createAttribute(name, floatDatatype, attrDataspace);
  att.write(floatDatatype, vals.dat()); 

  return att;

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
    hsize_t *dims = new hsize_t[ndims];
    dataspace.getSimpleExtentDims(dims);
    for (int ii = 0; ii < ndims; ii++) {
      out << "        dim[" << ii << "]: " << dims[ii] << endl;
    }
    delete[] dims;
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

    Radx::si64 *lvals = new Radx::si64[npoints];

    if (sign == H5T_SGN_NONE) {
      
      // unsigned

      if (tsize == 1) {
        
        Radx::ui08 *ivals = new Radx::ui08[npoints];
        ds.read(ivals, dtype);
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (Radx::si64) ivals[ii];
        }
        delete[] ivals;

      } else if (tsize == 2) {

        Radx::ui16 *ivals = new Radx::ui16[npoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::ui16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::ui16), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (Radx::si64) ivals[ii];
        }
        delete[] ivals;

      } else if (tsize == 4) {

        Radx::ui32 *ivals = new Radx::ui32[npoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::ui32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::ui32), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (Radx::si64) ivals[ii];
        }
        delete[] ivals;

      } else if (tsize == 8) {

        Radx::ui64 *ivals = new Radx::ui64[npoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::ui64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::ui64), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (Radx::si64) ivals[ii];
        }
        delete[] ivals;
      }

    } else {

      // signed

      if (tsize == 1) {

        Radx::si08 *ivals = new Radx::si08[npoints];
        ds.read(ivals, dtype);
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (Radx::si64) ivals[ii];
        }
        delete[] ivals;

      } else if (tsize == 2) {

        Radx::si16 *ivals = new Radx::si16[npoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::si16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::si16), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (Radx::si64) ivals[ii];
        }
        delete[] ivals;

      } else if (tsize == 4) {

        Radx::si32 *ivals = new Radx::si32[npoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::si32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::si32), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (Radx::si64) ivals[ii];
        }
        delete[] ivals;

      } else if (tsize == 8) {

        Radx::si64 *ivals = new Radx::si64[npoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::si64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::si64), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          lvals[ii] = (Radx::si64) ivals[ii];
        }
        delete[] ivals;
      }

    }

    if (printData) {
      _printDataVals(out, npoints, lvals);
    }
    delete[] lvals;

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

    Radx::fl64 *dvals = new Radx::fl64[npoints];

    if (tsize == 4) {

      Radx::fl32 *fvals = new Radx::fl32[npoints];
      ds.read(fvals, dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap32(fvals, npoints * sizeof(Radx::fl32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap32(fvals, npoints * sizeof(Radx::fl32), true);
        }
      }
      for (int ii = 0; ii < npoints; ii++) {
        dvals[ii] = fvals[ii];
      }
      delete[] fvals;

    } else if (tsize == 8) {

      Radx::fl64 *fvals = new Radx::fl64[npoints];
      ds.read(fvals, dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap64(fvals, npoints * sizeof(Radx::fl64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap64(fvals, npoints * sizeof(Radx::fl64), true);
        }
      }
      for (int ii = 0; ii < npoints; ii++) {
        dvals[ii] = fvals[ii];
      }
      delete[] fvals;

    }

    if (printData) {
      _printDataVals(out, npoints, dvals);
    }
    delete[] dvals;

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
          
          Radx::ui08 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          out << "      val: " << (int) ival << endl;
          
        } else if (tsize == 2) {
          
          Radx::ui16 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          if (ByteOrder::hostIsBigEndian()) {
            if (order == H5T_ORDER_LE) {
              ByteOrder::swap16(&ival, sizeof(ival), true);
            }
          } else {
            if (order == H5T_ORDER_BE) {
              ByteOrder::swap16(&ival, sizeof(ival), true);
            }
          }
          out << "      val: " << ival << endl;
          
        } else if (tsize == 4) {
          
          Radx::ui32 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          if (ByteOrder::hostIsBigEndian()) {
            if (order == H5T_ORDER_LE) {
              ByteOrder::swap32(&ival, sizeof(ival), true);
            }
          } else {
            if (order == H5T_ORDER_BE) {
              ByteOrder::swap32(&ival, sizeof(ival), true);
            }
          }
          out << "      val: " << ival << endl;
          
        } else if (tsize == 8) {
          
          Radx::ui64 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          if (ByteOrder::hostIsBigEndian()) {
            if (order == H5T_ORDER_LE) {
              ByteOrder::swap64(&ival, sizeof(ival), true);
            }
          } else {
            if (order == H5T_ORDER_BE) {
              ByteOrder::swap64(&ival, sizeof(ival), true);
            }
          }
          out << "      val: " << ival << endl;
          
        }
        
      } else {
        
        // signed

        if (tsize == 1) {
          
          Radx::si08 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          out << "      val: " << (int) ival << endl;
          
        } else if (tsize == 2) {
          
          Radx::si16 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          if (ByteOrder::hostIsBigEndian()) {
            if (order == H5T_ORDER_LE) {
              ByteOrder::swap16(&ival, sizeof(ival), true);
            }
          } else {
            if (order == H5T_ORDER_BE) {
              ByteOrder::swap16(&ival, sizeof(ival), true);
            }
          }
          out << "      val: " << ival << endl;
          
        } else if (tsize == 4) {
          
          Radx::si32 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          if (ByteOrder::hostIsBigEndian()) {
            if (order == H5T_ORDER_LE) {
              ByteOrder::swap32(&ival, sizeof(ival), true);
            }
          } else {
            if (order == H5T_ORDER_BE) {
              ByteOrder::swap32(&ival, sizeof(ival), true);
            }
          }
          out << "      val: " << ival << endl;
          
        } else if (tsize == 8) {
          
          Radx::si64 ival;
          memcpy(&ival, buf + offset, sizeof(ival));
          if (ByteOrder::hostIsBigEndian()) {
            if (order == H5T_ORDER_LE) {
              ByteOrder::swap64(&ival, sizeof(ival), true);
            }
          } else {
            if (order == H5T_ORDER_BE) {
              ByteOrder::swap64(&ival, sizeof(ival), true);
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
        
        Radx::fl32 fval;
        memcpy(&fval, buf + offset, sizeof(fval));
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(&fval, sizeof(fval), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(&fval, sizeof(fval), true);
          }
        }
        out << "      val: " << fval << endl;
          
      } else if (tsize == 8) {
        
        Radx::fl64 fval;
        memcpy(&fval, buf + offset, sizeof(fval));
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(&fval, sizeof(fval), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(&fval, sizeof(fval), true);
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
                               Radx::fl64 *vals) const
  
{

  out << "================== Data ===================" << endl;
  int count = 1;
  Radx::fl64 prevVal = vals[0];
  string outStr;
  for (int ii = 1; ii < nPoints; ii++) {
    Radx::fl64 dval = vals[ii];
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
                               Radx::si64 *vals) const
  
{

  out << "================== Data ===================" << endl;
  int count = 1;
  Radx::si64 prevVal = vals[0];
  string outStr;
  for (int ii = 1; ii < nPoints; ii++) {
    Radx::si64 ival = vals[ii];
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

void Hdf5xx::_printPacked(Radx::fl64 val,
                                     int count,
                                     string &outStr) const

{
  
  char text[1024];
  if (count > 1) {
    sprintf(text, "%d*", count);
    outStr += text;
  }
  if (val == Radx::missingMetaDouble) {
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

void Hdf5xx::_printPacked(Radx::si64 val,
                             int count,
                             string &outStr) const

{
  
  char text[1024];
  if (count > 1) {
    sprintf(text, "%d*", count);
    outStr += text;
  }
  if (val == Radx::missingMetaInt) {
    outStr += "MISS";
  } else {
    sprintf(text, "%lld", (long long) val);
    outStr += text;
  }
  outStr += " ";
}

///////////////////////////////////////////////////////////////////
// print all attributes in an object

void Hdf5xx::printAttributes(H5Location &obj, ostream &out)

{
  
  vector<string> attrNames;
  obj.iterateAttrs(appendAttrNames, NULL, &attrNames);
  for (size_t ii = 0; ii < attrNames.size(); ii++) {
    Attribute *attr = NULL;
    try {
      attr = new Attribute(obj.openAttribute(attrNames[ii]));
    }
    catch (const H5::Exception &e) {
      cerr << "ERROR - cannot find attribute: " << attrNames[ii] << endl;
      if (attr) delete attr;
      continue;
    }
    printAttribute(*attr, out);
    delete attr;
  }

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
    hsize_t *dims = new hsize_t[ndims];
    dataspace.getSimpleExtentDims(dims);
    for (int ii = 0; ii < ndims; ii++) {
      out << "        dim[" << ii << "]: " << dims[ii] << endl;
    }
    delete[] dims;
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

        Radx::ui08 *ivals = new Radx::ui08[npoints];
        attr.read(dtype, ivals);
        for (int ii = 0; ii < npoints; ii++) {
          out << "      ival[" << ii << "]: " << (int) ivals[ii] << endl;
        }
        delete[] ivals;

      } else if (tsize == 2) {

        Radx::ui16 *ivals = new Radx::ui16[npoints];
        attr.read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::ui16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::ui16), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          out << "        ival[" << ii << "]: " << ivals[ii] << endl;
        }
        delete[] ivals;

      } else if (tsize == 4) {

        Radx::ui32 *ivals = new Radx::ui32[npoints];
        attr.read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::ui32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::ui32), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          out << "        ival[" << ii << "]: " << ivals[ii] << endl;
        }
        delete[] ivals;

      } else if (tsize == 8) {

        Radx::ui64 *ivals = new Radx::ui64[npoints];
        attr.read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::ui64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::ui64), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          out << "        ival[" << ii << "]: " << ivals[ii] << endl;
        }
        delete[] ivals;
      }

    } else {

      // signed

      if (tsize == 1) {

        Radx::si08 *ivals = new Radx::si08[npoints];
        attr.read(dtype, ivals);
        for (int ii = 0; ii < npoints; ii++) {
          out << "        ival[" << ii << "]: " << (int) ivals[ii] << endl;
        }
        delete[] ivals;

      } else if (tsize == 2) {

        Radx::si16 *ivals = new Radx::si16[npoints];
        attr.read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::si16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals, npoints * sizeof(Radx::si16), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          out << "        ival[" << ii << "]: " << ivals[ii] << endl;
        }
        delete[] ivals;

      } else if (tsize == 4) {

        Radx::si32 *ivals = new Radx::si32[npoints];
        attr.read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::si32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals, npoints * sizeof(Radx::si32), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          out << "        ival[" << ii << "]: " << ivals[ii] << endl;
        }
        delete[] ivals;

      } else if (tsize == 8) {

        Radx::si64 *ivals = new Radx::si64[npoints];
        attr.read(dtype, ivals);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::si64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals, npoints * sizeof(Radx::si64), true);
          }
        }
        for (int ii = 0; ii < npoints; ii++) {
          out << "        ival[" << ii << "]: " << ivals[ii] << endl;
        }
        delete[] ivals;
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

      Radx::fl32 *fvals = new Radx::fl32[npoints];
      attr.read(dtype, fvals);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap32(fvals, npoints * sizeof(Radx::fl32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap32(fvals, npoints * sizeof(Radx::fl32), true);
        }
      }
      for (int ii = 0; ii < npoints; ii++) {
        out << "      fval[" << ii << "]: " << fvals[ii] << endl;
      }
      delete[] fvals;

    } else if (tsize == 8) {

      Radx::fl64 *fvals = new Radx::fl64[npoints];
      attr.read(dtype, fvals);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap64(fvals, npoints * sizeof(Radx::fl64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap64(fvals, npoints * sizeof(Radx::fl64), true);
        }
      }
      for (int ii = 0; ii < npoints; ii++) {
        out << "      fval[" << ii << "]: " << fvals[ii] << endl;
      }
      delete[] fvals;

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
  Radx::addErrInt(_errStr, label, iarg, cr);
}

///////////////////////////////////////////////
// add labelled double value to error string,
// with optional following carriage return.
// Default format is %g.

void Hdf5xx::_addErrDbl(string label, double darg,
                           string format, bool cr)

{
  Radx::addErrDbl(_errStr, label, darg, format, cr);
}

////////////////////////////////////////
// add labelled string to error string
// with optional following carriage return.

void Hdf5xx::_addErrStr(string label, string strarg, bool cr)

{
  Radx::addErrStr(_errStr, label, strarg, cr);
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

void Hdf5xx::DecodedAttr::setAsInt(Radx::si64 val) 
{
  _intVal = val;
  _isInt = true;
}

void Hdf5xx::DecodedAttr::setAsDouble(double val) 
{
  _doubleVal = val;
  _isDouble = true;
}

Radx::fl64 Hdf5xx::DecodedAttr::getAsDouble() 
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

Radx::si64 Hdf5xx::DecodedAttr::getAsInt() 
{
  if (_isInt) return _intVal;
  if (_isDouble) return (Radx::si64) _doubleVal;
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

void Hdf5xx::ArrayAttr::setAsInts(const Radx::si64 *vals,
                                     size_t len) 
{
  if (_intVals) {
    delete[] _intVals;
    _intVals = NULL;
  }
  _len = len;
  _intVals = new Radx::si64[_len];
  memcpy(_intVals, vals, _len * sizeof(Radx::si64));
  _isInt = true;
}

void Hdf5xx::ArrayAttr::setAsDoubles(const Radx::fl64 *vals, 
                                        size_t len)
{
  if (_doubleVals) {
    delete[] _doubleVals;
    _doubleVals = NULL;
  }
  _len = len;
  _doubleVals = new Radx::fl64[_len];
  memcpy(_doubleVals, vals, _len * sizeof(Radx::fl64));
  _isDouble = true;
}

const Radx::fl64 *Hdf5xx::ArrayAttr::getAsDoubles() 
{
  if (_isDouble) return _doubleVals;
  if (_doubleVals == NULL) {
    _doubleVals = new Radx::fl64[_len];
    for (size_t ii = 0; ii < _len; ii++) {
      _doubleVals[ii] = (Radx::fl64) _intVals[ii];
    }
  }
  return _doubleVals;
}

const Radx::si64 *Hdf5xx::ArrayAttr::getAsInts() 
{
  if (_isInt) return _intVals;
  if (_intVals == NULL) {
    _intVals = new Radx::si64[_len];
    for (size_t ii = 0; ii < _len; ii++) {
      _intVals[ii] = (Radx::si64) floor(_doubleVals[ii] + 0.5);
    }
  }
  return _intVals;
}

