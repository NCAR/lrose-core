local kd_inc_dir = os.getenv("KD_INC")
local kd_lib_dir = os.getenv("KD_LIB")
local netcdf_inc_dir = os.getenv("NETCDF_INC")
local netcdf_lib_dir = os.getenv("NETCDF_LIB")

solution "kd"
   configurations { "Debug", "Release" }
 
   -- A project defines one build target
   project "kd"
      kind "StaticLib"
      language "C++"
      includedirs {kd_inc_dir, netcdf_inc_dir}
      targetdir(kd_lib_dir)
      files {
    "kd/fileoper.cc",
    "kd/kd.cc",
    "kd/metric.cc",
    "kd/naive.cc",
    "kd/pqueue.cc",
    "kd/kd_interp.cc",
    "kd/kd_query.cc",
    "kd/tokenize.cc",
    "include/kd/kd.hh",
    "include/kd/datatype.hh",
    "include/kd/fileoper.hh",
    "include/kd/kd_interp.hh",
    "include/kd/kd_query.hh",
    "include/kd/metric.hh",
    "include/kd/naive.hh",
    "include/kd/tokenize.hh"
 }
 
      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
	 postbuildcommands { "mkdir -p $(KD_INC)" } 
	 postbuildcommands { "cp include/kd/*.h* $(KD_INC)/" } 

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }    
	 postbuildcommands { "mkdir -p $(KD_INC)" } 
	 postbuildcommands { "cp include/kd/*.h* $(KD_INC)/" } 
