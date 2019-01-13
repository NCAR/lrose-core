%define build_timestamp  %(date +"%Y%m%d")
%define topDir           /root/rpmbuild
%define name             lrose-blaze
%define release          %{build_timestamp}
%define version          1.0
%define buildroot        %{topDir}/%{name}-%{release}-root
 
BuildRoot:      %{topDir}/install
Summary:        LROSE
License:        BSD
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source:         %{name}-%{release}.tgz
Prefix:         /usr
Group:          Scientific Tools
AutoReqProv:    no
 
%description
LROSE - The Lidar Radar Open Software Environment Core
 
%prep
%setup -q
 
%build
%configure
make -j 8

#rm -f %{topDir}/SPECS/lrose-pkg-files
#find %{topDir} -type d | sed 's/root/usr\/local/' > %{topDir}/SPECS/lrose-pkg-files
#find %{topDir} -type l | sed 's/root/usr\/local/' >> %{topDir}/SPECS/lrose-pkg-files

%install
make install

#mkdir -p %{buildroot}/usr/local/lrose
#rsync -ra /root/lrose %{buildroot}/usr/local

#%files -f %{topDir}/SPECS/lrose-pkg-files
