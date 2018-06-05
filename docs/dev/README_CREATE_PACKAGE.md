# Packaging LROSE with RPM

Use Docker images and RPM to package LROSE software for release.

1. Build a Docker container with the packages needed to build LROSE

in RPM/first_try ...
$ docker build --rm -t "centos-rpmbuilder" .

```
[eol-albireo:~/RPM/first_try/centos-rpmbuild] brenda% more Dockerfile

2. Package LROSE using RPM and Docker container.

#
# start with an image that contains all the packages needed to 
# build lrose-blaze + rpmbuilder
#
FROM centos-rpmbuilder

ADD ??? /home/rpmbuilder 

# create user rpmbuilder
RUN useradd -ms /bin/bash rpmbuilder 
USER rpmbuilder
WORKDIR /home/rpmbuilder

# build hierarchy for rpmbuild:
RUN cd ~
RUN mkdir lrose_blaze
RUN cd lrose_blaze 
RUN mkdir BUILD RPMS SOURCES SPECS SRPMS
RUN cd SOURCES
# ------- not tested ------
RUN git clone https://github.com/NCAR/lrose-core   <---- need to install git
#
RUN ./build/create_src_release.py --package=lrose-blaze
RUN mv ~/releases/tmp/lrose-blaze-20180516.src.tgz ~/SOURCES/.
RUN cd ..
# ------ end not tested --------
#
RUN rpmbuild -v -bb --clean SPECS/lrose-blaze.spec
```

#### The spec file for rpmbuild ...

```
[rpmbuilder@0d7c04aff58b ~]$ cat  SPECS/lrose-blaze.spec 
%define _topdir     /home/rpmbuilder
%define name        lrose 
%define release     blaze
%define version     20180516 
%define buildroot %{_topdir}/%{name}-%{release}-%{version}-root
 
BuildRoot:  %{buildroot}
Summary:        LROSE
License:        BSD LICENSE
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source:         %{name}-%{release}-%{version}.src.tgz
Prefix:         /usr/local/lrose
Group:          Scientific Tools
 
%description
LROSE - Lidar Radar Open Software Environment
 
%prep
%setup -q -n lrose-blaze-20180516.src

%build
sudo ./build_src_release.py
 
%install
 
%files
%defattr(-,root,root)
```
This causes an error when creating the /usr/local/lrose directory when building.  Is this because the command builds AND installs the software?
Do I need to separate the build from the install?  With a separate install, then the issue of access is avoided until the package is installed
via rpm?

Maybe, try build_src_release.py with --prefix=/some/alternate/location
then for %install, use move command??
--prefix /home/rpmbuilder/mytemploc  seems to work
Yep, that worked.  

The command is ...
```
~/RPM/first_try/centos-rpmbuild] brenda% docker build --rm -t "mytest_rpm" .
```

3. Now, how to test the package??

I need to extract the package from the container ...  <---
docker run --rm -it -v ~/RPM/first_try/centos-rpmbuild:/tmp/out mytest_rpm
then ...
cp RPMS/x86_64/lrose-blaze-20180516.x86_64.rpm /tmp/out/.

The naming is off ... rpmbuild makes ... lrose-20180516-blaze.x86_64.rpm, so I need to switch the version and release names <--- DONE

Test by installing into a clean container??
Nothing happens.  I think there needs to be something in the %install part of the spec file.
Oh, I also think I need to add the files in /home/rpmbuilder/mytemploc into the %files section of the spec file.

$ docker run --rm -it -v ~/RPM/first_try/centos-rpmbuild:/tmp/in centos
[]# rpm -i lrose-blaze-20180516.x86_64.rpm 

old version ... 
```
[eol-albireo:~/RPM/first_try/centos-rpmbuild] brenda% more lrose-blaze.spec
%define _topdir     /home/rpmbuilder
%define name        lrose 
%define release     blaze
%define version     yyyymmdd 
%define buildroot %{_topdir}/%{name}-%{version}-root
 
BuildRoot:  %{buildroot}
Summary:        LROSE
License:        BSD LICENSE
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source:         %{name}-%{version}.tar.gz
Prefix:         /usr/local/lrose
Group:          Scientific Tools
 
%description
LROSE - Lidar Radar Open Software Environment
 
%prep
%setup -q
 
%build
./lrose-blaze-20180516.src/build_src_release.py
 
%install
make install prefix=$RPM_BUILD_ROOT/usr
 
%files          <---  Looks in %{buildroot} for the files listed here
%defattr(-,root,root)
/usr/local/bin/wget
 
%doc %attr(0444,root,root) /usr/local/share/man/man1/wget.1
```

4. Verify the rpm contents ..

Where are the files from the build?
If root is the user building the rpm, then the files are installed in /usr/local/lrose (How to get these into the RPM?)
If a user is building the rpm, then the files are installed in the --prefix location. (How to get these into the RPM?)
The %files are relative to the %{buildroot} location. 
Q: How is the Prefix: variable used?

```
$ rpm -Vp RPMS/...
```

5. Install the RPM

```
$ sudo rpm -i lrose-blaze-20180516.x86...rpm
```


Current spec file ...
```
[rpmbuilder@d8f628101381 ~]$ cat SPECS/lrose-blaze.spec 
%define _topdir     /home/rpmbuilder
%define name        lrose 
%define release     20180516
%define version     blaze 
%define buildroot %{_topdir}/%{name}-%{version}-%{release}-root
 
BuildRoot:      %{buildroot}
Summary:        LROSE
License:        BSD LICENSE
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source:         %{name}-%{version}-%{release}.src.tgz
Prefix:         /usr/local/lrose
Group:          Scientific Tools
 
%description
LROSE - Lidar Radar Open Software Environment
 
%prep
%setup -q -n lrose-blaze-20180516.src
 
%build
./build_src_release.py --prefix=$RPM_BUILD_ROOT  <--- has the value rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64
 
%install

%files
/bin         <--- but this looks in 

[rpmbuilder@d8f628101381 ~]$ rpmbuild --quiet -bl  SPECS/lrose-blaze.spec
error: Could not open %files file /home/rpmbuilder/BUILD/lrose-blaze-20180516.src/debugfiles.list: No such file or directory
    Could not open %files file /home/rpmbuilder/BUILD/lrose-blaze-20180516.src/debugfiles.list: No such file or directory

But, then running as root, I get this error ...
ierror: File not found: /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local/lrose
    File not found: /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local/lrose


```
### just use rpmbuild -bl to check the list of files to include
### also, use -bp for %prep only; -bc for %build only; -bi for %install only

### After the %build step ...
[rpmbuilder@d8f628101381 ~]$ ls -R  rpmbuild 
rpmbuild:
BUILD  BUILDROOT  RPMS  SOURCES  SPECS  SRPMS

rpmbuild/BUILD:

