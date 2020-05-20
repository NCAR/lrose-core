# Set environment variables for building CIDD
# for csh and tcsh
# Source this file before the build

# Operating system

setenv HOST_OS LINUX_64_CIDD32

# directories

setenv LROSE_CORE_DIR ~/cidd/lrose-core
setenv LROSE_INSTALL_DIR ~/cidd

# path

set path = ($LROSE_INSTALL_DIR/bin $path)

