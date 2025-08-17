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
//////////////////////////////////////////////////////////////////////
//  Ncxx C++ classes for NetCDF4
//
//  NetCDF write for multiple fields in separate processes
//
//  Copied from code by:
//
//    Lynton Appel, of the Culham Centre for Fusion Energy (CCFE)
//    in Oxfordshire, UK.
//    The netCDF-4 C++ API was developed for use in managing
//    fusion research data from CCFE's innovative MAST
//    (Mega Amp Spherical Tokamak) experiment.
// 
//  Offical NetCDF codebase is at:
//
//    https://github.com/Unidata/netcdf-cxx4
//
//  Modification for LROSE made by:
//
//    Mike Dixon, EOL, NCAR
//    P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//  The base code makes extensive use of exceptions.
//  Additional methods have been added to return error conditions. 
//
//  December 2016
//
//////////////////////////////////////////////////////////////////////

#include <Ncxx/Ncxx.hh>
#include <Ncxx/NcxxFile.hh>
#include <Ncxx/NcxxGroup.hh>
#include <Ncxx/NcxxDim.hh>
#include <Ncxx/NcxxVar.hh>
#include <Ncxx/NcxxCheck.hh>
#include <Ncxx/NcxxException.hh>
#include <Ncxx/NcxxByte.hh>
#include <Ncxx/NcxxInt.hh>
#include <Ncxx/NcxxFloat.hh>
#include<iostream>
#include<string>
#include<sstream>
using namespace std;

int g_ncid = -1;

// destructor

NcxxFile::~NcxxFile()
{
  // destructor may be called due to an exception being thrown
  // hence throwing an exception from within a destructor
  // causes undefined behaviour! so just printing a warning message
  try
  {
    close();
  }
  catch (NcxxException &e)
  {
    cerr << e.what() << endl;
  }
}

void NcxxFile::close()
{
  if (!nullObject) {
    ncxxCheck(nc_close(myId),__FILE__,__LINE__);
    g_ncid = -1;
  }
  nullObject = true;
  _pathInUse.clear();
  _errStr.clear();
  _format = nc4;
  _mode = read;
}

// Default constructor generates a null object.

NcxxFile::NcxxFile() :
        NcxxGroup()  // invoke base class constructor
{
  close();
}

// constructor

NcxxFile::NcxxFile(const string& filePath, const FileMode fMode) :
        NcxxGroup()
{
  open(filePath, fMode);
}

// open a file from path and mode

void NcxxFile::open(const string& filePath, const FileMode fMode)
{

  if (!nullObject) {
    close();
  }

  switch (fMode) {
    case NcxxFile::write:
      _mode = write;
      ncxxCheck(nc_open(filePath.c_str(), NC_WRITE, &myId),
                __FILE__, __LINE__);
      break;
    case NcxxFile::read:
      _mode = read;
      ncxxCheck(nc_open(filePath.c_str(), NC_NOWRITE, &myId),
                __FILE__, __LINE__);
      break;
    case NcxxFile::newFile:
      _mode = newFile;
      ncxxCheck(nc_create(filePath.c_str(),
                          NC_NETCDF4 | NC_NOCLOBBER, &myId),
                __FILE__, __LINE__);
      break;
    case NcxxFile::replace:
      _mode = replace;
      ncxxCheck(nc_create(filePath.c_str(),
                          NC_NETCDF4 | NC_CLOBBER, &myId),
                __FILE__, __LINE__);
      break;
  }

  _pathInUse = filePath;
  g_ncid = myId;
  nullObject=false;

}

// constructor with file type specified

NcxxFile::NcxxFile(const string& filePath, 
                   const FileMode fMode,
                   const FileFormat fFormat )
{
  open(filePath, fMode, fFormat);
}

