#! /usr/bin/perl
#
# get_debian_version_name.pl
#
# Gets the nickname of the Debian version
# e.g sarge, etch etc.
#
#-------------------------------------------------------------------

use Env;
use Cwd;
Env::import();

$version_num = `cat /etc/debian_version`;
chop($version_num);

if ($version_num > 7.0) {
  print STDOUT "sid";
} elsif ($version_num == 7.0) {
  print STDOUT "wheezy";
} elsif ($version_num == 6.0) {
  print STDOUT "squeeze";
} elsif ($version_num == 5.0) {
  print STDOUT "lenny";
} elsif ($version_num == 4.0) {
  print STDOUT "etch";
} elsif ($version_num == 3.1) {
  print STDOUT "sarge";
} elsif ($version_num == 3.0) {
  print STDOUT "woody";
} elsif ($version_num == 2.2) {
  print STDOUT "potato";
} elsif ($version_num == 2.1) {
  print STDOUT "slink";
} elsif ($version_num == 2.0) {
  print STDOUT "hamm";
} else {
  print STDOUT "unknown";
}
