%define build_timestamp  %(date +"%Y%m%d")
%define topDir           /root/rpmbuild
%define name             lrose-blaze
%define release          %{build_timestamp}
%define version          1
%define buildroot        %{topDir}/%{name}-%{release}-root
 
BuildRoot:      %{topDir}/install
Summary:        blaze release of lrose-core
License:        BSD
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source:         %{name}-%{release}.tgz
Prefix:         /usr
Group:          Scientific Tools
AutoReqProv:    no
URL:            https://github.com/NCAR/lrose-core
 
BuildRequires:  gcc
BuildRequires:  g++
BuildRequires:  make

%description
LROSE-CORE - The Lidar Radar Open Software Environment
Blaze release (2018)
NCAR/EOL and CSU/AtmosScience
 
%prep
%setup -q
 
%build
%configure
make -j 8 %{?_smp_mflags}

#rm -f %{topDir}/SPECS/lrose-pkg-files
#find %{topDir} -type d | sed 's/root/usr\/local/' > %{topDir}/SPECS/lrose-pkg-files
#find %{topDir} -type l | sed 's/root/usr\/local/' >> %{topDir}/SPECS/lrose-pkg-files

%install
%make_install

%files
%license LICENSE.txt
%{_bindir}/RadxPrint
%{_bindir}/RadxConvert
%{_bindir}/Radx2Grid
%{_bindir}/HawkEye

#mkdir -p %{buildroot}/usr/local/lrose
#rsync -ra /root/lrose %{buildroot}/usr/local

#%files -f %{topDir}/SPECS/lrose-pkg-files
