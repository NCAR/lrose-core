                            GTOPO30 Documentation

Table of Contents:

 1.0  Introduction
 2.0  Data Set Characteristics
 3.0  Data Format
    3.1  DEM File
    3.2  Header File
    3.3  World File
    3.4  Statistics File
    3.5  Projection File
    3.6  Shaded Relief Image
    3.7  Source Map
    3.8  Source Map Header File
 4.0  Data Distribution
    4.1  Procedures for Obtaining Data
    4.2  File Sizes
 5.0  Notes and Hints for GTOPO30 Users
 6.0  Data Set Development
    6.1  Data Sources
       6.1.1  Digital Terrain Elevation Data
       6.1.2  Digital Chart of the World
       6.1.3  USGS Digital Elevation Models
       6.1.4  Army Map Service Maps
       6.1.5  International Map of the World
       6.1.6  Peru Map
       6.1.7  New Zealand DEM
       6.1.8  Antarctic Digital Database
    6.2  Data Processing
       6.2.1  Raster Source Processing
       6.2.2  Vector Source Processing
       6.2.3  DEM Merging
       6.2.4  Global Product Assembly
 7.0  Accuracy
 8.0  GTOPO30 Caveats
    8.1  Grid Spacing and Resolution
    8.2  Topographic Detail and Accuracy
    8.3  Production Artifacts
 9.0  Summary
10.0  References
11.0  Disclaimers

1.0  Introduction

GTOPO30 is a global digital elevation model (DEM) resulting from a
collaborative effort led by the staff at the U.S. Geological Survey's
EROS Data Center in Sioux Falls, South Dakota.  Elevations in GTOPO30 are
regularly spaced at 30-arc seconds (approximately 1 kilometer).  GTOPO30
was developed to meet the needs of the geospatial data user community for
regional and continental scale topographic data.  This release represents
the completion of global coverage of 30-arc second elevation data that
have been available from the EROS Data Center beginning in 1993.   Several
areas have been updated and the entire global data set has been repackaged,
so these data supersede the previously released continental data sets.
Comments from users of GTOPO30 are welcomed and encouraged.  Please send
your comments to Dean Gesch at gesch@edcmail.cr.usgs.gov or to Sue Greenlee
at sgreenlee@edcmail.cr.usgs.gov.

2.0  Data Set Characteristics

GTOPO30 is a global data set covering the full extent of latitude from 90
degrees south to 90 degrees north, and the full extent of longitude from
180 degrees west to 180 degrees east.  The horizontal grid spacing is
30-arc seconds (0.008333333333333 degrees), resulting in a DEM having
dimensions of 21,600 rows and 43,200 columns.  The horizontal coordinate
system is decimal degrees of latitude and longitude referenced to WGS84.
The vertical units represent elevation in meters above mean sea level.
The elevation values range from -407 to 8,752 meters.  In the DEM, ocean
areas have been masked as "no data" and have been assigned a value of -9999.
Lowland coastal areas have an elevation of at least 1 meter, so in the
event that a user reassigns the ocean value from -9999 to 0 the land
boundary portrayal will be maintained.  Due to the nature of the raster
structure of the DEM, small islands in the ocean less than approximately
1 square kilometer will not be represented.

3.0  Data Format

To facilitate electronic distribution, GTOPO30 has been divided into 33
smaller pieces, or tiles.  The area from 60 degrees south latitude to
90 degrees north latitude and from 180 degrees west longitude to 180
degrees east longitude is covered by 27 tiles, with each tile covering 50
degrees of latitude and 40 degrees of longitude.  Antarctica (90 degrees
south latitude to 60 degrees south latitude and 180 degrees west longitude
to 180 degrees east longitude) is covered by 6 tiles, with each tile covering
30 degrees of latitude and 60 degrees of longitude.  The tiles names refer
to the longitude and latitude of the upper-left (northwest) corner of the
tile.  For example, the coordinates of the upper-left corner of tile
E020N40 are 20 degrees east longitude and 40 degrees north latitude.
There is one additional tile that covers all of Antarctica with data in
a polar stereographic projection.  The following table lists the name,
latitude and longitude extent, and elevation statistics for each tile.

             Latitude          Longitude                  Elevation
 Tile    Minimum  Maximum   Minimum  Maximum   Minimum  Maximum  Mean  Std.Dev.
-------  ----------------   ----------------   --------------------------------

W180N90     40       90       -180    -140         1      6098    448     482
W140N90     40       90       -140    -100         1      4635    730     596
W100N90     40       90       -100     -60         1      2416    333     280
W060N90     40       90        -60     -20         1      3940   1624     933
W020N90     40       90        -20      20       -30      4536    399     425
E020N90     40       90         20      60      -137      5483    213     312
E060N90     40       90         60     100      -152      7169    509     698
E100N90     40       90        100     140         1      3877    597     455
E140N90     40       90        140     180         1      4588    414     401
W180N40    -10       40       -180    -140         1      4148    827     862
W140N40    -10       40       -140    -100       -79      4328   1321     744
W100N40    -10       40       -100     -60         1      6710    375     610
W060N40    -10       40        -60     -20         1      2843    212     168
W020N40    -10       40        -20      20      -103      4059    445     298
E020N40    -10       40         20      60      -407      5825    727     561
E060N40    -10       40         60     100         1      8752   1804    1892
E100N40    -10       40        100     140       -40      7213    692     910
E140N40    -10       40        140     180         1      4628    549     715
W180S10    -60      -10       -180    -140         1      2732    188     297
W140S10    -60      -10       -140    -100         1       910     65     124
W100S10    -60      -10       -100     -60         1      6795   1076    1356
W060S10    -60      -10        -60     -20         1      2863    412     292
W020S10    -60      -10        -20      20         1      2590   1085     403
E020S10    -60      -10         20      60         1      3484    893     450
E060S10    -60      -10         60     100         1      2687    246     303
E100S10    -60      -10        100     140         1      1499    313     182
E140S10    -60      -10        140     180         1      3405    282     252
W180S60    -90      -60       -180    -120         1      4009   1616    1043
W120S60    -90      -60       -120     -60         1      4743   1616     774
W060S60    -90      -60        -60       0         1      2916   1866     732
W000S60    -90      -60          0      60         1      3839   2867     689
E060S60    -90      -60         60     120         1      4039   2951     781
E120S60    -90      -60        120     180         1      4363   2450     665
ANTARCPS   -90      -60       -180     180         1      4748   2198    1016