void NcxxFile::open(const string& filePath,
                    const FileMode fMode,
                    const FileFormat fFormat )
{

  if (!nullObject) {
    close();
  }

  _format = fFormat;
  int ncFormat = 0;
  switch (fFormat)
  {
    case NcxxFile::classic:
      ncFormat = 0;
      break;
    case NcxxFile::classic64:
      ncFormat = NC_64BIT_OFFSET;
      break;
    case NcxxFile::nc4:
      ncFormat = NC_NETCDF4;
      break;
    case NcxxFile::nc4classic:
      ncFormat = NC_NETCDF4 | NC_CLASSIC_MODEL;
      break;
  }
  switch (fMode)
  {
    case NcxxFile::write:
      _mode = write;
      ncxxCheck(nc_open(filePath.c_str(), ncFormat | NC_WRITE, &myId),
                __FILE__, __LINE__);
      break;
    case NcxxFile::read:
      _mode = read;
      ncxxCheck(nc_open(filePath.c_str(), ncFormat | NC_NOWRITE, &myId),
                __FILE__, __LINE__);
      break;
    case NcxxFile::newFile:
      _mode = newFile;
      ncxxCheck(nc_create(filePath.c_str(), ncFormat | NC_NOCLOBBER, &myId),
                __FILE__, __LINE__);
      break;
    case NcxxFile::replace:
      _mode = replace;
      ncxxCheck(nc_create(filePath.c_str(), ncFormat | NC_CLOBBER, &myId),
                __FILE__, __LINE__);
      break;
  }

  _pathInUse = filePath;
  g_ncid = myId;
  nullObject=false;

}

/////////////////////////////////////////////
// Synchronize an open netcdf dataset to disk

void NcxxFile::sync()
{
  ncxxCheck(nc_sync(myId), __FILE__, __LINE__);
}

//////////////////////////////////////////////
// Leave define mode, used for classic model

void NcxxFile::enddef()
{
  ncxxCheck(nc_enddef(myId), __FILE__, __LINE__);
}

#ifdef JUNK

// sys_resources.hpp  (C++17)
// Query CPU count & RAM, then choose a worker/process pool size.

#pragma once
#include <cstdint>
#include <algorithm>
#include <cstdlib>
#include <thread>

#if defined(_WIN32)
  #define NOMINMAX
  #include <windows.h>
#elif defined(__APPLE__) || defined(__MACH__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
  #include <sys/types.h>
  #include <sys/sysctl.h>
  #include <unistd.h>
#elif defined(__linux__)
  #include <unistd.h>
#endif

