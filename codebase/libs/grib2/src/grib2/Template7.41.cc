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
///////////////////////////////////////////////////
// Template7_pt_41 PNG Code Stream Format
//
// Jason Craig,  Jan 2008
//
//////////////////////////////////////////////////

#include <cmath>
#include <cstring>
#include <stdlib.h>

#include <grib2/Template7.41.hh>
#include <grib2/DS.hh>
#include <grib2/GDS.hh>
#include <grib2/BMS.hh>
#include <grib2/DRS.hh>
#include <grib2/DataRepTemp.hh>
#include <grib2/Template5.41.hh>
#include <dataport/port_types.h>


using namespace std;

namespace Grib2 {

Template7_pt_41::Template7_pt_41(Grib2Record::Grib2Sections_t sectionsPtr)
: DataTemp(sectionsPtr)
{
}


Template7_pt_41::~Template7_pt_41 () 
{
  if(_pdata)
    delete[] _pdata;
}

void Template7_pt_41::print(FILE *stream) const
{
  si32 gridSz = _sectionsPtr.gds->getNumDataPoints();
  fprintf(stream, "DS length: %d\n", gridSz);
  if(_data) {
    for (int i = 0; i < gridSz; i++) {
      fprintf(stream, "%f ", _data[i]);
    }
    fprintf(stream, "\n");
  }
}

int Template7_pt_41::pack (fl32 *dataPtr)
{
// SUBPROGRAM:    pngpack
//   PRGMMR: Gilbert          ORG: W/NP11    DATE: 2002-12-21
//
// ABSTRACT: This subroutine packs up a data field into PNG image format.
//   After the data field is scaled, and the reference value is subtracted out,
//   it is treated as a grayscale image and passed to a PNG encoder.
//   It also fills in GRIB2 Data Representation Template 5.41 or 5.40010 with the
//   appropriate values.
//
// PROGRAM HISTORY LOG:
// 2002-12-21  Gilbert


  fl32 *pdataPtr = _applyBitMapPack(dataPtr);
  si32 gridSz = _sectionsPtr.drs->getNumPackedDataPoints();
  DataRepTemp::data_representation_t drsConstants = _sectionsPtr.drs->getDrsConstants();

  si32 maxdif;

  int width = gridSz;
  int height = 1;

  Template5_pt_41 *template5_pt_41 = (Template5_pt_41 *)_sectionsPtr.drs->getDrsTemplate();

  if(_sectionsPtr.bms->getBitMap() == NULL) {
    width = _sectionsPtr.gds->getWidth();
    height = _sectionsPtr.gds->getHeight();
    if ( width == 0 || height == 0 ) {
      width = gridSz;
      height = 1;
      // } else if ( width == allones || height || allones ) {
      //width = gridSz;
      //height = 1;
    } else if ( (_sectionsPtr.gds->getIscan() & 32) > 0) {
      int temp = width;
      width = height;
      height = temp;
    }
  }

  if(width * height != gridSz) {
    cerr << "ERROR: Template7_pt_41::pack()" << endl;
    cerr << "Data width * height != drs.numberPoints" << endl;
    if(pdataPtr != dataPtr)
      delete [] pdataPtr;
    return( GRIB_FAILURE );
  }
  fl32 bscale = pow(2.0 , -drsConstants.binaryScaleFactor);
  double dscale = pow(10.0, drsConstants.decimalScaleFactor);

  //
  //  Find max and min values in the data
  //
  float rmax = pdataPtr[0];
  float rmin = pdataPtr[0];
  for(int j = 1; j < gridSz; j++) {
    if (pdataPtr[j] > rmax) rmax = pdataPtr[j];
    if (pdataPtr[j] < rmin) rmin = pdataPtr[j];
  }
  if (drsConstants.binaryScaleFactor == 0) 
    maxdif = int(rmax*dscale + .5) - int(rmin*dscale + .5);
  else
    maxdif = int((rmax-rmin)*dscale*bscale + .5);

  int nbits = 0, nbytes = 0;  
  //
  //  If max and min values are not equal, pack up field.
  //  If they are equal, we have a constant field, and the reference
  //  value (rmin) is the value for each point in the field and
  //  set nbits to 0.
  //
  if(rmin != rmax && maxdif != 0) 
  {
    int *ifld = new int[gridSz];
    //
    //  Determine which algorithm to use based on user-supplied 
    //  binary scale factor and number of bits.
    //
    if (drsConstants.binaryScaleFactor == 0)
    {
      //
      //  No binary scaling and calculate minimum number of 
      //  bits in which the data will fit.
      //
      int imin = int(rmin*dscale +.5);
      int imax = int(rmax*dscale + .5);
      int maxdif = imax-imin;
      fl32 temp = log(float(maxdif+1))/log(2.0);
      nbits = (int)ceil(temp);
      rmin = float(imin);
      //   scale data
      for(int j = 0; j < gridSz; j++)
	ifld[j] = int(pdataPtr[j]*dscale + .5)-imin;

    } else {
      //
      //  Use binary scaling factor and calculate minimum number of 
      //  bits in which the data will fit.
      //
      rmin = rmin*dscale;
      rmax = rmax*dscale;
      maxdif = int((rmax-rmin)*bscale + .5);
      fl32 temp = log(float(maxdif+1))/log(2.0);
      nbits = (int)ceil(temp);
      //   scale data
      for(int j = 0; j < gridSz; j++)
	ifld[j] = int(((pdataPtr[j]*dscale)-rmin)*bscale + .5);
    }

    //
    //  Pack data into full octets, then do PNG encode.
    //  and calculate the length of the packed data in bytes
    //
    if(nbits < 8) {
      nbits = 8;
    } if(nbits < 16) {
      nbits = 16;
    } if(nbits < 24) {
      nbits = 24;
    } else { 
      nbits = 32;
    }
    nbytes = (nbits+7)/8;
    ui08 *ctemp = (ui08 *)calloc(gridSz,nbytes);

    if(_pdata)
      delete[] _pdata;
    _pdata = new fl32[gridSz];
    
    DS::sbits(ctemp,ifld,0,nbytes*8,0,gridSz);
    _lcpack = encode_png(ctemp,width,height,nbits, (char*)_pdata);

    delete[] ctemp;
    delete[] ifld;
    if (_lcpack < 0) {
      cerr << "Template7_pt_41::pack()" << endl;
      cerr << "PNG Encoding Failed " << endl;
      if(pdataPtr != dataPtr)
	delete [] pdataPtr;
      return GRIB_FAILURE;
    }
    
  } else {
    nbits = 0;
    _lcpack = 0;
  }

  //
  //  Fill in ref value and number of bits in Template 5.0
  //
  drsConstants.referenceValue = rmin;
  drsConstants.numberOfBits = nbits;
  //drsConstants.origFieldTypes = 0;      // original data were reals
  template5_pt_41->setDrsConstants(drsConstants);

  //
  // If our data ptr is not the same as the input data ptr
  // then we packed the data with a bit map.  Delete the packed array
  //
  if(pdataPtr != dataPtr)
    delete [] pdataPtr;

  return GRIB_SUCCESS;
}

int Template7_pt_41::unpack (ui08 *dataPtr) 
{
  si32 gridSz = _sectionsPtr.drs->getNumPackedDataPoints();
  DataRepTemp::data_representation_t drsConstants = _sectionsPtr.drs->getDrsConstants();
  fl32 *outputData = new fl32[gridSz];

  fl32 bscale = pow(2.0, drsConstants.binaryScaleFactor);
  double dscale = pow(10.0, -drsConstants.decimalScaleFactor);   
  if(drsConstants.decimalScaleFactor == 1)
    dscale = 0.1;
  fl32 reference = drsConstants.referenceValue;
  
  int nbits = drsConstants.numberOfBits;
  if (nbits != 0) {
    
    char *tmp_data = new char[gridSz * (nbits / 8)];

    int ret = decode_png ((char *) dataPtr, tmp_data);

    if(ret != GRIB_SUCCESS) {
      cerr << "Template7_pt_41::unpack()" << endl;
      cerr << "PNG De-Ccoding Failed " << endl;
      delete[] tmp_data;
      delete[] outputData;
      return GRIB_FAILURE;
    }

    si32 *ifld = new si32[gridSz];

    DS::gbits ((ui08 *) tmp_data, ifld, 0, nbits, 0, gridSz);

    for (int i = 0; i < gridSz; i++) {
      outputData[i] = (((fl32) ifld[i] * bscale) + reference) * dscale;
    }

    delete[] tmp_data;
    delete[] ifld;
  }
  else {
    for (int i = 0; i < gridSz; i++)
      outputData[i] = reference;
  }
  
  _applyBitMapUnpack(outputData);
  return GRIB_SUCCESS;
}


// Following converted to C++ from dec_png.c and enc_png.c in the g2lib

int Template7_pt_41::decode_png (char *input, char *output) 
{
  png_structp png_ptr;
  png_infop info_ptr,end_info;
  png_bytepp row_pointers;
  png_stream read_io_ptr;
  
  /*  check if stream is a valid PNG format   */
  
  if ( png_sig_cmp((png_byte *)input, 0, 8) != 0) 
    return (-3);
  
  /* create and initialize png_structs  */
  
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, 
				   NULL, NULL);
  if (!png_ptr)
    return (-1);
  
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct(&png_ptr,(png_infopp)NULL,(png_infopp)NULL);
    return (-2);
  }
  
  end_info = png_create_info_struct(png_ptr);
  if (!end_info)
  {
    png_destroy_read_struct(&png_ptr,(png_infopp)info_ptr,(png_infopp)NULL);
    return (-2);
  }
  
  /*     Set Error callback   */
  
