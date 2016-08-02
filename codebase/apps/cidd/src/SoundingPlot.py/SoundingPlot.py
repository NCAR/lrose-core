#!/usr/bin/python

#This Python module script Open a sounding data file from 
#/tmp/sounding.class. To get such datas: data time, position,
#pressure(height), temp, dew point temp, wind dir, wind speed.
#Then call TKinter modules to make a graph of sounding data.
#Auther: Hai-Geng Chen     
#V1.0 09/19/00 with data output to xterm
#V2.0 09/20/00 with Tkinter (Toplevel) output---a stupid version
#V2.1 09/22/00 with Canvas --X, Y axis 
#V3.0 09/23/00 02:00 with ln(P)-SkewT, Boulder is almost snow!!
#V3.1 09/24/00 01:30 with wind bar, Boulder snowed from this morning!! 
#V3.1+ 09/27/00 17:00 fomula with more structure
#V3.1++ 09/28/00 10:20 depart codes to 2 functions, main program only has 4 lines!! 
#V3.1+++ 09/28/00 12:00 chang the slope of temp from 60 --> 45 degree!! 
#V3.1++++ 09/29/00 12:00 add the  parcel info space 
#V3.2 10/02/00 23:00 after 2hrs' hard work, add "parent.after(10000,..)"&"canvas.delete('tag')"
#V3.3 10/04/00 23:30 with Theta, Saturated Mixing Ratio
#V3.4 10/11/00 10:30 Need to be enhanced!!
#V3.5 10/12/00 21:00 Almost ok!!
#V3.6 10/19/00 21:30 Parcel Info use Frame
#V3.7 10/20/00 23:00 more functionable
#V3.8 10/23/00 11:00 Final Version for AOAWS!!
#still have long way to go!!V3.0
#there is no more a long way!!V3.1

############ Open inpute file, get the data, then close file ########
########## position data is in line_data[3][35:49] ####################
########## time stamp is in line_data[4][35:57] #######################
#### line_data(x), x form 13 - 49, map to FL010 - FL490 every FL10 ####

##################################################################################
def data_proceed():###############################################################
##################################################################################

        input_file = open ('/tmp/sounding.class','r')
	line_data = input_file.readlines()
	input_file.close()

#---------------------------- this part is for "time_stamp"-------------------------------------------------------

        posit = ' '
	posit = line_data[3][35:49]
	position = string.split(posit, ',')
	pos_lon = position[0]
	pos_lat = position[1]
	time_stamp = line_data[4][35:57]
        top_title_input = {}
        position_tmp = line_data[2][35:]
        position_tuple = string.split(position_tmp)
        position = position_tuple[0] 

#--------------------------- this part is for the top title of sounding for with position & time data---------------

        month = line_data[4][41:43]
        line2l = "Valid: " + line_data[4][35:39]+"/"+line_data[4][41:43]+"/"+line_data[4][45:47]+" "+line_data[4][49:51]+" UTC" 
        line3l = "Temperature"
        line4l = "Dew Point Temp."
        line3r = "Closest nav point : " + position
        line4r = "lat,lon= " + pos_lat + ", " + pos_lon
        top_title_input={'ln2l':line2l,'ln3l':line3l,'ln4l':line4l,'ln3r':line3r,'ln4r':line4r}

#------use list "snd_data" store each level's(line_data[13:50]) "press","temp","temd","wndir","wndspd" data as a dictionary "snd_data"----- 

        lines = len(line_data)
        snd_data_input = []
        for i in range(13,lines): #start to scan data from each level
#
# Pressure used to be scanned from 8:13 but this causes
# problems if the pressure is greater than or equal to 1000.0
#
           pressure = string.atof(line_data[i][7:13])
           temp = string.atof(line_data[i][13:19])
           temd = string.atof(line_data[i][19:25])
           wndir = string.atoi(line_data[i][52:55])
           wndspd0 = 1.9438 * string.atof(line_data[i][46:49]) #  change wind speed unit from m/s to knot============
           spd1, spd2 = math.modf(wndspd0)
           if ( spd1 >= 0.5):    ###for wind speed, eg.14.3=14, 14.5=15------------------
               wndspd = int(spd2) + 1  
           else:
               wndspd = int(spd2)
           meteo = {'press' : pressure, 'temp' : temp, 'temd' : temd, 'wndir' : wndir, 'wndspd' : wndspd}
           snd_data_input.append((meteo))
        return snd_data_input, top_title_input, month

