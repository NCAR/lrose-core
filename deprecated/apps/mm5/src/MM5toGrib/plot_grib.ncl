;************************************************
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_code.ncl"  
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_csm.ncl"  
;************************************************
begin
;************************************************
; create pointer to file and read in data
;************************************************

in = addfile("./output3.grib.25.grb","r")

; Uncomment these two lines to print out the GRIB file info
;print (in)  
;exit

T = in->TMP_0_SFC   
;T = in->TMP_0_ISBL

;nLevels = dimsizes(T(:,0,0))

print ("Min var = "+min(T(:,:))+" Max var = "+max(T(:,:)))

xlat = in->gridlat_0
xlon = in->gridlon_0

T@lat2d = xlat
T@lon2d = xlon

nsizes = dimsizes(xlat)
nlat = nsizes(0)
nlon = nsizes(1)

print ("Min lat = "+min(xlat)+" Max lat = "+max(xlat))
print ("Min lon = "+min(xlon)+" Max lon = "+max(xlon))

print ("nlat = "+nlat)
print ("nlon = "+nlon)

;printVarSummary(T)
;************************************************
; create default plot
;************************************************
wks = gsn_open_wks("ps","ers40")               ; open a ps file
gsn_define_colormap(wks,"BlAqGrYeOrReVi200")        ; choose colormap

res                       = True               ; plot mods desired
res@cnFillOn              = True               ; turn on color fill
res@cnLinesOn             = False              ; turn off contour lines
res@gsnSpreadColors       = True               ; use full range of color map

res@mpProjection      = "LambertConformal"
res@mpLambertParallel1F = xlat@Latin1
res@mpLambertParallel2F = xlat@Latin2
res@mpLambertMeridianF  = xlat@Lov

res@mpLimitMode       = "Corners" ; choose range of map
res@mpLeftCornerLatF  = xlat(0,0)
res@mpLeftCornerLonF  = xlon(0,0)
res@mpRightCornerLatF = xlat(nlat-2,nlon-2)
res@mpRightCornerLonF = xlon(nlat-2,nlon-2)
resgsnAddCyclic         = False

res@tfDoNDCOverlay = False

res@mpGeophysicalLineColor = "black" ; color of continental outlines
res@mpPerimOn              = True ; draw box around map
res@mpPerimDrawOrder       = "PostDraw"
res@mpGridLineDashPattern  = 2 ; lat/lon lines as dashed
res@mpOutlineBoundarySets = "GeophysicalAndUSStates"
res@mpUSStateLineColor    = "black"
res@pmTickMarkDisplayMode = "Always"
;res@mpDataBaseVersion = "HighRes"

;printVarSummary(xlat)
print("xlat(0,0) = "+ xlat(0,0))
print("xlat(nlat-2,0) = "+ xlat(nlat-2,0))
print("xlat(0,nlon-2) = "+ xlat(0,nlon-2))
print("xlat(nlat-2,nlon-2) = "+ xlat(nlat-2,nlon-2))
print("xlon(0,0) = "+ xlon(0,0))
print("xlon(nlat-2,0) = "+ xlon(nlat-2,0))
print("xlon(0,nlon-2) = "+ xlon(0,nlon-2))
print("xlon(nlat-2,nlon-2) = "+ xlon(nlat-2,nlon-2))

print("xlat(nlat-1,nlon-1) = "+ xlat(nlat-1,nlon-1))
print("xlon(nlat-1,nlon-1) = "+ xlon(nlat-1,nlon-1))

map = gsn_csm_contour_map(wks,T(:,:),res)

end