#if (PNG_LIBPNG_VER < 10400 || PNG_LIBPNG_VER >= 10500)
  if(setjmp(png_jmpbuf(png_ptr)))
#else
/* Warning is unavoidable if #define PNG_DEPSTRUCT is not present */
  if (setjmp(png_ptr->jmpbuf))
#endif
  {
    png_destroy_read_struct(&png_ptr, &info_ptr,&end_info);
    return (-3);
  }

  /*    Initialize info for reading PNG stream from memory   */
  read_io_ptr.stream_ptr = input;
  read_io_ptr.stream_len = 0;

  /*    Set new custom read function    */
  png_set_read_fn(png_ptr,(png_voidp)&read_io_ptr,(png_rw_ptr)&user_read_data);
  /*     png_init_io(png_ptr, fptr);   */

  /*     Read and decode PNG stream   */
  png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
  
  /*     Get pointer to each row of image data   */
  row_pointers = png_get_rows(png_ptr, info_ptr);

  /*     Get image info, such as size, depth, colortype, etc...   */
  int interlace, color, compres, filter, bit_depth;
  png_uint_32 width, height;
  /*printf("SAGT:png %d %d %d\n",info_ptr->width,info_ptr->height,info_ptr->bit_depth);*/
  int ret = png_get_IHDR(png_ptr, info_ptr, &width, &height,
		     &bit_depth, &color, &interlace, &compres, &filter);
  
  /*     Check if image was grayscale      */
  
  /*
    if (color != PNG_COLOR_TYPE_GRAY ) {
    fprintf(stderr,"dec_png: Grayscale image was expected. \n");
    }
  */
  if ( color == PNG_COLOR_TYPE_RGB ) {
    bit_depth=24;
  }
  else if ( color == PNG_COLOR_TYPE_RGB_ALPHA ) {
    bit_depth=32;
  }

  if(width == 0)
    return -4;

  /*     Copy image data to output string   */  
  si32 n = 0;
  si32 bytes = bit_depth/8;
  si32 clen = width * bytes;
  for(si32 j = 0; j < (int) height; j++) {
    for(si32 k = 0; k < clen; k++) {
      output[n] = *(row_pointers[j]+k);
      n++;
    }
  }

  /*      Clean up   */
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  
  return GRIB_SUCCESS;
}