The 27 tiles that individually cover 50 degrees of latitude and 40 degrees
of longitude each have 6,000 rows and 4,800 columns.  The 6 Antarctica tiles
that individually cover 30 degrees of latitude and 60 degrees of longitude
each have 3,600 rows and 7,200 columns.  There is no overlap among the tiles
so the global data set may be assembled by simply abutting the adjacent
tiles.

The tile named ANTARCPS includes the same data as the 6 geographic tiles
covering Antarctica, but is presented in a polar stereographic projection.
The horizontal grid spacing is 1,000 meters, and the tile has 5,400 rows
and 5,400 columns.  The projection parameters used for the polar
stereographic projection are: 0 degrees for the longitude of the central
meridian, 71 degrees south for the latitude of true scale, and 0 for the
false easting and false northing.

Data for each tile are provided in a set of 8 files.  The files are named
with the tile name and a file name extension indicating the contents of the
file.  The following extensions are used:

Extension               Contents
---------               --------

   DEM        digital elevation model data
   HDR        header file for DEM
   DMW        world file
   STX        statistics file
   PRJ        projection information file
   GIF        shaded relief image
   SRC        source map
   SCH        header file for source map

The simple format should allow for easy ingest into most popular image
processing and geographic information systems packages.  Further information
on the contents of the files is provided below.

3.1  DEM File (.DEM)

The DEM is provided as 16-bit signed integer data in a simple binary raster.
There are no header or trailer bytes imbedded in the image.  The data are
stored in row major order (all the data for row 1, followed by all the data
for row 2, etc.).

3.2  Header File (.HDR)

The DEM header file is an ASCII text file containing size and coordinate
information for the DEM.  The following keywords are used in the header file:

BYTEORDER      byte order in which image pixel values are stored
                  M = Motorola byte order (most significant byte first)
LAYOUT         organization of the bands in the file
                  BIL = band interleaved by line (note: the DEM is a single
                  band image)
NROWS          number of rows in the image
NCOLS          number of columns in the image
NBANDS         number of spectral bands in the image (1 for a DEM)
NBITS          number of bits per pixel (16 for a DEM)
BANDROWBYTES   number of bytes per band per row (twice the number of columns
                  for a 16-bit DEM)
TOTALROWBYTES  total number of bytes of data per row (twice the number of
                  columns for a single band 16-bit DEM)
BANDGAPBYTES   the number of bytes between bands in a BSQ format image
                  (0 for a DEM)
NODATA         value used for masking purposes
ULXMAP         longitude of the center of the upper-left pixel (decimal degrees)
ULYMAP         latitude  of the center of the upper-left pixel (decimal degrees)
XDIM           x dimension of a pixel in geographic units (decimal degrees)
YDIM           y dimension of a pixel in geographic units (decimal degrees)

Example header file (W100N40.HDR):

BYTEORDER      M
LAYOUT       BIL
NROWS         6000
NCOLS         4800
NBANDS        1
NBITS         16
BANDROWBYTES         9600
TOTALROWBYTES        9600
BANDGAPBYTES         0
NODATA        -9999
ULXMAP        -99.99583333333334
ULYMAP        39.99583333333333
XDIM          0.00833333333333
YDIM          0.00833333333333

3.3  World File (.DMW)

The world file is an ASCII text file containing coordinate information.  It
is used by some packages for georeferencing of image data.  The following is
an example world file (W100N40.DMW) with a description of each record:

     0.00833333333333     x dimension of a pixel (decimal degrees)
     0.00000000000000     rotation term (will always be zero)
     0.00000000000000     rotation term (will always be zero)
    -0.00833333333333     negative y dimension of a pixel (decimal degrees)
   -99.99583333333334     longitude of the center of the upper-left pixel
    39.99583333333333     latitude of the center of the upper-left pixel

3.4  Statistics File (.STX)

The statistics file is an ASCII text file which lists the band number,
minimum value, maximum value, mean value, and standard deviation of the
values in the DEM data file.

Example statistics file (W100N40.STX):

1 -9999 6710 -6078.8 5044.2

3.5  Projection File (.PRJ)

The projection information file is an ASCII text file which describes the
projection of the DEM and source map image.

Example projection file (W100N40.PRJ):

Projection    GEOGRAPHIC
Datum         WGS84
Zunits        METERS
Units         DD
Spheroid      WGS84
Xshift        0.0000000000
Yshift        0.0000000000
Parameters

3.6  Shaded Relief Image (.GIF)

A shaded relief image is provided as an overview of the data in each tile.
The images were derived from a generalized version of GTOPO30 with a
horizontal grid spacing of 240-arc seconds (approximately 8 kilometers), so
many small islands and features will not be visible.  The images are meant
to provide a convenient way for users to view the general topographic
features portrayed in each tile.  The shaded relief images are provided as
GIF images which can be displayed by many popular image display programs and
World Wide Web browsers.  An image size of 750 rows by 600 columns is used
for the tiles covering 50 degrees of latitude by 40 degrees of longitude.
An image size of 450 rows by 900 columns is used for the Antarctica tiles
which cover 30 degrees of latitude by 60 degrees of longitude each.  The
Antarctica polar stereographic tile is portrayed by a shaded relief image
having 675 rows by 675 columns.

