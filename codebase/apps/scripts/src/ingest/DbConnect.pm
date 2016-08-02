package    DbConnect;
require    Exporter;
require 5.002;
@ISA = qw(Exporter);
@EXPORT = qw(connectDb);
#
# This module contains a PERL routine(s) for connecting to the MySQL
# database (name of Db passed in as an argument) and will read the
# username/passwd info from the default MySQL file.
#
# Greg Thompson, RAP, NCAR, Boulder, CO, USA Mar 2003
#
1;
# --------------------------------------------------------------------
# External Libraries needed
    use strict;
    use DBI;
# --------------------------------------------------------------------
# Current user required to choose correct defaults file
    use Env qw(USER HOME);
# --------------------------------------------------------------------

sub connectDb {
    my ($dbName) = @_;
    my ($dbHandle, $dbUser, $dbPass, $dbHost, $dsn, $line);

    $dbUser = undef;
    $dbPass = undef;
    $dbHost = undef;
    
    open (CONFFILE, "$HOME/.my.cnf.$USER");
	while ($line = <CONFFILE>) {
        if ($line =~ /user=(.*)/) { 
            $dbUser = $1;
        }
        if ($line =~ /password=(.*)/) {
            $dbPass = $1;
        }
        if ($line =~ /host=(.*)/) {
            $dbHost = $1;
        }
        if ($line =~ /database=(.*)/) {
            $dbName = $1;
        }
    }
    close (CONFFILE);

    $dsn = "DBI:mysql:$dbName:$dbHost";

    $dbHandle = DBI->connect($dsn, $dbUser, $dbPass, { RaiseError => 1 } );
    return $dbHandle;
}

# --------------------------------------------------------------------
