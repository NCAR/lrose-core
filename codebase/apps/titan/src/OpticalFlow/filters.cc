// %=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%
// ** Ancilla Radar Quality Control System (ancilla)
// ** Copyright BOM (C) 2013
// ** Bureau of Meteorology, Commonwealth of Australia, 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from the BOM.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of the BOM nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// %=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%

#include "filters.h"

#include <alloca.h>
#include <stdexcept>

using namespace ancilla;

static void run_kernel_x(
      array2<real>& output
    , const array2<real>& input
    , const real kernel[]
    , int kernel_size)
{
  // get signed versions of our dims (eliminates many casts)
  const int dims_x = output.cols();
  const int dims_y = output.rows();

  // note: to enable safe in-place operation (ie: src == dst), a buffer is used that delays
  //       output of calculated values back into the image until the kernel has fully passed
  //       over the value in question.  Output is delayed by 'side' iterations.
  // note: the use of negative array indexes in this function is intentional (not a bug)
  if (dims_x < kernel_size)
    throw std::runtime_error("filter field smaller than kernel - unimplemented feature");

  auto inp = input.data();
  auto out = output.data();
  auto buf = static_cast<real*>(alloca(kernel_size * sizeof(real)));

  int side = kernel_size / 2;
  for (int y = 0; y < dims_y; ++y)
  {
    // left border (replicate left most pixel)
    for (int x = 0; x < side; ++x)
    {
      real val = 0.0_r;
      for (int i = 0; i < side - x; ++i)
        val += kernel[i] * inp[-x];
      for (int i = side - x; i < kernel_size; ++i)
        val += kernel[i] * inp[i - side];
      ++inp;

      // just fill our buffer
      buf[x % side] = val;
    }

    // middle section of row
    for (int x = side; x < dims_x - side; ++x)
    {
      real val = 0.0_r;
      for (int i = 0; i < kernel_size; ++i)
        val += kernel[i] * inp[i - side];
      ++inp;

      // output buffered value, and fill more space in it
      *out = buf[x % side];
      buf[x % side] = val;
      ++out;
    }

    // right border (replicate right most pixel)
    for (int x = dims_x - side; x < dims_x; ++x)
    {
      real val = 0.0_r;
      for (int i = 0; i < kernel_size - ((side + 1) - (dims_x - x)); ++i)
        val += kernel[i] * inp[i - side];
      for (int i = kernel_size - ((side + 1) - (dims_x - x)); i < kernel_size; ++i)
        val += kernel[i] * inp[dims_x - x - 1];
      ++inp;

      // output buffered value and fill more space
      *out = buf[x % side];
      buf[x % side] = val;
      ++out;
    }

    // flush remaining buffered values
    for (int x = dims_x; x < dims_x + side; ++x)
    {
      *out = buf[x % side];
      ++out;
    }
  }
}

static void run_kernel_y(
      array2<real>& output
    , const array2<real>& input
    , const real kernel[]
    , int kernel_size)
{
  // get signed versions of our dims (eliminates many casts)
  const int dims_x = output.cols();
  const int dims_y = output.rows();

  // note: to enable safe in-place operation (ie: src == dst), a buffer is used that delays
  //       output of calculated values back into the image until the kernel has fully passed
  //       over the value in question.  Output is delayed by 'side' iterations.
  // note: the use of negative array indexes in this function is intentional (not a bug)
  if (dims_y < kernel_size)
    throw std::runtime_error("filter field smaller than kernel - unimplemented feature");

  int side = kernel_size / 2;
  for (int x = 0; x < dims_x; ++x)
  {
    auto inp = &input.data()[x];
    auto out = &output.data()[x];
    auto buf = static_cast<real*>(alloca(kernel_size * sizeof(real)));

    // top border (replicate top most pixel)
    for (int y = 0; y < side; ++y)
    {
      real val = 0.0_r;
      for (int i = 0; i < side - y; ++i)
        val += kernel[i] * inp[-y * dims_x];
      for (int i = side - y; i < kernel_size; ++i)
        val += kernel[i] * inp[(i - side) * dims_x];
      inp += dims_x;

      // just fill our buffer
      buf[y % side] = val;
    }

    // middle section of column
    for (int y = side; y < dims_y - side; ++y)
    {
      real val = 0.0_r;
      for (int i = 0; i < kernel_size; ++i)
        val += kernel[i] * inp[(i - side) * dims_x];
      inp += dims_x;

      // output a buffered value and record the currently calculated one
      *out = buf[y % side];
      buf[y % side] = val;
      out += dims_x;
    }

    // bottom border (replicate bottom most pixel)
    for (int y = dims_y - side; y < dims_y; ++y)
    {
      real val = 0.0_r;
      for (int i = 0; i < kernel_size - ((side + 1) - (dims_y - y)); ++i)
        val += kernel[i] * inp[(i - side) * dims_x];
      for (int i = kernel_size - ((side + 1) - (dims_y - y)); i < kernel_size; ++i)
        val += kernel[i] * inp[(dims_y - y - 1) * dims_x];
      inp += dims_x;

      // output a buffered value and record current calculation
      *out = buf[y % side];
      buf[y % side] = val;
      out += dims_x;
    }

    // flush remaining buffered values
    for (int y = dims_y; y < dims_y + side; ++y)
    {
      *out = buf[y % side];
      out += dims_x;
    }
  }
}

void ancilla::filters::gaussian_blur(
      array2<real>& output
    , const array2<real>& input
    , int kernel_size
    , real sigma)
{
  // sanity checks
  if (output.size() != input.size())
    throw std::logic_error("array size mismatch");

  // auto calculate the kernel size from sigma or vice versa (if desired)
  if (kernel_size == 0 && sigma > 0.0_r)
    kernel_size = static_cast<int>(std::round(sigma * 4 * 2 + 1)) | 1;
  else if (sigma <= 0.0_r && kernel_size > 0)
    sigma = ((kernel_size - 1) * 0.5_r - 1.0_r) * 0.3_r + 0.8_r;

  // sanity checks
  if (kernel_size < 0 || kernel_size % 2 != 1)
    throw std::invalid_argument("gaussian_blur: invalid kernel size (must be 0 or odd)");

  // build the kernel
  real* kernel = static_cast<real*>(alloca(kernel_size * sizeof(real)));
  {
    real scale_2x = -0.5_r / (sigma * sigma);
    real sum = 0.0_r;
    for (int i = 0; i < kernel_size; ++i)
    {
      real x = i - (kernel_size - 1) * 0.5_r;
      kernel[i] = std::exp(scale_2x * x * x);
      sum += kernel[i];
    }
    sum = 1.0_r / sum;
    for (int i = 0; i < kernel_size; ++i)
      kernel[i] *= sum;
  }

  // as our kernel is symetrical, we can do 2 passes of the 1D kernel
  run_kernel_x(output, input, kernel, kernel_size);
  run_kernel_y(output, output, kernel, kernel_size);
}

