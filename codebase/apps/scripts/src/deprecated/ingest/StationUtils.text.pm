package    StationUtils;
require    Exporter;

@ISA = qw(Exporter);
@EXPORT = qw(stationInfo_lookup airsigmetId_lookup comp_midpoint comp_new gc_dist gc_bear haversine ahaversine asin);

#
# This module contains PERL routines for navigation functions including
# computing a midpoint lat/lon from 2 points, computing great-circle distance
# and bearing given 2 lat/lons.
#
# Greg Thompson, RAP, NCAR, Boulder, CO, USA August 1998
#

1;

%icaos, %iatas, %airports;

$CF = 57.295779513;               # radians to degrees
$RADIUS = 6378.140;               # radius of Earth (km)

###################################################################
sub init{
   my $fileName = $ENV{'STATION_FILE'};
   warn "WARNING, using file: $ENV{'STATION_FILE'}";
   open(FILE, $fileName) or die "Cannot open file";
   @data = <FILE>;
   close(FILE);

   foreach(@data)
   {
      $line = $_;
      if(length($line) >= 54 && substr($line,0,1) ne "!" && !(substr($line,0,1) eq "C" && substr($line,1,1) eq "D"))
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

          if($icao ne "    ") # has icao
          {
              if($iata ne "   ") # has icao and iata
              {
		  if(exists $iatas{$iata})
                  {
 		      warn "$iata already found in stations list!\n";
                      print "Existing: $iatas{$iata}[0]{'lat'} $iatas{$iata}[0]{'lon'} $iatas{$iata}[0]{'elev'}\n";
                      print "New: $lat $lon $elev\n";
                      $size = scalar @{$iatas{$iata}};
                      $iatas{$iata}[$size]{"lat"} = $lat;
                      $iatas{$iata}[$size]{"lon"} = $lon;
                      $iatas{$iata}[$size]{"elev"} = $elev;
		  }
		  else 
                  {
#                    warn "$iata found for first time\n";
                    $iatas{$iata}[0]{"lat"} = $lat;
                    $iatas{$iata}[0]{"lon"} = $lon;
                    $iatas{$iata}[0]{"elev"} = $elev;
		  }

		  if(exists $icaos{$icao}) {
                    warn "ICAO $icao already exists";
		  }
		  else {
                    $icaos{$icao}{'lat'} = $lat;
		    $icaos{$icao}{'lon'} = $lon;
		    $icaos{$icao}{'elev'} = $elev;
		  }

	      } 
              else # has icao but no iata
              {
		  if(exists $icaos{$icao})
                  {
 		      warn "WARNING: $icao already found in icaos list! Should not have duplicates\n";
                      print "Existing: $icaos{$icao}{'lat'} $icaos{$icao}{'lon'} $icaos{$icao}{'elev'}\n";
                      print "New: $lat $lon $elev\n\n";
                      $icaos{$icao}{'lat'} = $lat;
  		      $icaos{$icao}{'lon'} = $lon;
                      $icaos{$icao}{'elev'} = $elev;
                  }
                 else
                 {
                   $icaos{$icao}{'lat'} = $lat;
  		   $icaos{$icao}{'lon'} = $lon;
                   $icaos{$icao}{'elev'} = $elev;
		 }
              }
	  }
          elsif($iata ne "   ") # has iata but no icao
          {
	      if(exists $iatas{$iata})
              {
		  print "$iata already found in stations list\n";
                  print "Existing: $iatas{$iata}[0]{'lat'} $iatas{$iata}[0]{'lon'} $iatas{$iata}[0]{'elev'}\n";
                  print "New: $lat $lon $elev\n\n";
                  $size = scalar @{$iatas{$iata}};
                  $iatas{$iata}[$size]{"lat"} = $lat;
                  $iatas{$iata}[$size]{"lon"} = $lon;
                  $iatas{$iata}[$size]{"elev"} = $elev;
	      }
              else
              {
                $iatas{$iata}[0]{"lat"} = $lat;
                $iatas{$iata}[0]{"lon"} = $lon;
                $iatas{$iata}[0]{"elev"} = $elev;            
	      }
          }
      }
   }

   my $fileName = $ENV{'AIRPORT_FILE'};
   warn "WARNING, using file: $ENV{'AIRPORT_FILE'}";
   open(FILE, $fileName) or warn "Cannot open airport file: $filename";
   return NULL if(tell(FILE) == -1);
   @adata = <FILE>;
   close(FILE);

   foreach(@adata)
   {
     $line = $_;
     $id = substr $line, 0, 3;
     $lat = substr $line, 4, 5;
     $lon = substr $line, 11, 6;
     $elev = 0;

     if(exists $airports{$id})
     {
       warn "$id already found in stations list!\n";
       print "Existing: $airports{$id}[0]{'lat'} $airports{$id}[0]{'lon'} $airports{$id}[0]{'elev'}\n";
       print "New: $lat $lon $elev\n";
       $size = scalar @{$airports{$id}};
       $airports{$id}[$size]{"lat"} = $lat;
       $airports{$id}[$size]{"lon"} = $lon;
       $airports{$id}[$size]{"elev"} = $elev;
     }
     else 
     {
       $airports{$id}[0]{"lat"} = $lat;
       $airports{$id}[0]{"lon"} = $lon;
       $airports{$id}[0]{"elev"} = $elev;
     }
   }

   return NULL;
}
###################################################################
sub shutdown{

}

