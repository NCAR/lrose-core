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
///////////////////////////////////////////////////////////////
// BrightBandFilter.cc
//
// BrightBandFilter object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 1999
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include "BrightBandFilter.hh"
#include <set>
using namespace std;

extern void svdcmp ( float **a, int m, int n, float w[], float **v);

// Constructor

BrightBandFilter::BrightBandFilter(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "BrightBandFilter";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // initialize the data input object

  if (_params.mode == Params::REALTIME) {
    if (_input.setRealtime(_params.input_url, 600,
			   PMU_auto_register)) {
      isOK = false;
    }
  } else if (_params.mode == Params::ARCHIVE) {
    if (_input.setArchive(_params.input_url,
			  _args.startTime,
			  _args.endTime)) {
      isOK = false;
    }
  } else if (_params.mode == Params::FILELIST) {
    if (_input.setFilelist(_args.inputFileList)) {
      isOK = false;
    }
  }

  return;

}

// destructor

BrightBandFilter::~BrightBandFilter()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int BrightBandFilter::Run()
{
  
  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");

  // loop until end of data
  
  _input.reset();
  while (!_input.endOfData()) {
    
    PMU_auto_register("In main loop");
    
    // create input DsMdvx object
    
    DsMdvx inMdvx;
    if (_params.debug) {
      inMdvx.setDebug(true);
    }
    
    // do the read

    if (_doRead(inMdvx)) {
      continue;
    }
    
    PMU_auto_register("Before write");
    
    // create output DsMdvx object
    
    DsMdvx outMdvx;
    if (_params.debug) {
      outMdvx.setDebug(true);
    }
    outMdvx.setWriteLdataInfo();
    Mdvx::master_header_t mhdr = inMdvx.getMasterHeader();
    outMdvx.setMasterHeader(mhdr);
    string info = inMdvx.getMasterHeader().data_set_info;
    info += " : Bright band removed by BrightBandFilter";
    outMdvx.setDataSetInfo(info.c_str());
    outMdvx.clearFields();
    
    // add any chunks
    
    outMdvx.clearChunks();
    for (int i = 0; i < inMdvx.getNChunks(); i++) {
      MdvxChunk *chunk = new MdvxChunk(*inMdvx.getChunkByNum(i));
      outMdvx.addChunk(chunk);
    }
    
    // extract vertical profile and load up output MDV plane
    // into the output object
    
    _extractVrps(inMdvx, outMdvx);
    
    // write out
    
    PMU_auto_register("Before write");
    if(outMdvx.writeToDir(_params.output_url)) {
      cerr << "ERROR - BrightBandFilter::Run" << endl;
      cerr << "  Cannot write data set." << endl;
      cerr << outMdvx.getErrStr() << endl;
      iret = -1;
    }
    
  } // while
  
  return iret;

}

/////////////////////////////////////////////////////////
// perform the read
//
// Returns 0 on success, -1 on failure.

int BrightBandFilter::_doRead(DsMdvx &inMdvx)
  