namespace sysres {

inline unsigned logical_cpus() {
#if defined(_WIN32)
  SYSTEM_INFO si{};
  GetSystemInfo(&si);
  return si.dwNumberOfProcessors ? static_cast<unsigned>(si.dwNumberOfProcessors)
                                 : std::max(1u, std::thread::hardware_concurrency());
#else
  // POSIX
  long n = -1;
  #if defined(_SC_NPROCESSORS_ONLN)
    n = ::sysconf(_SC_NPROCESSORS_ONLN);
  #endif
  if (n > 0) return static_cast<unsigned>(n);
  // Fallback
  const unsigned hc = std::thread::hardware_concurrency();
  return hc ? hc : 1u;
#endif
}

inline std::uint64_t total_memory_bytes() {
#if defined(_WIN32)
  MEMORYSTATUSEX status{};
  status.dwLength = sizeof(status);
  if (GlobalMemoryStatusEx(&status))
    return static_cast<std::uint64_t>(status.ullTotalPhys);
  return 0;
#elif defined(__APPLE__) || defined(__MACH__)
  std::uint64_t mem = 0;
  size_t len = sizeof(mem);
  int mib[2] = {CTL_HW, HW_MEMSIZE};
  if (::sysctl(mib, 2, &mem, &len, nullptr, 0) == 0) return mem;
  return 0;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
  // Try hw.physmem or hw.realmem depending on BSD flavor
  std::uint64_t mem = 0;
  size_t len = sizeof(mem);
  int mib[2] = {CTL_HW,
  #if defined(HW_PHYSMEM64)
    HW_PHYSMEM64
  #elif defined(HW_REALMEM)
    HW_REALMEM
  #else
    HW_PHYSMEM
  #endif
  };
  if (::sysctl(mib, 2, &mem, &len, nullptr, 0) == 0) return mem;
  return 0;
#elif defined(__linux__)
  long pages = ::sysconf(_SC_PHYS_PAGES);
  long page_sz = ::sysconf(_SC_PAGE_SIZE);
  if (pages > 0 && page_sz > 0)
    return static_cast<std::uint64_t>(pages) * static_cast<std::uint64_t>(page_sz);
  return 0;
#else
  return 0;
#endif
}

// Heuristic selector.
// - mem_per_worker bytes is your *peak* per-process budget (chunk cache + buffers).
// - io_bound: allow mild oversubscription when true.
// - hard_cap: optional absolute cap (e.g., from a config/env var).
inline unsigned choose_worker_count(std::uint64_t mem_per_worker,
                                    bool io_bound = false,
                                    unsigned hard_cap = 0) {
  unsigned cpus = std::max(1u, logical_cpus());

  // Allow mild oversubscription for I/O-bound tasks
  const double cpu_factor = io_bound ? 1.5 : 1.0;
  unsigned by_cpu = std::max(1u, static_cast<unsigned>(cpus * cpu_factor));

  std::uint64_t total_mem = total_memory_bytes();
  unsigned by_mem = 0;
  if (mem_per_worker > 0 && total_mem > 0) {
    // Keep a safety margin (leave ~10% RAM free)
    const double safety = 0.90;
    auto usable = static_cast<std::uint64_t>(total_mem * safety);
    by_mem = static_cast<unsigned>(std::max<std::uint64_t>(1, usable / mem_per_worker));
  }

  unsigned n = 0;
  if (by_mem > 0) n = std::min(by_cpu, by_mem);
  else           n = by_cpu;

  if (hard_cap) n = std::min(n, hard_cap);

  // Also respect an optional environment override: APP_WORKERS
  if (const char* env = std::getenv("APP_WORKERS")) {
    try {
      int v = std::stoi(env);
      if (v > 0) n = static_cast<unsigned>(v);
    } catch (...) {}
  }
  return std::max(1u, n);
}

} // namespace sysres

#include "sys_resources.hpp"
#include <iostream>

int main() {
  // Estimate: e.g., 512 MiB per worker (chunk cache + I/O buffers)
  const std::uint64_t mem_per_worker = 512ull * 1024 * 1024;

  const bool io_bound = false;   // true if your stage is I/O heavy; false for CPU-bound compression
  const unsigned hard_cap = 0;   // e.g., 32 to limit farm width

  unsigned nworkers = sysres::choose_worker_count(mem_per_worker, io_bound, hard_cap);

  std::cout << "CPUs: " << sysres::logical_cpus()
            << ", RAM: " << (sysres::total_memory_bytes() / (1024.0*1024.0*1024.0)) << " GiB"
            << ", chosen workers: " << nworkers << "\n";

  // … spawn a process pool of size nworkers …
}

long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
long pages = sysconf(_SC_PHYS_PAGES);
long page_size = sysconf(_SC_PAGE_SIZE);
unsigned long long total_mem = (unsigned long long) pages * page_size;

int mem_per_proc_mb = 512; // estimate per worker
int max_procs_by_mem = (int)(total_mem / (mem_per_proc_mb * 1024ULL * 1024ULL));

int nworkers = nprocs;
if (nworkers > max_procs_by_mem)
    nworkers = max_procs_by_mem;

// windows - memory

#include <windows.h>
MEMORYSTATUSEX status;
status.dwLength = sizeof(status);
GlobalMemoryStatusEx(&status);
unsigned long long total_mem = status.ullTotalPhys;

// posix / linux - memory

#include <unistd.h>
long pages = sysconf(_SC_PHYS_PAGES);
long page_size = sysconf(_SC_PAGE_SIZE);
unsigned long long total_mem = (unsigned long long) pages * page_size;

// macos bsd - memory

#include <sys/types.h>
#include <sys/sysctl.h>
int mib[2] = {CTL_HW, HW_MEMSIZE};
uint64_t mem;
size_t len = sizeof(mem);
sysctl(mib, 2, &mem, &len, NULL, 0);