##########################################################################################################
def parcel_info():#######################calcute for parcel info##########################################
##########################################################################################################

        Tsfc = snd_data[0]['temp']
        Tdsfc = snd_data[0]['temd']
        Presfc = snd_data[0]['press']
        T500 = who_you_are(500,'temp')
        T700 = who_you_are(700,'temp')
        T850 = who_you_are(850,'temp')
        Tsfc_500theta = (snd_data[0]['temp'] + 273.15) * math.pow(500.0/snd_data[0]['press'],0.286) - 273.15
        Tsfc_850theta = (snd_data[0]['temp'] + 273.15) * math.pow(850.0/snd_data[0]['press'],0.286) - 273.15
        Td700 = who_you_are(700,'temd')
        Td850 = who_you_are(850,'temd')
        Wd500 = who_you_are(500,'wndir')
        Wd850 = who_you_are(850,'wndir')
        Wspd500 = who_you_are(500,'wndspd')
        Wspd850 = who_you_are(850,'wndspd')

#======================Index calculate========================================================================

#----------T (Temperature of Surface)---------
        T = Tsfc
#----------LI (Lift Index)--------------------
        LI = T500 - Tsfc_500theta
#----------K (K Index)------------------------
        K = T850 - Td850 -T500 - (T700 - Td700)
#----------TT (Total Totals Index)------------
        TT = T850 - T500 + Td850 - T500
#----------SWI (Showalter Index)??????????????
        for k in range(1200):
            t = (T500 - 0.1*k) * 1.0 
            if (abs(Theta_e(850,Tsfc_850theta) - Theta_e(500,t)) < 1):
                SWI = t - T500
                break
#----------PW (Precipital Water)--------------
        spring = ['02', '03', '04']
        if month in spring:PW = 2.54
        else:PW = 5.09
#----------CAPE ??????????????????????????????
        CAPE = 0.0
#----------CIN ???????????????????????????????
        CIN = 0.0
#----------Tc ????????????????????????????????
        Tc = 0.0
#----------SREH ??????????????????????????????
        SREH = 0.0
#----------CELL ??????????????????????????????
        CELL = 0.0
#----------Td (Dew Point Temp. of Surface)----
        Td = Tdsfc
#----------LCL -------------------------------
        LCLP, LCLT = get_lcl(Tsfc, Tdsfc, Presfc)
 #       base_Ws = Ws(snd_data[0]['press'],snd_data[0]['temd'])
 #       base_Theta = ((snd_data[0]['temp']+273.15) * math.pow((1000./snd_data[0]['press']), 0.286))
 #       print base_Ws, base_Theta
 #       for i in range(1000):
 #           P = snd_data[0]['press'] - i * 0.01
 #           for j in range(100):
 #               T = snd_data[0]['temp'] - j * .1
 #               if ( (abs(base_Ws-Ws(P, T))<1) & 
 #                    (abs(base_Theta-((T+273.15) * math.pow((1000./P), 0.286)))<1) ):
 #                   LCLP = P
 #                   LCLT = T
 #                   break
       # print LCLP, LCLT, LCL0
#---------- LFC -------------------------------
        k_nums = len(snd_data)  # To learn how many lines of data#######
        LFCP = 0.0
        LFCT = 0.0
        for k in range(len(snd_data)):
            if (abs(Ws(snd_data[k]['press'],snd_data[k]['temp']) - Ws(Presfc,Tdsfc)) < 1):
                LFCP = snd_data[k]['press']
                LFCT = snd_data[k]['temp']
                break
#----------EL ????????????????????????????????
        EL = 0.0
#----------CCL ???????????????????????????????
        CCL = 0.0
#----------VGP ???????????????????????????????
        VGP = 0.0
#----------SWEAT (Sweat Index)----------------
        SWEAT = 12 * Td850 + 20 * (TT - 49) + 2 * Wspd850 + Wspd500 + 125 * (math.sin((Wd500-Wd850)*math.pi/180.) + 0.2)
#----------HWBZ ??????????????????????????????
        HWBZ = 0.0