{
  
  inMdvx.clearRead();
  inMdvx.addReadField(_params.dbz_field);
  inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
 
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    inMdvx.printReadRequest(cerr);
  }
  
  // read in
  
  PMU_auto_register("Before read");
  
  if (_input.readVolumeNext(inMdvx)) {
    cerr << "ERROR - BrightBandFilter::_doRead" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << _input.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Read in file: " << inMdvx.getPathInUse() << endl;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// compute the full composite for all the fields read in

void BrightBandFilter::_extractVrps(const DsMdvx &inMdvx,
				    DsMdvx &outMdvx)
  
{

  MdvxField *field = inMdvx.getField(0);
  const Mdvx::field_header_t &fhdr = field->getFieldHeader();
  
  int nx = fhdr.nx;
  int ny = fhdr.ny;
  int nz = fhdr.nz;
  int nPointsCappi = nx * ny;
  
  int dim = (int) (nx / 2);
  int cnt = 0;

  int base_cappi = (int)
    ((_params.analysis_min_altitude - fhdr.grid_minz) / fhdr.grid_dz + 0.5);
  base_cappi = MAX(0, base_cappi);

  int top_cappi = (int)
    ((_params.analysis_max_altitude - fhdr.grid_minz) / fhdr.grid_dz + 0.5);
  top_cappi = MIN(nz - 1, top_cappi);

  // limit to 6 for the moment

  if ((top_cappi - base_cappi) > 6) {
    top_cappi = base_cappi + 6;
  }
  
  // allocate

  vrp_t *pr = (vrp_t * ) umalloc ( nPointsCappi * sizeof (vrp_t));
  vrp_t *tmp = (vrp_t * ) umalloc ( nPointsCappi * sizeof (vrp_t));
    
  fl32 *vol = (fl32 *) field->getVol();
  fl32 *plane0 = vol;
  fl32 *plane1 = vol + nPointsCappi;

  for (int i = 0; i < nPointsCappi; i++) {    
    
    int iy = i / ny;
    int ix = i - iy * nx;
    
    // zone I
    // we're interested in raining profiles 
    // i.e. > low threshold values at levels 0 and 1
    
    fl32 dbz0 = plane0[i];
    fl32 dbz1 = plane1[i];
    
    if ( iy > dim
	 && dbz0 > _params.dbz_threshold 
	 && dbz1 > _params.dbz_threshold ) {
      
      cnt++;
      pr[cnt].x = ix;
      pr[cnt].y = iy;
      
      // extract levels between min and max for processing
      
      for (int iz = base_cappi; iz < top_cappi + 1; iz++) {
	
	fl32 *plane = vol + iz * nPointsCappi;
	
	pr[cnt].vert[iz] = plane[i];
	
	if ( pr[cnt].vert[iz] < 0. ) 
	  pr[cnt].vert[iz] = 0.;
	
      } // iz
      
      // perform a censoring of level 1 ( 2 km)
      
      for ( int iz = 0; iz < top_cappi; iz++ )
	if ( iz == 0 )
	  tmp[cnt].vert[iz] = pr[cnt].vert[iz];
	else
	  tmp[cnt].vert[iz] = pr[cnt].vert[iz + 1];
      
      tmp[cnt].vert[top_cappi] = 0.; 
      
    } // if
    
  } // i
   
  // start svd regression processing

  double coef[8];

  if (cnt > 0) {
    _doSvdRegr ( cnt, top_cappi, pr, coef);
  }

  // double coef2[8];
  //  _doSvdRegr ( cnt, top_cappi - 1, tmp, coef2);
  
  // apply regression coefficients to field outside 40 km

  //  _applyRegrCoeffs ( input, coef, coef2 );

  // load up output Mdv field, add to output object

  Mdvx::field_header_t outFhdr = fhdr;
  outFhdr.nz = 1;
  outFhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.level[0] = 0;
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  
  MdvxField *outField = new MdvxField(outFhdr, vhdr);

  int volumeSize = nPointsCappi * sizeof(fl32);
  outField->setVolData(plane0, volumeSize, Mdvx::ENCODING_FLOAT32);
  outField->convertRounded(Mdvx::ENCODING_INT8,
			   (Mdvx::compression_type_t)
			   _params.output_compression_type);

  outMdvx.addField(outField);
  
  // free

  ufree (pr);
  ufree (tmp);

}
    
  
//////////////////////////////////////////////////
// _doSvdRegr ()
//
// Use SVD to calculate regression coefficients
//

int BrightBandFilter::_doSvdRegr ( int m, int n,
				   vrp_t *profile, double *coeff)
  
{

  int i, j, k, l;
  double r2, ra2, SE, sumy, dif1, dif2, sumdif1, sumdif2;
  float *w, **u, **v, *wti, *ye, *std, **cvm, **cor, *sum, sum2, *a;
  float tmp, tmpa, tmpy;

  w = (float * ) umalloc ( (n+1) * sizeof (float));
  wti = (float * ) umalloc ( (n+1) * sizeof (float));
  std = (float * ) umalloc ( (n+1) * sizeof (float));
  a = (float * ) umalloc ( (n+1) * sizeof (float));
  ye = (float * ) umalloc ( (m+1) * sizeof (float));
  sum = (float * ) umalloc ( (n+1) * sizeof (float));

  u = (float **) umalloc ( (m+1) * sizeof (float *));
  v = (float **) umalloc ( (n+1) * sizeof (float *));
  cvm = (float **) umalloc ( (n+1) * sizeof (float *));
  cor = (float **) umalloc ( (n+1) * sizeof (float *));

  for ( k = 1; k <= m; k++ )
    u[k] = (float *) umalloc ( (n+1) * sizeof (float));

  for ( l = 1; l <= n; l++ ) {
    v[l] = (float *) umalloc ( (n+1) * sizeof (float));
    cvm[l] = (float *) umalloc ( (n+1) * sizeof (float));
    cor[l] = (float *) umalloc ( (n+1) * sizeof (float));
  }

  fprintf ( stderr, "Number of VRPs =  %d \n", m);

   // copy original matrix into u
   // accommodate different CONVSTRAT and height groups

  for ( k = 1; k < m + 1; k++)
    for ( l = 1; l < n + 1; l++) {
      u[k][l] = profile[k].vert[l - 1];
      //      fprintf (stderr, " %d %d %f \n", k, l, u[k][l]);
    }

   // perform decomposition

  svdcmp ( u, m, n, w, v);

  if ( u[1][1] < 0.) {
    for ( k = 1; k <= m; k++)
      for ( l = 1; l <= n; l++)
        u[k][l] = -1. * u[k][l];

    for ( k = 1; k <= n; k++)
      for ( l = 1; l <= n; l++)
        v[l][k] = -1. * v[l][k];
  }

  // write results

  // for ( k = 1; k <= m; k++) {
  //    for ( l = 1; l <= n; l++)
  //      fprintf (stderr, "%12.6f", u[k][l]);
  //   fprintf (stderr, "\n");
  //  }

  fprintf (stderr, "Diagonal of matrix w\n");

  for ( k = 1; k <= n; k++)
    fprintf (stderr, "%12.6f",w[k]);
  fprintf (stderr, "\nMatrix v-transpose\n");

  for (k = 1; k <= n; k++) {
    for ( l = 1; l <= n; l++)
      fprintf (stderr, "%12.6f",v[l][k]);
    fprintf (stderr, "\n");
  }

  /* 
   * get regression coefficients
   */

  tmp = tmpa = 0.;

  for ( l = 1; l <= n; l++) {
    sum[l] = 0.;

    for ( k = 1; k <= m; k++) {
      tmp = profile[k].vert[0] * u[k][l];
      sum[l] += tmp;
    }

    if ( w[l] != 0. )
      sum[l] /= w[l];
    else
      sum[l] = 0.;
  }

  for ( i = 1; i <= n; i++ ) {
    a[i] = 0.;
    for ( l = 1; l <= n; l++ ) {
       tmpa = sum[l] * v[i][l];
      a[i] += tmpa;
    }
    fprintf ( stderr, "\n Coefficient %d =  %f ", i, a[i] );
    coeff[i] = a[i];
  } 

  fprintf ( stderr, " \n\n");

  /*
   * calculate R^2
   */
  sumy = 0.;

  for ( k = 1; k <= m; k++ ) {
    ye[k] = 0.;
    for ( i = 1; i <= n; i++ ) {
      tmpy = a[i] * profile[k].vert[i];
      ye[k] += tmpy;
    }

    sumy += profile[k].vert[0];
  }

  sumdif1 = sumdif2 = 0.;

  sumy /= m;

  for ( k = 1; k <= m; k++ ) {

    dif1 = pow ((double) ( profile[k].vert[0] - ye[k] ), 2. );
    dif2 = pow ((double) ( profile[k].vert[0] - sumy ), 2. );

    sumdif1 += dif1;
    sumdif2 += dif2;

  }

  r2 = 1. - ( sumdif1 / sumdif2 );
  ra2 = 1. - (( sumdif1 / ( m - n )) / ( sumdif2 / ( m - 1. )));

  fprintf ( stderr, "\n R2 = %f   Ra2 = %f \n\n", r2, ra2);

  /*
   * calculate the residual mean square estimate of the error variance
   */

  SE = sqrt ( sumdif1 / ( m - n + 1. ));
  fprintf ( stderr, " SE = %f \n\n", SE);

  /*
   * calculate the dispersion matrix
   */

  for ( i = 1; i <= n; i++ ) {
    wti[i] = 0.;
    if ( w[i] != 0. )
      wti[i] = 1. / ( w[i] * w[i] );
    else
      wti[i] = 0.;
  }

  for ( i = 1; i <= n; i++ ) {
    for ( j = 1; j <= i; j++ ) {

      sum2 = 0.;

      for ( k = 1; k <= n; k++ ) {
        tmp = v[i][k] * v[j][k] * wti[k];
        sum2 += tmp;
      }

      cvm[j][i] = cvm[i][j] = sum2;

      if ( i == j )
        std[i] = sqrt ( cvm[i][j] );
    }
    fprintf ( stderr, "std[%d] = %f ", i, std[i]);
  }

  fprintf ( stderr, "\n\n");

  /*
   * get the correlation matrix
   */
  for ( i = 1; i <= n; i++ ) {
    for ( j = 1; j <= n; j++ ) {
      if ( i == j )
        cor[i][j] = 1.;
      else
        cor[i][j] = cvm[i][j] / sqrt ( cvm[i][i] * cvm[j][j] );
        fprintf ( stderr, " %7.4f ", cor[i][j]);
    }
    fprintf ( stderr, "\n");
  }

  /*
   * ufree all the allocated memory
   */

  for ( k = 1; k <= m; k++ ) {
    ufree (u[k]);
  }
  ufree (u);

  for ( l = 1; l <= n; l++ ) {
      ufree (v[l]);
      ufree (cor[l]);
      ufree (cvm[l]);
  }
  ufree (v);
  ufree (cvm);
  ufree (cor);

  ufree (w);
  ufree (wti);
  ufree (std);
  ufree (a);
  ufree (ye);
  ufree (sum);

  return 0;

}


