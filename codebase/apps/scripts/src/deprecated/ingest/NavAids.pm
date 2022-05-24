package    NavAids;
require    Exporter;

@ISA = qw(Exporter);
@EXPORT = qw(comp_midpoint comp_new gc_dist gc_bear haversine ahaversine asin);

#
# This module contains PERL routines for navigation functions including
# computing a midpoint lat/lon from 2 points, computing great-circle distance
# and bearing given 2 lat/lons.
#
# Greg Thompson, RAP, NCAR, Boulder, CO, USA August 1998
#

1;

$CF = 57.295779513;               # radians to degrees
$RADIUS = 6378.140;               # radius of Earth (km)


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