3.7  Source Map (.SRC)

The source map is a simple 8-bit binary image which has values that indicate
the source used to derive the elevation for every cell in the DEM.  The
source map is the same resolution and has the same dimensions and coordinate
system as the DEM.  Like the DEM, it has no header or trailer bytes and is
stored in row major order.  These codes are used in the source map image:

Value               Source
-----               ------

  0       Ocean
  1       Digital Terrain Elevation Data
  2       Digital Chart of the World
  3       USGS 1-degree DEM's
  4       Army Map Service 1:1,000,000-scale maps
  5       International Map of the World 1:1,000,000-scale maps
  6       Peru 1:1,000,000-scale map
  7       New Zealand DEM
  8       Antarctic Digital Database

More information on each of these sources is provided in section 6.1
(Data Sources).  The cells with value 0 (ocean) in the source map can
be used as an ocean mask (the ocean cells match exactly all the cells
masked as "no data" in the DEM with a value of -9999).  Likewise, the cells
with values 1-8 together constitute a global land mask.  Every cell in the
DEM with an elevation has a corresponding cell in the source map with a
value in the range 1-8.

3.8  Source Map Header File (.SCH)

The source map header file is an ASCII text file containing size and
coordinate information, similar to the DEM header file.  The following
keywords are used in the source map header file:

BYTEORDER      byte order in which image pixel values are stored
                  M = Motorola byte order (most significant byte first)
LAYOUT         organization of the bands in the file
                  BIL = band interleaved by line (note: the source map is
                  a single band image)
NROWS          number of rows in the image
NCOLS          number of columns in the image
NBANDS         number of spectral bands in the image (1 for the source map)
NBITS          number of bits per pixel (8 for the source map)
BANDROWBYTES   number of bytes per band per row (the number of columns for
                  an 8-bit source map)
TOTALROWBYTES  total number of bytes of data per row (the number of columns
                  for a single band 8-bit source map)
BANDGAPBYTES   the number of bytes between bands in a BSQ format image
                  (0 for the source map)
NODATA         value used for masking purposes
ULXMAP         longitude of the center of the upper-left pixel (decimal degrees)
ULYMAP         latitude  of the center of the upper-left pixel (decimal degrees)
XDIM           x dimension of a pixel in geographic units (decimal degrees)
YDIM           y dimension of a pixel in geographic units (decimal degrees)

Example source map header file (W100N40.SCH):

BYTEORDER      M
LAYOUT       BIL
NROWS         6000
NCOLS         4800
NBANDS        1
NBITS         8
BANDROWBYTES         4800
TOTALROWBYTES        4800
BANDGAPBYTES         0
NODATA        -9999
ULXMAP        -99.99583333333334
ULYMAP        39.99583333333333
XDIM          0.00833333333333
YDIM          0.00833333333333

4.0  Data Distribution

Data for each GTOPO30 tile are distributed electronically as a compressed tar
file.  The 8 files for each tile have been combined into one file with the
Unix "tar" command, and the tar file has been compressed with GNU "gzip"
utility.  To use the GTOPO30 data files, the tar file must first be
decompressed and then the individual data files extracted from the tar file.
For example, the following Unix command could be used:

   gunzip < w100n40.tar.gz | tar xvf -

If you do not have access to gzip, you can leave off the .gz extension and
the FTP server will decompress the tar file as it is downloaded.  However,
you will still have to run the tar command to extract separate files.
Please note that a decompressed file is typically many times larger than
the compressed version and therefore will take much longer to transmit.  If
you would like to obtain the gzip or tar programs they are available via
anonymous FTP from the following sites:

 Unix gzip:
        ftp://prep.ai.mit.edu/pub/gnu
        ftp://wuarchive.wustl.edu/systems/gnu
 Macintosh gzip and tar:
        ftp://mirrors.aol.com/pub/mac/util/compression
            macgzip0.3b2.sit.hqx
            suntar2.03.cpt.hqx
 DOS gzip and tar:
        ftp://prep.ai.mit.edu/pub/gnu
            gzip-1.2.4.tar
        ftp://ftp.uu.net/systems/ibmpc/msdos/pcroute
            tar.exe

4.1  Procedures for Obtaining Data

GTOPO30 is available electronically through an Internet anonymous File
Transfer Protocol (FTP) account at the EROS Data Center (at no cost).

To access this account:

   1. FTP to edcftp.cr.usgs.gov
   2. Enter "anonymous" at the Name prompt.
   3. Enter your email address at the Password prompt.
   4. Change ("cd") to the  "/pub/data/gtopo30/global" subdirectory.
   5. Files are named according to the longitude and latitude coordinates
      of the upper-left corner of the tile, followed by the extension
      ".tar.gz".
   6. Enter "binary" to set the transfer type.
   7. Use "get" or "mget" to retrieve the desired files.
   8. For the Antarctica polar stereographic data, change ("cd") to the
      "/pub/data/gtopo30/antarctica" subdirectory and use "get" to
      retrieve the file antarcps.tar.gz.

Orders for the data on CD-ROM or 8 mm high density tar tape may be submitted
to:

   EDC DAAC User Services
   EROS Data Center
   Sioux Falls, SD 57198 USA
   Tel: 605-594-6116 (7:30 am to 4:00 pm CT)
   Fax: 605-594-6963 (24 hours)
   Internet: edc@eos.nasa.gov (24 hours)

Data distributed on CD-ROM and 8 mm tape are provided as sets of files for
each tile as described above in section 3.0 (Data Format).  They are not
combined into one tar file or compressed as they are for electronic
distribution.

4.2  File Sizes

After decompression and extraction from the tar files, the following file
sizes are present for each of the 3 sizes of tiles:

    Tile size                File         Size (bytes)
    ---------                ----         ------------

50 degrees latitude           DEM           57600000
         by
40 degrees longitude       Source map       28800000

