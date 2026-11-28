MSC has developed a C sample program for converting the Himawari Standard Data to NetCDF Data.
This sample converts Himawari Standard Data to NetCDF Data.
It has been compiled and checked on Linux (RedHat, 64bit).
We would appreciate it if you could give us some comments or/and bug reports (E-mail: jma-msc-contact@ml.kishou.go.jp).

Files:
	hisd.h                - define the structure of the header of the Himawari Standard Data
	hisd2netcdf.h         - header file for main function
	date_utl.h            - header file for MJD function
	main.c                - main function
	hisd_pixlin2lonlat.c  - function for projection transformation
	hisd_read.c           - function for reading the Himawari Standard Data
	date_utl.c            - function for Modified Julian Day

Make/Run
	(1) extract zip file        (>unzip sample_code_netcdf121.zip      )
	(2) make executable file    (>make                       )
	(3) run the file            (>hisd2netcdf -i InFile1 [-i InFile2 ...] -o OutFile )

Note:
	There are 6 parameters to define the area scope of NetCDF Data.
	These parameters are defined in main function.
		WIDTH  : pixel number
		HEIGHT : line number
		LTLON  : left top longitude
		LTLAT  : left top latitude
		DLON   : spatial resolution (longitude)
		DLAT   : spatial resolution (latitude)

	Run the executable file without arguments. Usage is outlined below. 

	Usage : hisd2netcdf [OPTION]
		-i <InFile> [-i <InFile2> ...]
		-o <OutFile>
		-width  <Pixel Number>
		-height <Line Number>
		-lat    <Left top latitude>
		-lon    <Left top longitude>
		-dlat   <Spatial resolution (longitude)>
		-dlon   <Spatial resolution (latitude)>

	The output data area can also be specified using these options. 
	E.g. To output an area of (40°N, 130°E) - (30°N, 140°E) with a 0.02-degree grid scale, use:
	hisd2netcdf -width 501 -height 501 -lat 40 -lon 130 -dlat 0.02 -dlon 0.02 -i InFile1 -i InFile2 -i InFile3 … -o OutFile

	Missing grid points for any part of the area (40°N, 130°E) - (30°N, 140°E) in input file content will have invalid values in output file content. 

Disclaimer:
	MSC does not guarantee regarding the correctness, accuracy, reliability, or any other aspect regarding use of these sample codes.

Detail of Himawari Standard Format:
	For data structure of Himawari Standard Format, please refer to MSC Website and Himawari Standard Data User's Guide.

	MSC Website
	https://www.data.jma.go.jp/mscweb/en/index.html

	Himawari Standard Data User's Guide
	https://www.data.jma.go.jp/mscweb/en/himawari89/space_segment/hsd_sample/HS_D_users_guide_en_v13.pdf

History
	March,  2015  First release
	May,    2015  Change for version 1.2
	June,   2015  Version 2015-06
		      Fixed bug in function getData() fuction (main.c)
		      Fixed bug in function function hisd_read_header() (hisd_read.c)
		      Fixed bug in fucntion lonlat_to_pixlin() (hisd_pixlin2lonlat.c)
		      ((8) check the reverse side of the Earth)
	July,   2020  Version 2020-07
		      Fixed bug in function defNetcdf() function (main.c)
	June,	2021  Version 2021-06
		      Fixed bug in function defNetcdf() function (main.c)
					    getData()  function (main.c)