//        Custom read function used so that libpng will read a PNG stream
//        from memory instead of a file on disk.
void Template7_pt_41::user_read_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
     char *ptr;
     si32 offset;
     png_stream *mem;

     mem = (png_stream *)png_get_io_ptr(png_ptr);
     ptr = (char *)mem->stream_ptr;
     offset = mem->stream_len;
/*     printf("SAGrd %ld %ld %x\n",offset,length,ptr);  */
     memcpy(data,ptr+offset,length);
     mem->stream_len += length;
}


int Template7_pt_41::encode_png (ui08 *cin, int width, int height, int nbits, char *out)
  //int SUB_NAME(char *data,g2int *width,g2int *height,g2int *nbits,char *pngbuf)
{
  png_structp png_ptr;
  png_infop info_ptr;
  //    png_bytep *row_pointers[*height];
  png_bytep **row_pointers;
  png_stream write_io_ptr;
  
  /* create and initialize png_structs  */
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, 
				    NULL, NULL);
  if (!png_ptr)
    return (-1);
  
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
    return (-2);
  }
  
  /*     Set Error callback   */
#if (PNG_LIBPNG_VER < 10400 || PNG_LIBPNG_VER >= 10500)
  if(setjmp(png_jmpbuf(png_ptr)))
#else
/* Warning is unavoidable if #define PNG_DEPSTRUCT is not present */
  if (setjmp(png_ptr->jmpbuf))
