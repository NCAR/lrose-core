# LROSE DOCUMENTATION OVERVIEW

## Primary links

| Package           | URL      |
| -------------     |----------|
| core              | [https://github.com/NCAR/lrose-core](https://github.com/NCAR/lrose-core) |
| core overview     | [https://github.com/NCAR/lrose-core/tree/master/docs](https://github.com/NCAR/lrose-core/tree/master/docs) |
| user docs         | [https://github.com/NCAR/lrose-docs](https://github.com/NCAR/lrose-docs) |
| netcdf support    | [https://github.com/NCAR/lrose-netcdf](https://github.com/NCAR/lrose-netcdf) |
| display support   | [https://github.com/NCAR/lrose-displays](https://github.com/NCAR/lrose-displays) |
| Matlab display    | [https://github.com/NCAR/lrose-emerald](https://github.com/NCAR/lrose-emerald) |
| Java display      | [https://github.com/NCAR/lrose-jazz](https://github.com/NCAR/lrose-jazz) |
| Legacy C display  | [https://github.com/NCAR/lrose-soloii](https://github.com/NCAR/lrose-soloii) |

## Code organization

| LROSE code        | URL      |
| -------------     |----------|
| libs       | [https://github.com/NCAR/lrose-core/blob/master/docs/libs/lrose-libs-summary.pdf](https://github.com/NCAR/lrose-core/blob/master/docs/libs/lrose-libs-summary.pdf) |
| apps       | [https://github.com/NCAR/lrose-core/blob/master/docs/apps/lrose-apps-summary.pdf](https://github.com/NCAR/lrose-core/blob/master/docs/apps/lrose-apps-summary.pdf) |

## TDRP

Most LROSE apps use TDRP - Table Driven Runtime Parameters - to handle the parameters
that govern how the app runs.

| TDRP code         | URL      |
| -------------     |----------|
| lib       | [https://github.com/NCAR/lrose-core/tree/master/codebase/libs/tdrp](https://github.com/NCAR/lrose-core/tree/master/codebase/libs/tdrp) |
| apps      | [https://github.com/NCAR/lrose-core/tree/master/codebase/apps/tdrp](https://github.com/NCAR/lrose-core/tree/master/codebase/apps/tdrp) |

The most important TDRP app is `tdrp_gen`.

See the full TDRP docs at:

  http://htmlpreview.github.io/?https://github.com/NCAR/lrose-core/blob/master/docs/tdrp/index.html

## LROSE applications and data system

## Software functionality overview

These docs are in [https://github.com/NCAR/lrose-docs](https://github.com/NCAR/lrose-docs).

| Name | link to github webpage |
|------|------------------------|
| Software overview | [Overview](https://github.com/NCAR/lrose-docs/tree/master/lrose-core/lrose-overview.md) |
| Command line | [Command line](https://github.com/NCAR/lrose-docs/tree/master/lrose-core/lrose-command-line.md) |
| Parameters | [Parameters](https://github.com/NCAR/lrose-docs/tree/master/lrose-core/lrose-parameters.md) |
| App struture | [Anatomy of an app](https://github.com/NCAR/lrose-docs/tree/master/lrose-core/lrose-app-structure.md) |
| Real-time | [Real-time operations](https://github.com/NCAR/lrose-docs/tree/master/lrose-core/lrose-realtime.md) |
| Data-flow | [Data flow for real-time operations](https://github.com/NCAR/lrose-docs/tree/master/lrose-core/lrose-data-flow.md) |

### RADX library and applications

Radx is aimed specifically at handling radar and lidar data in polar/radial coordinates.

Fundamental to the Radx package is the RadxVol class, which represents a radar volume, and its associated classes.

| Radx code        | URL      |
| -------------    |----------|
| libs       | [https://github.com/NCAR/lrose-core/tree/master/codebase/libs/Radx](https://github.com/NCAR/lrose-core/tree/master/codebase/libs/Radx) |
| apps       | [https://github.com/NCAR/lrose-core/tree/master/codebase/apps/Radx](https://github.com/NCAR/lrose-core/tree/master/codebase/apps/Radx) |

| Class       | Purpose      |
| ----------- |-------------|
| RadxVol     | Represents a radar volume. Contains metadata, sweeps, rays and fields |
| RadxSweep   | Metadata and ray indexes for a sweep, not the data itself |
| RadxRay     | Represents a single ray (beam). Contains vector of RadxFields |
| RadxField   | Represents a single field variable |

The data model which Radx is intended to represent is documented as follows:

https://github.com/NCAR/CfRadial/blob/master/support_docs/WMO_IM_Radar_and_Lidar_v0.5.pdf