#===================Index ready to export==========================================================================

        pi1l = "T  =  %5.1f" % (T)
        pi2l = "LI =  %5.1f" % (LI)
        pi3l = "K  =  %5.1f" % (K)
        pi4l = "TT =  %5.1f" % (TT)
        pi5l = "SWI = %5.1f" % (SWI)
        pi6l = "PW =  %5.1f" % (PW)
        pi7l = "CAPE= %5.1f" % (CAPE)
        pi8l = "CIN = %5.1f" % (CIN)
        pi9l = "Tc =  %5.1f" % (Tc)
        pi10l = "SREH= %5.1f" % (SREH)
        pi11l = "CELL= %5.1f" %  (CELL)
        pi1r = "Td =  %5.1f" % (Td)
        pi2r = "LCL=  %5.1f" % (LCLP)
        pi3r = "LFC=  %5.1f" % (LFCP)
        pi4r = "EL =  %5.1f" % (EL)
        pi5r = "CCL=  %5.1f" % (CCL)
        pi6r = "VGP=  %5.1f" % (VGP)
        pi7r = "SWEAT=%5.1f" % (SWEAT)
        pi8r = "HWBZ =%5.1f" % (HWBZ)
        pi1 = pi1l+"  "+pi1r
        pi2 = pi2l+"  "+pi2r
        pi3 = pi3l+"  "+pi3r
        pi4 = pi4l+"  "+pi4r
        pi5 = pi5l+"  "+pi5r
        pi6 = pi6l+"  "+pi6r
        pi7 = pi7l+"  "+pi7r
        pi8 = pi8l+"  "+pi8r
        pi9 = pi9l+"             "
        pi10 = pi10l+"             "
        pi11 = pi11l+"             "
        prcl_info = [pi1,pi2,pi3,pi4,pi5,pi6,pi7,pi8,pi9,pi10,pi11,(LCLP,LCLT),(LFCP,LFCT)]
        return prcl_info

########################################################################################
def get_lcl(Temp,TempD,pressure):#######################################################
########################################################################################

        TempK =  Temp + 273.15
        TempDK = TempD + 273.15
        a = 1/(TempDK - 56.0)
        b = math.log(TempK / TempDK) / 800
        TlclK = 1 / (a + b) +56
        Tlcl = TlclK - 273.15
        theta = TempK * math.pow(1000/pressure,0.286)
        Plcl = 1000 * math.pow(TlclK/theta,3.48)
        return Plcl, Tlcl 

#####################################################################################
def Es(T,):#### for water vapor pressure############################################
#####################################################################################
    
        es = 10. * 0.6122 * math.exp((17.67*T)/((T+273.15)-29.65))
        return es
 
#####################################################################################
def Ws(P,T):#### for Saturated Mixing Ratio #########################################
#####################################################################################

        Ws_factor = 629.3706
        es = Es(T)
        Ws = Ws_factor * es / P
        return Ws

#####################################################################################
def Theta_e(P,T):### for Theta e ####################################################
#####################################################################################

        ThetaE =  ((T+273.15)*((1000.0/P)**0.286)) * (math.exp(2.5e3*Ws(P,T)/((T+273.15)*1004.5)) )   
        return ThetaE

#####################################################################################
def ThetaE_plot(P_Bottom, P_Upper, P0, T0, color):###################################
#####################################################################################

        thetaE_line = []
        flag_t = T0 + 5.0
        thetaET =  Theta_e(P0, T0)    
        for j in range((P_Bottom-P_Upper)/5):
            P = (P_Bottom - 5 * j)*1.0
            for k in range(1200):
                t = (flag_t - 0.1*k) * 1.0 
                P_thetaE =  Theta_e(P, t)    
                if (abs(P_thetaE - thetaET) < 1):
                    thetaE_x, thetaE_y = pt_trans_xy(P,t)
                    flag_t = t
                    end_thetaE_x = thetaE_x
                    end_thetaE_y = thetaE_y
                    if (thetaE_x < left_bound):
                         break
                    thetaE_line.append((thetaE_x, thetaE_y))
                    break
            if (thetaE_x < left_bound):
                break
        if (color == 'black'):
            canvas.create_line(thetaE_line, smooth=1, width=1, fill=color, tags=tagxx)
        else:
            for k in range(len(thetaE_line)/2):
                canvas.create_line(thetaE_line[k*2], thetaE_line[k*2+1], width=1, fill='green4')
            canvas.create_text(end_thetaE_x+4, end_thetaE_y+1, text = T0, fill=color, anchor=NW)                 

