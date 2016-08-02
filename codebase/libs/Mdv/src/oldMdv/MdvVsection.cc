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
//////////////////////////////////////////////////////////
// MdvVsection.cc
//
// Class for representing vertical sections from Mdv files
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1999
//
//////////////////////////////////////////////////////////

#include <Mdv/mdv/MdvVsection.hh>
#include <Mdv/mdv/MdvRead.hh>
#include <toolsa/mem.h>
#include <toolsa/pjg.h>
using namespace std;

/////////////////////////////
// Constructor
//

MdvVsection::MdvVsection()

{

  MEM_zero(_masterHeader);
  _clearPlaneLimits();
  setEncodingType();

}

/////////////////////////////
// Destructor

MdvVsection::~MdvVsection()

{

}

///////////////////////////////////
// clear request

void MdvVsection::clearRequest()

{
  setEncodingType(MDV_INT8);
  _clearRequestFieldNums();
  _clearRequestFieldNames();
  _clearPlaneLimits();
  _clearWayPts();
}

///////////////////////////////////////////
// set encoding type - defaults to MDV_INT8

void MdvVsection::setEncodingType(int encoding_type /* = MDV_INT8*/)

{
  _encodingType = encoding_type;
}

////////////////////////////////////
// add way point to vector

void MdvVsection::addWayPt(const double lat, const double lon)

{
  wayPt_t pt;
  pt.lat = lat;
  pt.lon = lon;
  _wayPts.push_back(pt);
}

////////////////////
// add field request

void MdvVsection::addFieldRequest(const int field_num)

{     
  if (_requestFieldNames.size() > 0) {
    _clearRequestFieldNames();
  }
  _requestFieldNums.push_back(field_num);
}

void MdvVsection::addFieldRequest(const string &field_name)

{
  if (_requestFieldNums.size() > 0) {
    _clearRequestFieldNums();
  }
  _requestFieldNames.push_back(field_name);
}

///////////////////////////////////////////////////////////////
// set plane num limits

void MdvVsection::setPlaneNumLimits(const int lower_plane_num,
                                    const int upper_plane_num)

{
  _planeNumLimits = true;
  _lowerPlaneNum = lower_plane_num;
  _upperPlaneNum = upper_plane_num;
  _planeVlevelLimits = false;
}

///////////////////////////////////////////////////////////////
// set plane vlevel limits

void MdvVsection::setPlaneVlevelLimits(const double lower_plane_vlevel,
                                       const double upper_plane_vlevel)

{
  _planeVlevelLimits = true;
  _lowerPlaneVlevel = lower_plane_vlevel;
  _upperPlaneVlevel = upper_plane_vlevel;
  _planeNumLimits = false;
}

/////////////////
// print request

void MdvVsection::printRequest(ostream &out)

{

  out << ">> Vsection request <<" << endl;

  if (_requestFieldNums.size() > 0) {
    out << "Field nums: ";
    for (size_t i = 0; i < _requestFieldNums.size(); i++) {
      out << _requestFieldNums[i] << " ";
    }
    out << endl;
  }

  if (_requestFieldNames.size() > 0) {
    out << "Field names: ";
    for (size_t i = 0; i < _requestFieldNames.size(); i++) {
      out << _requestFieldNames[i] << " ";
    }
    out << endl;
  }

  if (_planeNumLimits) {
    out << "Lower plane num: " << _lowerPlaneNum << endl;
    out << "Upper plane num: " << _upperPlaneNum << endl;
  }

  if (_planeVlevelLimits) {
    out << "Lower plane vlevel: " << _lowerPlaneVlevel << endl;
    out << "Upper plane vlevel: " << _upperPlaneVlevel << endl;
  }

  out << "Number of way points: " << _wayPts.size() << endl;

  for (size_t i = 0; i < _wayPts.size(); i++) {
    out << i << ": lat " << _wayPts[i].lat << ", lon "
        << _wayPts[i].lon << endl;
  }

  out << endl;

}

/////////////////////////
// print sampling summary

void MdvVsection::printSampleSummary(ostream &out)

{

  out << ">> Vsection sampling summary <<" << endl;

  out << "Route segments:" << endl;

  out << "  Total length (km): " << _totalLength << endl;

  for (size_t i = 0; i < _segments.size(); i++) {
    out << "  Segment " << i << ", length " << _segments[i].length
        << ", azimuth " << _segments[i].azimuth << endl;
  }
  
  out << "Number of sample points: " << _samplePts.size() << endl;
  out << "  dx (km): " << _dxKm << endl;
  
  for (size_t i = 0; i < _samplePts.size(); i++) {
    out << i
        << " lat: " << _samplePts[i].lat
        << " lon: " << _samplePts[i].lon
        << " segNum: " << _samplePts[i].segNum
        << endl;
  }

  out << endl;

}

