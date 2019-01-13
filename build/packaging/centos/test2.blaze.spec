%define build_timestamp %(date +"%Y%m%d")
%define _topdir     /root/rpmbuild
%define name        lrose 
%define release     %{build_timestamp}
%define version     blaze 
%define buildroot %{_topdir}/%{name}-%{version}-%{release}-root
 
BuildRoot:      %{_topdir}/installedhere
Summary:        LROSE
License:        BSD LICENSE
Name:           %{name}
Version:        %{version}
Release:        %{release}
#Source:         
# %{name}-%{version}-%{release}.src.tgz
Prefix:         /usr/local/lrose
Group:          Scientific Tools
AutoReqProv:    no
 
%description
LROSE - Lidar Radar Open Software Environment
 
#%prep
#%setup 
# -q -n lrose-blaze-20180516.src
 
%build
#git clone https://github.com/NCAR/lrose-core
#./lrose-core/build/checkout_and_build_auto.py  --package=lrose-blaze 
/root/git/lrose-core/build/checkout_and_build_auto.py --package lrose-blaze --prefix %{prefix}
rm -f %{_topdir}/SPECS/lrose-pkg-files
# find /usr/local/lrose -type d | sed 's/usr/duck/'
#find /root/lrose -type d | sed 's/root/usr\/local/' > %{_topdir}/SPECS/lrose-pkg-files
#find /root/lrose -type l | sed 's/root/usr\/local/' >> %{_topdir}/SPECS/lrose-pkg-files
find %{prefix} -type d > %{_topdir}/SPECS/lrose-pkg-files
find %{prefix} -type l >> %{_topdir}/SPECS/lrose-pkg-files

%install
mkdir -p %{buildroot}%{prefix}
rsync -av %{prefix}/* %{buildroot}%{prefix}

%files -f %{_topdir}/SPECS/lrose-pkg-files
