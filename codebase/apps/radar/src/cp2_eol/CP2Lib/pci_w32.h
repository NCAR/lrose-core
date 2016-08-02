#pragma once

#include <windows.h>
#include "tvichw32.h"

#define DATUM_VENDORID 0x12e2
#define DATUM_DEVICEID 0x4013

#define DIO_VENDORID 0x1307
#define DIO_DEVICEID 0xB

#define TIMER_VENDORID 0x10E8
#define TIMER_DEVICEID 0x8504

typedef struct PCI_CARD_TYP{
	BYTE *base;
	unsigned short bus;
	unsigned short fnum;
	unsigned short card;
	unsigned short deviceid;
	unsigned short vendorid;
	unsigned char ioregbase;
	unsigned long phys,phys2;

	int mapped_length;
	PCI_COMMON_CONFIG config;
}PCI_CARD;


// External variables
extern HANDLE PCI_HANDLE;

// Function prototypes
int init_pci(void);
PCI_CARD *find_pci_card(int vendorid,int deviceid,int n);
void pci_card_info(PCI_CARD *card);
void close_pci(void);
void out8(unsigned int port,unsigned char byte);
void out32(unsigned int port,unsigned long dword);
unsigned char in8(unsigned int port);
unsigned long in32(unsigned int port);
unsigned char pci_card_ioregbase(PCI_CARD *card);
BYTE *pci_card_membase(PCI_CARD *card,int length);
void close_pci_card(PCI_CARD *card);
void delete_pci_card(PCI_CARD *card);
unsigned long pci_read_config32(PCI_CARD *card,UINT offset);