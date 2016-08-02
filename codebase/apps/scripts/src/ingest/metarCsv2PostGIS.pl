#! /usr/bin/perl
#
# Inserts CSV METAR reports into PostGIS database
#
#  Author:  Padhrig McCarthy, May 2016
#
use Getopt::Std;
use Env qw(ADDSHOME);                    #...Get ADDS project dir
# use MetarIwxxmUsWriter;
use DBI;
use DbConnect;
use StationUtils;
use WMO_CodeLookup;
use Time::Local;
use File::Path qw(make_path);
no locale;

($prog = $0) =~ s%.*/%%;                 # Determine program basename.
umask 002;                               # Set file permissions.
$timeout = 1200;                         # Program will exit if stagnant this long.
our $output_dir = "./";  # default output directory
$dbName = "weather";
$metarTable = "MetarsTest";

#...------------...Usage Info...--------------------------------

$opt_v = $opt_h = $opt_u = $opt_i = $opt_L = $opt_O = 0;
$usage = <<EOF;
Usage: $prog [-hvui] [-L log dir] -O output_dir
  -h  Help:     Print out this usage information.
  -v  Verbose:  Print out verbose debug information (CAUTION).
  -i  insert_wfs:  Use wfs_insert.pl to insert to a WFSRI.
  -u  iwxxm-us: Use IWXXM-US extensions to write the document.
  -L  WXXM log file dir: Specify where the WXXM log file lives.
  -O  output dir: Specify where to output IWXXM-US files.
  expects standard input to be parsed so redirect a file to STDIN if need be.
EOF

#...------------...Sanity check the options...------------------

&getopts('vhuiL:O:') || die $usage;
die $usage if $opt_h;
$verbose = 1 if $opt_v;
$iwxxmus = 1 if $opt_u;
$insert_wfs = 1 if $opt_i;

#...------------...Redirect STDOUT/STDERROR...------------------

if ($opt_L) {
    open (LOG, ">$opt_L/metar2PostGIS.$$.log" ) or die "could not open $opt_L/metar2PostGIS.$$.log: $!\n";
    open (STDERR, ">&STDOUT" ) or die "could not dup stdout: $!\n";
}
if ($opt_O) {
    $output_dir = $opt_O;
}
select( STDERR ); $| = 1;
select( STDOUT ); $| = 1;
select( LOG ); $| = 1;

#...------------...Set interrupt handler...---------------------

$SIG{ 'INT' }   = 'atexit';
$SIG{ 'KILL' }  = 'atexit';
$SIG{ 'TERM' }  = 'atexit';
$SIG{ 'QUIT' }  = 'atexit';


