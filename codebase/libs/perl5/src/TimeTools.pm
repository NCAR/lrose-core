# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
# ** Copyright UCAR (c) 1992 - 2010 */
# ** University Corporation for Atmospheric Research(UCAR) */
# ** National Center for Atmospheric Research(NCAR) */
# ** Research Applications Laboratory(RAL) */
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
# ** 2010/10/7 23:12:42 */
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
sub date_retro {
  my ($orig_date,$back_hours)=@_;
  my ($year,$month,$day,$hour);
  my ($year_m,$month_m,$day_m,$hour_m);
 
  my @mdays=(31,31,28,31,30,31,30,31,31,30,31,30,31);
 
  $year=        substr($orig_date,0,4);
  $month=       substr($orig_date,4,2);
  $day=         substr($orig_date,6,2);
  $hour=        substr($orig_date,8,2);
 
  if($year%4 == 0) {
    if($year%100 == 0) {
      $mdays[2]=29 if($year%400 == 0);
    } else {
      $mdays[2]=29;
    }
  }
 
 
  $year_m=$year;
  $month_m=$month;
  $day_m=$day;
  $hour_m=$hour-$back_hours;
 
  while($hour_m < 0) {
# while($hour_m <= 0) {              ## obs hours are 1-24 !
    $hour_m += 24;
    $day_m -= 1;
    while($day_m <= 0) {
      $day_m += $mdays[$month_m-1];
      $month_m -= 1;
      while($month_m <= 0) {
        $month_m += 12;
        $year_m -= 1;
      }
    }
  }
 
  $date_m=$year_m*1000000+$month_m*10000+$day_m*100+$hour_m;
 
  return $date_m;
 
}


sub advance_h {

  %mon_days = (1,31,2,28,3,31,4,30,5,31,6,30,7,31,8,31,9,30,10,31,11,30,12,31);
  (my $s_date, my $advan_hh) = @_ ;

  my $yy = substr($s_date,0,4);
  my $mm = substr($s_date,4,2);
  my $dd = substr($s_date,6,2);
  my $hh = substr($s_date,8,2);

  my $feb = 2;
  $mon_days{$feb} = 29 if ($yy%4 == 0 && ($yy%400 == 0 || $yy%100 != 0));

 #print "hh: $hh - advan_hh: $advan_hh\n";
  $hh = $hh + $advan_hh;
  while($hh > 23) {
  $hh -= 24;
  $dd++;
  }
  while($dd > $mon_days{$mm+0}) {
  $dd = $dd - $mon_days{$mm+0};

  $mm++;

  while($mm > 12) {
  $mm -= 12;
  $yy++;
  }
  }

  while($hh < 0) {
  $hh += 24;
  $dd--;
  }
  if($dd < 1) {
  $mm--;
  while($mm < 1) {
  $mm += 12;
  $yy--;
  }
  $dd += $mon_days{$mm+0};
  }

  my $new_date = sprintf("%04d%02d%02d%02d",$yy,$mm,$dd,$hh);

  return $new_date;
}

sub advance_h_ori {
 
  my ($date,$add_hours)=@_;
  my ($yy,$mm,$dd,$hh);
  my @mdays=(31,31,28,31,30,31,30,31,31,30,31,30,31);
  my $time_new;
 
  $yy=int($date/1000000);
  $mm=int(($date%1000000)/10000);
  $dd=int(($date%10000)/100);
  $hh=$date%100;
 
  if($yy%400 == 0) {
    $mdays[2]=29;
  } else {
    $mdays[2]=29 if(($yy%4 == 0) && ($yy%100 != 0));
  }

  $hh += $add_hours;
  while($hh >= 24) {
    $hh -= 24;
    $dd += 1;
    if($dd > $mdays[$mm]) {
      $dd -= $mdays[$mm];
      $mm += 1;
      if($mm > 12) {
        $mm -= 12;
        $yy += 1;
      }
    }
  }
 
  $time_new=$yy*1000000+$mm*10000+$dd*100+$hh;
 
  return $time_new;

}

sub advance_m {

  my ($date,$add_mins)=@_;            # $date has to be YYYYMMDDHHMM
  my ($yy,$mm,$dd,$hh,$mn);
  my @mdays=(31,31,28,31,30,31,30,31,31,30,31,30,31);
  my $time_new;

  $yy=substr($date,0,4);
  $mm=substr($date,4,2);
  $dd=substr($date,6,2);
  $hh=substr($date,8,2);
  $mn=substr($date,10,2);

  if($yy%400 == 0) {
    $mdays[2]=29;
  } else {
    $mdays[2]=29 if(($yy%4 == 0) && ($yy%100 != 0));
  }

  $mn += $add_mins;

  while($mn >= 60) {
    $mn -= 60;
    $hh++;
    while($hh >= 24) {
      $hh -= 24;
      $dd += 1;
      if($dd > $mdays[$mm]) {
        $dd -= $mdays[$mm];
        $mm += 1;
        if($mm > 12) {
          $mm -= 12;
          $yy += 1;
        }
      }
    }
  }

  while($mn < 0) {
    $mn += 60;
    $hh--;
    while($hh < 0) {
      $hh += 24;
      $dd--;
      if($dd < 1) {
        $dd += $mdays[$mm-1];
        $mm--;
        if($mm < 1) {
          $mm += 12;
          $yy--;
        }
      }
    }
  }

  $time_new=$yy*100000000+$mm*1000000+$dd*10000+$hh*100+$mn;

  return $time_new;

}