#####################################################################################
def pt_trans_xy(P,T):################################################################
#####################################################################################
       
        Y = base_y + (math.log(P)-math.log(100.0)) * pressure_factor
        X = (base_x + (graphic_x /(10.0*temp_factor)) * (T + 30.0) +
                 (base_y + graphic_y - Y) / math.tan(math.pi/4.0))
        return X, Y

#####################################################################################
def who_you_are(P,name):#############################################################
#####################################################################################
        i_nums = len(snd_data) - 1  # To learn how many lines of data ##
        for i in range(i_nums):
            if (P <= snd_data[i]['press']) & (P > snd_data[i+1]['press']):
                value_P = snd_data[i][name] + ((snd_data[i+1][name]-snd_data[i][name]) * (P-snd_data[i]['press']) / (snd_data[i+1]['press']-snd_data[i]['press']))
        return value_P


#####################################################################################
def draw_base():#####################################################################
#####################################################################################

#----------- draw the bounary of 4 lines, below is top, bottom, left, right lines ---------------------

        canvas.create_line(base_x,base_y,base_x+graphic_x,base_y, width=2)
        canvas.create_line(base_x,base_y+graphic_y,base_x+graphic_x,base_y+graphic_y,width=2)
        canvas.create_line(base_x,base_y,base_x,base_y+graphic_y,width=2)
        canvas.create_line(base_x+graphic_x,base_y,base_x+graphic_x,base_y+graphic_y,width=2)

###########################################SKEW T(X) axis#####################################################################

#----------below is to make the skew-x axis(temp) scale. the slope is 45 deg(pi/4)-----------------------------------------
        for i in range(temp_factor-1):    #------------------ from bottom, temp tag at bottom------------------------------------
            Lpoint_x = base_x + ((i + 1) * graphic_x) / temp_factor
            Lpoint_y = base_y + graphic_y
            Rpoint_x = base_x + graphic_x
            Rpoint_y = base_y + graphic_y - (base_x + graphic_x - Lpoint_x) * math.tan(math.pi/4.0)
            if (Rpoint_y < top_bound):
                Rpoint_x = Rpoint_x - ((top_bound - Rpoint_y) * math.tan(math.pi/4.0))
                Rpoint_y = top_bound
                top_temp_base_x = Rpoint_x  #base point of making skew-x(temp) axis scale from top line
            if (Rpoint_x > right_bound):
                Rpoint_x = right_bound
                Rpoint_y = base_y + Lpoint_y- base_y - graphic_x
            canvas.create_line(Lpoint_x,Lpoint_y,Rpoint_x,Rpoint_y,width=1)
            canvas.create_text(Rpoint_x+4,Rpoint_y, text='%d' % ((10*i)-20), anchor=NW)
        for i in range(graphic_y * temp_factor / graphic_x): #-----from top, temp tag at top---------------------------
            Lpoint_x = base_x
            Lpoint_y = base_y + graphic_y - i * graphic_x / temp_factor
            Rpoint_x = base_x + (Lpoint_y - base_y) * math.tan(math.pi/4.0)
            Rpoint_y = base_y
            if (Rpoint_x > right_bound):
                Rpoint_x = right_bound
                Rpoint_y = base_y + Lpoint_y- base_y - graphic_x
            canvas.create_line(Lpoint_x, Lpoint_y, Rpoint_x, Rpoint_y, width = 1)   
            canvas.create_text(Rpoint_x, Rpoint_y, text='%d' % ((-10*(i+1))-20), anchor=SW)

######################################### log P(Y) axis #####################################################################33

#----------below is to make the y axis scale(ln(pressure))----------------------------------
        pressure_factor = graphic_y / (math.log(1050)-math.log(100))
        for i in range(9):
            y = base_y + (math.log((i+2)*100)-math.log(100)) * pressure_factor
            lnp_x = base_x
            canvas.create_line(lnp_x,y,base_x+graphic_x,y,width=1)
            canvas.create_text(base_x-15,y,text='%4d' % ((i+2)*100))