rpmbuild/BUILDROOT:
lrose-blaze-20180516.x86_64

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64:
bin  include  lib  share

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/bin:
HawkEye      RadxPrint  h52gif  h5copy   h5dump    h5jam    h5perf_serial  h5repart  nc-config  ncgen         nf-config  udunits2
Radx2Grid    TdrpTest   h5c++   h5debug  h5fc      h5ls     h5redeploy     h5stat    nccopy     ncgen3        tdrp_gen
RadxConvert  gif2h5     h5cc    h5diff   h5import  h5mkgrp  h5repack       h5unjam   ncdump     ncxx4-config  tdrp_test

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include:
Fmq             H5FaccProp.h     H5f90i.h               h5lt.mod                          ncType.h
H5ACpublic.h    H5FcreatProp.h   H5f90i_gen.h           h5o.mod                           ncUbyte.h
H5AbstractDs.h  H5File.h         H5overflow.h           h5o_provisional.mod               ncUint.h
H5Apublic.h     H5FloatType.h    H5pubconf.h            h5p.mod                           ncUint64.h
H5ArrayType.h   H5Fpublic.h      H5public.h             h5p_provisional.mod               ncUshort.h
H5AtomType.h    H5Gpublic.h      H5version.h            h5r.mod                           ncVar.h
H5Attribute.h   H5Group.h        Mdv                    h5r_provisional.mod               ncVarAtt.h
H5Classes.h     H5IMpublic.h     Ncxx                   h5s.mod                           ncVlenType.h
H5CommonFG.h    H5IdComponent.h  Radx                   h5t.mod                           ncvalues.h
H5CompType.h    H5Include.h      Spdb                   h5t_provisional.mod               netcdf
H5Cpp.h         H5IntType.h      converter.h            h5tb.mod                          netcdf.h
H5CppDoc.h      H5Ipublic.h      dataport               h5test_kind_storage_size_mod.mod  netcdf.hh
H5Cpublic.h     H5LTpublic.h     didss                  h5z.mod                           netcdf.inc
H5DOpublic.h    H5Library.h      dsdata                 hdf5.h                            netcdf.mod
H5DSpublic.h    H5Location.h     dsserver               hdf5.mod                          netcdf4_f03.mod
H5DataSet.h     H5Lpublic.h      euclid                 hdf5_hl.h                         netcdf4_nc_interfaces.mod
H5DataSpace.h   H5MMpublic.h     h5_dble_interface.mod  kd                                netcdf4_nf_interfaces.mod
H5DataType.h    H5Object.h       h5a.mod                ncAtt.h                           netcdf_f03.mod
H5DcreatProp.h  H5OcreatProp.h   h5a_provisional.mod    ncByte.h                          netcdf_fortv2_c_interfaces.mod
H5Dpublic.h     H5Opublic.h      h5d.mod                ncChar.h                          netcdf_mem.h
H5DxferProp.h   H5PLextern.h     h5d_provisional.mod    ncCheck.h                         netcdf_meta.h
H5EnumType.h    H5PLpublic.h     h5ds.mod               ncCompoundType.h                  netcdf_nc_data.mod
H5Epubgen.h     H5PTpublic.h     h5e.mod                ncDim.h                           netcdf_nc_interfaces.mod
H5Epublic.h     H5PacketTable.h  h5e_provisional.mod    ncDouble.h                        netcdf_nf_data.mod
H5Exception.h   H5Ppublic.h      h5f.mod                ncEnumType.h                      netcdf_nf_interfaces.mod
H5FDcore.h      H5PredType.h     h5f_provisional.mod    ncException.h                     netcdfcpp.h
H5FDdirect.h    H5PropList.h     h5fortran_types.mod    ncFile.h                          physics
H5FDfamily.h    H5Rpublic.h      h5g.mod                ncFloat.h                         radar
H5FDlog.h       H5Spublic.h      h5global.mod           ncGroup.h                         rapformats
H5FDmpi.h       H5StrType.h      h5i.mod                ncGroupAtt.h                      rapmath
H5FDmpio.h      H5TBpublic.h     h5im.mod               ncInt.h                           tdrp
H5FDmulti.h     H5Tpublic.h      h5l.mod                ncInt64.h                         toolsa
H5FDpublic.h    H5VarLenType.h   h5l_provisional.mod    ncOpaqueType.h                    typesizes.mod
H5FDsec2.h      H5Zpublic.h      h5lib.mod              ncShort.h                         udunits.h
H5FDstdio.h     H5api_adpt.h     h5lib_provisional.mod  ncString.h                        udunits2.h

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/Fmq:
DrawQueue.hh  DsFmqMsg.hh      Fmq.hh        FmqDeviceFile.hh   GenericQueue.hh    NowcastQueue.hh
DsFmq.hh      DsRadarQueue.hh  FmqDevice.hh  FmqDeviceShmem.hh  NowcastProcess.hh  RemoteUIQueue.hh

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/Mdv:
DsMdvx.hh          GenericRadxFile.hh  MdvxField.hh      MdvxRemapLut.hh    Mdvx_BE.hh         Mdvx_timelist.hh  Ncf2MdvTrans.hh
DsMdvxInput.hh     Mdv2NcfTrans.hh     MdvxFieldCode.hh  MdvxStdAtmos.hh    Mdvx_constants.hh  Mdvx_typedefs.hh  NcfFieldData.hh
DsMdvxMsg.hh       MdvRadxFile.hh      MdvxGrid.hh       MdvxTimeList.hh    Mdvx_enums.hh      Mdvx_vsect.hh     NcfGridInfo.hh
DsMdvxThreaded.hh  Mdvx.hh             MdvxPjg.hh        MdvxTimeStamp.hh   Mdvx_ncf.hh        Mdvx_write.hh     NcfMdv.hh
DsMdvxTimes.hh     MdvxChunk.hh        MdvxProj.hh       MdvxUrlWatcher.hh  Mdvx_print.hh      Mdvx_xml.hh       NcfMdvx.hh
GenPolyGrid.hh     MdvxContour.hh      MdvxRadar.hh      MdvxVsectLut.hh    Mdvx_read.hh       Ncf2MdvField.hh   NcfVlevelInfo.hh

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/Ncxx:
ByteOrder.hh  Ncxx.hh       NcxxCompoundType.hh  NcxxException.hh  NcxxInt.hh         NcxxString.hh  NcxxUshort.hh
Hdf5xx.hh     NcxxAtt.hh    NcxxDim.hh           NcxxFile.hh       NcxxInt64.hh       NcxxType.hh    NcxxVar.hh
Nc3File.hh    NcxxByte.hh   NcxxDouble.hh        NcxxFloat.hh      NcxxOpaqueType.hh  NcxxUbyte.hh   NcxxVarAtt.hh
Nc3Values.hh  NcxxChar.hh   NcxxEnumType.hh      NcxxGroup.hh      NcxxPort.hh        NcxxUint.hh    NcxxVlenType.hh
Nc3xFile.hh   NcxxCheck.hh  NcxxErrStr.hh        NcxxGroupAtt.hh   NcxxShort.hh       NcxxUint64.hh  Udunits2.hh

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/Radx:
BufrFile.hh           GemRadxFile.hh        NexradVcp.hh         RadxCfactors.hh   RadxRay.hh        SigmetData.hh
BufrProduct.hh        Hdf5Utils.hh          NidsData.hh          RadxComplex.hh    RadxRcalib.hh     SigmetRadxFile.hh
BufrRadxFile.hh       HrdData.hh            NidsRadxFile.hh      RadxEvent.hh      RadxReadDir.hh    TableMap.hh
ByteOrder.hh          HrdRadxFile.hh        NoaaFslRadxFile.hh   RadxField.hh      RadxRemap.hh      TableMapElement.hh
Cf2RadxFile.hh        LeoRadxFile.hh        NoxpNcRadxFile.hh    RadxFile.hh       RadxStr.hh        TableMapKey.hh
CfarrNcRadxFile.hh    NcfRadxFile.hh        NsslMrdRadxFile.hh   RadxFuzzy2d.hh    RadxSweep.hh      TdwrLoc.hh
D3rNcRadxFile.hh      NcxxRadxFile.hh       OdimHdf5RadxFile.hh  RadxFuzzyF.hh     RadxTime.hh       TdwrRadxFile.hh
DoeNcRadxFile.hh      NetcdfClassic.hh      PseudoRhi.hh         RadxGeoref.hh     RadxTimeList.hh   TwolfRadxFile.hh
DoradeData.hh         NetcdfCxxUtils.hh     Radx.hh              RadxMsg.hh        RadxVol.hh        Udunits2.hh
DoradeRadxFile.hh     NexradCmdRadxFile.hh  RadxAngleHist.hh     RadxPacking.hh    RadxXml.hh        UfData.hh
EdgeNcRadxFile.hh     NexradData.hh         RadxArray.hh         RadxPath.hh       RapicRadxFile.hh  UfRadxFile.hh
ForayNcRadxFile.hh    NexradLoc.hh          RadxAzElev.hh        RadxPlatform.hh   RayxData.hh
GamicHdf5RadxFile.hh  NexradRadxFile.hh     RadxBuf.hh           RadxRangeGeom.hh  RayxMapping.hh

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/Spdb:
ConvRegionHazard.hh     DsSpdbThreaded.hh          Product_defines.hh  Symprod.hh               WxHazardBuffer.hh
ConvRegionHazardExt.hh  DsSymprodServer.hh         Sounding.hh         SymprodObj.hh            WxHazardFactory.hh
DsSpdb.hh               FltRoute.hh                SoundingGet.hh      Symprod_typedefs.hh      sounding.h
DsSpdbInput.hh          LtgSpdbBuffer.hh           SoundingPut.hh      ThresholdBiasMapping.hh
DsSpdbMsg.hh            MultiThreshBiasMapping.hh  Spdb.hh             WayPoint.hh
DsSpdbServer.hh         PosnRpt.hh                 Spdb_typedefs.hh    WxHazard.hh

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/dataport:
bigend.h  port_types.h  smallend.h  swap.h

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/didss:
DataFileNames.hh  DsDataFile.hh   DsMsgPart.hh   DsUrlQueue.hh  RapDataDir.hh    ds_message.h
DsAccess.hh       DsInputPath.hh  DsTimeList.hh  IPAddress.hh   RapDataDir_r.hh  ds_msg_handle.h
DsAccessFile.hh   DsMessage.hh    DsURL.hh       LdataInfo.hh   ds_input_path.h  rap_data_dir.h

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/dsdata:
AcarsDataPoint.hh         DsInputDirTrigger.hh   DsSpecificFcstLdataTrigger.hh        DsUrlTriggerSubsample.hh
DsDirListTrigger.hh       DsInputPathTrigger.hh  DsSpecificGenLdataTrigger.hh         MdvTimeListHandler.hh
DsEnsembleAnyTrigger.hh   DsIntervalTrigger.hh   DsTimeListTrigger.hh                 SpdbTimeListHandler.hh
DsEnsembleDataTrigger.hh  DsLdataIntTrigger.hh   DsTrigger.hh                         TimeListHandler.hh
DsEnsembleGenTrigger.hh   DsLdataTrigger.hh      DsUrlTrigger.hh                      TriggerInfo.hh
DsEnsembleLeadTrigger.hh  DsMultFcstTrigger.hh   DsUrlTriggerArchive.hh               Tstorm.hh
DsFcstTime.hh             DsMultTrigElem.hh      DsUrlTriggerObject.hh                TstormGrid.hh
DsFcstTimeListTrigger.hh  DsMultipleTrigger.hh   DsUrlTriggerObjectDerived.hh         TstormGroup.hh
DsFileListTrigger.hh      DsOneFileTrigger.hh    DsUrlTriggerObjectVirtualMethods.hh  TstormMgr.hh
DsFmqTrigger.hh           DsOneTimeTrigger.hh    DsUrlTriggerRealtime.hh

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/dsserver:
DestUrlArray.hh  DsClient.hh       DsFileIo.hh     DsLdataMsg.hh        DsProcessServer.hh  DsSvrMgrSocket.hh    ProcessServer.hh
DmapAccess.hh    DsFileCopy.hh     DsFileIoMsg.hh  DsLdataServerMsg.hh  DsServer.hh         DsThreadedClient.hh
DmapMessage.hh   DsFileCopyMsg.hh  DsLdataInfo.hh  DsLocator.hh         DsServerMsg.hh      DsThreadedServer.hh

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/euclid:
AttributesEuclid.hh             Grid2dEdgeBuilder.hh  LineList.hh              PjgObliqueStereoMath.hh  SimpleGrid.hh
Box.hh                          Grid2dInside.hh       MotionVector.hh          PjgPolarRadarCalc.hh     SunPosn.hh
CenteredRectangularTemplate.hh  Grid2dLoop.hh         Pjg.hh                   PjgPolarRadarMath.hh     TypeGrid.hh
CircularTemplate.hh             Grid2dLoopA.hh        PjgAlbersMath.hh         PjgPolarStereoCalc.hh    WorldPoint2D.hh
CircularTemplateList.hh         Grid2dLoopAlg.hh      PjgAzimEquidistMath.hh   PjgPolarStereoMath.hh    WorldPolygon2D.hh
ConvexHull.hh                   Grid2dMedian.hh       PjgCalc.hh               PjgTransMercatorMath.hh  alloc.h
DPbasic.hh                      Grid2dOffset.hh       PjgFlatCalc.hh           PjgTypes.hh              boundary.h
DataAtt.hh                      Grid2dPolyFinder.hh   PjgGrib.hh               PjgVertPerspMath.hh      clump.h
DistPoint.hh                    GridAlgs.hh           PjgLambertAzimMath.hh    Point.hh                 copyright.h
EllipticalTemplate.hh           GridExpand.hh         PjgLambertConfMath.hh    PointList.hh             distance.h
EndPts.hh                       GridGeom.hh           PjgLatlonCalc.hh         Polyline.hh              euclid_macros.h
FuzzyFcn.hh                     GridOffset.hh         PjgLatlonMath.hh         ProjFlat.hh              geometry.h
GlobalCircularTemplate.hh       GridPoint.hh          PjgLc1Calc.hh            ProjLatlon.hh            link.h
GlobalGridTemplate.hh           GridTemplate.hh       PjgLc2Calc.hh            ProjRUC2Lambert.hh       node.h
Grid.hh                         GridTemplateList.hh   PjgMath.hh               ProjType.hh              point.h
Grid2d.hh                       Handedness.hh         PjgMercatorCalc.hh       Projection.hh            scan.h
Grid2dClump.hh                  IndexPoint.hh         PjgMercatorMath.hh       RectangularTemplate.hh   search.h
Grid2dDistToNonMissing.hh       Line.hh               PjgObliqueStereoCalc.hh  Rotate3d.hh              sincos.h

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/kd:
datatype.hh  fileoper.hh  kd.hh  kd_interp.hh  kd_query.hh  metric.hh  naive.hh  tokenize.hh

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/physics:
AdiabatTempLookupTable.hh  DtfMwave.hh      PhysicsLib.hh  _PhysicsLibImplementation.hh  physics.h         stn_pressure.h  vil.h
CapeCinSounding.hh         IcaoStdAtmos.hh  ZxRelation.hh  density.h                     physics_macros.h  thermo.h

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/radar:
AlternatingVelocity.hh  InterestMap.hh    KdpCompute.hh      PrecipRate.hh     RadxAppParams.hh     iwrf_rsm.h
AtmosAtten.hh           IntfLocator.hh    KdpFilt.hh         RadarComplex.hh   RadxAppTemplate.hh   iwrf_user_interface.h
BeamHeight.hh           IwrfCalib.hh      MomentsFields.hh   RadarFft.hh       RegressionFilter.hh  rsm_functions.hh
ClutFilter.hh           IwrfMomReader.hh  NcarParticleId.hh  RadarMoments.hh   SeaClutter.hh        spol_angles.hh
ConvStrat.hh            IwrfMoments.hh    NoiseLocator.hh    RadarTsInfo.hh    Sz864.hh             syscon_to_spol.h
DpolFilter.hh           IwrfTsBurst.hh    PhaseCoding.hh     RadarTsPulse.hh   TempProfile.hh
FilterUtils.hh          IwrfTsInfo.hh     PhidpFilt.hh       RadarTsReader.hh  chill_to_iwrf.hh
FindSurfaceVel.hh       IwrfTsPulse.hh    PhidpProc.hh       RadxApp.hh        chill_types.h
GateData.hh             IwrfTsReader.hh   PidImapManager.hh  RadxAppArgs.hh    iwrf_data.h
HsrlRawRay.hh           KdpBringi.hh      PidInterestMap.hh  RadxAppConfig.hh  iwrf_functions.hh

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/rapformats:
Amdar.hh                 HydroStation.hh                TiledMultiThresh.hh  clutter_table.h     nids_file.h
Bdry.hh                  KavLtgFile.hh                  UfRadar.hh           coord_export.h      nids_sat.h
BdryPoint.hh             KavMosaic.hh                   UfRecord.hh          dobson.h            nws_oso_local.h
BdryPointShearInfo.hh    LtgGroup.hh                    UsgsData.hh          ds_radar.h          pirep.h
BdryPolyline.hh          LtgWrapper.hh                  VerGridRegion.hh     ds_radar_calib.h    pirepXml.hh
Cedric.hh                Map.hh                         Windshear.hh         ds_radar_ts.h       polar2mdv_lookup.h
ChecktimeReport.hh       MapIcon.hh                     WindshearAlpha.hh    flt_path.h          prims.h
ComboPt.hh               MapIconDef.hh                  WindshearAlphas.hh   fos.h               rData.hh
DsBeamData.hh            MapIconPoint.hh                WindshearArena.hh    gailtg.h            r_data.h
DsBeamDataFieldParms.hh  MapObject.hh                   WinsRadar.hh         gate_data.h         radar_scan_table.h
DsFieldParams.hh         MapPoint.hh                    WxObs.hh             gint_user.h         ridds.h
DsPlatformGeoref.hh      MapPointOffset.hh              WxObsField.hh        hist_fore.h         rp7.h
DsRadarAz.hh             MapPolyline.hh                 WxTypeField.hh       iidabin.h           rvp8_ts_api.h
DsRadarBeam.hh           MapSimpleLabel.hh              ZVis.hh              iwrf_time_series.h  sounding_chunk.h
DsRadarCalib.hh          MitLtg.hh                      ZvisCal.hh           kav_grid.h          station_file.h
DsRadarElev.hh           MultBuf.hh                     ZvisFcast.hh         kavltg.h            station_reports.h
DsRadarFlags.hh          MultBufPart.hh                 acPosVector.hh       kavouras_data.h     swap.h
DsRadarMsg.hh            MultiThresh.hh                 ac_data.h            kavouras_io.h       tdwr_prims.h
DsRadarParams.hh         MultiThreshFcstBiasMapping.hh  ac_georef.hh         km.h                titan_grid.h
DsRadarPower.hh          MultiThreshItem.hh             ac_posn.h            lass.h              trec_gauge.h
DsRadarSweep.hh          MultiThresholdsBiasMapping.hh  ac_route.h           lincoln.h           tstorm_hull_smooth.h
Edr.hh                   NWS_WWA.hh                     acars.h              ltg.h               tstorm_spdb.h
Edr_expanded.hh          Pirep.hh                       acarsXml.hh          mcars.h             twnltg.h
FieldThresh.hh           RadarSpectra.hh                alenia.h             mcidas_area.h       uf_headers.h
FieldThresh2.hh          SigAirMet.hh                   ascii_shapeio.h      mdvt_data.h         v_data.h
FieldThresh2WithGrid.hh  Sndg.hh                        bdry.h               mdvt_user.h         var_elev.h
GaiLtgFile.hh            SsiFile.h                      bdry_extrap.h        metar.h             zr.h
GenPoly.hh               StationData.h                  bdry_typedefs.h      metar_decode.h      zrpf.h
GenPolyStats.hh          Taf.hh                         bprp.h               mitre.h
GenPt.hh                 TaiwanAwos.hh                  ccm_file.h           moments.h
GenPtArray.hh            TileInfo.hh                    cedric.h             nexrad.h
GldnLtgFile.hh           TileRange.hh                   chill.h              nexrad_com_hdrs.h

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/rapmath:
AngleCombiner.hh  LagrangePtFunc.hh  OrderedList.hh          RMmalloc.h         StepPtFunc.hh  round.h  usort.h
Fuzzy2d.hh        LinearPtFunc.hh    ParamsFuzzyFunction.hh  RapComplex.hh      TrapFuzzyF.hh  stats.h  usort_stl.h
FuzzyF.hh         Math.hh            PtFunction.hh           SFuzzyF.hh         bd_tree.h      trig.h   usvd.h
FuzzyFunction.hh  NewtonPtFunc.hh    PtFuzzyFunc.hh          ScaleFuzzyFunc.hh  math_macros.h  umath.h

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/tdrp:
tdrp.h  tdrp_obsolete.h  tdrp_p.h  tdrpbuf.h

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/include/toolsa:
ArchiveDates.hh     Log.hh               SimpleQueue.h             Tcp_Exchange.hh  globals.h           servmap.h
Attributes.hh       LogMsg.hh            SockUtil.hh               ThreadSocket.hh  gridLineConnect.hh  sincos.h
Benchmark.hh        LogMsgInit.hh        Socket.hh                 Tty.hh           heartbeat.h         smu.h
ComplexNumber.hh    LogMsgStreamInit.hh  TaArray.hh                URL.hh           http.h              sockutil.h
DataScaler.hh       LogStream.hh         TaArray2D.hh              ansi.h           ldata_info.h        sok2.h
Data_Row.hh         LogStreamInit.hh     TaFile.hh                 blockbuf.h       ldmFileStrobe.hh    str.h
DateTime.hh         MemBuf.hh            TaPjg.hh                  cal.hh           mb.h                tclRegexp.h
DeltaTime.hh        MsgLog.hh            TaStr.hh                  compress.h       mbq.h               toolsa.h
Except.hh           Path.hh              TaThread.hh               copyright.h      mem.h               toolsa_macros.h
GetHost.hh          PmuInfo.hh           TaThreadDoubleQue.hh      datamap.h        membuf.h            ttape.h
HostCache.hh        ReadDir.hh           TaThreadLog.hh            db_access.h      os_config.h         ucopyright.h
HttpConnection.hh   Semaphore.hh         TaThreadPollingQue.hh     dlm.h            pjg.h               udatetime.h
HttpSocket.hh       Server.hh            TaThreadPool.hh           err.h            pjg_flat.h          ugetenv.hh
HttpURL.hh          ServerSocket.hh      TaThreadQue.hh            file_io.h        pjg_types.h         umisc.h
InputDir.hh         Shmem.hh             TaThreadSimple.hh         fmq.h            pmu.h               ushmem.h
InputDirRecurse.hh  ShmemSem.hh          TaThreadSimplePolling.hh  fmq_private.h    port.h              utim.h
IpCache.hh          SignBit.hh           TaTriggerLog.hh           font.h           procmap.h           uusleep.h
KeyedList.h         SimpleList.h         TaXml.hh                  gdbm.h           reutil.h            xdru.h

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/lib:
libFmq.a          libdataport.so.0.0.0  libhdf5_cpp.so               libnetcdf.la             libradar.so.0.0.0
libFmq.la         libdidss.a            libhdf5_cpp.so.13            libnetcdf.settings       librapformats.a
libFmq.so         libdidss.la           libhdf5_cpp.so.13.0.0        libnetcdf.so             librapformats.la
libFmq.so.0       libdidss.so           libhdf5_fortran.a            libnetcdf.so.11          librapformats.so
libFmq.so.0.0.0   libdidss.so.0         libhdf5_fortran.la           libnetcdf.so.11.0.4      librapformats.so.0
libMdv.a          libdidss.so.0.0.0     libhdf5_fortran.so           libnetcdf_c++.a          librapformats.so.0.0.0
libMdv.la         libdsdata.a           libhdf5_fortran.so.10        libnetcdf_c++.la         librapmath.a
libMdv.so         libdsdata.la          libhdf5_fortran.so.10.0.4    libnetcdf_c++.so         librapmath.la
libMdv.so.0       libdsdata.so          libhdf5_hl.a                 libnetcdf_c++.so.4       librapmath.so
libMdv.so.0.0.0   libdsdata.so.0        libhdf5_hl.la                libnetcdf_c++.so.4.2.0   librapmath.so.0
libNcxx.a         libdsdata.so.0.0.0    libhdf5_hl.so                libnetcdf_c++4.a         librapmath.so.0.0.0
libNcxx.la        libdsserver.a         libhdf5_hl.so.10             libnetcdf_c++4.la        libtdrp.a
libNcxx.so        libdsserver.la        libhdf5_hl.so.10.1.1         libnetcdf_c++4.so        libtdrp.la
libNcxx.so.0      libdsserver.so        libhdf5_hl_cpp.a             libnetcdf_c++4.so.1      libtdrp.so
libNcxx.so.0.0.0  libdsserver.so.0      libhdf5_hl_cpp.la            libnetcdf_c++4.so.1.0.3  libtdrp.so.0
libRadx.a         libdsserver.so.0.0.0  libhdf5_hl_cpp.so            libnetcdff.a             libtdrp.so.0.0.0
libRadx.la        libeuclid.a           libhdf5_hl_cpp.so.11         libnetcdff.la            libtoolsa.a
libRadx.so        libeuclid.la          libhdf5_hl_cpp.so.11.1.0     libnetcdff.so            libtoolsa.la
libRadx.so.0      libeuclid.so          libhdf5hl_fortran.a          libnetcdff.so.6          libtoolsa.so
libRadx.so.0.0.0  libeuclid.so.0        libhdf5hl_fortran.la         libnetcdff.so.6.1.1      libtoolsa.so.0
libSpdb.a         libeuclid.so.0.0.0    libhdf5hl_fortran.so         libphysics.a             libtoolsa.so.0.0.0
libSpdb.la        libhdf5.a             libhdf5hl_fortran.so.10      libphysics.la            libudunits2.a
libSpdb.so        libhdf5.la            libhdf5hl_fortran.so.10.0.3  libphysics.so            libudunits2.la
libSpdb.so.0      libhdf5.settings      libkd.a                      libphysics.so.0          libudunits2.so
libSpdb.so.0.0.0  libhdf5.so            libkd.la                     libphysics.so.0.0.0      libudunits2.so.0
libdataport.a     libhdf5.so.10         libkd.so                     libradar.a               libudunits2.so.0.1.0
libdataport.la    libhdf5.so.10.2.1     libkd.so.0                   libradar.la              pkgconfig
libdataport.so    libhdf5_cpp.a         libkd.so.0.0.0               libradar.so
libdataport.so.0  libhdf5_cpp.la        libnetcdf.a                  libradar.so.0

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/lib/pkgconfig:
netcdf-cxx4.pc  netcdf-fortran.pc  netcdf.pc

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share:
doc  hdf5_examples  info  man  udunits

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/doc:
udunits

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/doc/udunits:
CHANGE_LOG  README                 udunits2-base.xml    udunits2-derived.xml   udunits2.xml
COPYRIGHT   udunits2-accepted.xml  udunits2-common.xml  udunits2-prefixes.xml

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/hdf5_examples:
README  c  c++  fortran  hl  run-all-ex.sh

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/hdf5_examples/c:
h5_attribute.c   h5_crtatt.c    h5_crtgrpd.c         h5_extend_write.c  h5_rdwt.c       h5_select.c       ph5example.c
h5_chunk_read.c  h5_crtdat.c    h5_drivers.c         h5_extlink.c       h5_read.c       h5_shared_mesg.c  run-c-ex.sh
h5_cmprss.c      h5_crtgrp.c    h5_elink_unix2win.c  h5_group.c         h5_ref2reg.c    h5_subset.c
h5_compound.c    h5_crtgrpar.c  h5_extend.c          h5_mount.c         h5_reference.c  h5_write.c

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/hdf5_examples/c++:
chunks.cpp    extend_ds.cpp      h5tutr_crtatt.cpp  h5tutr_crtgrpar.cpp  h5tutr_rdwt.cpp    run-c++-ex.sh
compound.cpp  h5group.cpp        h5tutr_crtdat.cpp  h5tutr_crtgrpd.cpp   h5tutr_subset.cpp  writedata.cpp
create.cpp    h5tutr_cmprss.cpp  h5tutr_crtgrp.cpp  h5tutr_extend.cpp    readdata.cpp

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/hdf5_examples/fortran:
compound.f90   h5_crtdat.f90    h5_crtgrpd.f90  h5_subset.f90     ph5example.f90     run-fortran-ex.sh
h5_cmprss.f90  h5_crtgrp.f90    h5_extend.f90   hyperslab.f90     refobjexample.f90  selectele.f90
h5_crtatt.f90  h5_crtgrpar.f90  h5_rdwt.f90     mountexample.f90  refregexample.f90

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/hdf5_examples/hl:
c  c++  fortran  run-hl-ex.sh

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/hdf5_examples/hl/c:
ex_ds1.c     ex_lite1.c  ex_table_01.c  ex_table_04.c  ex_table_07.c  ex_table_10.c  image24pixel.txt  ptExampleFL.c
ex_image1.c  ex_lite2.c  ex_table_02.c  ex_table_05.c  ex_table_08.c  ex_table_11.c  image8.txt        run-hlc-ex.sh
ex_image2.c  ex_lite3.c  ex_table_03.c  ex_table_06.c  ex_table_09.c  ex_table_12.c  pal_rgb.h

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/hdf5_examples/hl/c++:
ptExampleFL.cpp  run-hlc++-ex.sh

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/hdf5_examples/hl/fortran:
ex_ds1.f90  exlite.f90  run-hlfortran-ex.sh

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/info:
dir  netcdf-cxx.info  udunits2.info  udunits2lib.info  udunits2prog.info

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/man:
man1  man3

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/man/man1:
nccopy.1  ncdump.1  ncgen.1  ncgen3.1

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/man/man3:
netcdf.3  netcdf_fortran.3

rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/share/udunits:
udunits2-accepted.xml  udunits2-base.xml  udunits2-common.xml  udunits2-derived.xml  udunits2-prefixes.xml  udunits2.xml

rpmbuild/RPMS:

rpmbuild/SOURCES:

rpmbuild/SPECS:

rpmbuild/SRPMS:
[rpmbuilder@d8f628101381 ~]$ 

## Ok, make a small test case and figure out the %files thing. 

1. move /lrose to BUILDROOT area
doesn't work.  the

Ok, this worked ...
```
[root@68b36e518715 rpmbuilder]# more SPECS/mytest.spec 
%define _topdir     /home/rpmbuilder
%define name        lrose 
%define release     20180516
%define version     blaze 
%define buildroot %{_topdir}/%{name}-%{version}-%{release}-root
 
BuildRoot:      %{_topdir}/installedhere
Summary:        LROSE
License:        BSD LICENSE
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source:         %{name}-%{version}-%{release}.src.tgz
Prefix:         /usr/local/lrose
Group:          Scientific Tools
 
%description
LROSE - Lidar Radar Open Software Environment
 
%prep
%setup -q -n lrose-blaze-20180516.src
 
%build
echo "plane flying overhead" > fileToInstall
 
%install
mv fileToInstall /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/.

%files
/fileToInstall
```
----------

Executing(%install): /bin/sh -e /var/tmp/rpm-tmp.b2fDuB
+ umask 022
+ cd /home/rpmbuilder/BUILD
+ '[' /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64 '!=' / ']'
+ rm -rf /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64
++ dirname                             /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64
+ mkdir -p                             /root/rpmbuild/BUILDROOT
+ mkdir                                /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64
+ cd lrose-blaze-20180516.src
+ mkdir -p                             /root/rpmbuild/BUILDROOT/lrose-blaze-20180515.x86_64/usr/local
+ mv /usr/local/lrose                  /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local
mv: cannot move '/usr/local/lrose' to '/root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local': No such file or directory
error: Bad exit status from /var/tmp/rpm-tmp.b2fDuB (%install)


RPM build errors:
    Bad exit status from /var/tmp/rpm-tmp.b2fDuB (%install)
[root@68b36e518715 rpmbuilder]# rpmbuild -v -bl SPECS/lrose-blaze.spec 
Processing files: lrose-blaze-20180516.x86_64
error: File not found: /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local/lrose


RPM build errors:
    File not found: /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local/lrose
[root@68b36e518715 rpmbuilder]# 
 ----------------


[root@68b36e518715 rpmbuilder]# cat SPECS/lrose-blaze.spec 
%define _topdir     /home/rpmbuilder
%define name        lrose 
%define release     20180516
%define version     blaze 
%define buildroot %{_topdir}/%{name}-%{version}-%{release}-root
 
BuildRoot:      %{_topdir}/installedhere
Summary:        LROSE
License:        BSD LICENSE
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source:         %{name}-%{version}-%{release}.src.tgz
Prefix:         /usr/local/lrose
Group:          Scientific Tools
 
%description
LROSE - Lidar Radar Open Software Environment
 
%prep
%setup -q -n lrose-blaze-20180516.src
 
%build
./build_src_release.py 
mkdir -p /root/rpmbuild/BUILDROOT/lrose-blaze-20180515.x86_64/usr/local
mv /usr/local/lrose /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local
 
%install

%files
/usr/local/lrose

----------

   39  ls
   40  ls RPMS
   41  rpmbuild -v -bl SPECS/lrose-blaze.spec 
   42  rpm -Vp RPMS
   43  rpmbuild -v -bb SPECS/lrose-blaze.spec 
   44  rpm -Vp RPMS/x86_64/lrose-blaze-20180516.x86_64.rpm 
   45  vi SPECS/lrose-blaze.spec 
   46  cp SPECS/lrose-blaze.spec mytest.spec
   47  vi mytest.spec
   48  rpmbuild -v -bb SPECS/mytest.spec
   49  ls SPECS
   50  ls
   51  mv mytest.spec SPECS
   52  rpmbuild -v -bb SPECS/mytest.spec
   53  vi SPECS/mytest.spec
   54  rpmbuild -v -bb SPECS/mytest.spec
   55  ls RPMS/
   56  ls RPMS/x86_64/lrose-blaze-20180516.x86_64.rpm 
   57  ls RPMS/x86_64/lrose-blaze-20180516.x86_64.rpm  -lrt
   58  rpm -Vp RPMS/x86_64/lrose-blaze-20180516.x86_64.rpm 
   59  rpm -qa RPMS/x86_64/lrose-blaze-20180516.x86_64.rpm 
   60  ls /file*
   61  rpm -i RPMS/x86_64/lrose-blaze-20180516.x86_64.rpm 
   62  ls /file*
   63  more /fileToInstall 
   64  more SPECS/mytest.spec 
   65  vi SPECS/lrose-blaze.spec 
   66  rpmbuild -v -bb SPECS/lrose-blaze.spec 
   67  vi SPECS/lrose-blaze.spec 
   68  rpmbuild -v -bb SPECS/lrose-blaze.spec 
   69  vi SPECS/lrose-blaze.spec 
   70  rpmbuild -v -bi SPECS/lrose-blaze.spec 
   71  rpmbuild -v -bb SPECS/lrose-blaze.spec 
   72  rpmbuild -v -bl SPECS/lrose-blaze.spec 
   73  ls /usr/local/lrose
   74  ls /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/
   75  vi SPECS/lrose-blaze.spec 
   76  cat SPECS/lrose-blaze.spec 
   77  rpmbuild -v -bb SPECS/lrose-blaze.spec 
   78  ls /usr/local/lrose
   79  ls -R /usr/local/lrose
   80  ls -AR1 /usr/local/lrose
   81* ls -findAR1 /usr/local/lrose
   82  find /usr/local/lrose -type f
   83  cp SPECS/lrose-blaze.spec SPECS/lrose-blaze.spec.save
   84  find /usr/local/lrose -type f >> SPECS/lrose-blaze.spec
   85  vi SPECS/lrose-blaze.spec
   86  rpmbuild -v -bb SPECS/lrose-blaze.spec 
   87  ls /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local/lrose
   88  ls /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64
   89  ls /root/rpmbuild/BUILDROOT
   90  ls -lrt/root/rpmbuild/BUILDROOT/
   91  ls -lrt /root/rpmbuild/BUILDROOT/
   92  ls -lrt /root/rpmbuild/BUILDROOT/lrose-blaze-20180515.x86_64/
   93  history

----------

cat SPECS/lrose-blaze.spec 
%define _topdir     /home/rpmbuilder
%define name        lrose 
%define release     20180516
%define version     blaze 
%define buildroot %{_topdir}/%{name}-%{version}-%{release}-root
 
BuildRoot:      %{_topdir}/installedhere
Summary:        LROSE
License:        BSD LICENSE
URL:		https://nsf-lrose.github.io/
Requires:	fftw3
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source:         %{name}-%{version}-%{release}.src.tgz
Prefix:         /usr/local/lrose
Group:          Scientific Tools
 
%description
LROSE - Lidar Radar Open Software Environment
 
%prep
%setup -q -n lrose-blaze-20180516.src
 
%build

%install
mkdir -p %{buildroot}/usr/local
./build_src_release.py --prefix=%{buildroot}/usr/local/lrose

## try to remove the offending directories -- didn't work
rm -rf %{buildroot}/usr/lib
rm -rf %{buildroot}/usr/src


%files
/usr/local/lrose

## try to list each file to include -- didn't work; still included the offending debug files

## try to list the offending files with an '%exclude' macro -- didn't work; still included them. GRHH!

## try to list only the file name ...
%exclude lrose-blaze-20180516.x86_64

-------

./build_src_release.py --prefix=/root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local/lrose
 
%install
rm -rf /home/rpmbuilder/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/lib
rm -rt /home/rpmbuilder/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/src

%check

%files

%changelog

------

with files listed in %files,
=> if build_src_release is in %build, then when processing %file, the lrose-blaze-2018-516.x86_64 file is found and rpmbuild terminates
/root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local/lrose/lib/librapformats.la:libdir='/root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local/lrose/lib'
Binary file /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/lib/debug/usr/local/lrose/lib/libnetcdf_c++4.so.1.0.3.debug matches
Binary file /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/lib/debug/usr/local/lrose/lib/libnetcdf_c++.so.4.2.0.debug matches
Binary file /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/lib/debug/.dwz/lrose-blaze-20180516.x86_64 matches 
Found '/root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64' in installed files; aborting <--- HERE IS THE PROBLEM; The path is embedded in a file somewhere. 
error: Bad exit status from /var/tmp/rpm-tmp.fp7OdY (%install)

## try without prefix, then move /usr/local/lrose to ${buildroot} ?? I'll need to build rpm as root so the path is
mkdir -p            /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local
rsync /usr/local/lrose /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local


=> if build_src_release is in %install, then when processing %file, none of the files are found ...
    File not found: /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local/lrose/include/h5d_provisional.mod


---------

6/4/2018

Spec file for root:
[root@1a93e23f402f bj]# cat SPECS/lrose-blaze.spec 
%define _topdir     /tmp/bj
%define name        lrose 
%define release     20180516
%define version     blaze 
%define buildroot %{_topdir}/%{name}-%{version}-%{release}-root
 
BuildRoot:      %{_topdir}/installedhere  <---- this is useless
Summary:        LROSE
License:        BSD LICENSE
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source:         %{name}-%{version}-%{release}.src.tgz
Prefix:         /usr/local/lrose
Group:          Scientific Tools
 
%description
LROSE - Lidar Radar Open Software Environment
 
%prep
%setup -q -n lrose-blaze-20180516.src
 
%build
 
%install
./build_src_release.py
mkdir -p %{buildroot}/usr/local/lrose
# rsync /usr/local/lrose %{buildroot}/usr/local/lrose
  rsync -r usr/local/lrose %{buildroot}/usr/local

%files
/usr/local/lrose
----- 
result:

+ mkdir -p /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local/lrose
+ rsync -r /usr/local/lrose /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local
skipping non-regular file "lrose/lib/libFmq.so"
skipping non-regular file "lrose/lib/libFmq.so.0"
skipping non-regular file "lrose/lib/libMdv.so"
skipping non-regular file "lrose/lib/libMdv.so.0"
skipping non-regular file "lrose/lib/libNcxx.so"
skipping non-regular file "lrose/lib/libNcxx.so.0"
skipping non-regular file "lrose/lib/libRadx.so"
skipping non-regular file "lrose/lib/libRadx.so.0"
skipping non-regular file "lrose/lib/libSpdb.so"
skipping non-regular file "lrose/lib/libSpdb.so.0"
skipping non-regular file "lrose/lib/libdataport.so"
skipping non-regular file "lrose/lib/libdataport.so.0"
skipping non-regular file "lrose/lib/libdidss.so"
skipping non-regular file "lrose/lib/libdidss.so.0"
skipping non-regular file "lrose/lib/libdsdata.so"
skipping non-regular file "lrose/lib/libdsdata.so.0"
skipping non-regular file "lrose/lib/libdsserver.so"
skipping non-regular file "lrose/lib/libdsserver.so.0"
skipping non-regular file "lrose/lib/libeuclid.so"
skipping non-regular file "lrose/lib/libeuclid.so.0"
skipping non-regular file "lrose/lib/libhdf5.so"
skipping non-regular file "lrose/lib/libhdf5.so.10"
skipping non-regular file "lrose/lib/libhdf5_cpp.so"
skipping non-regular file "lrose/lib/libhdf5_cpp.so.13"
skipping non-regular file "lrose/lib/libhdf5_fortran.so"
skipping non-regular file "lrose/lib/libhdf5_fortran.so.10"
skipping non-regular file "lrose/lib/libhdf5_hl.so"
skipping non-regular file "lrose/lib/libhdf5_hl.so.10"
skipping non-regular file "lrose/lib/libhdf5_hl_cpp.so"
skipping non-regular file "lrose/lib/libhdf5_hl_cpp.so.11"
skipping non-regular file "lrose/lib/libhdf5hl_fortran.so"
skipping non-regular file "lrose/lib/libhdf5hl_fortran.so.10"
skipping non-regular file "lrose/lib/libkd.so"
skipping non-regular file "lrose/lib/libkd.so.0"
skipping non-regular file "lrose/lib/libnetcdf.so"
skipping non-regular file "lrose/lib/libnetcdf.so.11"
skipping non-regular file "lrose/lib/libnetcdf_c++.so"
skipping non-regular file "lrose/lib/libnetcdf_c++.so.4"
skipping non-regular file "lrose/lib/libnetcdf_c++4.so"
skipping non-regular file "lrose/lib/libnetcdf_c++4.so.1"
skipping non-regular file "lrose/lib/libnetcdff.so"
skipping non-regular file "lrose/lib/libnetcdff.so.6"
skipping non-regular file "lrose/lib/libphysics.so"
skipping non-regular file "lrose/lib/libphysics.so.0"
skipping non-regular file "lrose/lib/libradar.so"
skipping non-regular file "lrose/lib/libradar.so.0"
skipping non-regular file "lrose/lib/librapformats.so"
skipping non-regular file "lrose/lib/librapformats.so.0"
skipping non-regular file "lrose/lib/librapmath.so"
skipping non-regular file "lrose/lib/librapmath.so.0"
skipping non-regular file "lrose/lib/libtdrp.so"
skipping non-regular file "lrose/lib/libtdrp.so.0"
skipping non-regular file "lrose/lib/libtoolsa.so"
skipping non-regular file "lrose/lib/libtoolsa.so.0"
skipping non-regular file "lrose/lib/libudunits2.so"
skipping non-regular file "lrose/lib/libudunits2.so.0"
skipping non-regular file "lrose/share/doc/udunits/udunits2-accepted.xml"
skipping non-regular file "lrose/share/doc/udunits/udunits2-base.xml"
skipping non-regular file "lrose/share/doc/udunits/udunits2-common.xml"
skipping non-regular file "lrose/share/doc/udunits/udunits2-derived.xml"
skipping non-regular file "lrose/share/doc/udunits/udunits2-prefixes.xml"
skipping non-regular file "lrose/share/doc/udunits/udunits2.xml"
+ /usr/lib/rpm/find-debuginfo.sh --strict-build-id -m --run-dwz --dwz-low-mem-die-limit 10000000 --dwz-max-die-limit 110000000 /tmp/bj/BUILD/lrose-blaze-20180516.src
extracting debug info from /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local/lrose/bin/h5repart
Dest dir longer than base dir is not supported
error: Bad exit status from /var/tmp/rpm-tmp.W4WyQ1 (%install)