30 degrees latitude           DEM           51840000
         by
60 degrees longitude       Source map       25920000

Antarctica polar              DEM           58320000
stereographic data
(5,400 km x 5,400 km)      Source map       29160000

For each tile, the total for all the other file types (HDR, DMW, STX, PRJ,
GIF, and SCH) is well under 1 megabyte.

The global 16-bit DEM (21,600 rows by 43,200 columns) has a size of 1.74
gigabytes.  The global 8-bit source map of the same dimensions has a size of
889.9 megabytes.

Through the use of the gzip compression utility the total size of the global
data set is reduced about 90% from almost 2.72 gigabytes to under 290
megabytes.  The list below shows the compressed size for each tile.  The
sizes range from less than 1 megabyte to about 25 megabytes, with the average
at about 8 megabytes.  Decompressed, the tar file for each tile can be as
large as 84 megabytes.

      File        Size (bytes)
      ----        ------------

antarcps.tar.gz     10538463
 e020n40.tar.gz     26124072
 e020n90.tar.gz     16992230
 e020s10.tar.gz      8262946
 e060n40.tar.gz     17935016
 e060n90.tar.gz     22402428
 e060s10.tar.gz       113591
 e060s60.tar.gz      5308336
 e100n40.tar.gz     14175303
 e100n90.tar.gz     24994154
 e100s10.tar.gz      4361555
 e120s60.tar.gz      6131365
 e140n40.tar.gz      1140685
 e140n90.tar.gz      9222752
 e140s10.tar.gz      4059027
 w000s60.tar.gz      5080091
 w020n40.tar.gz     16938044
 w020n90.tar.gz      8844434
 w020s10.tar.gz      2927056
 w060n40.tar.gz      3721100
 w060n90.tar.gz      6820815
 w060s10.tar.gz      6738966
 w060s60.tar.gz      3558292
 w100n40.tar.gz     11330238
 w100n90.tar.gz     15656539
 w100s10.tar.gz      9575882
 w120s60.tar.gz      5677801
 w140n40.tar.gz      6497682
 w140n90.tar.gz     17031379
 w140s10.tar.gz        89706
 w180n40.tar.gz       131975
 w180n90.tar.gz      5477564
 w180s10.tar.gz       116231
 w180s60.tar.gz      3500153

5.0  Notes and Hints for GTOPO30 Users

Because the DEM data are stored in a 16-bit binary format, users must be
aware of how the bytes are addressed on their computers.  The DEM data are
provided in Motorola byte order, which stores the most significant byte
first ("big endian").  Systems such as Sun SPARC and Silicon Graphics
workstations use the Motorola byte order.  The Intel byte order, which
stores the least significant byte first ("little endian"), is used on DEC
Alpha systems and most PCs.  Users with systems that address bytes in the
Intel byte order may have to "swap bytes" of the DEM data unless their
application software performs the conversion during ingest.  The statistics
file (.STX) provided for each tile gives the range of values in the DEM
file, so users can check if they have the correct DEM values stored on
their system.

Users of ARC/INFO or ArcView can display the DEM data directly after simply
renaming the file extension from .DEM to .BIL.  However, if a user needs
access to the actual elevation values for analysis in ARC/INFO the DEM must
be converted to an ARC/INFO grid with the command IMAGEGRID.  IMAGEGRID does
not support conversion of signed image data, therefore the negative 16-bit
DEM values will not be interpreted correctly.  After running IMAGEGRID, an
easy fix can be accomplished using the following formula in Grid:

   out_grid = con(in_grid >= 32768, in_grid - 65536, in_grid)

The converted grid will then have the negative values properly represented,
and the statistics of the grid should match those listed in the .STX file.
If desired, the -9999 ocean mask values in the grid could then be set to
NODATA with the SETNULL function.

6.0  Data Set Development

GTOPO30, completed in late 1996, was developed over a 3 year period through
a collaborative effort led by staff at the U.S. Geological Survey's EROS Data
Center (EDC).  The following organizations participated by contributing
funding or source data: the National Aeronautics and Space Administration
(NASA), the United Nations Environment Programme/Global Resource Information
Database (UNEP/GRID), the U.S. Agency for International Development (USAID),
the Instituto Nacional de Estadistica Geografica e Informatica (INEGI) of
Mexico, the Geographical Survey Institute (GSI) of Japan, Manaaki Whenua
Landcare Research of New Zealand, and the Scientific Committee on Antarctic
Research (SCAR).

6.1  Data Sources

GTOPO30 is based on data derived from 8 sources of elevation information,
including vector and raster data sets.  The following table lists the
percentage of the global land surface area derived from each source (a full
description of each source is provided below):

                Source                                 % of global land area
                ------                                 ---------------------

Digital Terrain Elevation Data                                 50.0
Digital Chart of the World                                     29.9
USGS 1-degree DEM's                                             6.7
Army Map Service 1:1,000,000-scale maps                         1.1
International Map of the World 1:1,000,000-scale maps           3.7
Peru 1:1,000,000-scale map                                      0.1
New Zealand DEM                                                 0.2
Antarctic Digital Database                                      8.3

6.1.1  Digital Terrain Elevation Data

Digital Terrain Elevation Data (DTED) is a raster topographic data base with
a horizontal grid spacing of 3-arc seconds (approximately 90 meters) produced
by the National Imagery and Mapping Agency (NIMA) (formerly the Defense
Mapping Agency).  DTED was used as the source for most of Eurasia and large
parts of Africa, South America, Mexico, Canada, and Central America.  DTED
coverage for Mexico was provided by INEGI.

6.1.2  Digital Chart of the World

Digital Chart of the World (DCW) is a vector cartographic data set based on
the 1:1,000,000-scale Operational Navigation Chart (ONC) series, which is the
largest scale base map source with global coverage (Danko, 1992).  The DCW
and the ONC series are products of NIMA.