########################################## theta T ##############################################################################

        for i in range(15+6):#theta 250 - 390 degree K + 60 fegree K
            theta = 250 + 10 * i
            theta_line = []
            for j in range((1050-90)/5):
                P = 1050 - 5 * j
                T = (theta * math.pow((P/1000.0) , 0.286)) - 273.15
                theta_x, theta_y = pt_trans_xy(P,T)
                end_theta_x = theta_x
                end_theta_y = theta_y
                if ((theta_x < left_bound) | (theta_y < top_bound )):# | ((theta_x > left_prcl_bound) & (theta_y < bottom_prcl_bound))):
                    break
                if (theta_x < right_bound):
                    theta_line.append((theta_x, theta_y))
            canvas.create_line(theta_line, width=1, smooth=1, fill='red')
            canvas.create_text(end_theta_x+8+i, end_theta_y+15, text = theta, fill='red', anchor=NW)                 
            
########################################## theta e T ##############################################################################

        for T in range(-12,40,4):#theta -12 - 36 degree K
            ThetaE_plot(1050, 195, 1000., T, 'green4')

########################################## Saturated Mixing Ratio ##############################################################################

        for Ws_base in (2, 3, 5, 8, 12, 20):# Ws = 2, 3, 5, 8, 12, 20
            Ws_line = []
            for j in range((1050-680)/10):
                P = 1050. - 10 * j
                flat_t = 26.0
                for t in range(500):
                    T = flat_t - t * 0.1
                    Pesu_Ws = Ws(P,T) 
                    if (abs(Ws_base - Pesu_Ws) < .1):
                        flat_t = t
                        Ws_x, Ws_y = pt_trans_xy(P,T)
                        end_Ws_x = Ws_x
                        end_Ws_y = Ws_y
                        if (Ws_y < Ws_bound):
                            Ws_line.append((Ws_x, Ws_y))
                        break

            for k in range(len(Ws_line)/4):
                canvas.create_line(Ws_line[k*4], Ws_line[k*4+2], width=1, fill='yellow4')
            canvas.create_text(end_Ws_x+5, end_Ws_y-3, text = Ws_base, font=('MS',12), fill='yellow4')                 
#------------------------------------------------------------------------------------------------------------------------------

#####################################################################################
def draw_graphic():###########################################################
#####################################################################################

        snd_data, top_title, month = data_proceed()  # data reading and processing    
#        prcl_info_line = parcel_info()

#---------------Print top title text -----------------------------------------------------------------------

        canvas.create_text(425,17,text='MM5 Model Sounding',fill='black', font=('MS',22),tags=tagxx)
        canvas.create_text(10,40,text=top_title['ln2l'],fill='black',font=('MS',18),anchor=W,tags=tagxx)
        canvas.create_text(10,60,text=top_title['ln3l'],fill='blue' ,font=('MS',18),anchor=W,tags=tagxx)
        canvas.create_text(10,80,text=top_title['ln4l'],fill='red'  ,font=('MS',18),anchor=W,tags=tagxx)
        canvas.create_text(canvas_x-10,60,text=top_title['ln3r'],fill='blue' ,font=('MS',18),anchor=E,tags=tagxx)
        canvas.create_text(canvas_x-10,80,text=top_title['ln4r'],fill='red' ,font=('MS',18),anchor=E,tags=tagxx)

#################### draw wind data##############################################################################################
        windplot_x = (canvas_x+base_x+graphic_x)/2 + 8
        canvas.create_line(windplot_x,base_y,windplot_x,base_y+graphic_y, width=2,tags=tagxx)  #draw a vertical axis for vertical wind-------------
        temp_press_line = []   #position set for pressure, tempurature
        temd_press_line = []   #position set for pressure, dew point
        wnd_dir_length = 35    #changable parament for the length of wind directory bar
        i_nums = len(snd_data) # To learn how many lines of data ##
        for i in range(i_nums): #start to draw the graphiic ----------------------------------------------------------------
            press_y = base_y + (math.log(snd_data[i]['press'])-math.log(100.0)) * pressure_factor
            wndbar_base_x = windplot_x+wnd_dir_length*(math.sin((snd_data[i]['wndir']*math.pi)/180.0))
            wndbar_base_y = press_y-wnd_dir_length*(math.cos((snd_data[i]['wndir']*math.pi)/180.0))
            canvas.create_line(windplot_x, press_y, wndbar_base_x, wndbar_base_y, width=2, fill='blue',tags=tagxx)         # draw wind directory bar
