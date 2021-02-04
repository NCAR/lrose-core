#!/usr/bin/perl
# Author:  Jason Craig  Dec 2015

use strict;

if($#ARGV < 0) {
  print "Usage: create_build_track_table.pl track_file1.txt track_file2.txt .. track_fileN.txt\n";
}
my $numtracks = $#ARGV +1;

my %machine_descriptions = (
    "k5"  => "RedHat 5 32bit",
    "k6"  => "",
    "k7"  => "",
    "k8"  => "RedHat 5 64bit",
    "k9"  => "Debian wheezy 64bit",
    "k10" => "Debian wheezy 32bit",
    "k11"  => "Debian jessie 64bit",
    "k12"  => "CentOS6.7 (RedHat6)",
    "k12.rap.ucar.edu"  => "CentOS6.7 (RedHat6)",
    "k13"  => "CentOS7.2 (RedHat7)",
    "k13.rap.ucar.edu"  => "CentOS7.2 (RedHat7)",
    );

my %table;
my $filenum = 0;
my @macs = ();
my @oss = ();
my @days = ();
while($#ARGV >= 0)
{
  my $trackfile = shift(@ARGV);
  open TRACKFILE, "<$trackfile" or die $!;
  my $mac = <TRACKFILE>;
  my $os = <TRACKFILE>;
  my $day = <TRACKFILE>;
  chomp($mac);
  chomp($os);
  chomp($day);
  if($filenum > 0) {
     foreach my $key ( keys %table ) {
	$table{$key}[$filenum] = 'pass';
     }
  }
  $mac =~ s/.rap.ucar.edu//;
  push(@macs, $mac);
  push(@oss, $os);
  push(@days, $day);
  foreach my $line (<TRACKFILE>) 
  {
    chomp($line);
    my $isfailing = 0;
    my ( $fail, $date, $time) = split(", ", $line);
    if($fail ne '')
    {
      if($filenum == 0) {
	$table{$fail} = [$date];
      } else {
	  
	if(!exists $table{$fail}) {
	  $table{$fail} = ['pass'];
	  for (my $i=1; $i < $filenum; $i++) {
	    push( $table{$fail}, 'pass');
	  }
	}
	$table{$fail}[$filenum] = $date;
      }
    }
  }
  close TRACKFILE;
  $filenum ++;
}

my $html_header = <<'END_MESSAGE';
<HTML>
<HEAD>
<TITLE>RAP Nightly Build Failures Tracker</TITLE>
</HEAD>

<BODY BGCOLOR="white">

<H1>RAP Nightly Build Failures Tracker</H1>
<P>
<H4>Red dates are when the app/lib began failing to build.  Bold app/libs are not compiling on any os.</H4>
<P>
 <table border="1">
END_MESSAGE
print $html_header;
print "  <col width=\"300\">\n";
for (my $i=1; $i <= $filenum; $i++) {
    print "  <col width=\"215\">\n";
}
print "  <tr>\n    <th align=\"right\">Operating System:</th>\n";
foreach (@macs) {
    print "    <th>" . $machine_descriptions{$_} . "</th>\n";
}
print "  </tr>\n";
print "  <tr>\n    <th align=\"right\">Build Host:</th>\n";
print "    <th>" . join("</th>\n    <th>", @macs ) . "</th>\n";
print "  </tr>\n";
print "  <tr>\n    <th align=\"right\">HOST_OS:</th>\n";
print "    <th>" . join("</th>\n    <th>", @oss ) . "</th>\n";
print "  </tr>\n";
print "  <tr>\n    <th align=\"right\">Last nightly build:</th>\n";
print "    <th>" . join("</th>\n    <th>", @days ) . "</th>\n";
print "  </tr>\n";

foreach my $key (sort my_sort keys %table ) {
  my $pass = 0;
  foreach (@{$table{$key}}) {
    if($_ eq "pass") {
	$pass = 1;
    }
  }
  print "  <tr>\n";
  if($pass == 1) {
      print "    <td align=\"right\">$key</td>\n";
  } else {
      print "    <td align=\"right\"><b>$key</b></td>\n";
  }

  foreach (@{$table{$key}}) {
    if($_ eq "pass") {
      print "    <td bgcolor=\"LightGreen\"></td>\n";
    } else {
	print "    <td bgcolor=\"LightCoral\" align=\"center\">$_</td>\n"
    }
  }
  print "  </tr>\n";
}
print "  </table>\n\n</BODY>\n</HTML>\n";

sub my_sort {
  if(index($a, "lib") == 0 && index($b, "lib") == 0) {
      return $a cmp $b;
  } else {
      return -1 if(index($a, "lib") == 0);
      return 1 if(index($b, "lib") == 0);
      return $a cmp $b;
  }
}