#...------------...Keep STDIN attached unless timeout...----

    print LOG "Opening standard input ... \n" if $verbose;
    open (STDIN, '-');
    binmode (STDIN);
    vec($rin,fileno(STDIN),1) = 1;
    $nfound = select ( $rout = $rin, undef, undef, $timeout );

    &atexit("timeout", $dbh) if( ! $nfound );
    &atexit("eof", $dbh) if ( eof(STDIN) );
    
    $_ = <STDIN>;
    
   # Prepare PostGIS Insert
   $dbname = "css-wx";
   $host = "nnew-vm9";
   $port = "5432";
   $username = "nnew";
   $password = "";
   $options = "";

   $dbh = DBI->connect("dbi:Pg:dbname=$dbname;host=$host;port=$port;", "$username", "$password");
   #my @tables = $dbh->tables('','','','TABLE');
   #   if (@tables) {
   #      for (@tables) {
   #         next unless $_;
   #         print LOG "Table: $_\n";
   #      }
   #   }
   #my $table = $dbh->selectall_arrayref('SELECT * FROM "METAR"');
   #if (@$table) {
   #   foreach my $row (@$table) {
   #      print LOG "Row:\n";
   #      print LOG "$row->[0]\n";
   #      print LOG "$row->[1] $row->[2]\n";
   #      print LOG "$row->[3]\n";
   #      print LOG "$row->[4]\n";
   #      print LOG "$row->[5]\n";
   #      print LOG "$row->[6]\n";
   #      print LOG "$row->[7]\n";
   #      print LOG "\n";
   #   }
   #}

   $sth = $dbh->prepare(
      'INSERT INTO "METAR_OBS"("icaoId",
                               "the_geom",
                               "observationTime",
                               "issuanceTime",
                               "temperature_c",
                               "dewPoint_c",
                               "windDirection_deg",
                               "windSpeed_kn",
                               "windGust_kn",
                               "visibility_mi",
                               "altimeter_inmg",
                               "seaLevelPressure_hpa",
                               "weather",
                               "skyConditionAmount1",
                               "skyConditionAmount2",
                               "skyConditionAmount3",
                               "skyConditionAmount4",
                               "skyConditionAmount5",
                               "skyConditionAmount6",
                               "skyConditionHeight1_100ft",
                               "skyConditionHeight2_100ft",
                               "skyConditionHeight3_100ft",
                               "skyConditionHeight4_100ft",
                               "skyConditionHeight5_100ft",
                               "skyConditionHeight6_100ft",
                               "pressureTendency",
                               "maxTemperature6Hr_c",
                               "minTemperature6Hr_c",
                               "maxTemperature24Hr_c",
                               "minTemperature24Hr_c",
                               "precip1Hr_in",
                               "precip3Hr_in",
                               "precip6Hr_in",
                               "precip24Hr_in",
                               "verticalVisibility_100ft",
                               "type",
                               "rawText",
                               "remarks",
                               "skyConditionType1",
                               "skyConditionType2",
                               "skyConditionType3",
                               "skyConditionType4",
                               "skyConditionType5",
                               "skyConditionType6"
                              )
       values (?, ST_GeomFromText(?, 4326), ?, ?, ?, ?, ?, ?, ?, ?,
               ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,
               ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,
               ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,
               ?, ?, ?, ?
              )'
   ) or die "Could not prepare statement.\n" . $dbh->errstr() . "\n";

    while( <STDIN> ) {
      $currentMetar = $_;
      #Skip the line containing the CONFIDENCE/QC values   
      #Also skip empty lines, these newlines delineate one METAR "batch" from another
      $isConfidence = (m/^CONFIDENCE/i );
      $isHeader = ( m/^headerDescriptor/i );
      $isEmptyLine = (m/^\s*\n/);
      $isEof = (m/^eof on STDIN --shutting down\n/);
      if( !$isConfidence  && !$isHeader && !$isEmptyLine && !$isEof ){
         print LOG "Current METAR: '$currentMetar'\n";
         &insertPostGIS($currentMetar, $sth);
      }
    }