The topographic information of interest for generating DEM's is contained in
several DCW hypsography layers.  The primary contour interval on the source
ONC's is 1,000 feet (305 meters), and supplemental contours at an interval of
250 feet (76 meters) are shown in areas below 1,000 feet in elevation.  In
limited areas of higher elevation there supplemental contours at 500-foot
(152-meter) intervals.  The DCW drainage layers were also used as input to
the DEM generation process; this information included stream networks, lake
shorelines, lake elevations, and ocean coastlines.  The DCW was used as the
primary source for filling gaps in the DTED coverage, including all of
Australia, most of Greenland, and large areas of Africa, South America, and
Canada.

6.1.3  USGS Digital Elevation Models

USGS 1-degree DEM's with a horizontal grid spacing of 3-arc seconds
(approximately 90 meters) were used as the source data for the continental
United States, Alaska, and Hawaii.  The topographic information content is
similar to that of DTED.  The "1-degree" designation refers to the unit of
data distribution.

6.1.4  Army Map Service Maps

Paper maps at a scale of 1:1,000,000 produced by the Army Map Service (AMS),
a predecessor of DMA and NIMA, were acquired and digitized by GSI of Japan.
Contours (with intervals of 100, 150, 300, and 500 meters), spot heights,
drainage lines , and coastlines for some islands of southeast Asia and some
small areas in South America were delivered to EDC as digital vector
cartographic data sets.

6.1.5  International Map of the World

Paper maps from the 1:1,000,000-scale International Map of the World (IMW)
series were digitized by GSI to provide source data for the Amazon basin.
The International Map of the World includes national maps produced to a
United Nations specified standard for 1:1,000,000-scale mapping.  The maps
used for this project had a 100-meter contour interval.

6.1.6  Peru Map

Small areas of a 1:1,000,000-scale map from the Peruvian government were
digitized to fill gaps in source data for South America.  The map had a
contour interval of 1,000 meters.

6.1.7  New Zealand DEM

Manaaki Whenua Landcare Research contributed a DEM with a 500-meter
horizontal grid spacing for New Zealand.  The DEM was derived from elevation
information on 1:63,360-scale maps with a 100-foot (30-meter) contour
interval.

6.1.8  Antarctic Digital Database

The Antarctic Digital Database (ADD) was produced under the auspices of the
Scientific Committee on Antarctic Research.  Digital contours and coastlines
from the ADD were used as source material for Antarctica.  The ADD vector
data were compiled from maps ranging in scale from 1:200,000 to 1:5,000,000.
The detail, density, and interval of the contours in the ADD vary widely,
with the more detailed data near the coastline and very generalized data in
the interior of the continent.  Detailed metadata provided in the ADD
identifies the map scale from which each contour line was extracted.

6.2  Data Processing

GTOPO30 was developed over a 3 year period during which continental and
regional areas were produced individually.  As such, processing techniques
were developed and refined throughout the duration of the project.  Although
the techniques used for the various continental areas are very similar,
there were some differences in approach due to varying source material.
More details about data development for several of the continental areas are
reported by Verdin and Greenlee (1996), Bliss and Olsen (1996), and Gesch and
Larson (1996).

Data processing was accomplished using commercially available geographic
information system software, public domain image processing software,
vector-to-raster gridding software, and utilities developed specifically for
this project.  To more efficiently handle the numerous input data sets and to
standardize the proper sequence of processing steps, the production
procedures were automated to a great extent by employing preset parameter
values, scripted command files, and consistent naming schemes for input and
output data files.

6.2.1  Raster Source Processing

Processing of the raster source data, including DTED, USGS DEM's, and the New
Zealand DEM, involved generalizing the higher resolution data to the 30-arc
second horizontal grid spacing.  Because the DTED and USGS DEM's were already
in a geographic "projection" they only required a sampling of the full
resolution 3-arc second data.  One representative elevation value was selected
to represent the area covered by 100 full resolution cells (a 10 by 10
matrix).  As the project progressed, several methods of generalization were
used.  Selection of the representative 30-arc second value was accomplished
by systematic subsampling for North and South America, by calculation of the
median value for Eurasia, and by the breakline emphasis approach (Gesch and
Larson, 1996) for Africa.  The 500-meter New Zealand DEM was generalized to
30-arc seconds by reprojecting it from the New Zealand National Grid
projection to geographic coordinates using bilinear resampling.

6.2.2  Vector Source Processing

The topographic information from the vector cartographic sources, including
the DCW, the ADD, and the Army Map Service, International Map of the World,
and Peru 1:1,000,000-scale maps, was converted into elevation grids through
a vector-to-raster gridding approach.  Contours, spot heights, stream lines,
lake shorelines, and ocean coastlines were input to the ANUDEM surface
gridding program developed at the Australian National University (Hutchinson,
1989).  ANUDEM, specifically designed for creating DEM's from digital contour,
spot height, and stream line data, employs an approach known as drainage
enforcement to produce raster elevation models that represent more closely
the actual terrain surface and contain fewer artifacts than those produced
with more general purpose surface interpolation routines.  Drainage
enforcement was performed for all areas covered by vector source data except
Antarctica and Greenland.

A significant amount of preprocessing was required to prepare and format the
vector source data for input to ANUDEM.  This processing included editing and
updating the vector stream lines so that the direction of each was oriented
downstream (a requirement of ANUDEM).  Further preprocessing involved
detection and correction of erroneous contour and point elevations (Larson,
1996).  Ocean coastlines were assigned an elevation of zero for input as
contours.  Also, shorelines of lakes for which the DCW included elevations
were tagged and used as contour input.  The output from ANUDEM was an
elevation model grid referenced in the same horizontal coordinate system as
the generalized raster source data.  The output grid spacing of 30-arc
seconds has been shown to be appropriate for the information content present
in the DCW hypsography layers (Hutchinson, 1996; Shih and Chiu, 1996).

