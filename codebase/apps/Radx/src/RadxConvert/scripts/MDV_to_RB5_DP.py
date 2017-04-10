# -*- coding: utf-8 -*-
"""
Created on Fri Feb 17 14:15:54 2017

@author: Erik Becker <erik.becker@weathersa.co.za>
"""

import matplotlib.pyplot as plt

def plotImage(arr) :
    #arr[arr <= -99.0] = np.nan
    fig  = plt.figure(figsize=(5,5), dpi=200, facecolor='w',edgecolor='w',frameon=True)
    imAx = plt.imshow(arr, origin='lower', interpolation='nearest')
    imAx = plt.imshow(arr)
    fig.colorbar(imAx, pad=0.01, fraction=0.1, shrink=1.00, aspect=20)
    plt.clim(np.nanmin(arr),np.nanmax(arr))
    plt.show()

import numpy as np
import os
import array
import zlib
import struct
import sys

import time


class ReadMdv(object):
    
    def __init__(self, filename, **kwargs):
        
                
        #Open file as read binary
        self.mdvfile=open(filename, 'rb') 
                
        #Format Charactors of type (unsigned char, unsigned short, signed int, unsigned int, float, char)
        self.data_types = {'ui08':'B' ,'ui16':'h' ,'si32':'i' ,'ui32':'I' ,'fl32':'f','char':'s' }  

        #MDV file structure constants
        self.MDV_CHUNK_INFO_LEN = 480
        self.MDV_INFO_LEN = 512
        self.MDV_LONG_FIELD_LEN = 64
        self.MDV_MAX_PROJ_PARAMS = 8
        self.MDV_MAX_VLEVELS = 122
        self.MDV_NAME_LEN = 128
        self.MDV_SHORT_FIELD_LEN = 16
        self.MDV_TRANSFORM_LEN = 16
        self.MDV_UNITS_LEN = 16
        self.MDV_N_COORD_LABELS = 3
        self.MDV_COORD_UNITS_LEN = 32
        
        ### Get Master Header
        self.get_master_header()
        #print(self.master_header)   

        ### Get field Header
        self.get_field_headers()       
        #print(self.field_headers)      
        
        ### Get Vlevel Header
        self.get_Vlevel_headers()
        #print(self.Vlevel_headers)    

        ### Get Chunk Header
        if (self.master_header['n_chunks'] > 0):
            self.get_chunk_headers()
            #print(self.chunk_headers)       

        self.DsRadarParams_ID = 3            
        self.DsRadarElevs_ID = 4
  
        if (self.master_header['n_chunks'] > 0):
            ### Get Chunk Data            
            self.get_chunk_data()       

        ### Read Field Data Data            
        self.field_data = self.read_field_data()      

                
                
    def get_master_header(self):
        endian='>'
        MH_names=["record_len1","struct_id","revision_number","time_gen","user_time","time_begin","time_end","time_centroid",
                  "time_expire","num_data_times","index_number","data_dimension","data_collection_type","user_data",
                  "native_vlevel_type","vlevel_type","vlevel_included","grid_orientation","data_ordering","nfields","ngates",
                  "nrays","nsweeps","n_chunks","field_hdr_offset","vlevel_hdr_offset","chunk_hdr_offset","field_grids_differ",
                  "user_data_si32","time_written","unused_si32","user_data_fl32","sensor_lon","sensor_lat","sensor_alt",
                  "unused_fl32","data_set_info","data_set_name","data_set_source","record_len2"]
        
        i = self.data_types['si32']               
        f = self.data_types['fl32']
        c = self.data_types['char']
                
        MH_types= 28*[i] + ['8'+i] + [i] + ['5'+i] + ['6'+f] + 3*[f] + ['12'+f] + [str(self.MDV_INFO_LEN)+c] + [str(self.MDV_NAME_LEN)+c] + [str(self.MDV_NAME_LEN)+c] + [i]

        self.master_header = self.read_mdv_bytes(MH_names, MH_types, endian)
        
    def read_mdv_bytes(self, names, types, endian):
        
        byte_data = {}
        for i in range(len(types)):
            nbytes=struct.calcsize(types[i])
            bin_data=self.mdvfile.read(nbytes)
                     
            if 's' in types[i]:    
                byte_data.update({names[i]:  str(bin_data.decode('utf-8')).split('\x00')[0]})
            else:
                byte_data.update({names[i]: struct.unpack(endian+types[i], bin_data)})
                
            if len(byte_data[names[i]])==1:
                byte_data[names[i]]=byte_data[names[i]][0]
                
        return byte_data
        
        
    def get_field_headers(self):
        endian='>'
        FH_names=["record_len1","struct_id","field_code","user_time1","forecast_delta","user_time2","user_time3","forecast_time",
                  "user_time4","nx","ny","nz","proj_type","encoding_type","data_element_nbytes","field_data_offset",
                  "volume_size","user_data_si32","compression_type","transform_type","scaling_type","native_vlevel_type","vlevel_type",
                  "dz_constant","data_dimension","zoom_clipped","zoom_no_overlap","unused_si32","proj_origin_lat","proj_origin_lon",
                  "proj_param","vert_reference","grid_dx","grid_dy","grid_dz","grid_minx","grid_miny","grid_minz","scale","bias",
                  "bad_data_value","missing_data_value","proj_rotation","user_data_fl32","min_value","max_value","min_value_orig_vol",
                  "max_value_orig_vol","unused_fl32","field_name_long","field_name","units","transform","unused_char","record_len2"]
        
        i = self.data_types['si32']               
        f = self.data_types['fl32']
        c = self.data_types['char']   
        
        FH_types= 17*[i] + ['10'+i] + 9*[i] + ['4'+i] + 2*[f] + [str(self.MDV_MAX_PROJ_PARAMS)+f] + 12*[f] + ['4'+f] + 5*[f] + [str(self.MDV_LONG_FIELD_LEN)+c] + [str(self.MDV_SHORT_FIELD_LEN)+c] + [str(self.MDV_UNITS_LEN)+c] + [str(self.MDV_TRANSFORM_LEN)+c] + [str(self.MDV_UNITS_LEN)+c] + [i]
        
        nfields = self.master_header['nfields']
        
        field_header_list = []
        for i in range(nfields):
            temp_dict=self.read_mdv_bytes(FH_names, FH_types, endian)
            field_header_list.append(temp_dict)
        
        self.field_headers=field_header_list
        
    def get_Vlevel_headers(self):
        endian='>'
        VH_names=["record_len1","struct_id","type","unused_si32","level","unused_fl32","record_len2"]
        
        i = self.data_types['si32']               
        f = self.data_types['fl32']
        
        VH_types = 2*[i] + [str(self.MDV_MAX_VLEVELS)+i] + ['4'+i] + [str(self.MDV_MAX_VLEVELS)+f] + ['5'+f] + [i]


        nfields = self.master_header['nfields']
        
        vlevel_header_list = []
        for i in range(nfields):
            temp_dict=self.read_mdv_bytes(VH_names, VH_types, endian)
            vlevel_header_list.append(temp_dict)
        
        self.Vlevel_headers=vlevel_header_list

    def get_chunk_headers(self):
        endian='>'
        CH_names=["record_len1","struct_id","chunk_id","chunk_data_offset","size","unused_si32","info","record_len2"]
        
        i = self.data_types['si32']               
        c = self.data_types['char']
        
        CH_types= 5*[i] + ['2'+i] + [str(self.MDV_CHUNK_INFO_LEN)+c] + [i]
    
        n_chunks = self.master_header['n_chunks']
    
        chunk_header_list=[]
        for i in range(n_chunks):
            temp_dict=self.read_mdv_bytes(CH_names, CH_types, endian)
            chunk_header_list.append(temp_dict)

        self.chunk_headers=chunk_header_list
        
    def get_chunk_data(self):
        
        #One can find this info by using print_mdv -full "filename" on the commmand line
        
        #print(self.chunk_headers)
        
        for i in range(len(self.chunk_headers)):
            
            if self.chunk_headers[i]['chunk_id'] == self.DsRadarParams_ID:
                self.DsRadarParams = self.read_DsRadarParams(self.chunk_headers[i]['chunk_data_offset'])
                #print(self.DsRadarParams)
                
            if self.chunk_headers[i]['chunk_id'] == self.DsRadarElevs_ID:
                self.DsElev = self.read_elevs(self.chunk_headers[i]['chunk_data_offset'])
                #print(self.DsElev)

    def read_DsRadarParams(self,offset):
                
        self.mdvfile.seek(offset)
        
        endian = '>'
        DS_LABEL_LEN = 40
        
        #STRUCT INFO IN rapformats/ds_radar.h
        DsParam_Names = ['radar_id','radar_type','nfields','ngates','samples_per_beam','scan_type','scan_mode','nfields_current',
                         'field_flag','polarization','follow_mode','prf_mode','spare_ints','radar_constant','altitude','latitude',
                         'longitude','gate_spacing','start_range','horiz_beam_width','vert_beam_width','pulse_width','prf',
                         'wavelength','xmit_peak_pwr','receiver_mds','receiver_gain','antenna_gain','system_gain','unambig_vel',
                         'unambig_range','measXmitPowerDbmH','measXmitPowerDbmV','prt','prt2','spare_floats','radar_name','scan_type_name']
                         
        i = self.data_types['si32']               
        f = self.data_types['fl32']
        c = self.data_types['char']   
        
        DsParam_types = 12*[i] + ['2'+i] + 22*[f] + ['4'+f] + 2*[str(DS_LABEL_LEN)+c]
        
        param_data = self.read_mdv_bytes(DsParam_Names, DsParam_types, endian)        
                
        return param_data

    def read_elevs(self,offset):
        
        self.mdvfile.seek(offset)
        
        endian = '>'
        
        i = self.data_types['si32']               
        f = self.data_types['fl32']
        
        #STRUCT INFO IN rapformats/ds_radar.h
        n_elev = self.read_mdv_bytes(['n_elev'], [i], endian) 
        n_elev.update(self.read_mdv_bytes(['elev'], [str(n_elev['n_elev'])+f], endian))
        
        return n_elev
    
    def get_compression_info(self):
        
        endian = '>'
        I = self.data_types['ui32']
        
        compress_names = ['magic_cookie','nbytes_uncompressed','nbytes_compressed','nbytes_coded','spare']
        compress_types = 4*[I] + ['2'+I]
        
        comp_data = self.read_mdv_bytes(compress_names, compress_types, endian)
        
        return comp_data
        
    def decompress_data(self,compression):
        
        ### ONLY HANDLING ZLIB_COMPRESSED AT THE MOMENT
        
        hex_compress = hex(compression['magic_cookie'])
        
        #print(hex_compress)
        