#........................................................................#
#Insert CSV into GeoServer database
# Query to database needed to obtain the lon,lat of the METAR station
sub insertPostGIS{
   my ($metarCsvEntry, $sth) = @_; 
   my $curTime = time();
   my $reportTime;
   my $reportTimeIso;
   my $reportTimestamp;
   my $reportLongTimestamp;
   my $decodeTimeIso;
   my $decodeTimestamp;
   my $decodeLongTimestamp;
   my $stnLon;
   my $stnLat;
   my $stnElev;
   my $stnName;
   print LOG "Inserting CSV into PostGIS, current time: $curTime\n";

   @metarTokens = split(/,/, $metarCsvEntry);
   $headerDescriptor = $metarTokens[0];

   print LOG "Setting METAR tokens from csv file ... \n" if $verbose;

   #Assigne NULL to any empty fields
   $icaoId = $metarTokens[1];
   $decodeTime = $metarTokens[2];
   $reportTimeStr = $metarTokens[3];
   $temp = $metarTokens[4];
   $dewp = $metarTokens[5];
   $wdir = $metarTokens[6];
   $wspd = $metarTokens[7];
   $wgst = $metarTokens[8];
   $visib = $metarTokens[9];

   if ( $reportTimeStr eq q{}) {
       return;
   }
   
   if( $metarTokens[10] eq q{} ){
      $altim = "NULL";
   }else{
      $altim = $metarTokens[10];
   }

   $slp = $metarTokens[11];
   $qcField = $metarTokens[12];
   $wxString = $metarTokens[13];
   
   if( $metarTokens[14] eq q{} ){
      $cldCvg1 = "NULL";
   }else{
      $cldCvg1 = $metarTokens[14];   
   }

   if( $metarTokens[15] eq q{}){
      $cldCvg2 = "NULL";
   }else{
      $cldCvg2 = $metarTokens[15];   
   }

   if( $metarTokens[16] eq q{}){
      $cldCvg3 = "NULL";
   }else{
      $cldCvg3 = $metarTokens[16];   
   }

   if( $metarTokens[17] eq q{}){
      $cldCvg4 = "NULL";
   }else{
      $cldCvg4 = $metarTokens[17];   
   }
   
    if( $metarTokens[18] eq q{}){
      $cldCvg5 = "NULL";
   }else{
      $cldCvg5 = $metarTokens[18];   
   }
   
    if( $metarTokens[19] eq q{}){
      $cldCvg6 = "NULL";
   }else{
      $cldCvg6 = $metarTokens[19];   
   }

   if( $metarTokens[20] eq q{}){
      $cldBas1 = "NULL";
   }else{
      $cldBas1 = $metarTokens[20];
   }

   if( $metarTokens[21] eq q{}){
      $cldBas2 = "NULL";
   }else{
      $cldBas2 = $metarTokens[21];
   }

   if( $metarTokens[22] eq q{}){
      $cldBas3 = "NULL";
   }else{
      $cldBas3 = $metarTokens[22];
   }

   if( $metarTokens[23] eq q{}){
      $cldBas4 = "NULL";
   }else{
      $cldBas4 = $metarTokens[23];
   }
   
   if( $metarTokens[24] eq q{}){
      $cldBas5 = "NULL";
   }else{
      $cldBas5 = $metarTokens[24];
   }
   
   if( $metarTokens[25] eq q{}){
      $cldBas6 = "NULL";
   }else{
      $cldBas6 = $metarTokens[25];
   }

   $presTend = $metarTokens[26];
   $maxT = $metarTokens[27];
   $minT = $metarTokens[28];
   $maxT24 = $metarTokens[29];
   $minT24 = $metarTokens[30];
   $precip = $metarTokens[31];
   $pcp3hr = $metarTokens[32];
   $pcp6hr = $metarTokens[33];
   $pcp24hr = $metarTokens[34];
   $snow = $metarTokens[35];
   if( $metarTokens[36] eq q{} ){
      $vertVis = "NULL";
   }else{
      $vertVis = $metarTokens[37];
   }
   $metarType = $metarTokens[38];
   $rawOb = $metarTokens[39];
   $remarks = $metarTokens[40];

   foreach $element (@metarTokens){
      print LOG "$element ";
   }
   print LOG "\n";


   # Convert the reportTimeStr from metar format to a time and into a more readable format
   my ($reportYr, $reportMon, $reportDay, $reportHr, $reportMin, $reportSec) = $reportTimeStr =~ /(\d{4})-(\d{2})-(\d{2}) ([0-2][0-9]):(\d{2}):(\d{2})/;
   $reportTime = timegm($reportSec, $reportMin, $reportHr, $reportDay, $reportMon-1, $reportYr-1900);
   $reportTimeIso = sprintf("%04d-%02d-%02dT%02d:%02d:%02dZ", $reportYr, $reportMon, $reportDay, $reportHr, $reportMin, $reportSec);
   $reportTimestamp = sprintf("%04d%02d%02d%02d%02dZ", $reportYr, $reportMon, $reportDay, $reportHr, $reportMin);
   $reportLongTimestamp = sprintf("%04d%02d%02dT%02d%02d%02dZ", $reportYr, $reportMon, $reportDay, $reportHr, $reportMin, $reportSec);

   print LOG "Report time str: $reportTimeStr. Hr: $reportHr, Min: $reportMin\n";
   print LOG "Report time ISO str: $reportTimeIso. Hr: $reportHr, Min: $reportMin\n";
 
   # Convert the decodeTime from Unix/epoch time to a more readable format
   my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=gmtime($decodeTime);
   $decodeTimeIso = sprintf("%04d-%02d-%02dT%02d:%02d:%02dZ", $year+1900, $mon+1, $mday,$hour,$min,$sec);
   $decodeTimestamp = sprintf("%04d%02d%02d%02d%02dZ", $year+1900, $mon+1, $mday,$hour,$min);
   $decodeLongTimestamp = sprintf("%04d%02d%02dT%02d%02d%02dZ", $year+1900, $mon+1, $mday,$hour,$min,$sec);
 
   print LOG "Decode time: $decodeTimeIso. Hr: $hour, Min: $min\n";

   #Generate file name
   my $range = 1000;
   my $randNum = int(rand($range));
   $identifier = $metarType . "_" . $curTime . "_" . $randNum;
   print LOG "for $icaoId, identifier = $identifier\n";

   # Determine if automatic station -- qcField contains a 2
   my $isAutoString = 'false';
   if ( $qcField & 2 ) {
      $isAutoString = 'true';
   }

   # Determine status -- qcField contains a 1 if it's corrected
   my $statusString = 'NORMAL';
   if ( $qcField & 1 ) {
      $statusString = 'CORRECTED';
   }
   print LOG "\n\n\nqcField: $qcField. isAuto: $isAutoString. status: $statusString\n\n\n";

   my $encodeType = 'iwxxm';
   if ( $icaoId =~ /[HKP][0-9A-Z]{3}/) {
      $iwxxmus = 1;
      $encodeType = 'iwxxm-us';
   }
   else {
      $iwxxmus = 0;
      $encodeType = 'iwxxm';
   }

   my ($secR, $minR, $hourR, $mdayR, $monR, $yearR, $wdayR, $ydayR, $isdstR) = gmtime($reportTime);
   my $dateR = sprintf("%04d%02d%02d", $yearR+1900, $monR+1, $mdayR);
   my $pathR = sprintf("%s/%s/%s/%02d", $output_dir, $encodeType, $dateR, $hourR);
   # ZOOP! make_path($pathR);
   my $file = sprintf("%s/%s_%02d%02d_%s", $pathR, $dateR, $hourR, $minR, $icaoId);

   if( -e $file.$encodeType ) {
       return;
   }

   #Surround text strings with "
   $cldCvg1 =~ s/"//g;
   $cldCvg2 =~ s/"//g;
   $cldCvg3 =~ s/"//g;
   $cldCvg4 =~ s/"//g;
   $cldCvg5 =~ s/"//g;
   $cldCvg6 =~ s/"//g;
		
   #Use the WXXM descriptors for cloud coverage
   $cldCvg1 =~ s/SCT/SCATTERED-BROKEN/g;
   $cldCvg2 =~ s/SCT/SCATTERED-BROKEN/g;
   $cldCvg3 =~ s/SCT/SCATTERED-BROKEN/g;
   $cldCvg4 =~ s/SCT/SCATTERED-BROKEN/g;
   $cldCvg5 =~ s/SCT/SCATTERED-BROKEN/g;
   $cldCvg6 =~ s/SCT/SCATTERED-BROKEN/g;
  
   $cldCvg1 =~ s/OVC/OVERCAST/g;
   $cldCvg2 =~ s/OVC/OVERCAST/g;
   $cldCvg3 =~ s/OVC/OVERCAST/g;
   $cldCvg4 =~ s/OVC/OVERCAST/g;
   $cldCvg5 =~ s/OVC/OVERCAST/g;
   $cldCvg6 =~ s/OVC/OVERCAST/g;
		
   $cldCvg1 =~ s/OVX/OVERCAST/g;
   $cldCvg2 =~ s/OVX/OVERCAST/g;
   $cldCvg3 =~ s/OVX/OVERCAST/g;
   $cldCvg4 =~ s/OVX/OVERCAST/g;
   $cldCvg5 =~ s/OVX/OVERCAST/g;
   $cldCvg6 =~ s/OVX/OVERCAST/g;
		
   $cldCvg1 =~ s/BKN/BROKEN/g;
   $cldCvg2 =~ s/BKN/BROKEN/g;
   $cldCvg3 =~ s/BKN/BROKEN/g;
   $cldCvg4 =~ s/BKN/BROKEN/g;
   $cldCvg5 =~ s/BKN/BROKEN/g;
   $cldCvg6 =~ s/BKN/BROKEN/g;
		
   $cldCvg1 =~ s/CLR/CLEAR/g;
   $cldCvg2 =~ s/CLR/CLEAR/g;
   $cldCvg3 =~ s/CLR/CLEAR/g;
   $cldCvg4 =~ s/CLR/CLEAR/g;
   $cldCvg5 =~ s/CLR/CLEAR/g;
   $cldCvg6 =~ s/CLR/CLEAR/g;
		
   $cldCvg1 =~ s/SKC/CLEAR/g;
   $cldCvg2 =~ s/SKC/CLEAR/g;
   $cldCvg3 =~ s/SKC/CLEAR/g;
   $cldCvg4 =~ s/SKC/CLEAR/g;
   $cldCvg5 =~ s/CLR/CLEAR/g;
   $cldCvg6 =~ s/CLR/CLEAR/g;
		
   #$cldCvg1 =~ s/CAVOK/CLEAR/g; #CAVOK is inserted in cldCvg1 by the decoder to show actual CAVOK in the report.
   $cldCvg2 =~ s/CAVOK/CLEAR/g;
   $cldCvg3 =~ s/CAVOK/CLEAR/g;
   $cldCvg4 =~ s/CAVOK/CLEAR/g;
   $cldCvg5 =~ s/CAVOK/CLEAR/g;
   $cldCvg6 =~ s/CAVOK/CLEAR/g;

 
   #retreive the lon,lat for this station from the MySQL database
   #$dbh = &connectDb($dbName);   		
   #$sql = qq{ select lon, lat,elev from StationInfo WHERE icaoId = "$icaoId"};
   #$sth = $dbh->prepare($sql);
   #$rv = $sth->execute;
   #$rc = $sth->bind_columns(undef, \$lon,\$lat,\$elev);
   #$sth->fetch();
   #$sth->finish();

   my $stnArray;
   my $stnHash;
   my %stns = &getStations($icaoId,
                           # "METAR airport", # original
                           "METAR TAF SA FT FA FD SD ASOS ASOS-D ASOS-C ASOS-B ASOS-A AWSS AWOS AWOS-3 AWOS-3P/T SPECI SYNS CWA AIRPORT *", # from metarCsv2Mysql
                           "icao_code location_code lon lat elevation site_name");
   if( ( length %stns ) > 1 ) {
      print LOG "Found matches for $icaoId:\n";
      if ( $verbose ) {
         for $stnArray (values %stns) {
            for $stnHash (@$stnArray) {
               %stnHash = %$stnHash;
               print LOG "icao: $stnHash{icao_code} | location: $stnHash{location_code} | name: $stnHash{site_name} | lat: $stnHash{lat} | lon: $stnHash{lon} | elev: $stnHash{elevation}\n";
            }
         }
      }

      $stnLon = $stns{$icaoId}[0]{lon};
      $stnLat = $stns{$icaoId}[0]{lat};
      $stnElev = $stns{$icaoId}[0]{elevation};
      $stnName = $stns{$icaoId}[0]{site_name};
   } else {
      print LOG "\nWARNING WARNING WARNING\nNo potential AeroStation matches found for $icaoId:\nWARNING WARNING WARNING\n\n";
   }

   print LOG "Fetched station $icaoId from DB. Found LAT = $stnLat; LON = $stnLon; ELEV = $stnElev; NAME = $stnName\n" if $verbose;

   #replaced airPres => $presTend with airPres => $slp
   #pressure tendency indicates whether the pressure increases or decreases within a particular amount of time
   my %obs = (
        icao => $icaoId,
	windSpeed => $wspd,
	windDir => $wdir,
	temp => $temp,
	dewPt => $dewp,
	airPres => $altim,
	vertVis => $vertVis,
	minVis => $visib,
	cldCvg1 => $cldCvg1,
	cldCvg2 => $cldCvg2,
	cldCvg3 => $cldCvg3,
	cldCvg4 => $cldCvg4,
	cldCvg5 => $cldCvg5,
	cldCvg6 => $cldCvg6,
	cldBas1 => $cldBas1,
	cldBas2 => $cldBas2,
	cldBas3 => $cldBas3,
	cldBas4 => $cldBas4,
	cldBas5 => $cldBas5,
	cldBas6 => $cldBas6,
        qcField => $qcField,
        wxString => $wxString
	);

print LOG "Cloud cvg 1: $cldCvg1.\n";
my $lin = WMO_CodeLookup->lookupCldCvgLink($cldCvg1);
my $tit = WMO_CodeLookup->lookupCldCvgTitle($cldCvg1);
print LOG "Cloud cvg 1 link: $lin title: $tit\n";

   if( ($stnLon eq q{} || !defined($stnLon) ) ||  ($stnLat eq q{} || !defined($stnLat) ) ) {
	#$polyString = "-0.1 89.9, 0.1  90.1, -0.1 89.9";
	print LOG "found no LAT or LON, setting to default\n" if $verbose;
	$stnLat="89.9";
	$stnLon="-0.1";
   }
   if( $stnElev eq q{} ) {
	print LOG "found no ELEVATION setting to default\n" if $verbose ;
	$stnElev=0;
   }
   if( $stnName eq q{} ) {
	print LOG "found no STATION_NAME setting to default\n" if $verbose ;
	$stnName='UNKNOWN';
   }

   my $point = 'POINT(' . $stnLon . ' ' . $stnLat . ')';

                 if ( $icaoId eq "" )        {$icaoId = undef;}
                 if ( $point eq "" )         {$point = undef;}
                 if ( $decodeTimeIso eq "" ) {$decodeTimeIso = undef;}
                 if ( $reportTimeIso eq "" ) {$reportTimeIso = undef;}
                 if ( $temp eq "" || $temp eq "NULL" )             {$temp = undef;}
                 if ( $dewp eq "" || $dewp eq "NULL" )             {$dewp = undef;}
                 if ( $wdir eq "" || $wdir eq "NULL" )             {$wdir = undef;}
                 if ( $wspd eq "" || $wspd eq "NULL" )             {$wspd = undef;}
                 if ( $wgst eq "" || $wgst eq "NULL" )             {$wgst = undef;}
                 if ( $visib eq "" || $visib eq "NULL")            {$visib = undef;}
                 if ( $altim eq "" || $altim eq "NULL" )           {$altim = undef;}
                 if ( $slp eq "" || $slp eq "NULL" )               {$slp = undef;}
                 if ( $qcField eq "" || $qcField eq "NULL" )       {$qcField = undef;}
                 if ( $wxString eq "" || $wxString eq "NULL" )     {$wsString = undef;}
                 if ( $cldCvg1 eq "" || $cldCvg1 eq "NULL" )       {$cldCvg1 = undef;}
                 if ( $cldCvg2 eq "" || $cldCvg2 eq "NULL" )       {$cldCvg2 = undef;}
                 if ( $cldCvg3 eq "" || $cldCvg3 eq "NULL" )       {$cldCvg3 = undef;}
                 if ( $cldCvg4 eq "" || $cldCvg4 eq "NULL" )       {$cldCvg4 = undef;}
                 if ( $cldCvg5 eq "" || $cldCvg5 eq "NULL" )       {$cldCvg5 = undef;}
                 if ( $cldCvg6 eq "" || $cldCvg6 eq "NULL" )       {$cldCvg6 = undef;}
                 if ( $cldBas1 eq "" || $cldBas1 eq "NULL" )       {$cldBas1 = undef;}
                 if ( $cldBas2 eq "" || $cldBas2 eq "NULL" )       {$cldBas2 = undef;}
                 if ( $cldBas3 eq "" || $cldBas3 eq "NULL" )       {$cldBas3 = undef;}
                 if ( $cldBas4 eq "" || $cldBas4 eq "NULL" )       {$cldBas4 = undef;}
                 if ( $cldBas5 eq "" || $cldBas5 eq "NULL" )       {$cldBas5 = undef;}
                 if ( $cldBas6 eq "" || $cldBas6 eq "NULL" )       {$cldBas6 = undef;}
                 if ( $presTend eq "" )      {$presTend = undef;}
                 if ( $maxT eq "" )          {$maxT = undef;}
                 if ( $minT eq "" )          {$minT = undef;}
                 if ( $maxT24 eq "" )        {$maxT24 = undef;}
                 if ( $minT24 eq "" )        {$minT24 = undef;}
                 if ( $precip eq "" )        {$precip = undef;}
                 if ( $pcp3hr eq "" )        {$pcp3hr = undef;}
                 if ( $pcp6hr eq "" )        {$pcp6hr = undef;}
                 if ( $pcp24hr eq "" )       {$pcp24hr = undef;}
                 if ( $snow eq "" )          {$snow = undef;}
                 if ( $vertVis eq "" || $vertVis eq "NULL" )       {$vertVis = undef;}
                 if ( $metarType eq "" )     {$metarType = undef;}
                 if ( $rawOb eq "" )         {$rawOb = undef;}
                 if ( $mostRecent eq "" )    {$mostRecent = undef;}
                 if ( $hasRemark eq "" )     {$hasRemark = undef;}

                 print LOG "icaoId: $icaoId.\n" if $verbose;
                 print LOG "point: $point.\n" if $verbose;
                 print LOG "decodeTimeIso: $decodeTimeIso.\n" if $verbose;
                 print LOG "reportTimeIso: $reportTimeIso.\n" if $verbose;
                 print LOG "temp: $temp.\n" if $verbose;
                 print LOG "dewp: $dewp.\n" if $verbose;
                 print LOG "wdir: $wdir.\n" if $verbose;
                 print LOG "wspd: $wspd.\n" if $verbose;
                 print LOG "wgst: $wgst.\n" if $verbose;
                 print LOG "visib: $visib.\n" if $verbose;
                 print LOG "altim: $altim.\n" if $verbose;
                 print LOG "slp: $slp.\n" if $verbose;
                 print LOG "qcField: $qcField.\n" if $verbose;
                 print LOG "wxString: $wxString.\n" if $verbose;
                 print LOG "cldCvg1: $cldCvg1.\n" if $verbose;
                 print LOG "cldCvg2: $cldCvg2.\n" if $verbose;
                 print LOG "cldCvg3: $cldCvg3.\n" if $verbose;
                 print LOG "cldCvg4: $cldCvg4.\n" if $verbose;
                 print LOG "cldCvg5: $cldCvg5.\n" if $verbose;
                 print LOG "cldCvg6: $cldCvg6.\n" if $verbose;
                 print LOG "cldBas1: $cldBas1.\n" if $verbose;
                 print LOG "cldBas2 $cldBas2.\n" if $verbose;
                 print LOG "cldBas3: $cldBas3.\n" if $verbose;
                 print LOG "cldBas4: $cldBas4.\n" if $verbose;
                 print LOG "cldBas5: $cldBas5.\n" if $verbose;
                 print LOG "cldBas6: $cldBas6.\n" if $verbose;
                 print LOG "presTend: $presTend.\n" if $verbose;
                 print LOG "maxT: $maxT.\n" if $verbose;
                 print LOG "minT: $minT.\n" if $verbose;
                 print LOG "maxT24: $maxT24.\n" if $verbose;
                 print LOG "minT24: $minT24.\n" if $verbose;
                 print LOG "precip: $precip.\n" if $verbose;
                 print LOG "precip3hr: $pcp3hr.\n" if $verbose;
                 print LOG "precip6hr: $pcp6hr.\n" if $verbose;
                 print LOG "precip24hr: $pcp24hr.\n" if $verbose;
                 print LOG "snow: $snow.\n" if $verbose;
                 print LOG "vertVis: $vertVis.\n" if $verbose;
                 print LOG "metarType: $metarType.\n" if $verbose;
                 print LOG "rawOb: $rawOb.\n" if $verbose;
                 print LOG "mostRecent: false.\n" if $verbose;
                 print LOG "hasRemark: " . ($remarks.length > 0 ? true : false) . ".\n" if $verbose;

   $sth->execute($icaoId,
                 $point,
                 $decodeTimeIso,
                 $reportTimeIso, 
                 $temp,
                 $dewp,
                 $wdir,
                 $wspd,
                 $wgst,
                 $visib,
                 $altim,
                 $slp,
                 $wxString,
                 $cldCvg1,
                 $cldCvg2,
                 $cldCvg3,
                 $cldCvg4,
                 $cldCvg5,
                 $cldCvg6,
                 $cldBas1,
                 $cldBas2,
                 $cldBas3,
                 $cldBas4,
                 $cldBas5,
                 $cldBas6,
                 $presTend,
                 $maxT,
                 $minT,
                 $maxT24,
                 $minT24,
                 $precip,
                 $pcp3hr,
                 $pcp6hr,
                 $pcp24hr,
                 $vertVis,
                 $metarType,
                 $rawOb,
                 ($remarks.length > 0 ? remarks : undef),
                 undef, # skyConditionType1
                 undef, # skyConditionType2
                 undef, # skyConditionType3
                 undef, # skyConditionType4
                 undef, # skyConditionType5
                 undef  # skyConditionType6
   ) or die "Could not execute SQL.\n" . $sth->errstr() . "\n";

   undef($curTime);
   undef($identifier);
}
#........................................................................#

sub atexit {

    local($sig, $dbHandle) = @_ ;

    if ( $sig eq "eof" ) {
        print STDERR "\neof on STDIN --shutting down\n\n" ;
    } elsif ( $sig eq "timeout" ) {
        print STDERR ("$prog shutting down, timeout = ", $timeout/60, " minutes\n");
    } elsif ( defined($sig) ) {
        print STDERR "\nCaught SIG$sig --shutting down\n\n" ;
    }

    close (STDOUT);
    close (LOG);
    close (STDERR);
  
    $dbHandle ->disconnect if ( defined($dbHandle) );


    exit (0);
}

#........................................................................#
__END__