6.2.3  DEM Merging

Prior to merging with the generalized raster data, lakes for which the DCW
did not indicate an elevation were updated on the DCW grid with the lowest
grid cell elevation found along the shoreline.  When each of the vector
sources was gridded, an overlap area with the adjacent raster sources was
included so that smoothing could be performed to minimize the elevation
discrepancies among the sources.  Also, additional point control was input
into the ANUDEM gridding process so interpolated elevations in the overlap
region would more closely match the raster source elevations.  The additional
control was derived from the generalized raster sources within a 1-degree
buffer surrounding the vector source areas.

Merging of the generalized raster sources and the gridded vector sources was
accomplished by mosaicking the data sets.  The generalized raster sources
had the highest priority so coverage of the data with the greater
topographic detail and accuracy was maximized.  The grid derived from DCW
data had the highest priority among the vector sources, and the other
digitized map data was used when DCW hypsography was unavailable.  The
merging procedure including blending of the generalized raster sources and
the vector-derived grids within an approximate 1-degree overlap area along
the irregular boundaries.  The blending algorithm computes a weighted average
with the weights for each data source determined on a cell-by-cell basis
according to the cell's proximity to the edges of the overlap area (Franke,
1982).

A final processing step performed on the mosaicked and blended product
involved "clipping out" the land (as defined by vector coastline data) and
setting the ocean areas to a constant background value.  Use of vector
coastline data resulted in a more consistent portrayal of the land/ocean
interface, especially in areas where raster source data (which had an implied
coastline) met with vector source data.  The DCW coastline was used to clip
the following areas: Africa, Eurasia, South America, Australia, New Zealand,
Greenland, and isolated ocean islands.  The World Vector Shoreline (WVS), a
vector shoreline data set from NIMA, was used for North America, including
Hawaii, the Caribbean islands, and Central America.  The islands of Borneo
and Sulawesi in southeast Asia were clipped with the coastline digitized
from the 1:1,000,000-scale map source.  Antarctica was defined by the
coastline as portrayed in the ADD.

6.2.4  Global Product Assembly

The global product was assembled from the continental and regional DEM's.
Several areas of overlap due to different production stages of the project
were addressed and eliminated, most notably between the Africa and Eurasia
data sets.  The global source map was generated from masks of source data
coverage, and was verified to register with the DEM precisely.  Finally, the
entire data set was packaged into tiles for easier electronic distribution.

7.0  Accuracy

The absolute vertical accuracy of GTOPO30 varies by location according to
the source data.  Generally, the areas derived from the raster source data
have higher accuracy than those derived from the vector source data.  The
full resolution 3-arc second DTED and USGS DEM's have a vertical accuracy
of + or - 30 meters linear error at the 90 percent confidence level (Defense
Mapping Agency, 1986; U.S. Geological Survey, 1993).  If the error
distribution is assumed to be Gaussian with a mean of zero, the statistical
standard deviation of the errors is equivalent to the root mean square error
(RMSE).  Under those assumptions, vertical accuracy expressed as + or - 30
meters linear error at 90 percent can also be described as a RMSE of 18
meters.  The areas of GTOPO30 derived from DTED and USGS DEM's retain that
same level of accuracy because through generalization a representative
elevation value derived from the full resolution cells is chosen to represent
the area of the reduced resolution cell (although the area on the ground
represented by that one elevation value is now much larger than the area
covered by one full resolution cell).

The absolute vertical accuracy of the DCW, the vector source with the largest
area of coverage, is stated in its product specification as + or - 650 meters
linear error at the 90% confidence level (Defense Mapping Agency, 1990).
Experience has shown that the grids derived from DCW data should in many
areas be much more accurate than the 650-meter specification.  To better
characterize the accuracy of the areas of GTOPO30 derived from DCW vector
hypsography, the DCW grid was compared to 30-arc second DTED, which had been
aggregated by averaging.  By aggregating, the comparison could be done at the
30-arc second cell size of the DCW grid.  The comparison was done for
portions of southern Europe and the Mideast, and all of Africa.  Eliminated
from the comparison were those areas of the DCW grid for which supplemental
DTED point control had been included in the gridding process.  If the
averaged DTED are thought of as the reference data set, the RMSE of the DCW
grid is 95 meters.  To get an idea of the overall absolute accuracy of the
DCW grid, the relative error between the DCW and DTED can be combined with
the known error of the DTED itself in a sum of squares.  The root of that sum
of squares is 97 meters.  Using the assumptions about the error distribution
cited above, a RMSE of 97 meters can be expressed as + or - 160 meters linear
error at 90 percent confidence.  This number compares favorably with an
expected vertical accuracy (linear error at 90 percent) of one-half of the
primary contour interval of 1,000 feet (305 meters) for the topographic maps
on which the DCW is based.

The accuracy of the areas of GTOPO30 based on the other sources can only be
estimated based on that which is known about each source.  Using certain
assumptions, the vertical accuracy of each source (and the derived 30-arc
second grid) can be estimated from the contour interval.  One assumption is
that the original map sources meet the commonly used accuracy standard which
states that 90% of the map elevations are within + or - one-half of the
contour interval.  It is unknown if any of these maps actually meet this
standard.  Also, map digitizing and elevation surface interpolation errors
are unknown and therefore not included.  The table below lists the estimated
absolute vertical accuracy for the areas of GTOPO30 derived from each
source, with the method of estimating the accuracy also identified.  The
RMSE numbers were calculated using the assumptions about the error
distribution cited above (a Gaussian distribution with a mean of zero).

            Vertical accuracy (meters)
 Source        L.E. at 90%     RMSE                Estimation method
 ------     --------------------------             -----------------