#------------------this part is to calculate how many wind speed bars(50m/s ,10m/s, 5m/s)----------------------------------------------------------
            arrow50=arrow10=arrow5=0
            arrow50 = snd_data[i]['wndspd'] / 50
            arrow10 = (snd_data[i]['wndspd'] % 50) / 10
            if ((snd_data[i]['wndspd'] % 50) % 10) > 7.0:#when wind speed is 8~9 ==>10m/s
                arrow10 = arrow10 + 1
                if arrow10 == 5:
                    arrow50 = arrow50 + 1
                    arrow10 = 0
            else:
                if ((snd_data[i]['wndspd'] % 50) % 10) > 2.0:#when wind speed is 3~7 ==>5, 1~2 ==>0
                    arrow5 = 1
#------------------draw the wind speed arrows in wind dir. bar----------------------------------------------------------
            for ibar50 in range(arrow50):#draw each level's each 50m/s  wind speed bar, use two canvas.create_line() to make a triangle area for 50 m/s wind
                canvas.create_polygon(wndbar_base_x,wndbar_base_y,wndbar_base_x+math.sin(math.pi*(snd_data[i]['wndir']+60)/180.0)
                                   *(wnd_dir_length/2), wndbar_base_y-(math.cos(math.pi*(snd_data[i]['wndir']+60)/180.0))*(wnd_dir_length/2),
                                  windplot_x+(math.sin((snd_data[i]['wndir']*math.pi)/180.0))*wnd_dir_length*(1.0-0.18*(ibar50+1)),
                                  press_y-(math.cos((snd_data[i]['wndir']*math.pi)/180.0))*wnd_dir_length*(1.0-0.18*(ibar50+1)),width=1,fill='blue',tags=tagxx)
#--------------------change position to the next speed bar------------------------------------------------------------------------------------
                wndbar_base_x = windplot_x+(math.sin((snd_data[i]['wndir']*math.pi)/180.0))*wnd_dir_length*(1.0-0.18*(ibar50+1))
                wndbar_base_y = press_y-(math.cos((snd_data[i]['wndir']*math.pi)/180.0))*wnd_dir_length*(1.0-0.18*(ibar50+1))
            for ibar10 in range(arrow10):#draw 10 m/s wind speed bar--------------------------------------
                canvas.create_line(wndbar_base_x,wndbar_base_y,wndbar_base_x+(math.sin(math.pi*(snd_data[i]['wndir']+60)/180))
                                   *(wnd_dir_length/2), wndbar_base_y-(math.cos(math.pi*(snd_data[i]['wndir']+60)/180))
                                   *(wnd_dir_length/2), width=1, fill='blue',tags=tagxx)
#--------------------change position to the next speed bar------------------------------------------------------------------------------------
                wndbar_base_x = windplot_x+(math.sin((snd_data[i]['wndir']*math.pi)/180.0))*wnd_dir_length*(1.0-0.18*arrow50-0.10*(ibar10+1))
                wndbar_base_y = press_y-(math.cos((snd_data[i]['wndir']*math.pi)/180.0))*wnd_dir_length*(1.0-0.18*arrow50-0.10*(ibar10+1))
            if arrow5:#draw 5m/s wind speed bar--------------------------------------------------------------------------------------------
                if (arrow50==0)&(arrow10==0):  #special case for (2 < wind speed < 8)
                    wndbar_base_x = windplot_x+(math.sin((snd_data[i]['wndir']*math.pi)/180.0))*wnd_dir_length*(1.0-0.18)
                    wndbar_base_y = press_y-(math.cos((snd_data[i]['wndir']*math.pi)/180.0))*wnd_dir_length*(1.0-0.18)
                canvas.create_line(wndbar_base_x,wndbar_base_y,wndbar_base_x+(math.sin(math.pi*(snd_data[i]['wndir']+60)/180))
                                   *(wnd_dir_length/3), wndbar_base_y-(math.cos(math.pi*(snd_data[i]['wndir']+60)/180))
                                   *(wnd_dir_length/3), width=1, fill='blue',tags=tagxx)
