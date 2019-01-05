# Dual-Polarization Processing in LROSE

In this document we illustrate some of the basic operations
that the Radx apps in LROSE can perform for dual-polarization processing.

In these examples we are using some data from the NCAR S-Pol radar at the
site near Hays, Kansas during the PECAN field project in 2015.

## The measured fields

* DBZ
* ZDR
* RHOHV
* PHIDP
* TEMP
<!---
* VEL
* WIDTH
--->

### DBZ

PPI 
![alt text](./images/ppi_dbz.png "PPI DBZ")

RHI 
![alt text](./images/rhi_dbz.png "RHI DBZ")

<!---

### VEL

PPI 
![alt text](./images/ppi_vel.png "PPI VEL")

RHI 
![alt text](./images/rhi_vel.png "RHI VEL")

### WIDTH

PPI 
![alt text](./images/ppi_width.png "PPI WIDTH")

RHI 
![alt text](./images/rhi_width.png "RHI WIDTH")

--->

### ZDR

<!---
PPI 
![alt text](./images/ppi_zdr.png "PPI ZDR")
--->

RHI 
![alt text](./images/rhi_zdr.png "RHI ZDR")

### RHOHV

<!---
PPI 
![alt text](./images/ppi_rhohv.png "PPI RHOHV")
--->

RHI 
![alt text](./images/rhi_rhohv.png "RHI RHOHV")

### PHIDP

<!---
PPI 
![alt text](./images/ppi_phidp.png "PPI PHIDP")
--->

RHI 
![alt text](./images/rhi_phidp.png "RHI PHIDP")

### TEMPERATURE

<!---
PPI 
![alt text](./images/ppi_temp.png "PPI TEMP")
--->

RHI 
![alt text](./images/rhi_temp.png "RHI TEMP")

## The derived fields

* KDP
* PID (Particle ID, Hydrometeor type)
* PRECIP RATE

### KDP

We compute KDP from PHIDP.

It is defined as the slope of PHIDP in range, divied by 2.

KDP can be noisy - considerable smoothing and other filtering is required for good results.

<!---
PPI 
![alt text](./images/ppi_kdp.png "PPI KDP")
--->

RHI 
![alt text](./images/rhi_kdp.png "RHI KDP")

## We compute PDI from the above dual-pol fields, in addition to temperature

### PID

PID is computed using a fuzzy-logic approach.

<!---
PPI 
![alt text](./images/ppi_pid.png "PPI PID")
--->

RHI 
![alt text](./images/rhi_pid.png "RHI PID")

## Precip rate

Precip rate may be computed using DBZH, Z/ZDR and KDP.

### RATE_ZH

PPI 
![alt text](./images/ppi_rate_zh.png "PPI RATE_ZH")

<!---
RHI 
![alt text](./images/rhi_rate_zh.png "RHI RATE_ZH")
--->

### RATE_Z_ZDR

PPI 
![alt text](./images/ppi_rate_z_zdr.png "PPI RATE_Z_ZDR")

<!---
RHI 
![alt text](./images/rhi_rate_z_zdr.png "RHI RATE_Z_ZDR")
--->

### RATE_KDP

PPI 
![alt text](./images/ppi_rate_kdp.png "PPI RATE_KDP")

<!---
RHI 
![alt text](./images/rhi_rate_kdp.png "RHI RATE_KDP")
--->

### RATE_HYBRID

A hybrid rate may be estimated using a combination of the computed rates.

PPI 
![alt text](./images/ppi_rate_hybrid.png "PPI RATE_HYBRID")

<!---
RHI 
![alt text](./images/rhi_rate_hybrid.png "RHI RATE_HYBRID")
--->
