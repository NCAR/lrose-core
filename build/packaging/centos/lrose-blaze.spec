%define build_timestamp  %(date +"%Y%m%d")
%define topDir           /root/build
%define name             lrose
%define release          %{build_timestamp}
%define version          blaze 
%define buildroot        %{topDir}/%{name}-%{version}-%{release}-root
 
BuildRoot:      %{topDir}/installedhere
Summary:        LROSE
License:        BSD
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source:         %{name}-%{version}-%{release}.tgz
Prefix:         /usr/local/lrose
Group:          Scientific Tools
AutoReqProv:    no
 
%description
LROSE - The Lidar Radar Open Software Environment Core
 
%prep
%setup -q
 
%build
%configure
%make_install

rm -f %{topDir}/SPECS/lrose-pkg-files
find /root/lrose -type d | sed 's/root/usr\/local/' > %{topDir}/SPECS/lrose-pkg-files
find /root/lrose -type l | sed 's/root/usr\/local/' >> %{topDir}/SPECS/lrose-pkg-files

%install
mkdir -p %{buildroot}/usr/local/lrose
rsync -ra /root/lrose %{buildroot}/usr/local

%files -f %{topDir}/SPECS/lrose-pkg-files
