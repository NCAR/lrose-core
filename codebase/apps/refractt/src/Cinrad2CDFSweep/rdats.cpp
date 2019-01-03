#include "rdats.h"
#include<vector>
#include <stdio.h>
#include <string.h>
size_t getSweepLength(TSSweepHeader *swphdr)
{
	int chan=swphdr->chan;
	if(chan==0) chan=1;
	
	return sizeof(*swphdr)+chan*swphdr->binnum*2*sizeof(float);
}
TSSweepHeader *getNextSwpHeader(TSSweepHeader *swphdr)
{
	int swplen=getSweepLength(swphdr);
	return (TSSweepHeader*) ((char*)swphdr+swplen);
}
Iqcmpl *getIQData(TSSweepHeader *swphdr)
{
	return (Iqcmpl*)((char*)swphdr+sizeof(TSSweepHeader));
}
vector<char> fileCache;
int scanIQFile(const char*fname,TSHeader *tsh,SwpHdrList &swplist)
{
	FILE *fp=fopen(fname,"rb");
	if(fp==NULL)
		return 0;
	fseek(fp,0,SEEK_END);
    int fileLength=ftell(fp);
	fileCache.resize(fileLength);
	rewind(fp);
	fread(&fileCache[0],1,fileLength,fp);
/*
	// �����ļ��ں˶�������������hFile
	HANDLE hFile;
	int error;
	hFile = CreateFile((LPCSTR)fname,GENERIC_READ ,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		error=GetLastError();
	}
	 DWORD dwBytesInBlock,fileLength;
	 dwBytesInBlock= GetFileSize(hFile,&fileLength);  
	 if(INVALID_FILE_SIZE==dwBytesInBlock)
	 {
		error=GetLastError();
	 }
	//ӳ�����
	HANDLE hFileMapping = CreateFileMapping(hFile,NULL,PAGE_READONLY,0,dwBytesInBlock,NULL);
	
	if (hFileMapping==NULL)
	{
		error=GetLastError();
		return false;
	}
	//���︳ֵ��һ���ļ�ӳ���ָ�룬��ַ���ļ�ͷ��ʼ
	TCHAR *pbFile = (TCHAR *)MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
*/
	char *pbFile=&fileCache[0];
	TSHeader *IQFileHead=(TSHeader *)pbFile; 
	memcpy(tsh,IQFileHead,sizeof(*tsh));

	TSSweepHeader *SWFileHead =(TSSweepHeader*)(pbFile+sizeof(*IQFileHead)+256);
	TSSweepHeader *nexSWFileHead;
	nexSWFileHead=SWFileHead;

	size_t totalen=fileLength;
	 
	size_t datalen;
	datalen=sizeof(IQFileHead)+256;

	while(datalen<totalen)
	{	
		swplist.push_back(nexSWFileHead);
		size_t swpLEN=getSweepLength(nexSWFileHead);	 
		nexSWFileHead=getNextSwpHeader(nexSWFileHead);
		datalen=datalen+swpLEN;	
	}
	return 1;
}
size_t depackSweeps(TSSweepHeader *swpin,size_t len,TSSweepHeader *swpout)
{
	size_t curlen=0;	
	size_t out_swps_len=0;
	TSSweepHeader *packswp=swpin,*unpackswp=swpout;
	while(curlen<len)
	{
		size_t packiq_count=(packswp->binnum*packswp->chan+packswp->burstbinnum)*2;
		int packswplen=sizeof(TSSweepHeader)+packiq_count*sizeof(short);
		if(packswplen+curlen>len||packswplen<0)
		{
			//fprintf("unexepcted swp seq num %d\n",packswp->seqnum);
			return out_swps_len;
		}
		memcpy(unpackswp,packswp,sizeof(TSSweepHeader));	
		//patch to set nextswp,
		unsigned char *ptmp=(unsigned char*)unpackswp;
		//set nextswp null,offset of nextswp is 70;
		memset(&ptmp[70],0,sizeof(int));

		depackIQ((const unsigned short*)packswp->iq[0],unpackswp->iq[0],packiq_count);		
		packswp=(TSSweepHeader*)((char*)packswp+packswplen);
		out_swps_len+=getSweepLength(unpackswp);
		unpackswp=getNextSwpHeader(unpackswp);
		curlen+=packswplen;
	}
	return out_swps_len;
}
