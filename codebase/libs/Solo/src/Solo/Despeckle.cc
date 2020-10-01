#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <string>

// extern "C" is needed for python access to library
// but this causes problems when linking from C++ code
//extern "C"
//{

// A function adding two integers and returning the result
int SampleAddInt(int i1, int i2)
{
  return i1 + i2;
}

  //
  //  end points to first bad after length of good
  // 
void zap_speckle(size_t start, size_t end, float *data, float bad) {
  for (size_t i = start; i<end; i++) 
    data[i] = bad; 
}


//int se_despeckle(arg, cmds)             // #despeckle# 
//int arg;
//struct ui_command *cmds;
// clip_gate determines the end of the ray
// a_speckle determines the number of contiguous good data considered a speckle
void se_despeckle(const float *data, float *newData, size_t nGates, float bad, int a_speckle,
		  size_t dgi_clip_gate, bool *boundary_mask) 
{
  //  struct ui_command *cmdq=cmds+1; // point to the first argument 
  //  struct solo_edit_stuff *seds, *return_sed_stuff();
  //  struct dd_general_info *dgi, *dd_window_dgi();
  //  struct dds_structs *dds;
  //  char *dst_name;

  int nc;
  int ii, nn, mark;
  float  *ss, *zz, *tt, *nss;
  bool *bnd;


  //  dst_name = (cmdq++)->uc_text;
  //  nd = strlen(dst_name);

  //  seds = return_sed_stuff();
  //  if(seds->finish_up) {
  //    return(1);
  //  }
  //  seds->modified = YES;
  //  a_speckle = seds->a_speckle;
  //  dgi = dd_window_dgi(seds->se_frame);
  //  dds = dgi->dds;
  //  if (dgi_clip_gate >= nGates) throw std::invalid_argument("dgi_clip_gate greater than number of gates");
  if (dgi_clip_gate > nGates) throw std::invalid_argument("dgi_clip_gate greater than number of gates");
  nc = dgi_clip_gate;
  bnd = boundary_mask;

  // memcopy data into newData
  memcpy(newData, data, nGates*sizeof(float));
  /*
  //printf("\n");
  for (size_t i = 0; i<nGates; i++) {
    newData[i] = data[i];
    //printf("%f,", newData[i]);
  }
  //printf("\n");
  */

  size_t zzIdx = nGates;
  if (dgi_clip_gate < nGates)
    zzIdx = dgi_clip_gate;
  //                                                                                                             
  // loop through the data                                                                                       
  //

  size_t ssIdx = 0;
  size_t length = 0;
  // accumulate length of good data until we hit a bad data;
  // if the length is a speckle, zap it
  // reset the length if we are out of boundary, and
  // after zapping a speckle
  //
  
  while (ssIdx < zzIdx) {
    if (boundary_mask[ssIdx]) {
      bool bad_data = abs(newData[ssIdx] - bad) < 0.00001; 
      if (bad_data) {
        if ((length > 0) && (length <= a_speckle)) {
          size_t start = ssIdx - length;
          size_t end = ssIdx;
          if (start == 0) { // is a speckle; otherwise, skip it
	    zap_speckle(start, end, newData, bad);
	  } else {  // do some further investigation;
	    // if this is a boundary
            if (boundary_mask[start-1]) { // it's a speckle
	      zap_speckle(start, end, newData, bad);
	    } // else,  we cannot be sure what is on the other
	      // side of the boundary, so skip it
	  }
        }
        length = 0;
      } else { // good data
        length += 1;
      }
    } else { // outside boundary
      length = 0;
    }
    ssIdx += 1;
    // printf("ssIdx=%d length=%d\n", ssIdx, length);
  }
  // consider any ending length
  bool at_end_of_ray = ssIdx >= nGates;
  if ((length > 0) && (length <= a_speckle) && at_end_of_ray) {
    size_t start = ssIdx - length;
    size_t end = ssIdx;
    zap_speckle(start, end, newData, bad);
  }
  
  /*
  size_t ssIdx = 0;
  bool done = false;
  while (!done) {  // (ssIdx < zzIdx) {
    // move to first good gate inside a boundary
    while (ssIdx < zzIdx && (data[ssIdx] == bad || !(boundary_mask[ssIdx]))) {
      ssIdx += 1;
    }
    if (ssIdx >= zzIdx) {
      done = true;
    } else {
      // now move forward to the next bad flag                                                                   
      //
      size_t nNotBad = 0;
      size_t speckleStartIdx = ssIdx;
      size_t speckleEndIdx = ssIdx;
      while (ssIdx < zzIdx && boundary_mask[ssIdx] && data[ssIdx] != bad) {
        nNotBad += 1;
        ssIdx += 1;
      } 
      //(tt=ss,nn=0; ss < zz && *bnd && *ss != bad; nn++,ss++,bnd++);
      // Why did we exit the loop?
      // ssIdx could be past the end of the array
      if (ssIdx >= zzIdx) {
        done = true;
      } else { 
        // not at end of array; process this possible speckle 
        // could be outside boundary
        // found a good data value  ==> determine if the length
     
        if(boundary_mask[ssIdx] && nNotBad <= a_speckle) {
          while (ttIdx < ssIdx) {
            newData[ttIdx] = bad;      // zap speckle 
            ttIdx+= 1;
          } 
≈ß        }
      } // else not past the end of the array
    } // else 
  } // end while !done = past the end of the array
  */

  /*
  for(; ss < zz; ss++,bnd++) {
    // move to first good gate inside a boundary                                                               
    //
    for(; ss < zz && (*ss == bad || !(*bnd)); ss++,bnd++);
    if(ss >= zz) break;
    // now move forward to the next bad flag                                                                   
    //
    for(tt=ss,nn=0; ss < zz && *bnd && *ss != bad; nn++,ss++,bnd++);
    if(!(*bnd) || nn > a_speckle)
      continue;         // not a speckle or outside boundary 
    for(;tt < ss;)
      *tt++ = bad;      // zap speckle 
  }
  return(fn);
  */
}

//}
