#! /usr/bin/perl

use Getopt::Std;

%icaos, %stations, %icaoOnly;

$icaoId = "icaoId";
$latName = "latitude";
$lonName = "longitude";
$altName = "altitude_m";

sub findIcao{
    $station = shift;
    if(exists $icaos{$station})
    {
	$iata = $icaos{$station}{'station'};
	$idx = $icaos{$station}{'index'};
        return $stations{$iata}[$idx];
    }
    elsif(exists $icaoOnly{$station})
    {
	return $icaoOnly{$station};
    }
    else
    {
	return NULL;
    }
}

sub findIata{ # not used currently
    $station = shift;

    if(exists $stations{$station})
    {
	return $stations{$station};
    }
    else
    {
	return NULL;
    }

}
###################################################################
my $filename;
&getopts('s:');
if($opt_s) {
    $fileName = $opt_s;
}else {
  $fileName = "stations.txt";
}
open(FILE, $fileName) or die "Cannot open file";
@data = <FILE>;
close(FILE);

foreach(@data)
{
  $line = $_;
  if(length($line) == 84)
  {
    $icao = substr $line, 20, 4;
    $iata = substr $line, 26, 3;
    $latdeg = substr($line, 39, 2);
    $latmin = substr($line,42,2)/60;
    $lat = sprintf "%.2f", ($latdeg+$latmin); # remove whitespace from beginning??
    $londeg = substr($line, 47, 3);
    $lonmin = substr($line,51,2)/60;
    $lon = sprintf "%.2f", ($londeg+$lonmin);
    $elev = substr $line, 55, 4;
    $elev =~ s/^\s+//; # remove whitespace from beginning
    if(substr($line, 53, 1) eq "W")
    {
	    $lon = 0 - $lon;
    }

    if(substr($line, 44, 1) eq "S")
    {
	    $lat = 0 - $lat;
    }

    if($icao ne "    ") # has icao
    {
      if($iata ne "   ") # has icao and iata
      {
        if(exists $stations{$iata})
        {
          $size = scalar @{$stations{$iata}};
          $stations{$iata}[$size]{"lat"} = $lat;
          $stations{$iata}[$size]{"lon"} = $lon;
          $stations{$iata}[$size]{"elev"} = $elev;
          $icaos{$icao}{'station'} = $iata;
          $icaos{$icao}{'index'} = $size;
		    }
		    else
        {
          $stations{$iata}[0]{"lat"} = $lat;
          $stations{$iata}[0]{"lon"} = $lon;
          $stations{$iata}[0]{"elev"} = $elev;
          $icaos{$icao}{'station'} = $iata;
          $icaos{$icao}{'index'} = 0;
		    }
	    } 
      else # has icao but no iata
      {
		    if(not exists $icaoOnly{$icao})
        {
          $icaoOnly{$icao}{"lat"} = $lat;
          $icaoOnly{$icao}{"lon"} = $lon;
          $icaoOnly{$icao}{"elev"} = $elev;
		    }
      }
	  }    
  }
}

#if($#ARGV != 0)
#{
#		print "Must supply 1 filename on command line\n";
#		exit();
#}
#$fileName = $ARGV[0];

#...------------...Keep STDIN attached unless timeout...----

print "Opening standard input ... \n" if $verbose;
open (STDIN, '-');
binmode (STDIN);
vec($rin,fileno(STDIN),1) = 1;
$nfound = select ($rin, undef, undef, $timeout );
   
&atexit("timeout") if ( ! $nfound );
&atexit("eof") if ( eof(STDIN));

#$_ = <STDIN>;
@data = <STDIN>;

close(STDIN);
#open(FILE, $fileName) or die "Cannot open file";
#@data = <FILE>;
#close(FILE);

$first=1;
my $idx = -9;
foreach(@data)
{
  $line = $_;
  if($first)
  {
      $i = 0;
      @headers = split(",",$line);
      foreach my $header (@headers) {
	  $header =~ s/^\s+|\s+$//g ;
        if($header eq $icaoId)
        {
	  $idx = $i;
          last;
        }
        $i++;
      }
      if($idx == -9)
      {
     		print "$icaoId not found in header!\nExiting...";
        exit();
      }
      $line =~ s/\R//g;
      print "$latName,$lonName,$altName,$line\n";
      $first = 0;
  }
	else
  {
    @fields = split(",",$line);
    $id = $fields[$idx];
    if($id ne "")
    {
      $ica = findIcao($id);
			$lat = $ica->{"lat"};
			$lon = $ica->{"lon"};
			$elev = $ica->{"elev"};
      $line =~ s/\R//g;
      
      print "$lat,$lon,$elev,$line\n";
		}
  }  
}
