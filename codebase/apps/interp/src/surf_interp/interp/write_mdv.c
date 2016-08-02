

#define MAX_FIELDS 64

#include <stdio.h>

#include <mdv/mdv_grid.h>
#include <mdv/mdv_write.h>
#include <mdv/mdv_utils.h>
#include <mdv/mdv_read.h>
#include <mdv/mdv_handle.h>
#include <mdv/mdv_user.h>  
     
#include <mdv/mdv_file.h>
#include <mdv/mdv_macros.h>

#include <math.h> /* for pow function. */


#include <toolsa/umisc.h> /* For date_time_t  */


/* All these are for the call to mkdir. */
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
                       


/* Re-coded from Fortran, Niles Oien Jan 1999 
      subroutine write_mdv(ara2,nz,nx,ny,dxkm,dykm,dzkm,outarray,
     1           nxmm,nymm,nzmm,nfields,x0km,y0km,idtm,
     2           datatype,name_paramsv,unitsv,iyyd,
     3           immd,iddd,ihhd,imnd,issd,rlat,rlon,
     4           nfieldscount,bad,output_dir,debug)

     */



void write_mdv(float *ara2,
	       int nz,
	       int nx,
	       int ny,
	       float dxkm,
	       float dykm,
	       float dzkm,
	       unsigned char *outarray,
               int nxmm,
	       int nymm,
	       int nzmm,
	       int nfields,
	       float x0km,
	       float y0km,
	       int idtm,
               char *datatype,
	       char *name_paramsv,
	       char *unitsv,
	       int iyyd,
               int immd,
	       int iddd,
	       int ihhd,
	       int imnd,
	       int issd,
	       float rlat,
	       float rlon,
               int *nfieldscount, /* This seems to be passed back out - use pointers. */
	       float bad,
	       char *output_dir,
	       int debug)
{

  float scale_cidd[MAX_FIELDS];
  float offset_cidd[MAX_FIELDS];

  char var_unit[10]; 
  char var_name[10];
  float ara2max,ara2min;
  float offset,scale;
  MDV_master_header_t mheader;
  MDV_field_header_t fheader;

  int num_bytes=1;
  float scalebytes;
  int iyyy;
  date_time_t TheTime, Now;
  time_t itimes;

  char ifname2[MAX_PATH_LEN];
  char dirname[MAX_PATH_LEN];
    
  int i,j,k,nf;


  /* Put together the time. */
  TheTime.year=iyyd;
  TheTime.month=immd;
  TheTime.day=iddd;
  TheTime.hour=ihhd;
  TheTime.min=imnd;
  TheTime.sec=issd;

  itimes=uconvert_to_utime(&TheTime);

  ulocaltime(&Now);

  /* Set max and min to first non-bad value. */

  i=-1;
  do{
    i++;
    ara2max=ara2[i]; ara2min=ara2[i];
  } while((ara2max==bad) && (i<nymm*nxmm*nzmm));


  if (i==nymm*nxmm*nzmm) { /* No good data. */
    ara2max=bad; ara2min=bad;
  } else { /* find min and max. */

    for (j=i+1; j<nymm*nxmm*nzmm; j++){ /* do 10 j=1,nymm */
      
      if (ara2[j] != bad){
	if (ara2[j] < ara2min) ara2min=ara2[j];
	if (ara2[j] > ara2max) ara2max=ara2[j];
      } /* if */

    } /* for */
  } /* End of if statement. */
     

  offset = ara2min;
  scale  = ara2max - ara2min;

  scalebytes =(float)pow(2,num_bytes*8) - 1.0;

  if (scale < 1.0e-10) scale = 1.0; /* Avoid div by 0 */


  for (i=0;i<nxmm*nymm*nzmm;i++){ /* calculate byte values from data.. */

    if (ara2[i] == bad){
      outarray[i] = 0;
    }  else {
      outarray[i]=(unsigned char) (scalebytes*(ara2[i]-offset)/scale);
      if (outarray[i] ==0) outarray[i]=1;
    }

  }

  /* nfieldscount = nfieldscount + 1 */

  scale_cidd[*nfieldscount]  = scale/scalebytes;
  offset_cidd[*nfieldscount] = offset;
  sprintf(var_name,"%s",name_paramsv);
  sprintf(var_unit,"%s",unitsv);
  *nfieldscount++;

      /* I think this is just too weird ... Niles.
      if(nfieldscount .eq. 1)then

        ifname1 = 'temp'

        open(ifile1,file=ifname1,form='unformatted',status='unknown')

      endif

c..  now write data out.

      write(ifile1) (((outarray(i,j,k),i=1,nxmm),j=1,nymm),k=1,nzmm)

      if(nfieldscount .eq. nfields)then

      rewind(ifile1)

      
c.. write out master header information
c.. first define integer header variables
*/

      iyyy = iyyd-100*(iyyd/100);

      mheader.struct_id=MDV_MASTER_HEAD_MAGIC_COOKIE;
      mheader.revision_number=MDV_REVISION_NUMBER;

      mheader.time_gen = Now.unix_time;
      mheader.user_time = itimes;
      mheader.time_begin = itimes - 0.5*idtm;
      mheader.time_end = itimes + 0.5*idtm;
      mheader.time_centroid = itimes;
      mheader.time_expire = itimes + 3600;

      mheader.num_data_times = 1;                
      mheader.index_number = 1;
      mheader.data_dimension = 3;
      mheader.data_collection_type = MDV_DATA_MEASURED;
  
      mheader.user_data = MDV_PROJ_FLAT;
      mheader.vlevel_type = MDV_VERT_TYPE_SIGMA_Z;
      mheader.vlevel_included = MDV_VERT_TYPE_Z;
               
      mheader.vlevel_included = 0;             
      mheader.grid_order_direction=MDV_ORIENT_SN_WE;
      mheader.grid_order_indices = MDV_ORDER_XYZ;

      mheader.n_fields = nfields;
      mheader.max_nx = nxmm;
      mheader.max_ny = nymm;
      mheader.max_nz = nzmm;

      mheader.n_chunks = 0;
      mheader.field_hdr_offset = 1024;
      mheader.vlevel_hdr_offset = 0;
      mheader.chunk_hdr_offset = 0;
      mheader.field_grids_differ= 0;

      mheader.user_data_fl32[0] = dxkm;              /*! Grid spacing in X direction */
      mheader.user_data_fl32[1] = dykm;              /*! Grid spacing in Y direction */
      mheader.user_data_fl32[2] = dzkm;              /*! Grid spacing in Z direction */

      mheader.user_data_fl32[3] = x0km-0.5*(nx-2)*dxkm; /* ! Start point of data within projection (X) */
      mheader.user_data_fl32[4] = y0km-0.5*(ny-2)*dykm; /* ! Start point of data within projection (Y) */
      mheader.user_data_fl32[5] = 0.0;                  /* ! Start point of data within projection (Z) */


      mheader.sensor_lon = rlon; 
      mheader.sensor_lat = rlat;   
      mheader.sensor_alt = 1600.0; /* I don't like having this hard coded. */
      /*
c..  write out character information, make sure to null terminate strings.
*/
      sprintf(mheader.data_set_info,"%s","analysis field");
      sprintf(mheader.data_set_name,"%s","Radar volume file type 2");
      sprintf(mheader.data_set_source,"%s"," "); /* Should put didss URL here eventually. */


      sprintf(dirname,"%s%s%04d%02d%02d%s",output_dir,PATH_DELIM,
	      iyyd,immd,iddd,PATH_DELIM);

      i=mkdir(dirname,0777); /* 0777 is masked by umask. */
      if (i) {
	if (i==EEXIST){
	  fprintf(stderr,"Directory %s exists\n",dirname);
	} else {
	  fprintf(stderr,"Failed to create directory %s\n",dirname);
	  perror("mkdir failed"); exit(-1);
	}
      }


      sprintf(ifname2,"%s%02d%02d%02d.mdv",dirname,ihhd,imnd,issd);

      if (debug) fprintf(stdout," file name : %s\n",ifname2);

      /*
c
c      Open the output file
c

       call MF_wof_write_open(ifname2, return_status)
       
       if (return_status .ne. 0) then
         print *, "Error ", return_status, " opening output file ",
     &         ifname2
 
         stop
       end if

      

       call MF_wm_write_master_hdr(mheader_ints, mheader_reals,
     &                             mheader_info, mheader_chars(1),
     &                             mheader_chars(2), return_status)

       if (return_status .ne. 0) then

         print *, "Error ", return_status, " writing master header"
         stop

       end if

       */

      for (nf=0;nf<nfields;nf++){ /* do 2000 nf = 1,nfields */

	/*
c.. write out field header information
c.. integers first
*/

      fheader.struct_id = MDV_FIELD_HEAD_MAGIC_COOKIE;
      fheader.field_code = 0;

      fheader.user_time1 = Now.unix_time;
      fheader.forecast_delta = 0; /* This is not a forecast. */
      fheader.user_time2 = itimes - 0.5*idtm;
      fheader.user_time3 = itimes + 0.5*idtm;
      fheader.forecast_time = mheader.time_centroid; /* Not a forecast. */
      fheader.user_time4 = itimes + 3600; 
      fheader.nx = nxmm;
      fheader.ny = nymm;
      fheader.nz = nzmm;
      fheader.proj_type = MDV_PROJ_FLAT;
      fheader.encoding_type = MDV_INT8;
      fheader.data_element_nbytes = num_bytes; 
      fheader.field_data_offset = 1024+(416*nfields)+4+(nf-1)*(nxmm*nymm*nzmm*num_bytes+8);
      fheader.volume_size = nxmm*nymm*nzmm*num_bytes;

      /*
c.. then reals
*/
      fheader.proj_origin_lat = rlat;
      fheader.proj_origin_lon = rlon;
      fheader.proj_param[0] = 0.0;
      fheader.proj_param[1] = 0.0;
      fheader.proj_param[2] = 0.0;
      fheader.proj_param[3] = 0.0;
      fheader.proj_param[4] = 0.0;
      fheader.proj_param[5] = 0.0;
      fheader.proj_param[6] = 0.0;
      fheader.proj_param[7] = 0.0;
      fheader.vert_reference = 0.0;

      fheader.grid_dx = dxkm;
      fheader.grid_dy = dykm;
      fheader.grid_dz = dzkm;

      fheader.grid_minx = x0km-0.5*(nx-2)*dxkm;
      fheader.grid_miny = y0km-0.5*(ny-2)*dykm;
      fheader.grid_minz = 0.0;

      fheader.scale = scale_cidd[nf];
      fheader.bias = offset_cidd[nf];

      fheader.bad_data_value = 0.0;
      fheader.missing_data_value = 0.0;
      fheader.proj_rotation = 0.0;

      /*
c.. then characters
*/
      sprintf(fheader.field_name_long,"%s",var_name[nf]);
      sprintf(fheader.field_name,"%s"," ");
      sprintf(fheader.units,"%s",var_unit[nf]);
      sprintf(fheader.transform,"%s"," ");


      /*
      read(ifile1) (((outarray(i,j,k),i=1,nxmm),j=1,nymm),k=1,nzmm)

c
c        Write the field header and data to the output file
c
 
         field_num = nf - 1

         call MF_wf_write_field(field_num,
     &                          fheader_ints, fheader_reals,
     &                          long_field_name,field_chars(1),
     &                          field_chars(2),field_chars(3),
     &                          field_chars(4),
     &                          fheader_ints(16), outarray,
     &                          MDV_INT8, bytes_written,
     &                          return_status)

         if (return_status .ne. 0) then
           print *, "Error ", return_status, " writing field ",
     &           field_num, " to output file."
           stop
         end if
	 

         field_data_offset = field_data_offset + bytes_written + 8


       
      close(ifile1)


      call MF_wcf_write_close()

      *nfieldscount = 0

      endif
      */
      }

return;      

}


  
	   