#endif
  {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return (-3);
  }
  
  /*    Initialize info for writing PNG stream to memory   */
  write_io_ptr.stream_ptr = out;
  write_io_ptr.stream_len = 0;
  
  /*    Set new custom write functions    */
  png_set_write_fn(png_ptr,(png_voidp)&write_io_ptr,
                   (png_rw_ptr)&user_write_data,
		   (png_flush_ptr)&user_flush_data);
  /*    png_init_io(png_ptr, fptr);   */
  /*    png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);  */
  
  /*     Set the image size, colortype, filter type, etc...      */
  
  /*    printf("SAGTsettingIHDR %d %d %d\n",width,height,bit_depth); */
  si32 bit_depth = nbits;
  si32 color_type = PNG_COLOR_TYPE_GRAY;
  if (nbits == 24 ) {
    bit_depth=8;
    color_type=PNG_COLOR_TYPE_RGB;
  }
  else if (nbits == 32 ) {
    bit_depth=8;
    color_type=PNG_COLOR_TYPE_RGB_ALPHA;
  }
  png_set_IHDR(png_ptr, info_ptr, width, height,
	       bit_depth, color_type, PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  
  /*     Put image data into the PNG info structure    */
  
  /*bytes=bit_depth/8;*/
  si32 bytes = nbits/8;
  row_pointers = (png_bytep **) malloc((height)*sizeof(png_bytep));
  for(si32 j = 0; j < height; j++) 
    row_pointers[j] = (png_bytep *)(cin+(j*(width)*bytes));
  png_set_rows(png_ptr, info_ptr, (png_bytepp)row_pointers);
  
  /*     Do the PNG encoding, and write out PNG stream  */
  
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
  
  /*     Clean up   */
  
  png_destroy_write_struct(&png_ptr, &info_ptr);
  free(row_pointers);
  
  si32 pnglen = write_io_ptr.stream_len;
  return pnglen;
}

//  Custom write function used to that libpng will write
//  to memory location instead of a file on disk
void Template7_pt_41::user_write_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
  char *ptr;
  si32 offset;
  png_stream *mem;
  
  mem = (png_stream *)png_get_io_ptr(png_ptr);
  ptr = mem->stream_ptr;
  offset = mem->stream_len;
  /*     printf("SAGwr %ld %ld %x\n",offset,length,ptr);    */
  /*for (j=offset,k=0;k<length;j++,k++) ptr[j]=data[k];*/
  memcpy(ptr+offset,data,length);
  mem->stream_len += length;
}

//  Dummy Custom flush function
void Template7_pt_41::user_flush_data(png_structp png_ptr)
{
  // Do nothing
}



} // namespace Grib2