// posix/linux/bsd - cpu cores

#include <unistd.h>
long nprocs = sysconf(_SC_NPROCESSORS_ONLN);  // online (available) cores
long nprocs_conf = sysconf(_SC_NPROCESSORS_CONF); // configured cores

// windows - cpu cores

#include <windows.h>
SYSTEM_INFO sysinfo;
GetSystemInfo(&sysinfo);
DWORD nprocs = sysinfo.dwNumberOfProcessors;


// minimal C-style merge sketch - API equiv of "ncks -A -v var src.nc dest.nc
// NCO is a long-standing command-line suite developed by Charlie Zender (UC Irvine).
  
/* Pseudocode: copy one variable from src to dst */
int copy_var(const char* src_path, int dst_ncid, const char* varname) {
  int s_nc=-1, s_var=-1, ret=0;
  NC_CHECK(nc_open(src_path, NC_NOWRITE, &s_nc));
  NC_CHECK(nc_inq_varid(s_nc, varname, &s_var));

  // 1) Ensure needed dimensions exist in dst, create if missing
  int ndims=0; NC_CHECK(nc_inq_varndims(s_nc, s_var, &ndims));
  int dimids[NC_MAX_VAR_DIMS]; NC_CHECK(nc_inq_vardimid(s_nc, s_var, dimids));
  int dst_dimids[NC_MAX_VAR_DIMS];
  for (int i=0;i<ndims;i++){
    char dname[NC_MAX_NAME+1]; size_t dlen=0;
    NC_CHECK(nc_inq_dim(s_nc, dimids[i], dname, &dlen));
    int dst_dimid=-1;
    if (nc_inq_dimid(dst_ncid, dname, &dst_dimid) != NC_NOERR)
      NC_CHECK(nc_def_dim(dst_ncid, dname, dlen, &dst_dimid));
    dst_dimids[i]=dst_dimid;
  }

  // 2) Create target var with same type + compression + chunking
  nc_type vtype; NC_CHECK(nc_inq_vartype(s_nc, s_var, &vtype));
  int d_var=-1; NC_CHECK(nc_def_var(dst_ncid, varname, vtype, ndims, dst_dimids, &d_var));

  // mirror compression settings if present
  int shuffle=0, deflate=0, deflate_level=0;
  if (nc_inq_var_deflate(s_nc, s_var, &shuffle, &deflate, &deflate_level) == NC_NOERR)
    NC_CHECK(nc_def_var_deflate(dst_ncid, d_var, shuffle, deflate, deflate_level));

  // mirror chunking (if chunked)
  int storage=0; size_t chunks[NC_MAX_VAR_DIMS];
  if (nc_inq_var_chunking(s_nc, s_var, &storage, chunks) == NC_NOERR && storage==NC_CHUNKED)
    NC_CHECK(nc_def_var_chunking(dst_ncid, d_var, NC_CHUNKED, chunks));

  // copy attributes
  int natts=0; NC_CHECK(nc_inq_varnatts(s_nc, s_var, &natts));
  for (int a=0;a<natts;a++){
    char aname[NC_MAX_NAME+1];
    NC_CHECK(nc_inq_attname(s_nc, s_var, a, aname));
    NC_CHECK(nc_copy_att(s_nc, s_var, aname, dst_ncid, d_var));
  }
  NC_CHECK(nc_enddef(dst_ncid));

  // 3) Stream data
  // (For large arrays, loop with hyperslabs; here we assume a single read/write)
  size_t nels=1, shape[NC_MAX_VAR_DIMS];
  for (int i=0;i<ndims;i++){ size_t len; nc_inq_dimlen(dst_ncid, dst_dimids[i], &len); shape[i]=len; nels*=len; }
  // allocate buffer based on vtype; omitted for brevity
  // nc_get_var_* and nc_put_var_* with matching types

  NC_CHECK(nc_close(s_nc));
  return 0;
}


#endif

