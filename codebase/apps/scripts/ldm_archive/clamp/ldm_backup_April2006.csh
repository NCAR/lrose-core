#!/bin/csh -x 
#

#############################################################################################
## UGLY little scripting to hopefully get most/all of the data that got dropped when the MSS project # got changed.


#Eta104
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta104/20060420 mss:/RAPDMG/grib/Eta104
    msclass -class -R reliability=economy /RAPDMG/grib/Eta104/20060420
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta104/20060421 mss:/RAPDMG/grib/Eta104
    msclass -class -R reliability=economy /RAPDMG/grib/Eta104/20060421
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta104/20060422 mss:/RAPDMG/grib/Eta104
    msclass -class -R reliability=economy /RAPDMG/grib/Eta104/20060422
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta104/20060423 mss:/RAPDMG/grib/Eta104
    msclass -class -R reliability=economy /RAPDMG/grib/Eta104/20060423

#Eta215
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta215/20060420 mss:/RAPDMG/grib/Eta215
    msclass -class -R reliability=economy /RAPDMG/grib/Eta215/20060420
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta215/20060421 mss:/RAPDMG/grib/Eta215
    msclass -class -R reliability=economy /RAPDMG/grib/Eta215/20060421
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta215/20060422 mss:/RAPDMG/grib/Eta215
    msclass -class -R reliability=economy /RAPDMG/grib/Eta215/20060422
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta215/20060423 mss:/RAPDMG/grib/Eta215
    msclass -class -R reliability=economy /RAPDMG/grib/Eta215/20060423

#GFS003          
    msrcp -pr 48500002 -R -pe 4096 /grib/GFS003/20060420 mss:/RAPDMG/grib/GFS003
    msclass -class -R reliability=economy /RAPDMG/grib/GFS003/20060420
    msrcp -pr 48500002 -R -pe 4096 /grib/GFS003/20060421 mss:/RAPDMG/grib/GFS003
    msclass -class -R reliability=economy /RAPDMG/grib/GFS003/20060421
    msrcp -pr 48500002 -R -pe 4096 /grib/GFS003/20060422 mss:/RAPDMG/grib/GFS003
    msclass -class -R reliability=economy /RAPDMG/grib/GFS003/20060422
    msrcp -pr 48500002 -R -pe 4096 /grib/GFS003/20060423 mss:/RAPDMG/grib/GFS003
    msclass -class -R reliability=economy /RAPDMG/grib/GFS003/20060423

#RUC13kmDEV2s    
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC13kmDEV2s/20060420 mss:/RAPDMG/grib/RUC13kmDEV2s
    msclass -class -R reliability=economy /RAPDMG/grib/RUC13kmDEV2s/20060420
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC13kmDEV2s/20060421 mss:/RAPDMG/grib/RUC13kmDEV2s
    msclass -class -R reliability=economy /RAPDMG/grib/RUC13kmDEV2s/20060421
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC13kmDEV2s/20060422 mss:/RAPDMG/grib/RUC13kmDEV2s
    msclass -class -R reliability=economy /RAPDMG/grib/RUC13kmDEV2s/20060422
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC13kmDEV2s/20060423 mss:/RAPDMG/grib/RUC13kmDEV2s
    msclass -class -R reliability=economy /RAPDMG/grib/RUC13kmDEV2s/20060423

#RUC252s
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC252s/20060420 mss:/RAPDMG/grib/RUC252s
    msclass -class -R reliability=economy /RAPDMG/grib/RUC252s/20060420
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC252s/20060421 mss:/RAPDMG/grib/RUC252s
    msclass -class -R reliability=economy /RAPDMG/grib/RUC252s/20060421
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC252s/20060422 mss:/RAPDMG/grib/RUC252s
    msclass -class -R reliability=economy /RAPDMG/grib/RUC252s/20060422
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC252s/20060423 mss:/RAPDMG/grib/RUC252s
    msclass -class -R reliability=economy /RAPDMG/grib/RUC252s/20060423

#Eta211          
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta211/20060420 mss:/RAPDMG/grib/Eta211
    msclass -class -R reliability=economy /RAPDMG/grib/Eta211/20060420
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta211/20060421 mss:/RAPDMG/grib/Eta211
    msclass -class -R reliability=economy /RAPDMG/grib/Eta211/20060421
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta211/20060422 mss:/RAPDMG/grib/Eta211
    msclass -class -R reliability=economy /RAPDMG/grib/Eta211/20060422
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta211/20060423 mss:/RAPDMG/grib/Eta211
    msclass -class -R reliability=economy /RAPDMG/grib/Eta211/20060423

#Eta216          
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta216/20060420 mss:/RAPDMG/grib/Eta216
    msclass -class -R reliability=economy /RAPDMG/grib/Eta216/20060420
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta216/20060421 mss:/RAPDMG/grib/Eta216
    msclass -class -R reliability=economy /RAPDMG/grib/Eta216/20060421
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta216/20060422 mss:/RAPDMG/grib/Eta216
    msclass -class -R reliability=economy /RAPDMG/grib/Eta216/20060422
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta216/20060423 mss:/RAPDMG/grib/Eta216
    msclass -class -R reliability=economy /RAPDMG/grib/Eta216/20060423