DTED                30          18         product specification
DCW                160          97         calculated vs. DTED
USGS DEM            30          18         product specification
AMS maps           250         152         estimated from 500-meter interval
IMW maps            50          30         estimated from 100-meter interval
Peru map           500         304         estimated from 1,000-meter interval
N.Z. DEM            15           9         estimated from 100-foot interval
ADD               highly variable          wide range of scales and intervals

Local differences among DEM grid cells are often analyzed to calculate slope
and other land surface parameters.  The relative vertical accuracy (or
point-to-point accuracy on the surface of the elevation model), rather than
the absolute accuracy, determines the quality of such parameters derived from
local differencing operations.  Although not specified for this data set,
for many areas the relative accuracy is probably better than the estimated
absolute accuracy.

8.0  GTOPO30 Caveats

As with all digital geospatial data sets, users of GTOPO30 must be aware of
certain characteristics of the data set (resolution, accuracy, methods of
production and any resulting artifacts, etc.) in order to better judge its
suitability for a specific application.  A characteristic of GTOPO30 that
renders it unsuitable for one application may have no relevance as a
limiting factor for its use in a different application.  Because only the end
user can judge the applicability of the data set, it is the responsibility
of the data producer to describe the characteristics of the data as fully as
possible, so that an informed decision can be made by the user.

8.1  Grid Spacing and Resolution

For any application, the horizontal grid spacing (which limits the
resolution) and the vertical accuracy of GTOPO30 must be considered.  The
30-arc second grid spacing equates to about 1 kilometer, although that
number decreases in the east/west (longitudinal) direction as latitude
increases,  The table below lists the approximate distance covered by 30-arc
seconds at different latitudes.  Thus, at high latitudes there is an
unavoidable redundancy of data in order to keep the 30-arc second spacing
consistent for the global data set.  This is particularly true for the
geographic version of Antarctica where the ground distance for 30-arc seconds
of longitude converges to zero at the South Pole.

Latitude      Ground distance (meters)
(degrees)         E/W        N/S
---------     ------------------------

Equator           928        921
  10              914        922
  20              872        923
  30              804        924
  40              712        925
  50              598        927
  60              465        929
  70              318        930
  73              272        930
  78              193        930
  82              130        931

The variation in ground dimensions for one 30-arc second cell should be
especially considered for any application that measures area of or distance
across a group of cells.  Derivative products, such as slope maps, drainage
basin areas, and stream channel length, will be more reliable if they are
calculated from a DEM that has been first projected from geographic
coordinates to an equal area projection, so that each cell, regardless of
latitude, represents the same ground dimensions and area as every other cell.

Users should maintain the distinction between grid spacing and resolution.
Even though the global data set has a consistent 30-arc second grid spacing,
not all topographic features that one would expect to be resolved at that
spacing will be represented.  The level of detail of the source data
determines whether the 30-arc second sampling interval is truly appropriate
for resolving the important topographic features represented in the source.
Certainly, a 30-arc second grid spacing is appropriate for the areas derived
from higher resolution DEM's (DTED, USGS DEM's, and the New Zealand DEM), and
30-arc seconds has been shown to be suitable as the cell spacing for grids
derived from DCW hypsography (Hutchinson, 1996; Shih and Chiu, 1996).
However, coverage of DCW contours is not complete, and there are areas for
which elevations were interpolated based only on very sparse DCW point data
and/or distant contours.  Small areas of this nature are located in Africa,
South America, and islands of southeast Asia, while Australia and Greenland
contain larger such areas.  Also, the quality of the contours from the ADD
for the interior of Antarctica does not realistically support a 30-arc second
(or even 1-kilometer) grid spacing, although such data are provided for
completeness and consistency of the global product.

8.2  Topographic Detail and Accuracy

Differences in topographic detail among the sources are evident in GTOPO30.
This change in level of topographic information is especially evident at the
boundary between areas derived from DTED and DCW in regions of higher relief.
The mosaicking techniques that were used resulted in a smoothing of the
transition areas, but the change in detail between the two sources remains
very noticeable.  Even if the same topographic feature (ridge, stream valley,
lake, etc.) is represented in the data derived from the two sources, the
elevations across the feature may change somewhat abruptly due to the varying
accuracy of the sources.  Derived products, such as slope maps, for the
source transition areas also emphasize the differences in topographic
information derived from the varying sources.

Users are reminded that the accuracy levels described above are estimates,
and that the accuracy for specific locations within the overall area derived
from any one source can vary from the estimate.  For instance, approximately
30% of the DTED 1-degree by 1-degree tiles (the production and distribution
unit for full resolution DTED) have an absolute vertical accuracy worse than
the product specification of + or - 30 meters at 90% confidence.  Also, the
actual accuracy for some areas derived from the vector contour sources may
be better or worse than the estimate.  When the map source had multiple
contour intervals, the largest interval was used for a conservative estimate.
In contrast, some areas may be worse than the estimate because no contour
coverage was available for those specific locations.

8.3  Production Artifacts

Artifacts due to the production method are apparent in some areas of
GTOPO30.  While the magnitude of the artifacts in a local area are usually
well within the estimated accuracy for the source, users are nonetheless
made aware because the effects are plainly visible and they may affect some
applications of the DEM.  Some areas derived from DTED, especially in Africa
and the Mideast, exhibit a striping artifact, most likely due to the
production method of the DTED.  The artifact is very evident in the full
resolution data, but remains noticeable even in the generalized 30-arc
second version.  Generally, the pattern is more noticeable in low relief
areas, while in higher relief areas it is masked by the actual terrain
variation.  Another pattern seen in some areas derived from DTED is a blocky
appearance, which is reflection of the 1-degree tiling structure of the full
resolution DTED.  These areas derived from contiguous DTED 1-degree tiles
appear blocky because of vertical offsets among the tiles in the original
full resolution DTED.  The artifacts in the DTED areas may or may not be
visible, depending on the method used to display the data.  For instance,
when viewing the DEM data as an image either in shades of gray or color, the
artifacts may be hidden, depending on the number of shades or colors used.
If the data are displayed as a shaded relief image the appearance of the
artifacts will vary depending on the direction of illumination, vertical
exaggeration applied, and the scale of the display.  Generally, none of the
artifacts will be visible on a small scale portrayal of the global data set.