sub findIcao{
    $station = shift;
    if(exists $icaos{$station})
    {
	return $icaos{$station};
    }
    else
    {
	return NULL;
    }
}

sub findIata{
    $station = shift;

    if(exists $iatas{$station})
    {
	return $iatas{$station};
    }
    else
    {
	return NULL;
    }

}

###################################################################

sub stationInfo_lookup {
    my ($dbhandle, $stationId, $nearIcao) = @_;
    my ($clat, $clon, $lat, $lon, $elev, $count, @lats, @lons, @elevs);
    my ($n, $dist, $distMin, $latSave, $lonSave, $elevSave);
    my ($dbTable, $sql, $sth, $rv, $rc);

    $dbTable = "StationUtils";
    $count = 0;
    $clat = $lat = 39.9999;
    $clon = $lon = -98.9999;
    $elev = 0;
  
    if ($nearIcao eq "KWBC" || $nearIcao eq "KNKA" || $nearIcao eq "KKCI") {
      $clat = 39.9999;
      $clon = -98.9999;
    } else {
      $icao = findIcao($nearIcao);
      # check if not null, check if multiple stations were found?

      if($icao != NULL)
      {
        $clat = $icao->{"lat"};
        $clon = $icao->{"lon"};
        $elev = $icao->{"elev"};
      }
    }
    $count = 0;
    $iata = findIata($stationId); # get multiple stations?
    if($iata != NULL) {
      $size = scalar @{$iata} - 1;
      for $n (0..$size)
      {
        $lat = $iata->[$n]{"lat"};
        $lon = $iata->[$n]{"lon"};
        $elev = $iata->[$n]{"elev"};
        push (@lats, $lat);
        push (@lons, $lon);
        push (@elevs, $elev);
        $count++;
      }

      if ($count == 0) {
         $airport = findAirport($stationId);
         if($airport != NULL) {
            $lat = $airport{"lat"};
            $lon = $airport{"lon"};
            $elev = $airport{"elev"};
         }
         else {
            warn "WARNING, identifier $stationId not found in $dbTable (no lat/lon assigned)\n";
	 }
      } elsif ($count > 1) {
        print "Count is over 1\n\n";
        if($icao == NULL) {
          warn "No ICAO is specified to determine which IATA station to use";
	}
        $distMin = 2000000000;
        $dist = 0;
        for $n (0..$count-1) {
          $dist = &NavAids::gc_dist($clat, -1*$clon, $lats[$n], -1*$lons[$n]);
          if ($dist < $distMin) {
            $latSave = $lats[$n];
            $lonSave = $lons[$n];
            $elevSave = $elevs[$n];
            $distMin = $dist;
          }
        }
        $lat = $latSave;
        $lon = $lonSave;
        $elev = $elevSave;
      } else {
        $lat = $lats[0];
        $lon = $lons[0];
        $elev = $elevs[0];
      }
    }
    else { # iata not found, look for ICAO
      $icao = findIcao($stationId);
      if($icao != NULL) {
        $lat = $icao->{"lat"};
        $lon = $icao->{"lon"};
        $elev = $icao->{"elev"};
      }
      else {
        $lat = $clat;
        $lon = $clon;
      }
    }


    return ($lat, $lon, $elev);
}

