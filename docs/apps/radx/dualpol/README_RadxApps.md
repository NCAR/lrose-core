# RADX Dual-Polarization Applications

## KDP, PID and Precipitation Rate Applications - use of shared parameter files

The following applications form a group that handle KDP estimation, particle identification (PID) and precipitation rate estimation.

| App                       | KDP   | Z & ZDR Attenuation | NCAR PID | Precip Rate |
| -------------             | ----- | ------------------- | -------- | ----------- |
| [RadxKdp](./RadxKdp.md)   | :heavy_check_mark: | :heavy_check_mark: | :x: | :x: |
| [RadxPid](./RadxPid.md)   | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :x: |
| [RadxRate](./RadxRate.md) | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |

Each of these apps has a main parameter file, which controls file handling, operational modes etc.

The main file in turn points to shared parameter files for specific purposes.

RadxKdp reads in a KDP-specific parameter file.

RadxPid reads in the KDP parameter file, and in addition 2 PID-specific files: (a) the PID overview and (b) the PID-thredholds file that specifies the details of the fuzzy logic.

RadxRate reads in all of the above files, and in addition a file specific to the computation of precip rate.