sub advance_s {

  my ($date,$add_secs)=@_;            # $date has to be YYYYMMDDHHMMSS
  my ($yy,$mm,$dd,$hh,$mn,$ss);
  my @mdays=(31,31,28,31,30,31,30,31,31,30,31,30,31);
  my $time_new;

  $yy=substr($date,0,4);
  $mm=substr($date,4,2);
  $dd=substr($date,6,2);
  $hh=substr($date,8,2);
  $mn=substr($date,10,2);
  $ss=substr($date,12,2);

  if($yy%400 == 0) {
    $mdays[2]=29;
  } else {
    $mdays[2]=29 if(($yy%4 == 0) && ($yy%100 != 0));
  }

  $ss += $add_secs;

  while($ss >= 60) {
    $ss -= 60;
    $mn++;
    while($mn >= 60) {
      $mn -= 60;
      $hh++;
      while($hh >= 24) {
        $hh -= 24;
        $dd += 1;
        if($dd > $mdays[$mm]) {
          $dd -= $mdays[$mm];
          $mm += 1;
          if($mm > 12) {
            $mm -= 12;
            $yy += 1;
          }
        }
      }
    }
  }

  while($ss < 0) {
    $ss += 60;
    $mn--;
    while($mn < 0) {
      $mn += 60;
      $hh--;
      while($hh < 0) {
        $hh += 24;
        $dd--;
        if($dd < 1) {
          $dd += $mdays[$mm-1];
          $mm--;
          if($mm < 1) {
            $mm += 12;
            $yy--;
          }
        }
      }
    }
  }

  $time_new=$yy*10000000000+$mm*100000000+$dd*1000000+$hh*10000+$mn*100+$ss;

  return $time_new;

}

sub date2secs {

  my ($year,$month,$day,$hour,$min,$sec,$hour_diff)=@_;
  my @mdays=(31,31,28,31,30,31,30,31,31,30,31,30,31);

  my ($n_secs,$idays);
  my ($iy,$im,$iyLoopMax);

  $n_secs=0;
# foreach $iy (0..$year-1970-1) {
  $iyLoopMax = $year - 1970 - 1;
  for ($iy=0; $iy<=$iyLoopMax; $iy++) {

    $idays=365;

    if(($iy+1970)%100 == 0) {
      $idays=366 if(($iy+1970)%400 == 0);
    } else {
      $idays=366 if(($iy+2)%4 == 0);
    }

    $n_secs += $idays*86400;
  }

  if(($iy+1970)%100 == 0) {
    $mdays[2]=29 if(($iy+1970)%400 == 0);
  } else {
    $mdays[2]=29 if(($iy+2)%4 == 0);
  }

  foreach $im (1..$month-1) {
    $n_secs += $mdays[$im]*86400;
  }

  $n_secs += (($day-1)*86400+$hour*3600+$min*60+$sec);

  $n_secs += $hour_diff*3600;

  return $n_secs;
}

sub jday2secs {

  my ($year,$jday,$hour,$min,$sec,$hour_diff)=@_;

  my @mdays=(31,31,28,31,30,31,30,31,31,30,31,30,31);

  my ($n_secs,$idays);
  my ($iy,$im,$iyLoopMax);
  my $iydays; # total days before the "latest" or "current" year in consideration

  $n_secs=0;
# foreach $iy (0..$year-1970-1) {
  $iyLoopMax = $year - 1970 - 1;
  for ($iy=0; $iy<=$iyLoopMax; $iy++) {

    $idays=365;

    if(($iy+1970)%100 == 0) {
      $idays=366 if(($iy+1970)%400 == 0);
    } else {
      $idays=366 if(($iy+2)%4 == 0);
    }

    $n_secs += $idays*86400;
  }

  $iydays=$n_secs/86400;

  $n_secs += (($jday-1)*86400+$hour*3600+$min*60+$sec);

  $n_secs += $hour_diff*3600;

  return ($n_secs,$iydays);
}

sub nearest_hour {

  my $atime=$_[0];
  my $len;
  my $atime_new;
  my ($ayear,$amonth,$aday,$ahour);

  my @days=(31,31,28,31,30,31,30,31,31,30,31,30,31);
  my ($year,$month,$day,$hour,$minute,$sec);

  $len=length($atime);

  $year=substr($atime,0,4);
  $month=substr($atime,4,2);
  $day=substr($atime,6,2);
  $hour=substr($atime,8,2);
  $minute=substr($atime,10,2);

  if($len == 14) {
    $sec=substr($atime,12,2);
  } else {
    $sec=0;
  }

  if($year%400 == 0) {
    $days[2]=29;
  } else {
    $days[2]=29 if(($year%4 == 0) && ($year%100 != 0));
  }

  if($sec >= 30) {
    $minute++;
    if($minute >= 60) {
      $minute -= 60;
      $hour++;
      if($hour >= 24) {
        $hour -= 24;
        $day++;
        if($day > $days[$month]) {
          $day -= $days[$month];
          $month++;
          if($month > 12) {
            $month -= 12;
            $year++;
          }
        }
      }
    }
  }

  if($minute >= 30) {
    $minute = 0;
    $hour++;
    if($hour >= 24) {
      $hour -= 24;
      $day++;
      if($day > $days[$month]) {
        $day -= $days[$month];
        $month++;
        if($month > 12) {
          $month -= 12;
          $year++;
        }
      }
    }
  }

  $atime_new=$year*1000000+$month*10000+$day*100+$hour;

  return $atime_new;

}
1;