###################################################################

sub airsigmetId_lookup {
    warn "WARNING: airsigmetId_lookup not implemented...";
    return;
    my ($dbhandle, $refId, $nearIcao) = @_;
    my ($lat, $lon, $count);
    my ($dbTable, $sql, $sth, $rv, $rc);

    $dbTable = "AirSigmetsRefPoints";
    $lat = 0.0;
    $lon = 0.0;
    $count = 0;

    if ($dbhandle) {
        $sql = qq{SELECT lat,lon FROM $dbTable WHERE refId="$refId"};
        $sth = $dbhandle->prepare($sql);
        $rv = $sth->execute;
        $rc = $sth->bind_columns(undef, \$lat, \$lon);
        while ($sth->fetch) {
            $count++;
        }
        $rc = $sth->finish if (defined($rc));
        warn "WARNING: refId, $refId not found in table $dbTable\n" if ($count != 1);

    } else {
        warn "No database handle supplied, cannot retrieve lat/lon info for $refId";
    }

    return ($lat, $lon);

}

###################################################################

sub comp_midpoint {
    local($lat1, $lon1, $lat2, $lon2) = @_;
    local($dist, $bear);
    local($final_lat, $final_lon);

    $dist = &gc_dist($lat1, $lon1, $lat2, $lon2);
    $bear = &gc_bear($lat1, $lon1, $lat2, $lon2);

    ($final_lat, $final_lon) = &comp_new($lat1, $lon1, $bear, ($dist/2));
	$final_lat,$final_lon;

}

###################################################################

sub comp_new {
    local($lat1, $lon1, $bear, $dist) = @_;
    local($x, $y, $z, $t, $sp, $cp, $cl, $sl, $sb, $cb, $st, $ct);
	local($final_lat, $final_lon);

    $lat1 /= $CF; $lon1 /= $CF; $bear /= $CF;
    $t = $dist/$RADIUS;
    $sp = sin($lat1); $cp = cos($lat1);
    $sl = sin($lon1); $cl = cos($lon1);
    $sb = sin($bear); $cb = cos($bear);
    $st = sin($t);    $ct = cos($t);
    $x = $cp*$sl*$ct - $sp*$sl*$cb*$st - $cl*$sb*$st;
    $y = $cp*$cl*$ct - $sp*$cl*$cb*$st + $sl*$sb*$st;
    $z = $sp*$ct + $cp*$cb*$st;

    $final_lat = $CF*&asin($z);
    $final_lon = ( (abs($x)+abs($y)) < 0.0000001)? 0.0: $CF*atan2($x,$y);

	$final_lat,$final_lon;

}

###################################################################

sub gc_dist {
    local($lat1, $lon1, $lat2, $lon2) = @_;
    local($dp, $dl, $x);

    $dp = ($lat1 - $lat2) / $CF;
    $dl = ($lon1 - $lon2) / $CF;

    $x = &haversine($dp) + cos($lat1/$CF)*cos($lat2/$CF)*&haversine($dl);
    $x = $RADIUS*&ahaversine($x);

    $x;

}

###################################################################

sub gc_bear {
    local($lat1, $lon1, $lat2, $lon2) = @_;
    local($x, $y, $dl, $beta);

    $lat1 /= $CF; $lon1 /= $CF; $lat2 /= $CF; $lon2 /= $CF;
    $dl = $lon1 - $lon2;
    $x = cos($lat1)*sin($lat2) - sin($lat1)*cos($lat2)*cos($dl);
    $y = cos($lat2)*sin($dl);
    $beta = $CF*atan2($y, $x);
    $beta;

}

###################################################################

sub haversine {
    local($a) = @_;
    local($t);
    $t = sin($a/2.0);
    $t*$t;
}

###################################################################

sub ahaversine {
    local($t) = @_;
    local($a);
    $a = 2.0*&asin(sqrt($t));
    $a;
}

###################################################################

sub asin {
    local($y) = @_;
    local($a);
    $a = (atan2($y, sqrt(1.0-$y*$y)))? atan2($y, sqrt(1.0-$y*$y)) :atan2(-$y, sqrt(1.0-$y*$y));
    $a;
}