#RUC13kmDEV2b    
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC13kmDEV2b/20060420 mss:/RAPDMG/grib/RUC13kmDEV2b
    msclass -class -R reliability=economy /RAPDMG/grib/RUC13kmDEV2b/20060420
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC13kmDEV2b/20060421 mss:/RAPDMG/grib/RUC13kmDEV2b
    msclass -class -R reliability=economy /RAPDMG/grib/RUC13kmDEV2b/20060421
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC13kmDEV2b/20060422 mss:/RAPDMG/grib/RUC13kmDEV2b
    msclass -class -R reliability=economy /RAPDMG/grib/RUC13kmDEV2b/20060422
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC13kmDEV2b/20060423 mss:/RAPDMG/grib/RUC13kmDEV2b
    msclass -class -R reliability=economy /RAPDMG/grib/RUC13kmDEV2b/20060423

#RUC252b
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC252b/20060420 mss:/RAPDMG/grib/RUC252b
    msclass -class -R reliability=economy /RAPDMG/grib/RUC252b/20060420
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC252b/20060421 mss:/RAPDMG/grib/RUC252b
    msclass -class -R reliability=economy /RAPDMG/grib/RUC252b/20060421
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC252b/20060422 mss:/RAPDMG/grib/RUC252b
    msclass -class -R reliability=economy /RAPDMG/grib/RUC252b/20060422
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC252b/20060423 mss:/RAPDMG/grib/RUC252b
    msclass -class -R reliability=economy /RAPDMG/grib/RUC252b/20060423

#Eta212          
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta212/20060420 mss:/RAPDMG/grib/Eta212
    msclass -class -R reliability=economy /RAPDMG/grib/Eta212/20060420
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta212/20060421 mss:/RAPDMG/grib/Eta212
    msclass -class -R reliability=economy /RAPDMG/grib/Eta212/20060421
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta212/20060422 mss:/RAPDMG/grib/Eta212
    msclass -class -R reliability=economy /RAPDMG/grib/Eta212/20060422
    msrcp -pr 48500002 -R -pe 4096 /grib/Eta212/20060423 mss:/RAPDMG/grib/Eta212
    msclass -class -R reliability=economy /RAPDMG/grib/Eta212/20060423

#GFS002          
    msrcp -pr 48500002 -R -pe 4096 /grib/GFS002/20060420 mss:/RAPDMG/grib/GFS002
    msclass -class -R reliability=economy /RAPDMG/grib/GFS002/20060420
    msrcp -pr 48500002 -R -pe 4096 /grib/GFS002/20060421 mss:/RAPDMG/grib/GFS002
    msclass -class -R reliability=economy /RAPDMG/grib/GFS002/20060421
    msrcp -pr 48500002 -R -pe 4096 /grib/GFS002/20060422 mss:/RAPDMG/grib/GFS002
    msclass -class -R reliability=economy /RAPDMG/grib/GFS002/20060422
    msrcp -pr 48500002 -R -pe 4096 /grib/GFS002/20060423 mss:/RAPDMG/grib/GFS002
    msclass -class -R reliability=economy /RAPDMG/grib/GFS002/20060423

#RUC13kmDEV2p    
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC13kmDEV2p/20060420 mss:/RAPDMG/grib/RUC13kmDEV2p
    msclass -class -R reliability=economy /RAPDMG/grib/RUC13kmDEV2p/20060420
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC13kmDEV2p/20060421 mss:/RAPDMG/grib/RUC13kmDEV2p
    msclass -class -R reliability=economy /RAPDMG/grib/RUC13kmDEV2p/20060421
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC13kmDEV2p/20060422 mss:/RAPDMG/grib/RUC13kmDEV2p
    msclass -class -R reliability=economy /RAPDMG/grib/RUC13kmDEV2p/20060422
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC13kmDEV2p/20060423 mss:/RAPDMG/grib/RUC13kmDEV2p
    msclass -class -R reliability=economy /RAPDMG/grib/RUC13kmDEV2p/20060423

#RUC252p
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC252p/20060420 mss:/RAPDMG/grib/RUC252p
    msclass -class -R reliability=economy /RAPDMG/grib/RUC252p/20060420
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC252p/20060421 mss:/RAPDMG/grib/RUC252p
    msclass -class -R reliability=economy /RAPDMG/grib/RUC252p/20060421
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC252p/20060422 mss:/RAPDMG/grib/RUC252p
    msclass -class -R reliability=economy /RAPDMG/grib/RUC252p/20060422
    msrcp -pr 48500002 -R -pe 4096 /grib/RUC252p/20060423 mss:/RAPDMG/grib/RUC252p
    msclass -class -R reliability=economy /RAPDMG/grib/RUC252p/20060423



