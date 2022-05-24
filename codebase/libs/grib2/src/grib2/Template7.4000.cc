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
// Template7_pt_4000 JPEG 2000 Code Stream Format
//
// Jason Craig,  Aug 2006
//
//////////////////////////////////////////////////

#include <cmath>

#include <grib2/Template7.4000.hh>
#include <grib2/DS.hh>
#include <grib2/GDS.hh>
#include <grib2/BMS.hh>
#include <grib2/DRS.hh>
#include <grib2/DataRepTemp.hh>
#include <grib2/Template5.4000.hh>
#include <dataport/port_types.h>

#ifndef NO_JASPER_LIB
#include <jasper/jasper.h>
#endif

using namespace std;

namespace Grib2 {


Template7_pt_4000::Template7_pt_4000(Grib2Record::Grib2Sections_t sectionsPtr)
  : DataTemp(sectionsPtr), jpcminlen(200)
{
}


Template7_pt_4000::~Template7_pt_4000 () 
{
  if(_pdata)
    delete[] _pdata;
}

void Template7_pt_4000::print(FILE *stream) const
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

int Template7_pt_4000::pack (fl32 *dataPtr)
{
// SUBPROGRAM:    jpcpack
//   PRGMMR: Gilbert          ORG: W/NP11    DATE: 2002-12-17
//
// ABSTRACT: This subroutine packs up a data field into a JPEG2000 code stream.
//   After the data field is scaled, and the reference value is subtracted out,
//   it is treated as a grayscale image and passed to a JPEG2000 encoder.
//   It also fills in GRIB2 Data Representation Template 5.40 or 5.40000 with the
//   appropriate values.
//
// PROGRAM HISTORY LOG:
// 2002-12-17  Gilbert
// 2004-07-19  Gilbert - Added check on whether the jpeg2000 encoding was
//                       successful.  If not, try again with different encoder
//                       options.

#ifdef NO_JASPER_LIB
  cerr << "ERROR: Template7_pt_4000::pack()" << endl;
  cerr << " NO_JASPER_LIB installed - cannot encode jpeg2000" << endl;
  return GRIB_FAILURE;
#endif

  fl32 *pdataPtr = _applyBitMapPack(dataPtr);
  si32 gridSz = _sectionsPtr.drs->getNumPackedDataPoints();
  DataRepTemp::data_representation_t drsConstants = _sectionsPtr.drs->getDrsConstants();

  si32 maxdif;

  int width = gridSz;
  int height = 1;

  Template5_pt_4000 *template5_pt_4000 = (Template5_pt_4000 *)_sectionsPtr.drs->getDrsTemplate();

  si32 bitMapIndicator = _sectionsPtr.bms->getBitMapIndicator();

  if(bitMapIndicator == 255) {
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
    cerr << "ERROR: Template7_pt_4000::pack()" << endl;
    cerr << "Data width * height != drs.numberPoints" << endl;
    if(pdataPtr != dataPtr)
      delete [] pdataPtr;
    return( GRIB_FAILURE );
  }
  fl32 bscale = pow(2.0 , -drsConstants.binaryScaleFactor);
  fl32 dscale = pow(10.0, drsConstants.decimalScaleFactor);
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
    //  Pack data into full octets, then do JPEG2000 encode.
    //  and calculate the length of the packed data in bytes
    //
    int retry = 0;
    nbytes = (nbits+7)/8;
    ui08 *ctemp = (ui08 *)calloc(gridSz,nbytes);

    int jpclen = gridSz;
      if(jpclen < jpcminlen)
	 jpclen = jpcminlen;
    if(_pdata)
      delete[] _pdata;
    _pdata = new fl32[jpclen];
    
    DS::sbits(ctemp,ifld,0,nbytes*8,0,gridSz);
    delete[] ifld;

    _lcpack = encode_jpeg2000(ctemp,width,height,nbits,template5_pt_4000->getCompresssionType(),
			     template5_pt_4000->getTargetCompresssionRatio(),
			     retry, (char *)_pdata, jpclen * sizeof(fl32));

    if (_lcpack < 0) {
      cerr << "Template7_pt_4000::pack()" << endl;
      cerr << "Jpeg2000 Encoding Failed " << endl;
      if (_lcpack == -3) {
	retry = 1;
	cerr << "Retrying Encoding...." << endl;
	_lcpack = encode_jpeg2000(ctemp,width,height,nbits,template5_pt_4000->getCompresssionType(),
				 template5_pt_4000->getTargetCompresssionRatio(),	 
				 retry,(char *)_pdata, gridSz * sizeof(fl32));
	if (_lcpack < 0) {
	  cerr << "ERROR: Template7_pt_4000::pack()" << endl;
	  cerr << "Retry Failed" << endl;
	  free(ctemp);

	  if(pdataPtr != dataPtr)
	    delete [] pdataPtr;

	  return GRIB_FAILURE;

	} else
	  cerr << "Retry Successful" << endl;
      }
    }
    free(ctemp);
    
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
  template5_pt_4000->setDrsConstants(drsConstants);

  if(template5_pt_4000->getCompresssionType() == 0)
    template5_pt_4000->setTargetCompresssionRatio(255);   // lossy not used

  //
  // If our data ptr is not the same as the input data ptr
  // then we packed the data with a bit map.  Delete the packed array
  //
  if(pdataPtr != dataPtr)
    delete [] pdataPtr;

  return GRIB_SUCCESS;
}

int Template7_pt_4000::unpack (ui08 *dataPtr) 
{
  si32 gridSz = _sectionsPtr.drs->getNumPackedDataPoints();
  DataRepTemp::data_representation_t drsConstants = _sectionsPtr.drs->getDrsConstants();
  fl32 *outputData = new fl32[gridSz];

  fl32 bscale = pow(2.0, drsConstants.binaryScaleFactor);
  fl32 dscale = pow(10.0, -drsConstants.decimalScaleFactor);   
  fl32 reference = drsConstants.referenceValue;
  
  int nbits = drsConstants.numberOfBits;
  if (nbits != 0) {
    
    si32 *tmp_data = new si32 [gridSz];
    si32 compressed_len = _sectionsPtr.ds->getSize() - 5;

    if(decode_jpeg2000 ((char *) dataPtr, compressed_len, tmp_data) == GRIB_FAILURE) {
      delete [] tmp_data;
      return GRIB_FAILURE;
    }
    
    for (int i = 0; i < gridSz; i++) {
      outputData[i] = (((fl32) tmp_data[i] * bscale) + reference) * dscale;
    }
    delete [] tmp_data;
    
  }
  else {
    for (int i = 0; i < gridSz; i++)
      outputData[i] = reference;
  }
  
  _applyBitMapUnpack(outputData);
  
  return GRIB_SUCCESS;
}

int Template7_pt_4000::decode_jpeg2000 (char *input, si32 inputSize, si32 *output) 
{

#ifdef NO_JASPER_LIB

  cerr << "ERROR: Template7_pt_4000::decode_jpeg2000()" << endl;
  cerr << " NO_JASPER_LIB installed - cannot decode jpeg2000" << endl;
  return GRIB_FAILURE;

#else

// Converted to a C++ method from:
//  SUBPROGRAM:    decode_jpeg2000      Decodes JPEG2000 code stream
//   PRGMMR: Gilbert          ORG: W/NP11     DATE: 2002-12-02
//
//  ABSTRACT: This Function decodes a JPEG2000 code stream specified in the
//    JPEG2000 Part-1 standard (i.e., ISO/IEC 15444-1) using JasPer
//    Software version 1.500.4 (or 1.700.2) written by the University of British
//    Columbia and Image Power Inc, and others.
//    JasPer is available at http://www.ece.uvic.ca/~mdadams/jasper/.
//
// USAGE:     int dec_jpeg2000(char *injpc,si32 bufsize,si32 *outfld)
//
//   INPUT ARGUMENTS:
//      input   - Input JPEG2000 code stream.
//    inputSize - Length (in bytes) of the input JPEG2000 code stream
//
//   OUTPUT ARGUMENTS:
//     output - Output matrix of grayscale image values
//
//   RETURN VALUES :
//          0 = Successful decode
//         -3 = Error decode jpeg2000 code stream.
//         -5 = decoded image had multiple color components.
//              Only grayscale is expected
//
// REMARKS
//
//     Requires JasPer Software version 1.500.4 or 1.700.2
//

    char *opts=0;

    jas_image_t *image=0;
    jas_stream_t *jpcstream;
    jas_image_cmpt_t *componentInfo;
    jas_matrix_t *matrixData;

    // jas_init();

    //  Create jas_stream_t containing input JPEG200 codestream in memory.
    jpcstream = jas_stream_memopen (input, inputSize);

    //  Decode JPEG200 codestream into jas_image_t structure.
    image = jpc_decode (jpcstream, opts);
    if ( image == 0 ) {
     cerr << "ERROR: Template7_pt_4000::decode_jpeg2000()" << endl;
     cerr << "jpc_decode did not return an imager" << endl;
     return -3;
    }
    
    //   Expecting jpeg2000 image to be grayscale only.
    if (image->numcmpts_ != 1 ) {
      cerr << "ERROR: Template7_pt_4000::decode_jpeg2000()" << endl;
      cerr << "Too many components - implies color image - Grayscale expected." << endl;
      return (-5);
    }

    //  grab the grayscale component
    componentInfo = image->cmpts_[0];

    //    Create a data matrix of grayscale image values decoded from
    //    the jpeg2000 codestream.
    matrixData=jas_matrix_create (jas_image_height(image), jas_image_width(image));

    jas_image_readcmpt (image, 0, 0, 0, jas_image_width(image),
                                               jas_image_height(image), matrixData);

    //    Copy data matrix to output integer array.
    si32 k = 0;
    for (si32 i = 0; i < componentInfo->height_; i++) 
      for (si32 j=0; j < componentInfo->width_; j++) 
        output[k++]=matrixData->rows_[i][j];

    //  
    jas_matrix_destroy (matrixData);
    jas_stream_close (jpcstream);
    jas_image_destroy (image);

    return GRIB_SUCCESS;

// ifdef NO_JASPER_LIB
#endif

}


int Template7_pt_4000::encode_jpeg2000 (ui08 *cin,int pwidth,int pheight,int pnbits,
					int ltype, int ratio, int retry, char *outjpc, 
					int jpclen) 
/*$$$  SUBPROGRAM DOCUMENTATION BLOCK
*                .      .    .                                       .
* SUBPROGRAM:    enc_jpeg2000      Encodes JPEG2000 code stream
*   PRGMMR: Gilbert          ORG: W/NP11     DATE: 2002-12-02
*
* ABSTRACT: This Function encodes a grayscale image into a JPEG2000 code stream
*   specified in the JPEG2000 Part-1 standard (i.e., ISO/IEC 15444-1) 
*   using JasPer Software version 1.500.4 (or 1.700.2 ) written by the 
*   University of British Columbia, Image Power Inc, and others.
*   JasPer is available at http://www.ece.uvic.ca/~mdadams/jasper/.
*
* PROGRAM HISTORY LOG:
* 2002-12-02  Gilbert
* 2004-07-20  GIlbert - Added retry argument/option to allow option of
*                       increasing the maximum number of guard bits to the
*                       JPEG2000 algorithm.
*
* USAGE:    int enc_jpeg2000(unsigned char *cin,g2int *pwidth,g2int *pheight,
*                            g2int *pnbits, g2int *ltype, g2int *ratio, 
*                            g2int *retry, char *outjpc, g2int *jpclen)
*
*   INPUT ARGUMENTS:
*      cin   - Packed matrix of Grayscale image values to encode.
*    pwidth  - Pointer to width of image
*    pheight - Pointer to height of image
*    pnbits  - Pointer to depth (in bits) of image.  i.e number of bits
*              used to hold each data value
*    ltype   - Pointer to indicator of lossless or lossy compression
*              = 1, for lossy compression
*              != 1, for lossless compression
*    ratio   - Pointer to target compression ratio.  (ratio:1)
*              Used only when *ltype == 1.
*    retry   - Pointer to option type.
*              1 = try increasing number of guard bits
*              otherwise, no additional options
*    jpclen  - Number of bytes allocated for new JPEG2000 code stream in
*              outjpc.
*
*   OUTPUT ARGUMENTS:
*     outjpc - Output encoded JPEG2000 code stream
*
*   RETURN VALUES :
*        > 0 = Length in bytes of encoded JPEG2000 code stream
*         -3 = Error decode jpeg2000 code stream.
*         -5 = decoded image had multiple color components.
*              Only grayscale is expected.
*
* REMARKS:
*
*      Requires JasPer Software version 1.500.4 or 1.700.2
*
* ATTRIBUTES:
*   LANGUAGE: C
*   MACHINE:  IBM SP
*
*$$$*/
{

#ifdef NO_JASPER_LIB

  cerr << "ERROR: Template7_pt_4000::encode_jpeg2000()" << endl;
  cerr << " NO_JASPER_LIB installed - cannot encode jpeg2000" << endl;
  return GRIB_FAILURE;

#else

    int ier,rwcnt;
    jas_image_t image;
    jas_stream_t *jpcstream,*istream;
    jas_image_cmpt_t cmpt,*pcmpt;
#define MAXOPTSSIZE 1024
    char opts[MAXOPTSSIZE];

    si32 width,height,nbits;
    width = pwidth;
    height = pheight;
    nbits = pnbits;
/*
    printf(" enc_jpeg2000:width %ld\n",width);
    printf(" enc_jpeg2000:height %ld\n",height);
    printf(" enc_jpeg2000:nbits %ld\n",nbits);
    printf(" enc_jpeg2000:jpclen %ld\n",*jpclen);
*/
//    jas_init();

//
//    Set lossy compression options, if requested.
//
    if ( ltype != 1 ) {
       opts[0]=(char)0;
    }
    else {
       snprintf(opts,MAXOPTSSIZE,"mode=real\nrate=%f",1.0/(float)ratio);
    }
    if ( retry == 1 ) {             // option to increase number of guard bits
       strcat(opts,"\nnumgbits=4");
    }
    //printf("SAGopts: %s\n",opts);
    
//
//     Initialize the JasPer image structure describing the grayscale
//     image to encode into the JPEG2000 code stream.
//
    image.tlx_=0;
    image.tly_=0;
#ifdef JAS_1_500_4 
    image.brx_=(uint_fast32_t)width;
    image.bry_=(uint_fast32_t)height;
#else
    image.brx_=(jas_image_coord_t)width;
    image.bry_=(jas_image_coord_t)height;
#endif
    image.numcmpts_=1;
    image.maxcmpts_=1;
#ifdef JAS_1_500_4
    image.colormodel_=JAS_IMAGE_CM_GRAY;         /* grayscale Image */
#else
    image.clrspc_=JAS_CLRSPC_SGRAY;         /* grayscale Image */
    image.cmprof_=0; 
#endif
    //image.inmem_=1;

    cmpt.tlx_=0;
    cmpt.tly_=0;
    cmpt.hstep_=1;
    cmpt.vstep_=1;
#ifdef JAS_1_500_4
    cmpt.width_=(uint_fast32_t)width;
    cmpt.height_=(uint_fast32_t)height;
#else
    cmpt.width_=(jas_image_coord_t)width;
    cmpt.height_=(jas_image_coord_t)height;
    cmpt.type_=JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y);
#endif
    cmpt.prec_=nbits;
    cmpt.sgnd_=0;
    cmpt.cps_=(nbits+7)/8;

    pcmpt=&cmpt;
    image.cmpts_=&pcmpt;

//
//    Open a JasPer stream containing the input grayscale values
//
    istream=jas_stream_memopen((char *)cin,height*width*cmpt.cps_);
    cmpt.stream_=istream;

//
//    Open an output stream that will contain the encoded jpeg2000
//    code stream.
//
    jpcstream = jas_stream_memopen(outjpc,(int)(jpclen));

//
//     Encode image.
//
    ier=jpc_encode(&image,jpcstream,opts);
    if ( ier != 0 ) {
       printf("jpc_encode return = %d \n",ier);
       return -3;
    }
//
//     Clean up JasPer work structures.
//    
    rwcnt=jpcstream->rwcnt_;
    ier=jas_stream_close(istream);
    ier=jas_stream_close(jpcstream);
//
//      Return size of jpeg2000 code stream
//
    return (rwcnt);

// ifdef NO_JASPER_LIB
#endif

}



} // namespace Grib2