#        TA_NOT_COMPRESSED = hex(0x2f2f2f2f)   #The data is not compressed. Use as is it.
        GZIP_COMPRESSED = hex(0xf7f7f7f7)     #The data was compressed using GZIP. Uncompressed appropriately.
#        GZIP_NOT_COMPRESSED = hex(0xf8f8f8f8) #GZIP compression was tried, but it failed. The data is not compressed. Use as it is.
#        BZIP_COMPRESSED = hex(0xf3f3f3f3)     #The data was compressed with BZIP2 version 0.9.0c. Uncompress appropriately.
#        BZIP_NOT_COMPRESSED = hex(0xf4f4f4f4) #BZIP2 compression was tried, but it failed. The data is not compressed. Use it as it is.
        ZLIB_COMPRESSED = hex(0xf5f5f5f5)     #The data was compressed with ZLIB compression. This is the same as GZIP but without the GZIP header structure. Uncompress appropriately.
#        ZLIB_NOT_COMPRESSED = hex(0xf6f6f6f6) #ZLIB compression was tried, but it failed. Data is not compressed, us it as it is.
        
        if hex_compress == ZLIB_COMPRESSED:
            nbytes = compression['nbytes_compressed']
            bin_data=self.mdvfile.read(nbytes)
            
            uncompressed_bytes = zlib.decompress(bin_data)
            
            return uncompressed_bytes
            
        if hex_compress == GZIP_COMPRESSED:
            nbytes = compression['nbytes_compressed']
            bin_data=self.mdvfile.read(nbytes)
            
            #print(bin_data)

            uncompressed_bytes = zlib.decompress(bytes(bin_data), 15+32)
            
            return uncompressed_bytes
            
    def scale_data(self,uncompressed_bytes,encoding_type,data_element_nbytes,transform_type,scaling_type,scale,bias):
        
        endian = '>'
        
        ENCODING_INT8 = 1 
        ENCODING_INT16 = 2 
        ENCODING_FLOAT32 = 5 
        ENCODING_RFBA = 7 
        
        if encoding_type == ENCODING_INT8:
            encoding = self.data_types['ui08']
        elif encoding_type == ENCODING_INT16:
            encoding = self.data_types['ui16']
        elif encoding_type == ENCODING_FLOAT32:
            encoding = self.data_types['fl32']
        elif encoding_type == ENCODING_RFBA:
            print('ENCODING_RFBA needs to be added to src code. Exiting Program!')
            sys.exit(0)
        
        types = str(len(uncompressed_bytes)//data_element_nbytes) + encoding
        data = struct.unpack(endian+types, uncompressed_bytes)
        
#        SCALING_ROUNDED = 1
#        SCALING_INTEGRAL = 2
#        SCALING_DYNAMIC = 3
#        SCALING_SPECIFIED = 4
        
        sdata = np.array(data) * scale + bias    
        
        DATA_TRANSFORM_NONE = 0
        DATA_TRANSFORM_LOG = 1
        
        if transform_type == DATA_TRANSFORM_NONE:
            return sdata
        elif transform_type == DATA_TRANSFORM_LOG:
            print('DATA_TRANSFORM_LOG needs to be added to src code. Exiting Program!')
            sys.exit(0)

    def read_field_data(self):
        
        endian = '>'
        nfields = self.master_header['nfields']
        
        PROJ_LATLON = 0
        PROJ_LAMBERT_CONF = 3
        PROJ_FLAT = 8
        PROJ_POLAR_RADAR = 9
        PROJ_OBLIQUE_STEREO = 12
        PROJ_RHI_RADAR = 13
        
        I = self.data_types['ui32']
        
        data = {}
        self.field_names = []
        
        for i in range(nfields):
            
            proj_type = self.field_headers[i]['proj_type']
            offset = self.field_headers[i]['field_data_offset']
            nx = self.field_headers[i]['nx']
            ny = self.field_headers[i]['ny']
            nz = self.field_headers[i]['nz']
            field_name = self.field_headers[i]['field_name']
            
            encoding_type = self.field_headers[i]['encoding_type']
            data_element_nbytes = self.field_headers[i]['data_element_nbytes']
            transform_type = self.field_headers[i]['transform_type']
            scaling_type = self.field_headers[i]['scaling_type']
            scale = self.field_headers[i]['scale']
            bias = self.field_headers[i]['bias']
            
            self.mdvfile.seek(offset)
            
            vlevel_names = ['vlevel_offsets', 'vlevel_nbytes']
            vlevel_types = [str(nz)+I]*2            
            vlevel = self.read_mdv_bytes(vlevel_names, vlevel_types, endian)
            
            vlevel_nbytes = struct.calcsize("".join(vlevel_types))
            
            vdata = np.ones((nz,ny,nx),dtype=np.float)
            
            for j in range(len(vlevel['vlevel_nbytes'])):
                
                voffset = offset + vlevel['vlevel_offsets'][j] + vlevel_nbytes
                self.mdvfile.seek(voffset)
                
                compression = self.get_compression_info()
                uncompressed_bytes = self.decompress_data(compression)  
                
                sdata = np.array(self.scale_data(uncompressed_bytes,encoding_type,data_element_nbytes,transform_type,scaling_type,scale,bias))
                vdata[j,:,:] = sdata.reshape((ny,nx))
                
                if proj_type == PROJ_FLAT:
                    vdata[j,:,:] = vdata[j,::-1,:]
                elif proj_type == PROJ_POLAR_RADAR:
                    vdata[j,:,:] = vdata[j,:,:]

            data[field_name] = vdata
            self.field_names.append(field_name)
            
        return data
        

class MDV_to_RB5(object):
    
    def __init__(self, MDV, output_dir='.'):

        self.MDV = MDV
        
        self.File_Time = time.gmtime(self.MDV.master_header['time_begin'])
        
        self.cmprMethod = "qt"
        
        convert_field = ['DBZ', 'DBZ_F','VEL_F','WIDTH_F', 'KDP', 'PHIDP', 'RHOHV', 'SNR', 'ZDR' ]
        
        for field in MDV.field_names:
            if field in convert_field:
                
                self.sRB5_file = self.get_RB5_file_path(field,output_dir)
                
                self.rb5_File = open(self.sRB5_file, "w" )
                
                self.get_RB5_pargroup_var()
                self.write_RB5_pargroup()
                
                for elev in range(self.numEle):
                    self.get_RB5_slice_var(elev,field)
                    self.write_RB5_slice(elev)
                    
                self.get_RB5_sensorinfo()
                self.write_RB5_sensorinfo()
                
                self.rb5_File.close()
                
                self.write_blobs(field)

                
    def get_RB5_file_path(self,field,output_dir):
        
        if field=='DBZ_F': ext='dBZ.vol'
        if field=='DBZ': ext='dBuZ.vol'
        if field=='VEL_F': ext='V.vol'
        if field=='WIDTH_F': ext='W.vol'
        if field=='KDP': ext='KDP.vol'
        if field=='PHIDP': ext='PhiDP.vol'
        if field=='RHOHV': ext='RhoHV.vol'
        if field=='SNR': ext='SNR.vol'
        if field=='ZDR': ext='ZDR.vol'
        
        ftime = time.strftime("%Y%m%d%H%M%S00",self.File_Time)
        
        time_dir = time.strftime("%Y%m%d",self.File_Time)
        
        if not os.path.exists(output_dir + '/' + time_dir):
            os.makedirs(output_dir + '/' + time_dir)
            
        rb5_path = output_dir + '/' + time_dir + '/' + ftime + ext
        
        return rb5_path
        
    def get_RB5_pargroup_var(self):
        
        self.curdt = time.strftime("%Y-%m-%dT%H:%M:%S",time.gmtime(self.MDV.master_header['time_centroid']))   # datetime="2017-02-13T17:53:01"
        self.scantype = "vol"                                                                                  # type="vol"
        self.sdf =  self.MDV.DsRadarParams['scan_type_name'] + ".vol"                                          # scan name="volume_200km.vol"  
        self.dataTime = time.strftime("%H:%M:%S",self.File_Time)                                               # time="17:48:10"
        self.dataDate = time.strftime("%Y-%m-%d",self.File_Time)                                               # date="2017-02-13"
        self.scanTime = 360                                                                                    # <scantime>326</scantime>
        self.numEle = self.MDV.DsElev['n_elev']                                                                # <numele>12</numele>
        self.rangeSamp = 1                                                                                     # <rangesamp>1</rangesamp> 
        self.timeSamp = 20                                                                                     # <timesamp>31</timesamp>
        self.antSpeed = 24                                                                                     # <antspeed>24</antspeed>
           
    def write_RB5_pargroup(self):
        
        self.rb5_File.write('<volume version="5.30.0" datetime="%s" type="%s" owner="" >\n' % (self.curdt, self.scantype))
        self.rb5_File.write('   <scan name="%s" time="%s" date="%s" >\n' % (self.sdf, self.dataTime, self.dataDate))
        self.rb5_File.write('      <unitid>SI</unitid>\n')
        self.rb5_File.write('      <advancedchanged>0</advancedchanged>\n')
        self.rb5_File.write('      <detailedchanged>0</detailedchanged>\n')
        self.rb5_File.write('      <scantime>%d</scantime>\n' % ( self.scanTime,))
        self.rb5_File.write('      <pargroup refid="sdfbase" >\n')
        self.rb5_File.write('         <numele>%d</numele>\n' % ( self.numEle,))
        self.rb5_File.write('         <rangesamp>%s</rangesamp>\n' % (self.rangeSamp,))
        self.rb5_File.write('         <timesamp>%s</timesamp>\n' % (self.timeSamp,))
        self.rb5_File.write('         <antspeed>%s</antspeed>\n' % (self.antSpeed,))
        self.rb5_File.write('      </pargroup>\n')
        
    def get_RB5_slice_var(self,elev,field):
        
        field_idx = self.MDV.field_names.index(field)
        
        self.refid = elev                                                                    #refid="%d"
        self.dynz_min, self.dynz_max =  "-31.5","95.5"                                       #<dynz min="-31.5" max="95.5" />
        self.dynv_min, self.dynv_max =  "-32.6","32.6"                                       #<dynv min="-40.035" max="40.035" />
        self.dynw_min, self.dynw_max =  "0.0","20.0"                                         #<dynw min="0.0781934" max="19.9393" />
        self.dynzdr_min, self.dynzdr_max =  "-40.0","40.0"                                   #<dynzdr min="-39.195" max="39.195" />
        self.dynphidp_min, self.dynphidp_max =  "-180.0","180.0"                             #
        self.dynrhohv_min, self.dynrhohv_max =  "0.0","1.0"                                  #
        self.posangle = round(self.MDV.DsElev['elev'][elev],1)                               #<posangle>0.5</posangle>
        self.startangle = 0.0                                                                #<startangle>0.0</startangle>
        self.stopangle = 360.0                                                               #<stopangle>0.0</stopangle>
        self.stoprange = round(self.MDV.field_headers[field_idx]['nx'] * self.MDV.field_headers[field_idx]['grid_dx'],0)     #<stoprange>200</stoprange>
        self.rangestep = round(self.MDV.field_headers[field_idx]['grid_dx'],2)               #<rangestep>0.5</rangestep>
        
        self.stagger     = {"0":"None", "1":"3/2", "2":"4/3", "3":"5/4"}[ "0" ]              #<stagger>3/2</stagger>
        self.highPrf     = 250
        self.lowPrf      = 0                                                                 #<lowprf>500</lowprf>
        self.filterwidth = 0                                                                 
        self.filterdepth = 0                                              
        self.filtermode  = 'FFT'                                                             #<filtermode>FFT</filtermode>
        self.rangesamp   = 1                                                                 #<rangesamp>4</rangesamp>
        self.timesamp    = self.MDV.DsRadarParams['samples_per_beam']                        #<timesamp>59</timesamp>
        self.timesamp    = 20
        self.antspeed    = 18                                                                #<antspeed>10</antspeed>
        
        if field=='DBZ_F': 
            self.rb5Datatype='dBZ'
            self.smin=self.dynz_min
            self.smax=self.dynz_max
        if field=='DBZ': 
            self.rb5Datatype='dBuZ'
            self.smin=self.dynz_min
            self.smax=self.dynz_max
        if field=='VEL_F': 
            self.rb5Datatype='V'
            self.smin=self.dynv_min
            self.smax=self.dynv_max
        if field=='WIDTH_F': 
            self.rb5Datatype='W'
            self.smin=self.dynw_min
            self.smax=self.dynw_max
        if field=='RHOHV': 
            self.rb5Datatype='RhoHV'
            self.smin=self.dynrhohv_min
            self.smax=self.dynrhohv_max
        if field=='PHIDP': 
            self.rb5Datatype='PhiDP'
            self.smin=self.dynphidp_min
            self.smax=self.dynphidp_max
        if field=='KDP': 
            self.rb5Datatype='KDP'
            self.smin=self.dynz_min
            self.smax=self.dynz_max
        if field=='SNR': 
            self.rb5Datatype='SNR'
            self.smin=self.dynz_min
            self.smax=self.dynz_max
        if field=='ZDR': 
            self.rb5Datatype='ZDR'
            self.smin=self.dynzdr_min
            self.smax=self.dynzdr_max
        
        self.slicetime = time.strftime("%H:%M:%S",self.File_Time)
        self.slicedate = time.strftime("%Y-%m-%d",self.File_Time)
        
        self.rays = 360
        self.bins = self.MDV.DsRadarParams['ngates'] 
           
    def write_RB5_slice(self,elev):

        self.rb5_File.write('      <slice refid="%d" >\n' %(elev,))
        self.rb5_File.write('         <dynz min="%s" max="%s" />\n'   %(self.dynz_min, self.dynz_max))
        self.rb5_File.write('         <dynv min="%s" max="%s" />\n'   %(self.dynv_min, self.dynv_max))
        self.rb5_File.write('         <dynw min="%s" max="%s" />\n'   %(self.dynw_min, self.dynw_max))
        self.rb5_File.write('         <dynzdr min="%s" max="%s" />\n' %(self.dynzdr_min, self.dynzdr_max))
        self.rb5_File.write('         <dynrhohv min="%s" max="%s" />\n' %(self.dynrhohv_min, self.dynrhohv_max))
        self.rb5_File.write('         <dynphidp min="%s" max="%s" />\n' %(self.dynphidp_min, self.dynphidp_max))
        self.rb5_File.write('         <posangle>%f</posangle>\n' %(self.posangle,))
        self.rb5_File.write('         <startangle>%f</startangle>\n' %(self.startangle))
        self.rb5_File.write('         <stopangle>%f</stopangle>\n' %(self.stopangle))
        self.rb5_File.write('         <stoprange>%f</stoprange>\n' %(self.stoprange))
        self.rb5_File.write('         <rangestep>%f</rangestep>\n' %(self.rangestep))
        self.rb5_File.write('         <stagger>%s</stagger>\n' %(self.stagger))
        self.rb5_File.write('         <highprf>%d</highprf>\n' %( self.highPrf))
        self.rb5_File.write('         <lowprf>%s</lowprf>\n'   %(self.lowPrf))
        self.rb5_File.write('         <filtermode>%s</filtermode>\n'  %(self.filtermode))
        self.rb5_File.write('         <filterdepth>%s</filterdepth>\n'  %(self.filterdepth))
        self.rb5_File.write('         <filterwidth>%d</filterwidth>\n'  %(self.filterwidth))
        self.rb5_File.write('         <rangesamp>%d</rangesamp>\n' %(self.rangesamp))
        self.rb5_File.write('         <timesamp>%d</timesamp>\n' %(self.timesamp))
        self.rb5_File.write('         <antspeed>%d</antspeed>\n' %(self.antspeed))
        self.rb5_File.write('         <slicedata time="%s" date="%s" >\n' %(self.slicetime, self.slicedate))
        self.rb5_File.write('            <rayinfo refid="startangle" blobid="%d" rays="%s" depth="16" />\n' % (2*elev,self.rays))
        self.rb5_File.write('            <rawdata blobid="%d" rays="%s" type="%s" bins="%s" min="%s" max="%s" depth="8" />\n' % (2*elev+1, self.rays, self.rb5Datatype, self.bins, self.smin, self.smax))
        self.rb5_File.write('         </slicedata>\n' )
        self.rb5_File.write('      </slice>\n' )

    def get_RB5_sensorinfo(self):
        
        self.radar_alt = round(self.MDV.DsRadarParams['altitude'] * 1000,1)
        self.radar_lon = round(self.MDV.DsRadarParams['longitude'],4)
        self.radar_lat = round(self.MDV.DsRadarParams['latitude'],4)
        self.radar_ID  = self.MDV.DsRadarParams['radar_name']
        self.radar_Name = self.MDV.DsRadarParams['radar_name']
        self.wavelength = round(self.MDV.DsRadarParams['wavelength'] / 100,4)
        self.beamwidth =  round(self.MDV.DsRadarParams['horiz_beam_width'],2)

           
    def write_RB5_sensorinfo(self):

        self.rb5_File.write( '   </scan>\n' )
        self.rb5_File.write( '   <radarinfo alt="%s" lon="%s" lat="%s" id="%s" >\n' %(self.radar_alt, self.radar_lon, self.radar_lat, self.radar_ID))
        self.rb5_File.write( '      <name>%s</name>\n' %(self.radar_Name))
        self.rb5_File.write( '      <wavelen>%f</wavelen>\n' %(self.wavelength))
        self.rb5_File.write( '      <beamwidth>%f</beamwidth>\n' %(self.beamwidth))
        self.rb5_File.write( '   </radarinfo>\n' )
        self.rb5_File.write( '</volume>\n' )
        self.rb5_File.write( '<!-- END XML -->\n' )

    def create_angle_blob(self):

        self.angleBlob = array.array("H")
        for w in range(360):
            self.angleBlob.append(2**16 * w // 360)
        if sys.byteorder != 'big':
            self.angleBlob.byteswap()        
        
        self.angleBlob = self.angleBlob.tostring()
        if self.cmprMethod == "qt":
            self.angleBlob = struct.pack("!I", len(self.angleBlob)) + zlib.compress(self.angleBlob)
            
    def create_data_blob(self,elev,field):
                
        start = float(self.smin)
        stop = float(self.smax)
        step = (stop-start)/254.
        
        add = abs(start)+step
        
        data = self.MDV.field_data[field][elev,:,:]  
        data = np.ravel(data,order='C')
        data[data < start] = start-step
    
        self.dataBlob = array.array("B")
        for value in data:
            self.dataBlob.append(int((value + add)/step))
        if sys.byteorder != 'big':
            self.dataBlob.byteswap() 
            
        self.dataBlob = self.dataBlob.tostring()    
        
        if self.cmprMethod == "qt":
            self.dataBlob = struct.pack("!I", len(self.dataBlob)) + zlib.compress(self.dataBlob)
        

    def write_blobs(self,field):
        
        self.create_angle_blob()        
                
        for elev in range(self.numEle):
            
            self.rb5_File = open(self.sRB5_file, "a" )
            self.rb5_File.write('<BLOB blobid="%d" size="%d" compression="%s">\n' % (2*elev, len(self.angleBlob), self.cmprMethod))
            self.rb5_File.close()
            
            self.rb5_File = open(self.sRB5_file, "ab")
            self.rb5_File.write(self.angleBlob)
            self.rb5_File.close()
            
            self.create_data_blob(elev,field)
            
            self.rb5_File = open(self.sRB5_file, "a" )            
            self.rb5_File.write('\n</BLOB>\n' )
            self.rb5_File.write('<BLOB blobid="%d" size="%d" compression="%s">\n' %(2*elev+1, len(self.dataBlob), self.cmprMethod))
            self.rb5_File.close()
            

            self.rb5_File = open(self.sRB5_file, "ab")
            self.rb5_File.write(self.dataBlob)
            self.rb5_File.close()
            
            self.rb5_File = open(self.sRB5_file, "a") 
            self.rb5_File.write('\n</BLOB>\n')
            self.rb5_File.close()
      
        
if __name__== "__main__":
    
    MDV_file = sys.argv[1]    
    output_dir = sys.argv[2]
    
#    MDV_file = "mdv/polar/20170213/175300.mdv"
#    output_dir = "rainbow"
#    MDV_file = "mdv_kdw/20170221/030353.mdv"
#    output_dir = "rainbow"


    print('Reading MDV:', MDV_file)    
    start_time = time.time()
    MDV = ReadMdv(MDV_file)
    print('TOTAL TIME = ', "%.6f" % (time.time() - start_time),"Seconds")
    
    print()
    
    print('Convert to RB5 in output directory:', output_dir)    
    start_time = time.time()
    MDV_to_RB5(MDV,output_dir)
    print('TOTAL TIME = ', "%.6f" % (time.time() - start_time),"Seconds")
    print()
