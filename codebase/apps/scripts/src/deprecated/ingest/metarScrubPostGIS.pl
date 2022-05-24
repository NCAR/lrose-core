#! /usr/bin/perl
#
# Scrubs METAR reports from PostGIS database
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

   # DELETE FROM "METAR_OBS" WHERE "observationTime" < NOW() - INTERVAL '48 hours';
   $sth = $dbh->prepare(
      'DELETE FROM "METAR_OBS" WHERE "observationTime" < NOW() - INTERVAL \'48 hours\';'
   ) or die "Could not prepare DELETE statement.\n" . $dbh->errstr() . "\n";

   $sth->execute() or die "Could not execute SQL.\n" . $sth->errstr() . "\n";

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