Some production artifacts are also present in the areas derived from the
vector sources.  Small artificial mounds and depressions may be present in
localized areas, particularly where steep topography is adjacent to
relatively level areas, and the hypsography data were sparse.  Additionally,
a "stair step" (or terracing) effect may be seen in profiles of some areas,
where the transition between contour line elevations does not slope
constantly across the area but instead is covered by a flat area with
sharper changes in slope at the locations of the contour lines.  When a
histogram of elevations is presented there are sharp peaks at elevations
that are multiples of the contour interval of the source.  This effect is
common in DEM's produced by gridding of contour data in which the
interpolation process favors elevations at or near the contour values, thus
leading to a greater frequency of those elevations.  Every effort to reduce
these effects has been made by careful selection of parameters for the
interpolation process, but some level of these conditions inevitably remain
due to the nature of vector-to-raster surface generation.

9.0  Summary

GTOPO30 provides a new level of detail in global topographic data.
Previously, the best available global DEM was the ETOPO5 data set, and its
successor TerrainBase, with a horizontal grid spacing of 5-arc minutes
(approximately 10 kilometers) (Row, Hastings, and Dunbar, 1995).  GTOPO30
data are suitable for many regional and continental applications, such as
climate modeling, continental-scale land cover mapping, extraction of
drainage features for hydrologic modeling (Danielson, 1996; Verdin and
Greenlee, 1996), and geometric and atmospheric correction of medium and coarse
resolution satellite image data (Gesch, 1994; Jet Propulsion Laboratory,
1997).

10.0  References

Bliss, N.B., and Olsen, L.M., 1996. Development of a 30-arc-second digital
elevation model of South America. In: Pecora Thirteen, Human Interactions
with the Environment - Perspectives from Space, Sioux Falls, South Dakota,
August 20-22, 1996.

Danielson, J.J., 1996. Delineation of drainage basins from 1 km African
digital elevation data. In: Pecora Thirteen, Human Interactions with the
Environment - Perspectives from Space, Sioux Falls, South Dakota, August
20-22, 1996.

Danko, D.M., 1992. The digital chart of the world. GeoInfo Systems, 2:29-36.

Defense Mapping Agency, 1986. Defense Mapping Agency product specifications
for digital terrain elevation data (DTED) (2d ed.). Defense Mapping Agency
Aerospace Center, St. Louis, Missouri, 26 p.

Defense Mapping Agency, 1990. Digitizing the future (3d ed.). Defense
Mapping Agency, Washington, D.C., 105 p.

Franke, R., 1982. Smooth interpolation of scattered data by local thin plate
splines. Computing & Mathematics with Applications, 8:273-281.

Gesch, D.B., 1994. Topographic data requirements for EOS global change
research. U.S. Geological Survey Open-File Report 94-626, 60 p.

Gesch, D.B., and Larson, K.S., 1996. Techniques for development of global
1-kilometer digital elevation models. In: Pecora Thirteen, Human Interactions
with the Environment - Perspectives from Space, Sioux Falls, South Dakota,
August 20-22, 1996.

Hutchinson, M.F., 1989. A new procedure for gridding elevation and stream
line data with automatic removal of spurious pits. Journal of Hydrology,
106:211-232.

Hutchinson, M.F., 1996. A locally adaptive approach to the interpolation
of digital elevation models. In: Proceedings, Third International
Conference/Workshop on Integrating GIS and Environmental Modeling, Santa
Fe, New Mexico, January 21-26, 1996. National Center for Geographic
Information and Analysis, Santa Barbara, California.

Jet Propulsion Laboratory, 1997. DEM auxiliary datasets preparation plan:
digital elevation mapping support to the EOS/AM1 platform - JPL D13508,
Release 2. Jet Propulsion Laboratory, California Institute of Technology,
Pasadena, California, 65 p.

Larson, K.S., 1996. Error detection and correction of hypsography layers.
In: Proceedings, Sixteenth Annual ESRI User Conference, May 20-24, 1996.
Environmental Systems Research Institute, Inc., Redlands, California.

Row, L.W., Hastings, D.A., and Dunbar, P.K., 1995. TerrainBase Worldwide
Digital Terrain Data - Documentation Manual, CD-ROM Release 1.0. National
Geophysical Data Center, Boulder, Colorado.

Shih, T.Y., and Chiu, Y.C., 1996. On the quality of DCW hypsographic data,
a study for Taiwan. In: Technical Papers, ASPRS/ACSM Annual Convention and
Exhibition, April 22-25, 1996, Baltimore, Maryland. American Society for
Photogrammetry and Remote Sensing, Bethesda, Maryland, Volume III, p. 248-257.

U.S. Geological Survey, 1993. Digital elevation models, data user guide 5.
Reston, Virginia, 50 p.

Verdin, K.L., and Greenlee, S.K., 1996. Development of continental scale
digital elevation models and extraction of hydrographic features. In:
Proceedings, Third International Conference/Workshop on Integrating GIS and
Environmental Modeling, Santa Fe, New Mexico, January 21-26, 1996. National
Center for Geographic Information and Analysis, Santa Barbara, California.

11.0  Disclaimers

Any use of trade, product, or firm names is for descriptive purposes only
and does not imply endorsement by the U.S. Government.

Please note that some U.S. Geological Survey (USGS) information contained
in this data set and documentation may be preliminary in nature and
presented prior to final review and approval by the Director of the USGS.
This information is provided with the understanding that it is not guaranteed
to be correct or complete and conclusions drawn from such information are the
sole responsibility of the user.
