# LROSE DOCUMENTATION OVERVIEW

## Primary links

| Package           | URL      |
| -------------     |:-------------:|
| core              | https://github.com/NCAR/lrose-core |
| core overview     | https://github.com/NCAR/lrose-core/tree/master/docs |
| user docs         | https://github.com/NCAR/lrose-docs |
| netcdf support    | https://github.com/NCAR/lrose-netcdf |
| display support   | https://github.com/NCAR/lrose-displays |
| Matlab display    | https://github.com/NCAR/lrose-emerald |
| Java display      | https://github.com/NCAR/lrose-jazz |
| Legacy C display  | https://github.com/NCAR/lrose-soloii |

## Code organization

| LROSE code        | URL      |
| -------------     |:-------------:|
| libs       | https://github.com/NCAR/lrose-core/blob/master/docs/libs/lrose-libs-summary.pdf |
| apps       | https://github.com/NCAR/lrose-core/blob/master/docs/apps/lrose-apps-summary.pdf |

## TDRP

Most LROSE apps use TDRP - Table Driven Runtime Parameters - to handle the parameters
that govern how the app runs.

| TDRP code         | URL      |
| -------------     |:-------------:|
| lib       | https://github.com/NCAR/lrose-core/tree/master/codebase/libs/tdrp |
| apps      | https://github.com/NCAR/lrose-core/tree/master/codebase/apps/tdrp |

The most important TDRP app is `tdrp_gen`.

See the full TDRP docs at:

 https://rawgit.com/NCAR/lrose-core/master/tdrp/index.html

## LROSE applications and data system

LROSE is a derivation of the older TITAN distribution. TITAN is now included in LROSE.

LROSE apps are run in the same manner as the TITAN apps. The following documents 
are useful in the short term. We need to update these and bring them into the
LROSE documentation.

| TITAN doc         | URL      |
| -------------     |:-------------:|
| overview       | http://www.ral.ucar.edu/projects/titan/home/ |
| runtime        | http://www.ral.ucar.edu/projects/titan/docs/TitanRunning.pdf |
| data system    | http://www.ral.ucar.edu/projects/titan/docs/TitanDataSystem.pdf |

### RADX library and applications

Radx is aimed specifically at handling radar and lidar data in polar/radial coordinates.

Fundamental to the Radx package is the RadxVol class, which represents a radar volume, and its associated classes.

| Radx code        | URL      |
| -------------    |:-------------:|
| libs       | https://github.com/NCAR/lrose-core/tree/master/codebase/libs/Radx |
| apps       | https://github.com/NCAR/lrose-core/tree/master/codebase/apps/Radx |

| Class       | Purpose      |
| ----------- |:-------------:|
| RadxVol     | Represents a radar volume. Contains metadata, sweeps, rays and fields |
| RadxSweep   | Metadata and ray indexes for a sweep, not the data itself |
| RadxRay     | Represents a single ray (beam). Contains vector of RadxFields |
| RadxField   | Represents a single field variable |

The data model which Radx is intended to represent is documented as follows:

https://github.com/NCAR/CfRadial/blob/master/docs/WMO_IM_Radar_and_Lidar_v0.5.pdf

