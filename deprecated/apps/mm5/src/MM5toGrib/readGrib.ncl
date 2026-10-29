begin

   FileName = "outputFCS.grib"
   ff = addfile(FileName,"r")
	 ;;print(ff)

   print("File: "+FileName)
   printVarSummary (ff);
   names = getfilevarnames(ff)
	 do i = 0,dimsizes(names)-1
	    data = ff->$names(i)$
	    print(names(i)+": "+min(data)+"  "+max(data))
			delete(data)
	end do

end