/////////////////////////
// print field summary

void MdvVsection::printFieldSummary(ostream &out, int field_num)

{
  _fields[field_num].printSummary(out);
}

/////////////////////////
// print field data

void MdvVsection::printFieldData(ostream &out, int field_num)

{
  _fields[field_num].printData(out);
}


////////////////////////////////////
// load()
//
// Load vsection from data in MDV file

int MdvVsection::load(DsDataFile &file, int n_samples, string &errStr)
  
{

  errStr = "\nERROR - MdvVsection::read(): ";

  MdvRead mdv;
  
  // Check encoding type
  if (_encodingType != MDV_INT8) {
    errStr += "The only supported encoding type for vertical sections ";
    errStr += "is MDV_INT8.";
    return (-1);
  }

  // open file

  if (mdv.openFile(file.getFileStr())) {
    return (-1);
  }

  // copy the master header
  if (mdv.readMasterHeader()) {
    return (-1);
  }
  _masterHeader = mdv.getMasterHeader();

  // set field numbers and names

  if (mdv.loadFieldNames()) {
    return (-1);
  }

  if (_requestFieldNums.size() == 0 &&
      _requestFieldNames.size() == 0) {

    // no fields specified - get all fields

    for (int i = 0; i < _masterHeader.n_fields; i++) {
      _requestFieldNums.push_back(i);
      _requestFieldNames.push_back(mdv.getFieldName(i));
    }

  } else if (_requestFieldNums.size() > 0) {

    // field nums specified, load field names

    for (size_t i = 0; i < _requestFieldNums.size(); i++) {
      int fnum = _requestFieldNums[i];
      if (fnum > _masterHeader.n_fields - 1) {
        errStr += " Invalid  field number: ";
        char str[128];
        sprintf(str, "%d - ", fnum);
        errStr += str;
        errStr += file.getFileStr();
        return (-1);
      }
      _requestFieldNames.push_back(mdv.getFieldName(fnum));
    }

  } else {

    // field names specified, load field nums

    for (size_t i = 0; i < _requestFieldNames.size(); i++) {
      const char *fname = _requestFieldNames[i].c_str();
      int fnum = mdv.getFieldNum(fname);
      if (fnum < 0) {
        errStr += " Invalid  field name: ";
        errStr += fname;
        errStr += " - ";
        errStr += file.getFileStr();
        return (-1);
      }
      _requestFieldNums.push_back(mdv.getFieldNum(fname));
    }

  }
  
  _masterHeader.n_fields = _requestFieldNums.size();

  // compute segments distances and store

  _totalLength = 0.0;
  _clearSegments();
  for (size_t i = 1; i < _wayPts.size(); i++) {
    segment_t seg;
    PJGLatLon2RTheta(_wayPts[i-1].lat, _wayPts[i-1].lon,
                     _wayPts[i].lat, _wayPts[i].lon,
                     &seg.length, &seg.azimuth);
    _totalLength += seg.length;
    _segments.push_back(seg);
  }

  // compute dkm from length and number of sampling points

  _dxKm = _totalLength / n_samples;
  
  // load up the lat/lon for each sample point

  _clearSamplePts();
  if (_wayPts.size() == 1) {
    // only 1 way point - single point request
    samplePt_t pt;
    pt.lat = _wayPts[0].lat;
    pt.lon = _wayPts[0].lon;
    pt.segNum = 0;
    _samplePts.push_back(pt);
  } else {
    // more than 1 way point - segment request
    double posInSeg = _dxKm / 2.0;
    double lengthSoFar = posInSeg;
    for (size_t i = 0; i < _segments.size(); i++) {
      while (posInSeg < _segments[i].length) {
        double lat, lon;
        PJGLatLonPlusRTheta(_wayPts[i].lat, _wayPts[i].lon,
                            posInSeg, _segments[i].azimuth,
                            &lat, &lon);
        samplePt_t pt;
        pt.lat = lat;
        pt.lon = lon;
        pt.segNum = i;
        _samplePts.push_back(pt);
        posInSeg += _dxKm;
        lengthSoFar += _dxKm;
      }
      posInSeg -= _segments[i].length;
    } // i
  }

  // create the field array
  
  _fields.erase(_fields.begin(), _fields.end());
  for (size_t ifield = 0; ifield < _requestFieldNums.size(); ifield++) {
    MdvVsectionField vsectFld(ifield);
    _fields.push_back(vsectFld);
  }

  // load the vsection for each of the requested fields

  for (size_t ifield = 0; ifield < _requestFieldNums.size(); ifield++) {
    
    bool isFloatField = false;

    MdvVsectionField &vsectFld = _fields[ifield];

    // read volume

    int fieldNum = _requestFieldNums[ifield];
    MDV_field_header_t tmpHdr;
    int status = mdv.getFieldHeader(fieldNum, tmpHdr);
    if (status < 0) {
      errStr += "Could not read field header to find encoding type.";
      return (-1);
    }
    
    // Todo: Take this out when design a scheme for reading floats as chars.
    //   Temporary hack.
    // 
    if (tmpHdr.encoding_type == MDV_FLOAT32) {
      isFloatField = true;
    }

    if (isFloatField) {
      if (mdv.readVol(fieldNum, MDV_FLOAT32)) {
        errStr += " Reading MDV_FLOAT32 volume: ";
        errStr += file.getFileStr();
        return (-1);
      }
    }
    else {
      if (mdv.readVol(fieldNum, _encodingType)) {
        errStr += " Reading volume: ";
        errStr += file.getFileStr();
        return (-1);
      }
    }

    MdvReadField &readFld = mdv.getField(fieldNum);
    MDV_field_header_t &fHdr = readFld.getFieldHeader();
    MDV_vlevel_header_t &vlvHdr = readFld.getVlevelHeader();
    mdv_grid_t &grid = readFld.getGrid();

    // if vlevel is not filled out, do so

    if (!_masterHeader.vlevel_included) {
      for (int iz = 0; iz < fHdr.nz; iz++) {
        vlvHdr.vlevel_params[iz] =
          fHdr.grid_minz + iz * fHdr.grid_dz;
      }
    }
    _masterHeader.vlevel_included = true;

    // determine which planes are included in the request

    if (_planeVlevelLimits) {
      _lowerPlaneNum = fHdr.nz - 1;
      for (int iz = 0; iz < fHdr.nz; iz++) {
        if (vlvHdr.vlevel_params[iz] >= _lowerPlaneVlevel) {
          _lowerPlaneNum = iz;
          break;
        }
      }
      _upperPlaneNum = 0;
      for (int iz = fHdr.nz - 1; iz >= 0; iz--) {
        if (vlvHdr.vlevel_params[iz] <= _upperPlaneVlevel) {
          _upperPlaneNum = iz;
          break;
        }
      }
      _planeNumLimits = true;
    }

    if (!_planeNumLimits) {
      _lowerPlaneNum = 0;
      _upperPlaneNum = fHdr.nz - 1;
    }
    int nPlanes = _upperPlaneNum - _lowerPlaneNum + 1;

    // get data element size
    // Took this out -- should be size of _encodingType... -PTM
    // int elemSize = readFld.getVolElemSize();
    int inElemSize = readFld.getVolElemSize();
    int outElemSize = 1; // For now always MDV_INT8
    
    // set up vsection field

    vsectFld._fieldHeader = fHdr;
    if (isFloatField) {
      // Change the bad/missing to values appropriate to the 
      //   outgoing char data, rather than leaving them as they
      //   were in the incoming float data.
      // 
      vsectFld._fieldHeader.bad_data_value = 0.0;
      vsectFld._fieldHeader.missing_data_value = 0.0;
    }
    vsectFld._fieldHeader.encoding_type = _encodingType;
    vsectFld._fieldHeader.data_element_nbytes = outElemSize;
    vsectFld._vlevelHeader = vlvHdr;
    vsectFld.allocData(nPlanes, _samplePts.size(), outElemSize, _encodingType);
    memset(vsectFld._data1D, (int) fHdr.missing_data_value,
           nPlanes * _samplePts.size() * outElemSize);

    // adjust the headers appropriately if plane nums were limited

    MDV_vlevel_header_t &vhdr = vsectFld._vlevelHeader;
    if (_planeNumLimits) {
      for (int iz = 0; iz < nPlanes; iz++) {
        vhdr.vlevel_params[iz] = vhdr.vlevel_params[iz+_lowerPlaneNum];
      }
      vsectFld._fieldHeader.nz = nPlanes;
      vsectFld._fieldHeader.grid_minz +=
        _lowerPlaneNum * vsectFld._fieldHeader.grid_dz;
    }
    
    // Todo: Take this out.
    // Calculate scale and bias for float disk files
    float scale = 1.0,
          bias  = 0.0;
    if (isFloatField) {
      float * in = (float *) readFld.getVol1D();
      float min = in[0],
            max = in[0];
      int numPoints = grid.nx * grid.ny * grid.nz;
      if (numPoints <= 0) {
        errStr += "No (MDV_FLOAT32) input data.";
        return (-1);
      }

      for (int i = 0; i < numPoints; i++) {
        if (in[i] < min) min = in[i];
        if (in[i] > max) max = in[i];
      }

      scale = (max - min) / 250.0;
      if ( fabs( scale ) <= 0.0001 ) {
         scale = 1.0;
      }
      bias = min - (2.0 * scale);

      vsectFld._fieldHeader.scale = scale;
      vsectFld._fieldHeader.bias = bias;
    }

    // copy the data over to the vsection

    mdv_grid_comps_t gcomps;
    MDV_init_proj(&grid, &gcomps);
    int inPlaneNBytes = grid.nx * grid.ny * inElemSize;
    int outRowNbytes = _samplePts.size() * outElemSize;
    
    for (size_t ipt = 0; ipt < _samplePts.size(); ipt++) {
      
      double xx, yy;
      MDV_latlon2xy(&gcomps,
                    _samplePts[ipt].lat, _samplePts[ipt].lon,
                    &xx, &yy);

      int ix = (int) ((xx - grid.minx) / grid.dx + 0.5);
      int iy = (int) ((yy - grid.miny) / grid.dy + 0.5);
      
      if (ix >= 0 && ix < grid.nx && iy >= 0 && iy < grid.ny) {

        int inOffset = ((_lowerPlaneNum * inPlaneNBytes) +
                        (iy * grid.nx + ix) * inElemSize);
        int outOffset = ipt * outElemSize;

        char * in = (char *) readFld.getVol1D() + inOffset;
        char *out = (char *) vsectFld._data1D + outOffset;
        
        for (int iz = 0; iz < nPlanes; iz++) {

          // Todo: Take out this temporary support of float disk files.
          // 
          if (isFloatField) {
            float dataVal = ((float *) in)[0];
            char c = (char)(((dataVal - bias) / scale) + 0.5);
            memcpy(out, &c, 1);
if (ipt == 2) {
  cerr << "Float Data for sample point " << ipt << " at z level " << iz << ": "
       << dataVal << " (" << c << ")" << endl;
}
          }
          else {
if (ipt == 2) {
  cerr << "Non-Float Data for sample point " << ipt << " at z level " << iz << ": "
       << in[0] << endl;
}
            memcpy(out, in, outElemSize);
          }
          in += inPlaneNBytes;
          out += outRowNbytes;
        } // iz

      } // if (ix >= 0 ...
      
    } // ipt

  } // ifield

  return (0);

}

