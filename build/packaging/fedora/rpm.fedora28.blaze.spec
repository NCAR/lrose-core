%define build_timestamp %(date +"%Y%m%d")
%define _topdir     /root/rpmbuild
%define name        lrose 
%define release     %{build_timestamp}.fedora28
%define version     blaze
%define buildroot %{_topdir}/%{name}-%{version}-%{release}-root
 
BuildRoot:      %{_topdir}/installedhere
Summary:        LROSE
License:        BSD LICENSE
Name:           %{name}
Version:        %{version}
Release:        %{release}
Prefix:         /usr/local/lrose
Group:          Scientific Tools
AutoReqProv:    no
 
Requires: libX11-devel, libXext-devel
Requires: libpng-devel, libtiff-devel, zlib-devel
Requires: expat-devel, libcurl-devel
Requires: flex-devel, fftw3-devel
Requires: bzip2-devel, qt5-qtbase-devel
Requires: hdf5-devel, netcdf-devel
Requires: xorg-x11-xauth, xorg-x11-apps

%description
LROSE - Lidar Radar Open Software Environment
 
%build
rm -f %{_topdir}/SPECS/lrose-pkg-files
find %{prefix} -type d > %{_topdir}/SPECS/lrose-pkg-files
find %{prefix} -type l >> %{_topdir}/SPECS/lrose-pkg-files

# The build has already been completed
# we just need to install the files into place

%install
mkdir -p %{buildroot}%{prefix}
rsync -av %{prefix}/* %{buildroot}%{prefix}

%files -f %{_topdir}/SPECS/lrose-pkg-files