RPM build errors:
    Bad exit status from /var/tmp/rpm-tmp.W4WyQ1 (%instal
------------

with this spec file ...
%define _topdir     /tmp/bj
%define name        lrose
%define release     20180516
%define version     blaze
%define buildroot %{_topdir}/%{name}-%{version}-%{release}-root

BuildRoot:      %{_topdir}/installedhere
Summary:        LROSE
License:        BSD LICENSE
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source:         %{name}-%{version}-%{release}.src.tgz
Prefix:         /usr/local/lrose
Group:          Scientific Tools

%description
LROSE - Lidar Radar Open Software Environment

%prep
%setup -q -n lrose-blaze-20180516.src

%build
./build_src_release.py

%install
mkdir -p %{buildroot}/usr/local/lrose
rsync -r /usr/local/lrose %{buildroot}/usr/local

%files
/usr/local/lrose
-----------
result:

skipping non-regular file "lrose/share/doc/udunits/udunits2.xml"
+ /usr/lib/rpm/find-debuginfo.sh --strict-build-id -m --run-dwz --dwz-low-mem-die-limit 10000000 --dwz-max-die-limit 110000000 /tmp/bj/BUILD/lrose-blaze-20180516.src
extracting debug info from /root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64/usr/local/lrose/bin/h5repart
Dest dir longer than base dir is not supported
error: Bad exit status from /var/tmp/rpm-tmp.zZ8FYr (%install)
------

ok, try adding the list of files to the spec file ...

-------
result:

same error as above.
Let's investigate a bit ...

+ echo buildroot=/root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64
buildroot=/root/rpmbuild/BUILDROOT/lrose-blaze-20180516.x86_64
-----

this version of the spec file worked ...
%define _topdir     /tmp/bj
%define name        lrose 
%define release     20180516
%define version     blaze 
%define buildroot %{_topdir}/%{name}-%{version}-%{release}-root
 
BuildRoot:      %{_topdir}/installedhere
Summary:        LROSE
License:        BSD LICENSE
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source:         %{name}-%{version}-%{release}.src.tgz
Prefix:         /usr/local/lrose
Group:          Scientific Tools
 
%description
LROSE - Lidar Radar Open Software Environment
 
%prep
%setup -q -n lrose-blaze-20180516.src
 
%build
./build_src_release.py
rm -f %{_topdir}/SPECS/lrose-pkg-files
find /usr/local/lrose -type d > %{_topdir}/SPECS/lrose-pkg-files
find /usr/local/lrose -type l >> %{_topdir}/SPECS/lrose-pkg-files

%install
mkdir -p %{buildroot}/usr/local/lrose
rsync -ra /usr/local/lrose %{buildroot}/usr/local

%files -f %{_topdir}/SPECS/lrose-pkg-files
...
----------

rpm build command:

rpmbuild -v -bb --define "debug_package %{nil}" SPECS/lrose-blaze.spec  <<<---- the debug_package option did the trick!

check build:
rpm -Vp RPMS/x86_64/lrose-blaze-20180516.x86_64.rpm 

rpm install command:
rpm -i lrose-blaze-20180516.x86_64.rpm
rpm install; ignoring conflicts
rpm -i --force 


-------
this works; just needed to add -a to rsync to include symbolic links

more SPECS/lrose-blaze.spec 
%define _topdir     /tmp/bj
%define name        lrose 
%define release     20180516
%define version     blaze 
%define buildroot %{_topdir}/%{name}-%{version}-%{release}-root
 
BuildRoot:      %{_topdir}/installedhere
Summary:        LROSE
License:        BSD LICENSE
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source:         %{name}-%{version}-%{release}.src.tgz
Prefix:         /usr/local/lrose
Group:          Scientific Tools
 
%description
LROSE - Lidar Radar Open Software Environment
 
%prep
%setup -q -n lrose-blaze-20180516.src
 
%build
./build_src_release.py
rm -f %{_topdir}/SPECS/lrose-pkg-files
find /usr/local/lrose -type d > %{_topdir}/SPECS/lrose-pkg-files
find /usr/local/lrose -type l >> %{_topdir}/SPECS/lrose-pkg-files

%install
mkdir -p %{buildroot}/usr/local/lrose
rsync -ra /usr/local/lrose %{buildroot}/usr/local

%files -f %{_topdir}/SPECS/lrose-pkg-files

-----------

Q: does just /usr/local/lrose work? Yes
or do I need all the files listed? No

%files

/usr/local/lrose/bin/h5repart
/usr/local/lrose/bin/TdrpTest
/usr/local/lrose/bin/HawkEye
/usr/local/lrose/bin/h5debug
/usr/local/lrose/bin/ncxx4-config
/usr/local/lrose/bin/udunits2
/usr/local/lrose/bin/h52gif
/usr/local/lrose/bin/h5c++
/usr/local/lrose/bin/h5unjam
/usr/local/lrose/bin/h5fc
/usr/local/lrose/bin/ncgen
/usr/local/lrose/bin/gif2h5
/usr/local/lrose/bin/nc-config
/usr/local/lrose/bin/tdrp_test
/usr/local/lrose/bin/nf-config
/usr/local/lrose/bin/h5ls
/usr/local/lrose/bin/ncgen3
/usr/local/lrose/bin/h5dump
/usr/local/lrose/bin/h5diff
/usr/local/lrose/bin/h5repack
/usr/local/lrose/bin/h5copy
/usr/local/lrose/bin/h5stat
/usr/local/lrose/bin/ncdump
/usr/local/lrose/bin/tdrp_gen
/usr/local/lrose/bin/h5import
/usr/local/lrose/bin/h5perf_serial
/usr/local/lrose/bin/h5cc
/usr/local/lrose/bin/h5mkgrp
/usr/local/lrose/bin/Radx2Grid
/usr/local/lrose/bin/RadxConvert
/usr/local/lrose/bin/h5redeploy
/usr/local/lrose/bin/h5jam
/usr/local/lrose/bin/nccopy
/usr/local/lrose/bin/RadxPrint
/usr/local/lrose/share/man/man1/ncgen.1
/usr/local/lrose/share/man/man1/nccopy.1
/usr/local/lrose/share/man/man1/ncdump.1
/usr/local/lrose/share/man/man1/ncgen3.1
/usr/local/lrose/share/man/man3/netcdf.3
/usr/local/lrose/share/man/man3/netcdf_fortran.3
/usr/local/lrose/share/doc/udunits/COPYRIGHT
/usr/local/lrose/share/doc/udunits/README
/usr/local/lrose/share/doc/udunits/CHANGE_LOG
/usr/local/lrose/share/udunits/udunits2-derived.xml
/usr/local/lrose/share/udunits/udunits2-common.xml
/usr/local/lrose/share/udunits/udunits2-prefixes.xml
/usr/local/lrose/share/udunits/udunits2.xml
/usr/local/lrose/share/udunits/udunits2-accepted.xml
/usr/local/lrose/share/udunits/udunits2-base.xml
/usr/local/lrose/share/hdf5_examples/c++/readdata.cpp
/usr/local/lrose/share/hdf5_examples/c++/h5tutr_cmprss.cpp
/usr/local/lrose/share/hdf5_examples/c++/create.cpp
/usr/local/lrose/share/hdf5_examples/c++/h5tutr_crtgrp.cpp
/usr/local/lrose/share/hdf5_examples/c++/writedata.cpp
/usr/local/lrose/share/hdf5_examples/c++/h5tutr_crtdat.cpp
/usr/local/lrose/share/hdf5_examples/c++/h5tutr_crtgrpd.cpp
/usr/local/lrose/share/hdf5_examples/c++/h5tutr_rdwt.cpp
/usr/local/lrose/share/hdf5_examples/c++/chunks.cpp
/usr/local/lrose/share/hdf5_examples/c++/h5group.cpp
/usr/local/lrose/share/hdf5_examples/c++/h5tutr_crtgrpar.cpp
/usr/local/lrose/share/hdf5_examples/c++/compound.cpp
/usr/local/lrose/share/hdf5_examples/c++/run-c++-ex.sh
/usr/local/lrose/share/hdf5_examples/c++/extend_ds.cpp
/usr/local/lrose/share/hdf5_examples/c++/h5tutr_extend.cpp
/usr/local/lrose/share/hdf5_examples/c++/h5tutr_crtatt.cpp
/usr/local/lrose/share/hdf5_examples/c++/h5tutr_subset.cpp
/usr/local/lrose/share/hdf5_examples/fortran/h5_subset.f90
/usr/local/lrose/share/hdf5_examples/fortran/ph5example.f90
/usr/local/lrose/share/hdf5_examples/fortran/h5_crtgrpd.f90
/usr/local/lrose/share/hdf5_examples/fortran/h5_crtatt.f90
/usr/local/lrose/share/hdf5_examples/fortran/mountexample.f90
/usr/local/lrose/share/hdf5_examples/fortran/selectele.f90
/usr/local/lrose/share/hdf5_examples/fortran/hyperslab.f90
/usr/local/lrose/share/hdf5_examples/fortran/h5_crtgrpar.f90
/usr/local/lrose/share/hdf5_examples/fortran/h5_rdwt.f90
/usr/local/lrose/share/hdf5_examples/fortran/refobjexample.f90
/usr/local/lrose/share/hdf5_examples/fortran/h5_crtgrp.f90
/usr/local/lrose/share/hdf5_examples/fortran/h5_crtdat.f90
/usr/local/lrose/share/hdf5_examples/fortran/h5_extend.f90
/usr/local/lrose/share/hdf5_examples/fortran/refregexample.f90
/usr/local/lrose/share/hdf5_examples/fortran/h5_cmprss.f90
/usr/local/lrose/share/hdf5_examples/fortran/compound.f90
/usr/local/lrose/share/hdf5_examples/fortran/run-fortran-ex.sh
/usr/local/lrose/share/hdf5_examples/README
/usr/local/lrose/share/hdf5_examples/c/h5_group.c
/usr/local/lrose/share/hdf5_examples/c/h5_shared_mesg.c
/usr/local/lrose/share/hdf5_examples/c/h5_crtgrpar.c
/usr/local/lrose/share/hdf5_examples/c/h5_extend_write.c
/usr/local/lrose/share/hdf5_examples/c/h5_crtatt.c
/usr/local/lrose/share/hdf5_examples/c/ph5example.c
/usr/local/lrose/share/hdf5_examples/c/h5_drivers.c
/usr/local/lrose/share/hdf5_examples/c/h5_crtdat.c
/usr/local/lrose/share/hdf5_examples/c/h5_subset.c
/usr/local/lrose/share/hdf5_examples/c/h5_write.c
/usr/local/lrose/share/hdf5_examples/c/h5_cmprss.c
/usr/local/lrose/share/hdf5_examples/c/h5_mount.c
/usr/local/lrose/share/hdf5_examples/c/run-c-ex.sh
/usr/local/lrose/share/hdf5_examples/c/h5_extend.c
/usr/local/lrose/share/hdf5_examples/c/h5_select.c
/usr/local/lrose/share/hdf5_examples/c/h5_crtgrpd.c
/usr/local/lrose/share/hdf5_examples/c/h5_attribute.c
/usr/local/lrose/share/hdf5_examples/c/h5_rdwt.c
/usr/local/lrose/share/hdf5_examples/c/h5_read.c
/usr/local/lrose/share/hdf5_examples/c/h5_crtgrp.c
/usr/local/lrose/share/hdf5_examples/c/h5_ref2reg.c
/usr/local/lrose/share/hdf5_examples/c/h5_elink_unix2win.c
/usr/local/lrose/share/hdf5_examples/c/h5_extlink.c
/usr/local/lrose/share/hdf5_examples/c/h5_reference.c
/usr/local/lrose/share/hdf5_examples/c/h5_chunk_read.c
/usr/local/lrose/share/hdf5_examples/c/h5_compound.c
/usr/local/lrose/share/hdf5_examples/run-all-ex.sh
/usr/local/lrose/share/hdf5_examples/hl/c++/run-hlc++-ex.sh
/usr/local/lrose/share/hdf5_examples/hl/c++/ptExampleFL.cpp
/usr/local/lrose/share/hdf5_examples/hl/fortran/ex_ds1.f90
/usr/local/lrose/share/hdf5_examples/hl/fortran/run-hlfortran-ex.sh
/usr/local/lrose/share/hdf5_examples/hl/fortran/exlite.f90
/usr/local/lrose/share/hdf5_examples/hl/run-hl-ex.sh
/usr/local/lrose/share/hdf5_examples/hl/c/ex_table_01.c
/usr/local/lrose/share/hdf5_examples/hl/c/ptExampleFL.c
/usr/local/lrose/share/hdf5_examples/hl/c/image8.txt
/usr/local/lrose/share/hdf5_examples/hl/c/ex_table_09.c
/usr/local/lrose/share/hdf5_examples/hl/c/ex_table_11.c
/usr/local/lrose/share/hdf5_examples/hl/c/ex_table_12.c
/usr/local/lrose/share/hdf5_examples/hl/c/ex_image1.c
/usr/local/lrose/share/hdf5_examples/hl/c/ex_table_02.c
/usr/local/lrose/share/hdf5_examples/hl/c/image24pixel.txt
/usr/local/lrose/share/hdf5_examples/hl/c/pal_rgb.h
/usr/local/lrose/share/hdf5_examples/hl/c/ex_table_05.c
/usr/local/lrose/share/hdf5_examples/hl/c/ex_table_06.c
/usr/local/lrose/share/hdf5_examples/hl/c/ex_lite3.c
/usr/local/lrose/share/hdf5_examples/hl/c/ex_ds1.c
/usr/local/lrose/share/hdf5_examples/hl/c/ex_table_03.c
/usr/local/lrose/share/hdf5_examples/hl/c/ex_image2.c
/usr/local/lrose/share/hdf5_examples/hl/c/ex_lite1.c
/usr/local/lrose/share/hdf5_examples/hl/c/ex_table_10.c
/usr/local/lrose/share/hdf5_examples/hl/c/run-hlc-ex.sh
/usr/local/lrose/share/hdf5_examples/hl/c/ex_table_07.c
/usr/local/lrose/share/hdf5_examples/hl/c/ex_table_08.c
/usr/local/lrose/share/hdf5_examples/hl/c/ex_table_04.c
/usr/local/lrose/share/hdf5_examples/hl/c/ex_lite2.c
/usr/local/lrose/share/info/dir
/usr/local/lrose/share/info/udunits2lib.info
/usr/local/lrose/share/info/netcdf-cxx.info
/usr/local/lrose/share/info/udunits2.info
/usr/local/lrose/share/info/udunits2prog.info
/usr/local/lrose/lib/libnetcdf.la
/usr/local/lrose/lib/libnetcdf.settings
/usr/local/lrose/lib/librapmath.la
/usr/local/lrose/lib/libkd.la
/usr/local/lrose/lib/libMdv.a
/usr/local/lrose/lib/libSpdb.la
/usr/local/lrose/lib/libtdrp.so.0.0.0
/usr/local/lrose/lib/libdataport.a
/usr/local/lrose/lib/libdidss.a
/usr/local/lrose/lib/libnetcdff.so.6.1.1
/usr/local/lrose/lib/libradar.so.0.0.0
/usr/local/lrose/lib/libhdf5_hl_cpp.la
/usr/local/lrose/lib/libFmq.a
/usr/local/lrose/lib/libFmq.la
/usr/local/lrose/lib/libradar.la
/usr/local/lrose/lib/libMdv.la
/usr/local/lrose/lib/libhdf5_hl.a
/usr/local/lrose/lib/libhdf5_hl_cpp.so.11.1.0
/usr/local/lrose/lib/libnetcdf_c++4.so.1.0.3
/usr/local/lrose/lib/libNcxx.so.0.0.0
/usr/local/lrose/lib/libnetcdf.a
/usr/local/lrose/lib/libRadx.so.0.0.0
/usr/local/lrose/lib/libdataport.la
/usr/local/lrose/lib/libMdv.so.0.0.0
/usr/local/lrose/lib/librapmath.so.0.0.0
/usr/local/lrose/lib/libFmq.so.0.0.0
/usr/local/lrose/lib/libnetcdf_c++.la
/usr/local/lrose/lib/libhdf5_hl_cpp.a
/usr/local/lrose/lib/libnetcdf_c++4.la
/usr/local/lrose/lib/libhdf5_fortran.a
/usr/local/lrose/lib/libnetcdf_c++4.a
/usr/local/lrose/lib/libeuclid.a
/usr/local/lrose/lib/libnetcdf_c++.so.4.2.0
/usr/local/lrose/lib/libnetcdf.so.11.0.4
/usr/local/lrose/lib/libtdrp.la
/usr/local/lrose/lib/libdsdata.la
/usr/local/lrose/lib/libhdf5hl_fortran.a
/usr/local/lrose/lib/libNcxx.a
/usr/local/lrose/lib/libhdf5_cpp.a
/usr/local/lrose/lib/libnetcdff.la
/usr/local/lrose/lib/libudunits2.la
/usr/local/lrose/lib/libRadx.la
/usr/local/lrose/lib/libdsdata.so.0.0.0
/usr/local/lrose/lib/librapmath.a
/usr/local/lrose/lib/libudunits2.so.0.1.0
/usr/local/lrose/lib/libradar.a
/usr/local/lrose/lib/libdsdata.a
/usr/local/lrose/lib/libhdf5hl_fortran.so.10.0.3
/usr/local/lrose/lib/libdidss.la
/usr/local/lrose/lib/libtoolsa.a
/usr/local/lrose/lib/librapformats.a
/usr/local/lrose/lib/libkd.a
/usr/local/lrose/lib/libNcxx.la
/usr/local/lrose/lib/libphysics.a
/usr/local/lrose/lib/libRadx.a
/usr/local/lrose/lib/libeuclid.la
/usr/local/lrose/lib/libhdf5_cpp.la
/usr/local/lrose/lib/libhdf5_hl.la
/usr/local/lrose/lib/libudunits2.a
/usr/local/lrose/lib/libSpdb.a
/usr/local/lrose/lib/libphysics.so.0.0.0
/usr/local/lrose/lib/libtoolsa.la
/usr/local/lrose/lib/libhdf5_hl.so.10.1.1
/usr/local/lrose/lib/libeuclid.so.0.0.0
/usr/local/lrose/lib/libhdf5hl_fortran.la
/usr/local/lrose/lib/librapformats.so.0.0.0
/usr/local/lrose/lib/libdsserver.so.0.0.0
/usr/local/lrose/lib/libhdf5.la
/usr/local/lrose/lib/libtoolsa.so.0.0.0
/usr/local/lrose/lib/libnetcdff.a
/usr/local/lrose/lib/libdsserver.la
/usr/local/lrose/lib/pkgconfig/netcdf-cxx4.pc
/usr/local/lrose/lib/pkgconfig/netcdf-fortran.pc
/usr/local/lrose/lib/pkgconfig/netcdf.pc
/usr/local/lrose/lib/libphysics.la
/usr/local/lrose/lib/libnetcdf_c++.a
/usr/local/lrose/lib/libhdf5.a
/usr/local/lrose/lib/libSpdb.so.0.0.0
/usr/local/lrose/lib/libhdf5.so.10.2.1
/usr/local/lrose/lib/libtdrp.a
/usr/local/lrose/lib/libhdf5.settings
/usr/local/lrose/lib/libkd.so.0.0.0
/usr/local/lrose/lib/libhdf5_cpp.so.13.0.0
/usr/local/lrose/lib/libhdf5_fortran.so.10.0.4
/usr/local/lrose/lib/libdataport.so.0.0.0
/usr/local/lrose/lib/libhdf5_fortran.la
/usr/local/lrose/lib/libdsserver.a
/usr/local/lrose/lib/libdidss.so.0.0.0
/usr/local/lrose/lib/librapformats.la
/usr/local/lrose/include/H5VarLenType.h
/usr/local/lrose/include/H5FaccProp.h
/usr/local/lrose/include/ncGroup.h
/usr/local/lrose/include/H5Group.h
/usr/local/lrose/include/H5Object.h
/usr/local/lrose/include/udunits2.h
/usr/local/lrose/include/H5PLpublic.h
/usr/local/lrose/include/netcdf4_nf_interfaces.mod
/usr/local/lrose/include/H5FDcore.h
/usr/local/lrose/include/H5Fpublic.h
/usr/local/lrose/include/H5FDmpi.h
/usr/local/lrose/include/H5CppDoc.h
/usr/local/lrose/include/h5l_provisional.mod
/usr/local/lrose/include/h5f.mod
/usr/local/lrose/include/H5FloatType.h
/usr/local/lrose/include/H5Tpublic.h
/usr/local/lrose/include/Mdv/Mdvx_xml.hh
/usr/local/lrose/include/Mdv/Mdvx_BE.hh
/usr/local/lrose/include/Mdv/Mdv2NcfTrans.hh
/usr/local/lrose/include/Mdv/MdvxFieldCode.hh
/usr/local/lrose/include/Mdv/MdvxRadar.hh
/usr/local/lrose/include/Mdv/MdvxTimeStamp.hh
/usr/local/lrose/include/Mdv/NcfVlevelInfo.hh
/usr/local/lrose/include/Mdv/GenericRadxFile.hh
/usr/local/lrose/include/Mdv/Mdvx_print.hh
/usr/local/lrose/include/Mdv/Ncf2MdvField.hh
/usr/local/lrose/include/Mdv/MdvxProj.hh
/usr/local/lrose/include/Mdv/MdvxChunk.hh
/usr/local/lrose/include/Mdv/Mdvx_constants.hh
/usr/local/lrose/include/Mdv/NcfMdvx.hh
/usr/local/lrose/include/Mdv/DsMdvxMsg.hh
/usr/local/lrose/include/Mdv/Mdvx_enums.hh
/usr/local/lrose/include/Mdv/GenPolyGrid.hh
/usr/local/lrose/include/Mdv/DsMdvxInput.hh
/usr/local/lrose/include/Mdv/MdvxGrid.hh
/usr/local/lrose/include/Mdv/MdvxPjg.hh
/usr/local/lrose/include/Mdv/Mdvx_ncf.hh
/usr/local/lrose/include/Mdv/MdvRadxFile.hh
/usr/local/lrose/include/Mdv/MdvxStdAtmos.hh
/usr/local/lrose/include/Mdv/MdvxVsectLut.hh
/usr/local/lrose/include/Mdv/DsMdvxThreaded.hh
/usr/local/lrose/include/Mdv/Mdvx_timelist.hh
/usr/local/lrose/include/Mdv/Mdvx_vsect.hh
/usr/local/lrose/include/Mdv/DsMdvxTimes.hh
/usr/local/lrose/include/Mdv/NcfFieldData.hh
/usr/local/lrose/include/Mdv/Mdvx_typedefs.hh
/usr/local/lrose/include/Mdv/Mdvx_read.hh
/usr/local/lrose/include/Mdv/Mdvx.hh
/usr/local/lrose/include/Mdv/MdvxContour.hh
/usr/local/lrose/include/Mdv/Mdvx_write.hh
/usr/local/lrose/include/Mdv/NcfMdv.hh
/usr/local/lrose/include/Mdv/MdvxRemapLut.hh
/usr/local/lrose/include/Mdv/MdvxUrlWatcher.hh
/usr/local/lrose/include/Mdv/DsMdvx.hh
/usr/local/lrose/include/Mdv/MdvxField.hh
/usr/local/lrose/include/Mdv/NcfGridInfo.hh
/usr/local/lrose/include/Mdv/Ncf2MdvTrans.hh
/usr/local/lrose/include/Mdv/MdvxTimeList.hh
/usr/local/lrose/include/typesizes.mod
/usr/local/lrose/include/euclid/RectangularTemplate.hh
/usr/local/lrose/include/euclid/PjgAlbersMath.hh
/usr/local/lrose/include/euclid/PjgPolarStereoMath.hh
/usr/local/lrose/include/euclid/ProjType.hh
/usr/local/lrose/include/euclid/PjgMercatorCalc.hh
/usr/local/lrose/include/euclid/DataAtt.hh
/usr/local/lrose/include/euclid/Grid2d.hh
/usr/local/lrose/include/euclid/Grid2dDistToNonMissing.hh
/usr/local/lrose/include/euclid/GridTemplate.hh
/usr/local/lrose/include/euclid/GridTemplateList.hh
/usr/local/lrose/include/euclid/FuzzyFcn.hh
/usr/local/lrose/include/euclid/GlobalCircularTemplate.hh
/usr/local/lrose/include/euclid/GridExpand.hh
/usr/local/lrose/include/euclid/EndPts.hh
/usr/local/lrose/include/euclid/PjgLambertConfMath.hh
/usr/local/lrose/include/euclid/GridGeom.hh
/usr/local/lrose/include/euclid/PjgLambertAzimMath.hh
/usr/local/lrose/include/euclid/GlobalGridTemplate.hh
/usr/local/lrose/include/euclid/DistPoint.hh
/usr/local/lrose/include/euclid/PointList.hh
/usr/local/lrose/include/euclid/PjgPolarRadarMath.hh
/usr/local/lrose/include/euclid/LineList.hh
/usr/local/lrose/include/euclid/GridOffset.hh
/usr/local/lrose/include/euclid/clump.h
/usr/local/lrose/include/euclid/WorldPoint2D.hh
/usr/local/lrose/include/euclid/CircularTemplate.hh
/usr/local/lrose/include/euclid/SimpleGrid.hh
/usr/local/lrose/include/euclid/EllipticalTemplate.hh
/usr/local/lrose/include/euclid/sincos.h
/usr/local/lrose/include/euclid/AttributesEuclid.hh
/usr/local/lrose/include/euclid/PjgLatlonMath.hh
/usr/local/lrose/include/euclid/Grid2dEdgeBuilder.hh
/usr/local/lrose/include/euclid/Grid2dMedian.hh
/usr/local/lrose/include/euclid/Box.hh
/usr/local/lrose/include/euclid/copyright.h
/usr/local/lrose/include/euclid/ProjFlat.hh
/usr/local/lrose/include/euclid/Rotate3d.hh
/usr/local/lrose/include/euclid/MotionVector.hh
/usr/local/lrose/include/euclid/geometry.h
/usr/local/lrose/include/euclid/Grid2dInside.hh
/usr/local/lrose/include/euclid/Point.hh
/usr/local/lrose/include/euclid/Grid2dLoopA.hh
/usr/local/lrose/include/euclid/PjgTypes.hh
/usr/local/lrose/include/euclid/PjgPolarStereoCalc.hh
/usr/local/lrose/include/euclid/PjgAzimEquidistMath.hh
/usr/local/lrose/include/euclid/GridPoint.hh
/usr/local/lrose/include/euclid/Handedness.hh
/usr/local/lrose/include/euclid/Grid2dClump.hh
/usr/local/lrose/include/euclid/CenteredRectangularTemplate.hh
/usr/local/lrose/include/euclid/Grid2dPolyFinder.hh
/usr/local/lrose/include/euclid/PjgPolarRadarCalc.hh
/usr/local/lrose/include/euclid/node.h
/usr/local/lrose/include/euclid/PjgTransMercatorMath.hh
/usr/local/lrose/include/euclid/scan.h
/usr/local/lrose/include/euclid/euclid_macros.h
/usr/local/lrose/include/euclid/point.h
/usr/local/lrose/include/euclid/DPbasic.hh
/usr/local/lrose/include/euclid/Pjg.hh
/usr/local/lrose/include/euclid/ProjRUC2Lambert.hh
/usr/local/lrose/include/euclid/PjgLatlonCalc.hh
/usr/local/lrose/include/euclid/Grid2dLoopAlg.hh
/usr/local/lrose/include/euclid/SunPosn.hh
/usr/local/lrose/include/euclid/WorldPolygon2D.hh
/usr/local/lrose/include/euclid/ConvexHull.hh
/usr/local/lrose/include/euclid/ProjLatlon.hh
/usr/local/lrose/include/euclid/distance.h
/usr/local/lrose/include/euclid/PjgFlatCalc.hh
/usr/local/lrose/include/euclid/PjgMath.hh
/usr/local/lrose/include/euclid/Line.hh
/usr/local/lrose/include/euclid/PjgCalc.hh
/usr/local/lrose/include/euclid/Polyline.hh
/usr/local/lrose/include/euclid/search.h
/usr/local/lrose/include/euclid/Grid.hh
/usr/local/lrose/include/euclid/PjgLc2Calc.hh
/usr/local/lrose/include/euclid/PjgObliqueStereoMath.hh
/usr/local/lrose/include/euclid/CircularTemplateList.hh
/usr/local/lrose/include/euclid/PjgGrib.hh
/usr/local/lrose/include/euclid/IndexPoint.hh
/usr/local/lrose/include/euclid/boundary.h
/usr/local/lrose/include/euclid/Projection.hh
/usr/local/lrose/include/euclid/link.h
/usr/local/lrose/include/euclid/alloc.h
/usr/local/lrose/include/euclid/TypeGrid.hh
/usr/local/lrose/include/euclid/Grid2dOffset.hh
/usr/local/lrose/include/euclid/PjgObliqueStereoCalc.hh
/usr/local/lrose/include/euclid/GridAlgs.hh
/usr/local/lrose/include/euclid/PjgMercatorMath.hh
/usr/local/lrose/include/euclid/PjgVertPerspMath.hh
/usr/local/lrose/include/euclid/PjgLc1Calc.hh
/usr/local/lrose/include/euclid/Grid2dLoop.hh
/usr/local/lrose/include/ncUint64.h
/usr/local/lrose/include/H5ACpublic.h
/usr/local/lrose/include/netcdf4_f03.mod
/usr/local/lrose/include/ncOpaqueType.h
/usr/local/lrose/include/H5Rpublic.h
/usr/local/lrose/include/h5test_kind_storage_size_mod.mod
/usr/local/lrose/include/H5StrType.h
/usr/local/lrose/include/ncShort.h
/usr/local/lrose/include/h5global.mod
/usr/local/lrose/include/ncDim.h
/usr/local/lrose/include/H5DataSpace.h
/usr/local/lrose/include/didss/DsTimeList.hh
/usr/local/lrose/include/didss/ds_msg_handle.h
/usr/local/lrose/include/didss/rap_data_dir.h
/usr/local/lrose/include/didss/LdataInfo.hh
/usr/local/lrose/include/didss/DsInputPath.hh
/usr/local/lrose/include/didss/DsURL.hh
/usr/local/lrose/include/didss/DsUrlQueue.hh
/usr/local/lrose/include/didss/IPAddress.hh
/usr/local/lrose/include/didss/ds_message.h
/usr/local/lrose/include/didss/DataFileNames.hh
/usr/local/lrose/include/didss/DsAccess.hh
/usr/local/lrose/include/didss/DsMsgPart.hh
/usr/local/lrose/include/didss/DsDataFile.hh
/usr/local/lrose/include/didss/RapDataDir_r.hh
/usr/local/lrose/include/didss/DsMessage.hh
/usr/local/lrose/include/didss/RapDataDir.hh
/usr/local/lrose/include/didss/ds_input_path.h
/usr/local/lrose/include/didss/DsAccessFile.hh
/usr/local/lrose/include/H5api_adpt.h
/usr/local/lrose/include/ncUshort.h
/usr/local/lrose/include/H5Exception.h
/usr/local/lrose/include/H5FDmulti.h
/usr/local/lrose/include/ncFloat.h
/usr/local/lrose/include/ncAtt.h
/usr/local/lrose/include/H5File.h
/usr/local/lrose/include/ncInt64.h
/usr/local/lrose/include/ncVlenType.h
/usr/local/lrose/include/H5AtomType.h
/usr/local/lrose/include/H5DSpublic.h
/usr/local/lrose/include/H5PropList.h
/usr/local/lrose/include/H5Cpp.h
/usr/local/lrose/include/H5IntType.h
/usr/local/lrose/include/netcdf_meta.h
/usr/local/lrose/include/H5AbstractDs.h
/usr/local/lrose/include/ncChar.h
/usr/local/lrose/include/H5FDdirect.h
/usr/local/lrose/include/H5FcreatProp.h
/usr/local/lrose/include/hdf5.mod
/usr/local/lrose/include/h5f_provisional.mod
/usr/local/lrose/include/radar/InterestMap.hh
/usr/local/lrose/include/radar/PhidpProc.hh
/usr/local/lrose/include/radar/IwrfMomReader.hh
/usr/local/lrose/include/radar/syscon_to_spol.h
/usr/local/lrose/include/radar/iwrf_data.h
/usr/local/lrose/include/radar/ClutFilter.hh
/usr/local/lrose/include/radar/iwrf_functions.hh
/usr/local/lrose/include/radar/IwrfMoments.hh
/usr/local/lrose/include/radar/rsm_functions.hh
/usr/local/lrose/include/radar/TempProfile.hh
/usr/local/lrose/include/radar/RadxAppConfig.hh
/usr/local/lrose/include/radar/SeaClutter.hh
/usr/local/lrose/include/radar/HsrlRawRay.hh
/usr/local/lrose/include/radar/IwrfTsInfo.hh
/usr/local/lrose/include/radar/PidInterestMap.hh
/usr/local/lrose/include/radar/FilterUtils.hh
/usr/local/lrose/include/radar/RadarMoments.hh
/usr/local/lrose/include/radar/RadxApp.hh
/usr/local/lrose/include/radar/IwrfTsPulse.hh
/usr/local/lrose/include/radar/AtmosAtten.hh
/usr/local/lrose/include/radar/BeamHeight.hh
/usr/local/lrose/include/radar/IntfLocator.hh
/usr/local/lrose/include/radar/chill_types.h
/usr/local/lrose/include/radar/chill_to_iwrf.hh
/usr/local/lrose/include/radar/PhaseCoding.hh
/usr/local/lrose/include/radar/RadxAppArgs.hh
/usr/local/lrose/include/radar/PhidpFilt.hh
/usr/local/lrose/include/radar/RadarComplex.hh
/usr/local/lrose/include/radar/RadarTsPulse.hh
/usr/local/lrose/include/radar/PidImapManager.hh
/usr/local/lrose/include/radar/NoiseLocator.hh
/usr/local/lrose/include/radar/RadxAppParams.hh
/usr/local/lrose/include/radar/RadxAppTemplate.hh
/usr/local/lrose/include/radar/RadarTsReader.hh
/usr/local/lrose/include/radar/IwrfCalib.hh
/usr/local/lrose/include/radar/MomentsFields.hh
/usr/local/lrose/include/radar/KdpBringi.hh
/usr/local/lrose/include/radar/PrecipRate.hh
/usr/local/lrose/include/radar/iwrf_rsm.h
/usr/local/lrose/include/radar/Sz864.hh
/usr/local/lrose/include/radar/RegressionFilter.hh
/usr/local/lrose/include/radar/KdpCompute.hh
/usr/local/lrose/include/radar/IwrfTsReader.hh
/usr/local/lrose/include/radar/RadarTsInfo.hh
/usr/local/lrose/include/radar/ConvStrat.hh
/usr/local/lrose/include/radar/GateData.hh
/usr/local/lrose/include/radar/FindSurfaceVel.hh
/usr/local/lrose/include/radar/iwrf_user_interface.h
/usr/local/lrose/include/radar/NcarParticleId.hh
/usr/local/lrose/include/radar/RadarFft.hh
/usr/local/lrose/include/radar/AlternatingVelocity.hh
/usr/local/lrose/include/radar/DpolFilter.hh
/usr/local/lrose/include/radar/IwrfTsBurst.hh
/usr/local/lrose/include/radar/KdpFilt.hh
/usr/local/lrose/include/radar/spol_angles.hh
/usr/local/lrose/include/H5DxferProp.h
/usr/local/lrose/include/h5im.mod
/usr/local/lrose/include/H5DataSet.h
/usr/local/lrose/include/h5l.mod
/usr/local/lrose/include/H5PTpublic.h
/usr/local/lrose/include/ncVarAtt.h
/usr/local/lrose/include/ncEnumType.h
/usr/local/lrose/include/H5Gpublic.h
/usr/local/lrose/include/h5t_provisional.mod
/usr/local/lrose/include/H5PredType.h
/usr/local/lrose/include/udunits.h
/usr/local/lrose/include/h5p.mod
/usr/local/lrose/include/netcdf_f03.mod
/usr/local/lrose/include/h5a.mod
/usr/local/lrose/include/H5FDstdio.h
/usr/local/lrose/include/netcdf_nf_data.mod
/usr/local/lrose/include/H5PLextern.h
/usr/local/lrose/include/ncInt.h
/usr/local/lrose/include/ncString.h
/usr/local/lrose/include/h5e_provisional.mod
/usr/local/lrose/include/H5Spublic.h
/usr/local/lrose/include/h5o.mod
/usr/local/lrose/include/h5lib_provisional.mod
/usr/local/lrose/include/h5r_provisional.mod
/usr/local/lrose/include/dataport/port_types.h
/usr/local/lrose/include/dataport/swap.h
/usr/local/lrose/include/dataport/bigend.h
/usr/local/lrose/include/dataport/smallend.h
/usr/local/lrose/include/tdrp/tdrp_obsolete.h
/usr/local/lrose/include/tdrp/tdrpbuf.h
/usr/local/lrose/include/tdrp/tdrp_p.h
/usr/local/lrose/include/tdrp/tdrp.h
/usr/local/lrose/include/H5Zpublic.h
/usr/local/lrose/include/H5IMpublic.h
/usr/local/lrose/include/h5a_provisional.mod
/usr/local/lrose/include/H5Location.h
/usr/local/lrose/include/H5FDpublic.h
/usr/local/lrose/include/dsserver/ProcessServer.hh
/usr/local/lrose/include/dsserver/DsThreadedClient.hh
/usr/local/lrose/include/dsserver/DsFileIo.hh
/usr/local/lrose/include/dsserver/DsThreadedServer.hh
/usr/local/lrose/include/dsserver/DmapAccess.hh
/usr/local/lrose/include/dsserver/DsLdataMsg.hh
/usr/local/lrose/include/dsserver/DsProcessServer.hh
/usr/local/lrose/include/dsserver/DsLdataInfo.hh
/usr/local/lrose/include/dsserver/DsSvrMgrSocket.hh
/usr/local/lrose/include/dsserver/DmapMessage.hh
/usr/local/lrose/include/dsserver/DsFileCopy.hh
/usr/local/lrose/include/dsserver/DsLdataServerMsg.hh
/usr/local/lrose/include/dsserver/DsServerMsg.hh
/usr/local/lrose/include/dsserver/DsClient.hh
/usr/local/lrose/include/dsserver/DsServer.hh
/usr/local/lrose/include/dsserver/DsLocator.hh
/usr/local/lrose/include/dsserver/DsFileCopyMsg.hh
/usr/local/lrose/include/dsserver/DestUrlArray.hh
/usr/local/lrose/include/dsserver/DsFileIoMsg.hh
/usr/local/lrose/include/H5Include.h
/usr/local/lrose/include/H5LTpublic.h
/usr/local/lrose/include/H5Lpublic.h
/usr/local/lrose/include/ncGroupAtt.h
/usr/local/lrose/include/netcdf_nf_interfaces.mod
/usr/local/lrose/include/netcdfcpp.h
/usr/local/lrose/include/h5o_provisional.mod
/usr/local/lrose/include/netcdf_nc_data.mod
/usr/local/lrose/include/H5Dpublic.h
/usr/local/lrose/include/H5MMpublic.h
/usr/local/lrose/include/ncCheck.h
/usr/local/lrose/include/H5IdComponent.h
/usr/local/lrose/include/toolsa/TaThreadDoubleQue.hh
/usr/local/lrose/include/toolsa/TaFile.hh
/usr/local/lrose/include/toolsa/IpCache.hh
/usr/local/lrose/include/toolsa/HttpURL.hh
/usr/local/lrose/include/toolsa/LogStreamInit.hh
/usr/local/lrose/include/toolsa/port.h
/usr/local/lrose/include/toolsa/umisc.h
/usr/local/lrose/include/toolsa/Except.hh
/usr/local/lrose/include/toolsa/xdru.h
/usr/local/lrose/include/toolsa/LogMsgStreamInit.hh
/usr/local/lrose/include/toolsa/ldata_info.h
/usr/local/lrose/include/toolsa/heartbeat.h
/usr/local/lrose/include/toolsa/Path.hh
/usr/local/lrose/include/toolsa/dlm.h
/usr/local/lrose/include/toolsa/MemBuf.hh
/usr/local/lrose/include/toolsa/pjg.h
/usr/local/lrose/include/toolsa/PmuInfo.hh
/usr/local/lrose/include/toolsa/TaStr.hh
/usr/local/lrose/include/toolsa/DeltaTime.hh
/usr/local/lrose/include/toolsa/HttpSocket.hh
/usr/local/lrose/include/toolsa/TaPjg.hh
/usr/local/lrose/include/toolsa/cal.hh
/usr/local/lrose/include/toolsa/Socket.hh
/usr/local/lrose/include/toolsa/procmap.h
/usr/local/lrose/include/toolsa/pjg_flat.h
/usr/local/lrose/include/toolsa/TaThreadPool.hh
/usr/local/lrose/include/toolsa/Data_Row.hh
/usr/local/lrose/include/toolsa/MsgLog.hh
/usr/local/lrose/include/toolsa/GetHost.hh
/usr/local/lrose/include/toolsa/Benchmark.hh
/usr/local/lrose/include/toolsa/blockbuf.h
/usr/local/lrose/include/toolsa/KeyedList.h
/usr/local/lrose/include/toolsa/sincos.h
/usr/local/lrose/include/toolsa/Tcp_Exchange.hh
/usr/local/lrose/include/toolsa/gdbm.h
/usr/local/lrose/include/toolsa/copyright.h
/usr/local/lrose/include/toolsa/udatetime.h
/usr/local/lrose/include/toolsa/compress.h
/usr/local/lrose/include/toolsa/gridLineConnect.hh
/usr/local/lrose/include/toolsa/TaThreadSimplePolling.hh
/usr/local/lrose/include/toolsa/Tty.hh
/usr/local/lrose/include/toolsa/LogStream.hh
/usr/local/lrose/include/toolsa/TaTriggerLog.hh
/usr/local/lrose/include/toolsa/pjg_types.h
/usr/local/lrose/include/toolsa/datamap.h
/usr/local/lrose/include/toolsa/TaThreadLog.hh
/usr/local/lrose/include/toolsa/TaThreadSimple.hh
/usr/local/lrose/include/toolsa/TaArray2D.hh
/usr/local/lrose/include/toolsa/DataScaler.hh
/usr/local/lrose/include/toolsa/ansi.h
/usr/local/lrose/include/toolsa/DateTime.hh
/usr/local/lrose/include/toolsa/HttpConnection.hh
/usr/local/lrose/include/toolsa/servmap.h
/usr/local/lrose/include/toolsa/ReadDir.hh
/usr/local/lrose/include/toolsa/sok2.h
/usr/local/lrose/include/toolsa/tclRegexp.h
/usr/local/lrose/include/toolsa/http.h
/usr/local/lrose/include/toolsa/Semaphore.hh
/usr/local/lrose/include/toolsa/toolsa.h
/usr/local/lrose/include/toolsa/TaThread.hh
/usr/local/lrose/include/toolsa/URL.hh
/usr/local/lrose/include/toolsa/ugetenv.hh
/usr/local/lrose/include/toolsa/TaXml.hh
/usr/local/lrose/include/toolsa/ComplexNumber.hh
/usr/local/lrose/include/toolsa/os_config.h
/usr/local/lrose/include/toolsa/toolsa_macros.h
/usr/local/lrose/include/toolsa/mbq.h
/usr/local/lrose/include/toolsa/SimpleList.h
/usr/local/lrose/include/toolsa/file_io.h
/usr/local/lrose/include/toolsa/err.h
/usr/local/lrose/include/toolsa/ShmemSem.hh
/usr/local/lrose/include/toolsa/InputDirRecurse.hh
/usr/local/lrose/include/toolsa/fmq_private.h
/usr/local/lrose/include/toolsa/ThreadSocket.hh
/usr/local/lrose/include/toolsa/font.h
/usr/local/lrose/include/toolsa/Attributes.hh
/usr/local/lrose/include/toolsa/smu.h
/usr/local/lrose/include/toolsa/LogMsg.hh
/usr/local/lrose/include/toolsa/uusleep.h
/usr/local/lrose/include/toolsa/Log.hh
/usr/local/lrose/include/toolsa/membuf.h
/usr/local/lrose/include/toolsa/ArchiveDates.hh
/usr/local/lrose/include/toolsa/Server.hh
/usr/local/lrose/include/toolsa/globals.h
/usr/local/lrose/include/toolsa/SignBit.hh
/usr/local/lrose/include/toolsa/LogMsgInit.hh
/usr/local/lrose/include/toolsa/SockUtil.hh
/usr/local/lrose/include/toolsa/reutil.h
/usr/local/lrose/include/toolsa/fmq.h
/usr/local/lrose/include/toolsa/mem.h
/usr/local/lrose/include/toolsa/TaArray.hh
/usr/local/lrose/include/toolsa/ushmem.h
/usr/local/lrose/include/toolsa/ServerSocket.hh
/usr/local/lrose/include/toolsa/utim.h
/usr/local/lrose/include/toolsa/TaThreadPollingQue.hh
/usr/local/lrose/include/toolsa/SimpleQueue.h
/usr/local/lrose/include/toolsa/sockutil.h
/usr/local/lrose/include/toolsa/db_access.h
/usr/local/lrose/include/toolsa/ucopyright.h
/usr/local/lrose/include/toolsa/str.h
/usr/local/lrose/include/toolsa/HostCache.hh
/usr/local/lrose/include/toolsa/pmu.h
/usr/local/lrose/include/toolsa/ttape.h
/usr/local/lrose/include/toolsa/TaThreadQue.hh
/usr/local/lrose/include/toolsa/mb.h
/usr/local/lrose/include/toolsa/Shmem.hh
/usr/local/lrose/include/toolsa/ldmFileStrobe.hh
/usr/local/lrose/include/toolsa/InputDir.hh
/usr/local/lrose/include/H5Attribute.h
/usr/local/lrose/include/H5Apublic.h
/usr/local/lrose/include/H5f90i.h
/usr/local/lrose/include/H5TBpublic.h
/usr/local/lrose/include/ncUint.h
/usr/local/lrose/include/rapformats/VerGridRegion.hh
/usr/local/lrose/include/rapformats/station_reports.h
/usr/local/lrose/include/rapformats/WindshearArena.hh
/usr/local/lrose/include/rapformats/alenia.h
/usr/local/lrose/include/rapformats/GenPoly.hh
/usr/local/lrose/include/rapformats/acarsXml.hh
/usr/local/lrose/include/rapformats/WxObs.hh
/usr/local/lrose/include/rapformats/DsPlatformGeoref.hh
/usr/local/lrose/include/rapformats/MapPoint.hh
/usr/local/lrose/include/rapformats/ridds.h
/usr/local/lrose/include/rapformats/titan_grid.h
/usr/local/lrose/include/rapformats/UfRadar.hh
/usr/local/lrose/include/rapformats/chill.h
/usr/local/lrose/include/rapformats/tstorm_spdb.h
/usr/local/lrose/include/rapformats/flt_path.h
/usr/local/lrose/include/rapformats/ZvisCal.hh
/usr/local/lrose/include/rapformats/ltg.h
/usr/local/lrose/include/rapformats/sounding_chunk.h
/usr/local/lrose/include/rapformats/DsFieldParams.hh
/usr/local/lrose/include/rapformats/ac_data.h
/usr/local/lrose/include/rapformats/nids_sat.h
/usr/local/lrose/include/rapformats/bprp.h
/usr/local/lrose/include/rapformats/DsRadarFlags.hh
/usr/local/lrose/include/rapformats/prims.h
/usr/local/lrose/include/rapformats/fos.h
/usr/local/lrose/include/rapformats/DsRadarSweep.hh
/usr/local/lrose/include/rapformats/gate_data.h
/usr/local/lrose/include/rapformats/km.h
/usr/local/lrose/include/rapformats/zrpf.h
/usr/local/lrose/include/rapformats/coord_export.h
/usr/local/lrose/include/rapformats/DsRadarParams.hh
/usr/local/lrose/include/rapformats/DsRadarBeam.hh
/usr/local/lrose/include/rapformats/mcars.h
/usr/local/lrose/include/rapformats/Sndg.hh
/usr/local/lrose/include/rapformats/bdry_extrap.h
/usr/local/lrose/include/rapformats/FieldThresh.hh
/usr/local/lrose/include/rapformats/gint_user.h
/usr/local/lrose/include/rapformats/Map.hh
/usr/local/lrose/include/rapformats/UfRecord.hh
/usr/local/lrose/include/rapformats/KavMosaic.hh
/usr/local/lrose/include/rapformats/MultiThreshItem.hh
/usr/local/lrose/include/rapformats/MitLtg.hh
/usr/local/lrose/include/rapformats/MapSimpleLabel.hh
/usr/local/lrose/include/rapformats/MultiThresh.hh
/usr/local/lrose/include/rapformats/twnltg.h
/usr/local/lrose/include/rapformats/mitre.h
/usr/local/lrose/include/rapformats/KavLtgFile.hh
/usr/local/lrose/include/rapformats/Taf.hh
/usr/local/lrose/include/rapformats/Edr_expanded.hh
/usr/local/lrose/include/rapformats/swap.h
/usr/local/lrose/include/rapformats/nexrad_com_hdrs.h
/usr/local/lrose/include/rapformats/clutter_table.h
/usr/local/lrose/include/rapformats/nws_oso_local.h
/usr/local/lrose/include/rapformats/acars.h
/usr/local/lrose/include/rapformats/r_data.h
/usr/local/lrose/include/rapformats/DsBeamData.hh
/usr/local/lrose/include/rapformats/GldnLtgFile.hh
/usr/local/lrose/include/rapformats/SigAirMet.hh
/usr/local/lrose/include/rapformats/kav_grid.h
/usr/local/lrose/include/rapformats/UsgsData.hh
/usr/local/lrose/include/rapformats/DsRadarAz.hh
/usr/local/lrose/include/rapformats/mdvt_data.h
/usr/local/lrose/include/rapformats/rData.hh
/usr/local/lrose/include/rapformats/MultiThreshFcstBiasMapping.hh
/usr/local/lrose/include/rapformats/FieldThresh2WithGrid.hh
/usr/local/lrose/include/rapformats/DsRadarElev.hh
/usr/local/lrose/include/rapformats/WindshearAlpha.hh
/usr/local/lrose/include/rapformats/Bdry.hh
/usr/local/lrose/include/rapformats/GenPolyStats.hh
/usr/local/lrose/include/rapformats/tstorm_hull_smooth.h
/usr/local/lrose/include/rapformats/ac_georef.hh
/usr/local/lrose/include/rapformats/TileInfo.hh
/usr/local/lrose/include/rapformats/WindshearAlphas.hh
/usr/local/lrose/include/rapformats/ChecktimeReport.hh
/usr/local/lrose/include/rapformats/WxObsField.hh
/usr/local/lrose/include/rapformats/MapIconPoint.hh
/usr/local/lrose/include/rapformats/DsRadarPower.hh
/usr/local/lrose/include/rapformats/nids_file.h
/usr/local/lrose/include/rapformats/RadarSpectra.hh
/usr/local/lrose/include/rapformats/ds_radar.h
/usr/local/lrose/include/rapformats/GaiLtgFile.hh
/usr/local/lrose/include/rapformats/MapIconDef.hh
/usr/local/lrose/include/rapformats/DsBeamDataFieldParms.hh
/usr/local/lrose/include/rapformats/kavltg.h
/usr/local/lrose/include/rapformats/hist_fore.h
/usr/local/lrose/include/rapformats/cedric.h
/usr/local/lrose/include/rapformats/Cedric.hh
/usr/local/lrose/include/rapformats/ascii_shapeio.h
/usr/local/lrose/include/rapformats/DsRadarCalib.hh
/usr/local/lrose/include/rapformats/v_data.h
/usr/local/lrose/include/rapformats/var_elev.h
/usr/local/lrose/include/rapformats/MapIcon.hh
/usr/local/lrose/include/rapformats/moments.h
/usr/local/lrose/include/rapformats/uf_headers.h
/usr/local/lrose/include/rapformats/rp7.h
/usr/local/lrose/include/rapformats/GenPtArray.hh
/usr/local/lrose/include/rapformats/MapPointOffset.hh
/usr/local/lrose/include/rapformats/ac_posn.h
/usr/local/lrose/include/rapformats/WinsRadar.hh
/usr/local/lrose/include/rapformats/polar2mdv_lookup.h
/usr/local/lrose/include/rapformats/ds_radar_calib.h
/usr/local/lrose/include/rapformats/zr.h
/usr/local/lrose/include/rapformats/iwrf_time_series.h
/usr/local/lrose/include/rapformats/mcidas_area.h
/usr/local/lrose/include/rapformats/bdry.h
/usr/local/lrose/include/rapformats/MapPolyline.hh
/usr/local/lrose/include/rapformats/lincoln.h
/usr/local/lrose/include/rapformats/pirep.h
/usr/local/lrose/include/rapformats/BdryPointShearInfo.hh
/usr/local/lrose/include/rapformats/radar_scan_table.h
/usr/local/lrose/include/rapformats/mdvt_user.h
/usr/local/lrose/include/rapformats/kavouras_data.h
/usr/local/lrose/include/rapformats/ComboPt.hh
/usr/local/lrose/include/rapformats/NWS_WWA.hh
/usr/local/lrose/include/rapformats/ccm_file.h
/usr/local/lrose/include/rapformats/ac_route.h
/usr/local/lrose/include/rapformats/TiledMultiThresh.hh
/usr/local/lrose/include/rapformats/Amdar.hh
/usr/local/lrose/include/rapformats/MultBuf.hh
/usr/local/lrose/include/rapformats/MapObject.hh
/usr/local/lrose/include/rapformats/Edr.hh
/usr/local/lrose/include/rapformats/pirepXml.hh
/usr/local/lrose/include/rapformats/metar_decode.h
/usr/local/lrose/include/rapformats/acPosVector.hh
/usr/local/lrose/include/rapformats/MultiThresholdsBiasMapping.hh
/usr/local/lrose/include/rapformats/WxTypeField.hh
/usr/local/lrose/include/rapformats/metar.h
/usr/local/lrose/include/rapformats/StationData.h
/usr/local/lrose/include/rapformats/dobson.h
/usr/local/lrose/include/rapformats/ZvisFcast.hh
/usr/local/lrose/include/rapformats/TaiwanAwos.hh
/usr/local/lrose/include/rapformats/kavouras_io.h
/usr/local/lrose/include/rapformats/HydroStation.hh
/usr/local/lrose/include/rapformats/gailtg.h
/usr/local/lrose/include/rapformats/lass.h
/usr/local/lrose/include/rapformats/Windshear.hh
/usr/local/lrose/include/rapformats/ZVis.hh
/usr/local/lrose/include/rapformats/Pirep.hh
/usr/local/lrose/include/rapformats/SsiFile.h
/usr/local/lrose/include/rapformats/ds_radar_ts.h
/usr/local/lrose/include/rapformats/iidabin.h
/usr/local/lrose/include/rapformats/DsRadarMsg.hh
/usr/local/lrose/include/rapformats/GenPt.hh
/usr/local/lrose/include/rapformats/tdwr_prims.h
/usr/local/lrose/include/rapformats/nexrad.h
/usr/local/lrose/include/rapformats/LtgGroup.hh
/usr/local/lrose/include/rapformats/trec_gauge.h
/usr/local/lrose/include/rapformats/TileRange.hh
/usr/local/lrose/include/rapformats/rvp8_ts_api.h
/usr/local/lrose/include/rapformats/BdryPolyline.hh
/usr/local/lrose/include/rapformats/FieldThresh2.hh
/usr/local/lrose/include/rapformats/MultBufPart.hh
/usr/local/lrose/include/rapformats/BdryPoint.hh
/usr/local/lrose/include/rapformats/station_file.h
/usr/local/lrose/include/rapformats/LtgWrapper.hh
/usr/local/lrose/include/rapformats/bdry_typedefs.h
/usr/local/lrose/include/h5z.mod
/usr/local/lrose/include/ncCompoundType.h
/usr/local/lrose/include/ncException.h
/usr/local/lrose/include/Ncxx/NcxxDouble.hh
/usr/local/lrose/include/Ncxx/NcxxUint64.hh
/usr/local/lrose/include/Ncxx/NcxxType.hh
/usr/local/lrose/include/Ncxx/NcxxVarAtt.hh
/usr/local/lrose/include/Ncxx/NcxxVar.hh
/usr/local/lrose/include/Ncxx/NcxxUbyte.hh
/usr/local/lrose/include/Ncxx/NcxxInt64.hh
/usr/local/lrose/include/Ncxx/NcxxGroup.hh
/usr/local/lrose/include/Ncxx/NcxxString.hh
/usr/local/lrose/include/Ncxx/Nc3File.hh
/usr/local/lrose/include/Ncxx/NcxxErrStr.hh
/usr/local/lrose/include/Ncxx/NcxxFile.hh
/usr/local/lrose/include/Ncxx/NcxxInt.hh
/usr/local/lrose/include/Ncxx/NcxxAtt.hh
/usr/local/lrose/include/Ncxx/NcxxCheck.hh
/usr/local/lrose/include/Ncxx/Nc3Values.hh
/usr/local/lrose/include/Ncxx/NcxxShort.hh
/usr/local/lrose/include/Ncxx/Ncxx.hh
/usr/local/lrose/include/Ncxx/ByteOrder.hh
/usr/local/lrose/include/Ncxx/NcxxUshort.hh
/usr/local/lrose/include/Ncxx/NcxxChar.hh
/usr/local/lrose/include/Ncxx/Hdf5xx.hh
/usr/local/lrose/include/Ncxx/NcxxEnumType.hh
/usr/local/lrose/include/Ncxx/Udunits2.hh
/usr/local/lrose/include/Ncxx/NcxxVlenType.hh
/usr/local/lrose/include/Ncxx/NcxxDim.hh
/usr/local/lrose/include/Ncxx/NcxxFloat.hh
/usr/local/lrose/include/Ncxx/NcxxException.hh
/usr/local/lrose/include/Ncxx/NcxxPort.hh
/usr/local/lrose/include/Ncxx/Nc3xFile.hh
/usr/local/lrose/include/Ncxx/NcxxUint.hh
/usr/local/lrose/include/Ncxx/NcxxOpaqueType.hh
/usr/local/lrose/include/Ncxx/NcxxGroupAtt.hh
/usr/local/lrose/include/Ncxx/NcxxCompoundType.hh
/usr/local/lrose/include/Ncxx/NcxxByte.hh
/usr/local/lrose/include/ncByte.h
/usr/local/lrose/include/ncFile.h
/usr/local/lrose/include/hdf5_hl.h
/usr/local/lrose/include/h5fortran_types.mod
/usr/local/lrose/include/H5Ipublic.h
/usr/local/lrose/include/h5r.mod
/usr/local/lrose/include/Spdb/DsSymprodServer.hh
/usr/local/lrose/include/Spdb/DsSpdbThreaded.hh
/usr/local/lrose/include/Spdb/Spdb_typedefs.hh
/usr/local/lrose/include/Spdb/ConvRegionHazard.hh
/usr/local/lrose/include/Spdb/sounding.h
/usr/local/lrose/include/Spdb/DsSpdb.hh
/usr/local/lrose/include/Spdb/WxHazardBuffer.hh
/usr/local/lrose/include/Spdb/FltRoute.hh
/usr/local/lrose/include/Spdb/DsSpdbServer.hh
/usr/local/lrose/include/Spdb/ThresholdBiasMapping.hh
/usr/local/lrose/include/Spdb/MultiThreshBiasMapping.hh
/usr/local/lrose/include/Spdb/SoundingPut.hh
/usr/local/lrose/include/Spdb/WxHazard.hh
/usr/local/lrose/include/Spdb/SymprodObj.hh
/usr/local/lrose/include/Spdb/ConvRegionHazardExt.hh
/usr/local/lrose/include/Spdb/SoundingGet.hh
/usr/local/lrose/include/Spdb/Product_defines.hh
/usr/local/lrose/include/Spdb/PosnRpt.hh
/usr/local/lrose/include/Spdb/Symprod.hh
/usr/local/lrose/include/Spdb/Spdb.hh
/usr/local/lrose/include/Spdb/DsSpdbMsg.hh
/usr/local/lrose/include/Spdb/LtgSpdbBuffer.hh
/usr/local/lrose/include/Spdb/Sounding.hh
/usr/local/lrose/include/Spdb/WxHazardFactory.hh
/usr/local/lrose/include/Spdb/DsSpdbInput.hh
/usr/local/lrose/include/Spdb/Symprod_typedefs.hh
/usr/local/lrose/include/Spdb/WayPoint.hh
/usr/local/lrose/include/rapmath/FuzzyF.hh
/usr/local/lrose/include/rapmath/ScaleFuzzyFunc.hh
/usr/local/lrose/include/rapmath/FuzzyFunction.hh
/usr/local/lrose/include/rapmath/usort.h
/usr/local/lrose/include/rapmath/LinearPtFunc.hh
/usr/local/lrose/include/rapmath/NewtonPtFunc.hh
/usr/local/lrose/include/rapmath/LagrangePtFunc.hh
/usr/local/lrose/include/rapmath/PtFuzzyFunc.hh
/usr/local/lrose/include/rapmath/round.h
/usr/local/lrose/include/rapmath/usort_stl.h
/usr/local/lrose/include/rapmath/trig.h
/usr/local/lrose/include/rapmath/TrapFuzzyF.hh
/usr/local/lrose/include/rapmath/bd_tree.h
/usr/local/lrose/include/rapmath/Fuzzy2d.hh
/usr/local/lrose/include/rapmath/stats.h
/usr/local/lrose/include/rapmath/PtFunction.hh
/usr/local/lrose/include/rapmath/SFuzzyF.hh
/usr/local/lrose/include/rapmath/RapComplex.hh
/usr/local/lrose/include/rapmath/umath.h
/usr/local/lrose/include/rapmath/OrderedList.hh
/usr/local/lrose/include/rapmath/ParamsFuzzyFunction.hh
/usr/local/lrose/include/rapmath/math_macros.h
/usr/local/lrose/include/rapmath/StepPtFunc.hh
/usr/local/lrose/include/rapmath/Math.hh
/usr/local/lrose/include/rapmath/usvd.h
/usr/local/lrose/include/rapmath/AngleCombiner.hh
/usr/local/lrose/include/rapmath/RMmalloc.h
/usr/local/lrose/include/H5Classes.h
/usr/local/lrose/include/H5DOpublic.h
/usr/local/lrose/include/h5s.mod
/usr/local/lrose/include/H5CommonFG.h
/usr/local/lrose/include/H5Cpublic.h
/usr/local/lrose/include/netcdf.h
/usr/local/lrose/include/H5FDlog.h
/usr/local/lrose/include/H5overflow.h
/usr/local/lrose/include/netcdf.hh
/usr/local/lrose/include/H5CompType.h
/usr/local/lrose/include/H5Library.h
/usr/local/lrose/include/netcdf.inc
/usr/local/lrose/include/H5version.h
/usr/local/lrose/include/h5lt.mod
/usr/local/lrose/include/h5g.mod
/usr/local/lrose/include/h5tb.mod
/usr/local/lrose/include/ncVar.h
/usr/local/lrose/include/H5ArrayType.h
/usr/local/lrose/include/H5pubconf.h
/usr/local/lrose/include/H5Ppublic.h
/usr/local/lrose/include/physics/thermo.h
/usr/local/lrose/include/physics/ZxRelation.hh
/usr/local/lrose/include/physics/DtfMwave.hh
/usr/local/lrose/include/physics/_PhysicsLibImplementation.hh
/usr/local/lrose/include/physics/physics_macros.h
/usr/local/lrose/include/physics/PhysicsLib.hh
/usr/local/lrose/include/physics/AdiabatTempLookupTable.hh
/usr/local/lrose/include/physics/IcaoStdAtmos.hh
/usr/local/lrose/include/physics/physics.h
/usr/local/lrose/include/physics/stn_pressure.h
/usr/local/lrose/include/physics/vil.h
/usr/local/lrose/include/physics/density.h
/usr/local/lrose/include/physics/CapeCinSounding.hh
/usr/local/lrose/include/H5Epubgen.h
/usr/local/lrose/include/H5PacketTable.h
/usr/local/lrose/include/converter.h
/usr/local/lrose/include/h5_dble_interface.mod
/usr/local/lrose/include/h5t.mod
/usr/local/lrose/include/H5FDfamily.h
/usr/local/lrose/include/netcdf_nc_interfaces.mod
/usr/local/lrose/include/Fmq/DsFmq.hh
/usr/local/lrose/include/Fmq/FmqDeviceShmem.hh
/usr/local/lrose/include/Fmq/NowcastProcess.hh
/usr/local/lrose/include/Fmq/DsFmqMsg.hh
/usr/local/lrose/include/Fmq/DrawQueue.hh
/usr/local/lrose/include/Fmq/NowcastQueue.hh
/usr/local/lrose/include/Fmq/RemoteUIQueue.hh
/usr/local/lrose/include/Fmq/GenericQueue.hh
/usr/local/lrose/include/Fmq/DsRadarQueue.hh
/usr/local/lrose/include/Fmq/FmqDevice.hh
/usr/local/lrose/include/Fmq/FmqDeviceFile.hh
/usr/local/lrose/include/Fmq/Fmq.hh
/usr/local/lrose/include/H5FDsec2.h
/usr/local/lrose/include/netcdf4_nc_interfaces.mod
/usr/local/lrose/include/h5e.mod
/usr/local/lrose/include/H5Opublic.h
/usr/local/lrose/include/netcdf.mod
/usr/local/lrose/include/ncUbyte.h
/usr/local/lrose/include/netcdf_mem.h
/usr/local/lrose/include/H5f90i_gen.h
/usr/local/lrose/include/H5Epublic.h
/usr/local/lrose/include/h5p_provisional.mod
/usr/local/lrose/include/netcdf_fortv2_c_interfaces.mod
/usr/local/lrose/include/H5EnumType.h
/usr/local/lrose/include/H5DataType.h
/usr/local/lrose/include/h5i.mod
/usr/local/lrose/include/h5ds.mod
/usr/local/lrose/include/ncType.h
/usr/local/lrose/include/H5public.h
/usr/local/lrose/include/hdf5.h
/usr/local/lrose/include/kd/kd_interp.hh
/usr/local/lrose/include/kd/naive.hh
/usr/local/lrose/include/kd/tokenize.hh
/usr/local/lrose/include/kd/kd.hh
/usr/local/lrose/include/kd/fileoper.hh
/usr/local/lrose/include/kd/datatype.hh
/usr/local/lrose/include/kd/metric.hh
/usr/local/lrose/include/kd/kd_query.hh
/usr/local/lrose/include/h5d.mod
/usr/local/lrose/include/H5OcreatProp.h
/usr/local/lrose/include/h5lib.mod
/usr/local/lrose/include/H5DcreatProp.h
/usr/local/lrose/include/netcdf
/usr/local/lrose/include/ncDouble.h
/usr/local/lrose/include/H5FDmpio.h
/usr/local/lrose/include/dsdata/DsUrlTriggerObjectVirtualMethods.hh
/usr/local/lrose/include/dsdata/DsOneFileTrigger.hh
/usr/local/lrose/include/dsdata/DsFcstTimeListTrigger.hh
/usr/local/lrose/include/dsdata/AcarsDataPoint.hh
/usr/local/lrose/include/dsdata/DsOneTimeTrigger.hh
/usr/local/lrose/include/dsdata/DsMultTrigElem.hh
/usr/local/lrose/include/dsdata/DsSpecificFcstLdataTrigger.hh
/usr/local/lrose/include/dsdata/DsInputDirTrigger.hh
/usr/local/lrose/include/dsdata/DsEnsembleDataTrigger.hh
/usr/local/lrose/include/dsdata/Tstorm.hh
/usr/local/lrose/include/dsdata/MdvTimeListHandler.hh
/usr/local/lrose/include/dsdata/DsUrlTriggerSubsample.hh
/usr/local/lrose/include/dsdata/DsUrlTriggerObject.hh
/usr/local/lrose/include/dsdata/TstormGrid.hh
/usr/local/lrose/include/dsdata/DsFcstTime.hh
/usr/local/lrose/include/dsdata/DsInputPathTrigger.hh
/usr/local/lrose/include/dsdata/DsLdataTrigger.hh
/usr/local/lrose/include/dsdata/DsTrigger.hh
/usr/local/lrose/include/dsdata/TriggerInfo.hh
/usr/local/lrose/include/dsdata/DsUrlTriggerRealtime.hh
/usr/local/lrose/include/dsdata/TimeListHandler.hh
/usr/local/lrose/include/dsdata/SpdbTimeListHandler.hh
/usr/local/lrose/include/dsdata/DsEnsembleGenTrigger.hh
/usr/local/lrose/include/dsdata/DsMultipleTrigger.hh
/usr/local/lrose/include/dsdata/TstormGroup.hh
/usr/local/lrose/include/dsdata/DsLdataIntTrigger.hh
/usr/local/lrose/include/dsdata/DsEnsembleAnyTrigger.hh
/usr/local/lrose/include/dsdata/DsEnsembleLeadTrigger.hh
/usr/local/lrose/include/dsdata/DsIntervalTrigger.hh
/usr/local/lrose/include/dsdata/DsDirListTrigger.hh
/usr/local/lrose/include/dsdata/DsUrlTriggerObjectDerived.hh
/usr/local/lrose/include/dsdata/DsUrlTriggerArchive.hh
/usr/local/lrose/include/dsdata/DsUrlTrigger.hh
/usr/local/lrose/include/dsdata/DsTimeListTrigger.hh
/usr/local/lrose/include/dsdata/DsFileListTrigger.hh
/usr/local/lrose/include/dsdata/DsSpecificGenLdataTrigger.hh
/usr/local/lrose/include/dsdata/DsFmqTrigger.hh
/usr/local/lrose/include/dsdata/TstormMgr.hh
/usr/local/lrose/include/dsdata/DsMultFcstTrigger.hh
/usr/local/lrose/include/ncvalues.h
/usr/local/lrose/include/Radx/NexradCmdRadxFile.hh
/usr/local/lrose/include/Radx/HrdData.hh
/usr/local/lrose/include/Radx/RadxAngleHist.hh
/usr/local/lrose/include/Radx/RadxArray.hh
/usr/local/lrose/include/Radx/Cf2RadxFile.hh
/usr/local/lrose/include/Radx/RadxPlatform.hh
/usr/local/lrose/include/Radx/RadxRemap.hh
/usr/local/lrose/include/Radx/TableMapKey.hh
/usr/local/lrose/include/Radx/NoxpNcRadxFile.hh
/usr/local/lrose/include/Radx/EdgeNcRadxFile.hh
/usr/local/lrose/include/Radx/NetcdfClassic.hh
/usr/local/lrose/include/Radx/BufrFile.hh
/usr/local/lrose/include/Radx/RadxRcalib.hh
/usr/local/lrose/include/Radx/NsslMrdRadxFile.hh
/usr/local/lrose/include/Radx/RadxBuf.hh
/usr/local/lrose/include/Radx/SigmetData.hh
/usr/local/lrose/include/Radx/RadxTimeList.hh
/usr/local/lrose/include/Radx/CfarrNcRadxFile.hh
/usr/local/lrose/include/Radx/NcxxRadxFile.hh
/usr/local/lrose/include/Radx/NexradRadxFile.hh
/usr/local/lrose/include/Radx/RadxCfactors.hh
/usr/local/lrose/include/Radx/OdimHdf5RadxFile.hh
/usr/local/lrose/include/Radx/NexradVcp.hh
/usr/local/lrose/include/Radx/TdwrRadxFile.hh
/usr/local/lrose/include/Radx/ForayNcRadxFile.hh
/usr/local/lrose/include/Radx/RadxStr.hh
/usr/local/lrose/include/Radx/TwolfRadxFile.hh
/usr/local/lrose/include/Radx/GamicHdf5RadxFile.hh
/usr/local/lrose/include/Radx/RayxMapping.hh
/usr/local/lrose/include/Radx/HrdRadxFile.hh
/usr/local/lrose/include/Radx/TableMapElement.hh
/usr/local/lrose/include/Radx/RadxTime.hh
/usr/local/lrose/include/Radx/NidsData.hh
/usr/local/lrose/include/Radx/RadxVol.hh
/usr/local/lrose/include/Radx/RadxSweep.hh
/usr/local/lrose/include/Radx/RadxAzElev.hh
/usr/local/lrose/include/Radx/RadxMsg.hh
/usr/local/lrose/include/Radx/RadxFuzzyF.hh
/usr/local/lrose/include/Radx/RadxField.hh
/usr/local/lrose/include/Radx/ByteOrder.hh
/usr/local/lrose/include/Radx/RadxEvent.hh
/usr/local/lrose/include/Radx/RadxRay.hh
/usr/local/lrose/include/Radx/PseudoRhi.hh
/usr/local/lrose/include/Radx/BufrRadxFile.hh
/usr/local/lrose/include/Radx/SigmetRadxFile.hh
/usr/local/lrose/include/Radx/RadxPath.hh
/usr/local/lrose/include/Radx/RadxComplex.hh
/usr/local/lrose/include/Radx/NetcdfCxxUtils.hh
/usr/local/lrose/include/Radx/D3rNcRadxFile.hh
/usr/local/lrose/include/Radx/BufrProduct.hh
/usr/local/lrose/include/Radx/RayxData.hh
/usr/local/lrose/include/Radx/UfRadxFile.hh
/usr/local/lrose/include/Radx/GemRadxFile.hh
/usr/local/lrose/include/Radx/TableMap.hh
/usr/local/lrose/include/Radx/NexradLoc.hh
/usr/local/lrose/include/Radx/Udunits2.hh
/usr/local/lrose/include/Radx/DoradeData.hh
/usr/local/lrose/include/Radx/RapicRadxFile.hh
/usr/local/lrose/include/Radx/RadxGeoref.hh
/usr/local/lrose/include/Radx/RadxFile.hh
/usr/local/lrose/include/Radx/RadxFuzzy2d.hh
/usr/local/lrose/include/Radx/NexradData.hh
/usr/local/lrose/include/Radx/UfData.hh
/usr/local/lrose/include/Radx/NidsRadxFile.hh
/usr/local/lrose/include/Radx/RadxRangeGeom.hh
/usr/local/lrose/include/Radx/NcfRadxFile.hh
/usr/local/lrose/include/Radx/Hdf5Utils.hh
/usr/local/lrose/include/Radx/LeoRadxFile.hh
/usr/local/lrose/include/Radx/DoradeRadxFile.hh
/usr/local/lrose/include/Radx/RadxPacking.hh
/usr/local/lrose/include/Radx/TdwrLoc.hh
/usr/local/lrose/include/Radx/Radx.hh
/usr/local/lrose/include/Radx/DoeNcRadxFile.hh
/usr/local/lrose/include/Radx/RadxXml.hh
/usr/local/lrose/include/Radx/RadxReadDir.hh
/usr/local/lrose/include/Radx/NoaaFslRadxFile.hh
/usr/local/lrose/include/h5d_provisional.mod


6/5/2018

ok, now build rpm in a container
- export the rpm
- install the rpm in a clean new container and test using X11 tunneling

Here is the Dockerfile to build the rpm; it is exported to the directory in which it is started.

[eol-albireo:~/RPM/first_try/centos-rpmbuild] brenda% more Dockerfile
#
# start with an image that contains all the packages needed to 
# build lrose-blaze + rpmbuilder
#
FROM centos-rpmbuilder
 
# make the spec file available to the container
#
ADD . /tmp/rpm_files

# create user rpmbuilder
#RUN useradd -ms /bin/bash rpmbuilder 
#USER rpmbuilder
#WORKDIR /home/rpmbuilder

# build hierarchy for rpmbuild:
RUN cd ~
RUN mkdir BUILD RPMS SOURCES SPECS SRPMS
#
# import the files
#
RUN cp /tmp/rpm_files/lrose-blaze.spec SPECS/lrose-blaze.spec
RUN cp /tmp/bj/lrose-blaze-20180516.src.tgz SOURCES/lrose-blaze-20180516.src.tgz
# 
# RUN cd lrose_blaze/SOURCES
# RUN git clone https://github.com/NCAR/lrose-core
#
# RUN ./build/create_src_release.py --package=lrose-blaze
# RUN mv ~/releases/tmp/lrose-blaze-20180516.src.tgz ~/SOURCES/.
# RUN cd ..
#
# RUN rpmbuild -v -bb --clean SPECS/lrose-blaze.spec
# RUN rpmbuild -quiet -bb SPECS/lrose-blaze.spec

#
# export the package
#
RUN rsync RPMS /tmp/rpm_files/RPMS

-------

[eol-albireo:~/RPM/first_try/centos-rpmbuild] brenda% docker build --rm -t "mytest_rpm" .

[eol-albireo:~/RPM/first_try/centos-rpmbuild] brenda% docker run --rm -it -v ~/RPM/first_try/centos-rpmbuild:/tmp/out mytest_rpm

[root@bedafd04ecb7 bj]# cp -R  RPMS /tmp/out 
[root@bedafd04ecb7 bj]# exit

### Now, move the RPM into a clean container, install it and then test it ...


[eol-albireo:~/RPM/first_try/centos-rpmbuild] brenda% docker run --rm -it -v ~/RPM/first_try/centos-rpmbuild:/tmp/out centos
[root@3de595bf4ef4 /]# ls /tmp/out
Dockerfile  RPMS  lrose-blaze-20180516.x86_64.rpm  lrose-blaze.spec  lrose-blaze.spec.backup
[root@3de595bf4ef4 /]# whoami
root
[root@3de595bf4ef4 /]# rpm         
RPM version 4.11.3
Copyright (C) 1998-2002 - Red Hat, Inc.
This program may be freely redistributed under the terms of the GNU GPL

Usage: rpm [-aKfgpqVcdLilsiv?] [-a|--all] [-f|--file] [-g|--group] [-p|--package] [--pkgid] [--hdrid] [--triggeredby] [--whatrequires] [--whatprovides] [--nomanifest] [-c|--configfiles]
        [-d|--docfiles] [-L|--licensefiles] [--dump] [-l|--list] [--queryformat=QUERYFORMAT] [-s|--state] [--nofiledigest] [--nofiles] [--nodeps] [--noscript] [--allfiles] [--allmatches]
        [--badreloc] [-e|--erase <package>+] [--excludedocs] [--excludepath=<path>] [--force] [-F|--freshen <packagefile>+] [-h|--hash] [--ignorearch] [--ignoreos] [--ignoresize] [-i|--install]
        [--justdb] [--nodeps] [--nofiledigest] [--nocontexts] [--noorder] [--noscripts] [--notriggers] [--nocollections] [--oldpackage] [--percent] [--prefix=<dir>] [--relocate=<old>=<new>]
        [--replacefiles] [--replacepkgs] [--test] [-U|--upgrade <packagefile>+] [-D|--define 'MACRO EXPR'] [--undefine=MACRO] [-E|--eval 'EXPR'] [--macros=<FILE:...>] [--noplugins] [--nodigest]
        [--nosignature] [--rcfile=<FILE:...>] [-r|--root ROOT] [--dbpath=DIRECTORY] [--querytags] [--showrc] [--quiet] [-v|--verbose] [--version] [-?|--help] [--usage] [--scripts]
        [--setperms] [--setugids] [--conflicts] [--obsoletes] [--provides] [--requires] [--info] [--changelog] [--xml] [--triggers] [--last] [--dupes] [--filesbypkg] [--fileclass]
        [--filecolor] [--fscontext] [--fileprovide] [--filerequire] [--filecaps]
[root@3de595bf4ef4 /]# ls /usr/local/lrose
ls: cannot access /usr/local/lrose: No such file or directory
[root@3de595bf4ef4 /]# rpm -i /tmp/out/RPMS/x86_64/lrose-blaze-20180516.x86_64.rpm 
error: Failed dependencies:
	libQt5Core.so.5()(64bit) is needed by lrose-blaze-20180516.x86_64
	libQt5Core.so.5(Qt_5)(64bit) is needed by lrose-blaze-20180516.x86_64
	libQt5Core.so.5(Qt_5.9)(64bit) is needed by lrose-blaze-20180516.x86_64
	libQt5Gui.so.5()(64bit) is needed by lrose-blaze-20180516.x86_64
	libQt5Gui.so.5(Qt_5)(64bit) is needed by lrose-blaze-20180516.x86_64
	libQt5Network.so.5()(64bit) is needed by lrose-blaze-20180516.x86_64
	libQt5Widgets.so.5()(64bit) is needed by lrose-blaze-20180516.x86_64
	libQt5Widgets.so.5(Qt_5)(64bit) is needed by lrose-blaze-20180516.x86_64
	libX11.so.6()(64bit) is needed by lrose-blaze-20180516.x86_64
	libXext.so.6()(64bit) is needed by lrose-blaze-20180516.x86_64
	libfftw3.so.3()(64bit) is needed by lrose-blaze-20180516.x86_64
	libgfortran.so.3()(64bit) is needed by lrose-blaze-20180516.x86_64
	libgfortran.so.3(GFORTRAN_1.0)(64bit) is needed by lrose-blaze-20180516.x86_64
	libjasper.so.1()(64bit) is needed by lrose-blaze-20180516.x86_64
	libpng15.so.15()(64bit) is needed by lrose-blaze-20180516.x86_64
	libquadmath.so.0()(64bit) is needed by lrose-blaze-20180516.x86_64
[root@3de595bf4ef4 /]# 

Ok, I need to install the dependencies in the clean container ...

Actually, I better use a container with X11 forwarding, and the needed dependencies ...
I'll call it centos-lrose-dependencies-x11; this will be the directory name, and the associated Dockerfile, along with README file containing the command to build the container and run it.

I'll need a Dockerfile for this ... 


FROM centos

ADD . /tmp/bj
WORKDIR /tmp/bj

#
RUN yum -y install rsync
RUN yum -y install gcc 
RUN yum -y install gcc-gfortran
RUN yum -y install gcc-c++
RUN yum -y install make
RUN yum -y install wget
RUN yum -y install expat-devel
RUN yum -y install m4
RUN yum -y install jasper-devel
RUN yum -y install flex-devel
RUN yum -y install zlib-devel
RUN yum -y install libpng-devel
# RUN yum -y install python-devel
 RUN yum -y install qt5
RUN yum -y install fftw3-devel