#---------------------just for other to check the wind output is correct or wrong, will be delete after beta version---------------------
#            canvas.create_text(windplot_x-15,press_y,text='%s' % snd_data[i]['wndir'], fill='black',tags=tagxx)
#            canvas.create_text(windplot_x+28,press_y,text='%s' % snd_data[i]['wndspd'], fill='red',tags=tagxx)

####################### draw temp, dew point #########################################################################################

#----------transfer Temp & Td into skew line------------------------------------------------------------------------------------
            temp_x, temp_y = pt_trans_xy(snd_data[i]['press'], snd_data[i]['temp'])
            temd_x, temd_y = pt_trans_xy(snd_data[i]['press'], snd_data[i]['temd'])

#---------graw the ln(p)-skewT & ln(p)-skewTd lines-----------------------------------------------------------------
            temp_press_line.append((temp_x, temp_y))
            temd_press_line.append((temd_x, temd_y))
        canvas.create_line(temp_press_line, width = 2, fill = 'blue',tags=tagxx)          
        canvas.create_line(temd_press_line, width = 2, fill = 'red',tags=tagxx)

######################## Plot <-- LCL mark ####################################################################3

#        LCLP, LCLT = prcl_info_line[-2]
#        LCLx, LCLy = pt_trans_xy(LCLP, LCLT)
#        canvas.create_text(LCLx,LCLy,text="<--LCL",fill="blue",anchor=W,tags=tagxx)

######################## Plot LFC --> mark ####################################################################3

#        LFCP, LFCT = prcl_info_line[-1]
#        if (LFCP != 0.0):
#            LFCx, LFCy = pt_trans_xy(LFCP, LFCT)
#            canvas.create_text(LFCx,LFCy,text="LFC-->",fill="blue",anchor=E,tags=tagxx)

################################## print parcel infomation ##############################################################################

#        frm = Frame(canvas, relief=GROOVE, borderwidth=0, bg='white')
#        Label(frm, text="  Parcel Info  ", fg='blue', bg='white', font=("MS",15)).pack()
#        for i in range(11):
#            Label(frm, text=prcl_info_line[i], fg='blue', bg='white').pack()
#        canvas.create_window(base_x+graphic_x-2, base_y+2 ,window=frm, anchor= NE, tags=tagxx)
#------------------------------------------------------------------------------------------------------------------
#        ThetaE_plot(LCLP, 200, LCLP, LCLT, 'black')  # plot the parcel track, 'black' means without label #########

#==========================================================================
#####################################################################################
def new_graphic(parent,):###########################################################
#####################################################################################

        global top_title, snd_data, month

        time_tag = top_title['ln4r'] + top_title['ln2l']
        snd_data0, top_title0, month0 = data_proceed()  # data reading and processing    
        time_change = top_title0['ln4r'] + top_title0['ln2l']

        if (time_tag != time_change):
            top_title = top_title0
            snd_data = snd_data0
            month = month0
            canvas.delete(tagxx)
            draw_graphic() 
        parent.after(3000,new_graphic,root,)#---------------------------00


#$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
################################### Main program #######################################################
#$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

import string, math, sys, os  
from Tkinter import *

if __name__ == '__main__':
        canvas_x = 750
        canvas_y = 780
        base_x = 50
        base_y = 120
        graphic_x = canvas_x - base_x - 100
        graphic_y = canvas_y - base_y - 30
        temp_factor = 8  #---decide the x(temp) axis intervals-----
        pressure_factor = graphic_y / (math.log(1050)-math.log(100))#decide Y axis from log(1050)-log(100)
        top_bound = base_y
        bottom_bound = base_y + graphic_y
        right_bound = base_x + graphic_x
        left_bound = base_x
        Ws_bound = bottom_bound - math.log(1050/700) * graphic_y / math.log(1050/100)
        snd_data, top_title, month = data_proceed()  # data reading and processing    
        time_tag = top_title['ln4r'] +top_title['ln2l']
        tagxx = "to_be_refreshed"

#-------------------------------------------------------------------------------------------------

        root = Tk()#-----------------------------00
        root.title('Sounding')#-----------------------00
        canvas = Canvas(root, width=canvas_x, height=canvas_y, bg = 'white')  #setup canvas size
        canvas.pack()
#        Button(root, text='BYE!!', command=root.quit).pack()

        draw_base()
        draw_graphic()  # draw sounding graphic
        new_graphic(root,)

        root.mainloop()#-------------------00
