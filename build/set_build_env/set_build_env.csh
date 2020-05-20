# Set environment variables for building LROSE
# for csh and tcsh
# Source this file before the build

# Operating system

setenv HOST_OS LINUX_LROSE

# directories

setenv LROSE_CORE_DIR ~/git/lrose-core
setenv LROSE_INSTALL_DIR ~/lrose

# path

set path = ($LROSE_INSTALL_DIR/bin $path)
