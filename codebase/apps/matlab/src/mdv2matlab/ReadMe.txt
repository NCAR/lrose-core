To compile mdv2matlab the following directories need to be in you path:

    /tools/matlab/extern/lib/glnx86 \
    /tools/matlab/bin \

and the environment variable LD_LIBRARY_PATH needs to be defined:

  setenv LD_LIBRARY_PATH /tools/matlab/extern/lib/glnx86

Then type 'make' in this directory to compile mdv2matlab.