///////////////////////////////////
// clear request field nums vector

void MdvVsection::_clearRequestFieldNums()

{
  _requestFieldNums.erase(_requestFieldNums.begin(),
                          _requestFieldNums.end());
}

///////////////////////////////////
// clear request field names vector

void MdvVsection::_clearRequestFieldNames()

{
  _requestFieldNames.erase(_requestFieldNames.begin(),
                           _requestFieldNames.end());
}

//////////////////////
// clear plane limits

void MdvVsection::_clearPlaneLimits()

{
  _planeNumLimits = false;
  _planeVlevelLimits = false;
}

///////////////////////////////////
// clear way point vector

void MdvVsection::_clearWayPts()

{
  _wayPts.erase(_wayPts.begin(), _wayPts.end());
}

////////////////////////////////////
// clear sample point vector

void MdvVsection::_clearSamplePts()

{
  _samplePts.erase(_samplePts.begin(), _samplePts.end());
}

////////////////////////////////////
// clear segment vector

void MdvVsection::_clearSegments()

{
  _segments.erase(_segments.begin(), _segments.end());
}

////////////////////////////////////
// clear field vector

void MdvVsection::_clearFields()

{
  _fields.erase(_fields.begin(), _fields.end());
}

///////////////////////////////////////
// clear memory associated with vsection

void MdvVsection::_clearAll()

{
  
  MEM_zero(_masterHeader);
  setEncodingType();
  _clearRequestFieldNums();
  _clearRequestFieldNames();
  _clearPlaneLimits();
  _clearWayPts();
  _clearSamplePts();
  _clearSegments();
  _clearFields();
  
}

////////////////////////////////////
// add sample point to vector

void MdvVsection::_addSamplePt(const double lat, const double lon,
                               const int segment_num)

{
  samplePt_t pt;
  pt.lat = lat;
  pt.lon = lon;
  pt.segNum = segment_num;
  _samplePts.push_back(pt);
}

////////////////////////////////////
// add a segment to vector
  
void MdvVsection::_addSegment(const double length, const double azimuth)
  
{
  segment_t seg;
  seg.length = length;
  seg.azimuth = azimuth;
  _segments.push_back(seg);
}





