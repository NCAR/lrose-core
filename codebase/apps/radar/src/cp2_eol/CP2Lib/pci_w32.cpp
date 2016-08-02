#include <windows.h>
#include <stdio.h>
#include "pci_w32.h"

HANDLE PCI_HANDLE;

/// Initializes the PCI bus
int init_pci(void)
{

	// Set PCI handle to NULL to begin with
	PCI_HANDLE = NULL;

	// initialize (open) the driver 
	PCI_HANDLE = OpenTVicHW32( PCI_HANDLE, "TVICHW32","TVicDevice0");   

	// Make sure there was no failure. 
	if(!GetActiveHW(PCI_HANDLE))
	{
		printf("Couldn't open PCI driver!\n");
		return(-1);
	}
	return(0);
}

/// Finds a PCI card and returns it's information in a PCI_CARD struct
PCI_CARD *find_pci_card(int vendorid,int deviceid,int n)
{
	PCI_COMMON_CONFIG Info;
	PCI_CARD *card[4];
	int buses,bus,dev,func,i,j;

	i=0;

	// Assume six PCI busses
	buses = 5;

	for (bus = 0; bus<=buses; bus++) { // sort out all the buses    
		for (dev = 0; dev<32; dev++)   { // sort out all the devices  
			for (func= 0; func<8; func++) { // sort out all the functions  

				// Get the device at this spot
				if ( GetPciDeviceInfo(PCI_HANDLE,bus,dev,func,&Info) &&             
					(Info.VendorID == vendorid) && (Info.DeviceID == deviceid) ) {   

						card[i] = new PCI_CARD;
						card[i]->deviceid = deviceid;
						card[i]->vendorid = vendorid;
						card[i]->bus = bus;
						card[i]->fnum = func;

						if(Info.u.type1.BaseAddresses[1] & 1)
							card[i]->phys = Info.u.type1.BaseAddresses[1]-1;
						else
							card[i]->phys = Info.u.type1.BaseAddresses[1];

						if(Info.u.type1.BaseAddresses[0] & 1)
							card[i]->phys2 = Info.u.type1.BaseAddresses[0]-1;
						else
							card[i]->phys2 = Info.u.type1.BaseAddresses[0];


						card[i]->base = NULL;
						card[i]->mapped_length = 0;
						memcpy(&card[i]->config,&Info,sizeof(PCI_COMMON_CONFIG));
						i++;
				}  
			}
		}
	}

	// We did not find as many cards as we were looking for, throw an error
	if(i<=n) 
	{
		printf("Only found %d cards, and you wanted card %d\n",i,n);

		// Free the cards, return NULL
		for(n=0;n<i;n++)
		{
			delete(card[n]);
		}
		return(NULL);
	}

	// We found the card we wanted, free the others and return it
	for(j=0;j<i;j++)
	{
		if(card[j] != card[n])
			free(card[j]);
	}

	return(card[n]);
}

void pci_card_info(PCI_CARD *card)
{
	printf("VendorID: %4X, DeviceID: %4X, Physical Bases: %8X,%8X\n",card->vendorid,card->deviceid,card->phys,card->phys2);
}

/// Releases the PCI bus
void close_pci(void)
{
	PCI_HANDLE = CloseTVicHW32(PCI_HANDLE);
}

/// Outputs a byte to the selected port
void out8(unsigned int port,unsigned char byte)
{
	SetPortByte(PCI_HANDLE,port,byte);
}

/// Outputs a 32 bit value to the selected port
void out32(unsigned int port,unsigned long dword)
{
	SetPortLong(PCI_HANDLE,port,dword);
}

/// Inputs a byte from the given port
unsigned char in8(unsigned int port)
{
	return(GetPortByte(PCI_HANDLE,port));
}

unsigned long in32(unsigned int port)
{
	return(GetPortLong(PCI_HANDLE,port));
}

/// Gets the PCI cards io register base address
unsigned char pci_card_ioregbase(PCI_CARD *card)
{
	return(card->ioregbase);
}

/// Maps the memory for a pci card, from the base address up to "length" in size
BYTE *pci_card_membase(PCI_CARD *card,int length)
{
	card->base = (BYTE *)MapPhysToLinear(PCI_HANDLE,card->phys,length);
	card->mapped_length = length;
	return(card->base);
}


/// frees a pci card's resources
void delete_pci_card(PCI_CARD *card)
{
	if(card == NULL)
		return;

	if(card->base)
		UnmapMemory(PCI_HANDLE,card->phys,card->mapped_length);

	delete(card);
	card = NULL;
}

/// Reads a 32 bit value at the specified offset from the devices hardware info
unsigned long pci_read_config32(PCI_CARD *card,UINT offset)
{
	ULONG *ptr = (ULONG *)&card->config + offset/4;
	return(*ptr);
}
