

#include <netcdf.h>
#include <stdio.h>

void print_type(FILE *out, nc_type Type);
void check_status(int status, char *FailMessage);

int main(int argc, char *argv[])
{


 char FileName[1024];
 char DimName[NC_MAX_NAME];
 char VarName[NC_MAX_NAME];
 char LAttName[NC_MAX_NAME];
 size_t Len;
 int status, ncID;
 int NDims, NVars, NAtts, Unlimited;
 int i,j;


 int NVDims, NLAtts;
 int VdimID[NC_MAX_VAR_DIMS];

 nc_type Type;
 
 fprintf(stdout,"netCDF version : %s\n", nc_inq_libvers() );

 if (argc > 1){
   /* The argument count is more than one, take the second
      argument as the file name. */
   sprintf(FileName,"%s", argv[1]);
 } else {
   /* No filename specified, ask for one. */
   fprintf(stdout,"Input file name --->");
   fscanf(stdin,"%s",FileName);
 }

 status = nc_open(FileName, NC_NOWRITE, &ncID);
 check_status(status, "Failed to open dataset.\n");
 
 status = nc_inq(ncID, &NDims, &NVars, &NAtts, &Unlimited);
 check_status(status,"nc_inq failed.\n");

 if (Unlimited == -1){
   fprintf(stdout,"No unlimited dimensions defined\n");
 } else {
   fprintf(stdout,"Unlimited dimensions : %d\n",Unlimited);
 }

 fprintf(stdout,"%d dimension(s).\n",NDims);
 fprintf(stdout,"%d Variable(s).\n",NVars);
 fprintf(stdout,"%d Global attribute(s).\n",NAtts);

 /*
  * Print about global attributes, if any.
  */

 if (NAtts > 0) fprintf(stdout,"\nGlobal Attributes :\n");

 for (j=0; j<NAtts; j++){

   nc_inq_attname(ncID,NC_GLOBAL,j,LAttName);
   nc_inq_att(ncID,NC_GLOBAL,LAttName,&Type,&Len);
   fprintf(stdout," %d : %s, Length %d",
	   j+1,LAttName,Len);
   fprintf(stdout," (Type ");
   print_type(stdout,Type);
   fprintf(stdout,")\n");
 }

 /*
  * Then the dimensions.
  */


 if (NDims > 0) fprintf(stdout,"\nGlobal Dimensions :\n");

 for (i=0; i< NDims; i++){
   nc_inq_dim(ncID,i,DimName,&Len);
   fprintf(stdout,"\t%d : %s, Length %d\n",i+1,DimName, Len);
 }

 /* 
  * And finally, the variables, with their local
  * dimensions and attributes.
  */

 if (NVars > 0) fprintf(stdout,"\nGlobal Variables :\n");

 for (i=0; i< NVars; i++){
   nc_inq_var(ncID,i,VarName,&Type, &NVDims, VdimID,
	      &NLAtts);

   fprintf(stdout,"\t%d : %s (Type ",i+1,VarName);
   print_type(stdout,Type);
   fprintf(stdout,")\n");

   fprintf(stdout,"\t\t%d dimension(s)\n",NVDims);

   for (j=0; j<NVDims; j++){

     nc_inq_dim(ncID,VdimID[j],DimName,&Len);
     fprintf(stdout,"\t\t %d : %s, Length %d\n",
	     j+1,DimName,Len);
   }


   /*
    * Now local attributes.
    */
   fprintf(stdout,"\t\t%d local attributes\n",NLAtts);

   for (j=0; j<NLAtts; j++){

     nc_inq_attname(ncID,i,j,LAttName);
     nc_inq_att(ncID,i,LAttName,&Type,&Len);
     fprintf(stdout,"\t\t %d : %s, Length %d",
	     j+1,LAttName,Len);
     fprintf(stdout," (Type ");
     print_type(stdout,Type);
     fprintf(stdout,")\n");
   }


 }



 nc_close(ncID);

 return 0;

}

void check_status(int status, char *s)
{

  if (status != NC_NOERR){
    fprintf(stderr,"%s\n",s);
    exit(-1);
  }

}



void print_type(FILE *out, nc_type Type)
{

  switch (Type){

  case NC_BYTE :
    fprintf(out,"Byte");
    break;

  case NC_CHAR :
    fprintf(out,"Char");
    break;

  case NC_SHORT :
    fprintf(out,"Short");
    break;

  case NC_INT :
    fprintf(out,"Int");
    break;

  case NC_FLOAT :
    fprintf(out,"Float");
    break;

  case NC_DOUBLE :
    fprintf(out,"Double");
    break;

  default :
    fprintf(out,"Unknown");
    break;

  }


}
