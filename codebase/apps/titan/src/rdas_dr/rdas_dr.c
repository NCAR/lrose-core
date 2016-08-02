/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/malloc.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/tqueue.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include "rdas.h"

static int MajorDev = 0; /* dyna,ic allocation of major no */
static struct file_operations Fops;
static unsigned int BaseAddr = _RDAS_DEFAULT_BASE_ADDR;
static unsigned int ProgramSize = 0;
static int ProgramLoaded = 0;
static int DspReset = 0;
static int DataSpaceAddressing = 1;
static int IrqNum = -1;

static int rdas_open(struct inode *inode, struct file *filp)
{
  MOD_INC_USE_COUNT;
  return (0);
}

static int rdas_release (struct inode *inode, struct file *filp)
{
  MOD_DEC_USE_COUNT;
  return (0);
}

static void rdas_reset_dsp()
{
  inb(BaseAddr + 2);
  outb(0xfb, BaseAddr + 1);
  mdelay(100);
  outb(0xfe, BaseAddr + 1); /* interrupts disabled */
  DspReset = 1;
  ProgramLoaded = 0;
}

void rdas_interrupt(int irq, void *dev_id, struct pt_regs *regs)

{

  static int count = 0;

  count++;
  if (count % 30 == 0) {
    printk("<0>Received %d interrupts so far\n", count);
  }

}


static int rdas_load_program(ui32 *progCode)

{

  int i;
  int nleft;
  int iret;
  ui16 msw, lsw;
  ui32 code_word;
  ui32 *progCopy;

  /*
   * copy the code from user space
   */

  progCopy = kmalloc(ProgramSize * sizeof(ui16), GFP_KERNEL);
  nleft = copy_from_user(progCopy, progCode, ProgramSize * sizeof(ui32));
  if (nleft > 0) {
    printk("<0>ERROR in copy_from_user, %d bytes left\n", nleft);
    kfree(progCopy);
    return (-1);
  }

  /* latch starting address */
  
  outb(0xff, BaseAddr + 1); /* set IALC high */
  outw(0x0001, BaseAddr + 2); /* start at address 0001 */
  outb(0xf7, BaseAddr + 1); /* set IALC low */
  
  /*
   * load all code except the first command
   */

  for (i = 1; i < ProgramSize; i++) {
    msw = (ui16) (progCopy[i] >> 8);           /* msw of 24 bit value */
    lsw = (ui16) ((progCopy[i] << 16) >> 16);  /* lsw of 24 bit value */
    outw(msw, BaseAddr);
    outw(lsw, BaseAddr);
  }
      
  /*
   * load first command
   */
  
  outb(0xff, BaseAddr + 1); /* set IALC high */
  outw(0x0000, BaseAddr + 2); /* start at address 0001 */
  outb(0xf7, BaseAddr + 1); /* set IALC low */
  
  msw = (ui16) (progCopy[0] >> 8);           /* msw of 24 bit value */
  lsw = (ui16) ((progCopy[0] << 16) >> 16);  /* lsw of 24 bit value */
  outw(msw, BaseAddr);
  outw(lsw, BaseAddr);
  
  /*
   * Go
   */
  
  outb(0xff, BaseAddr + 1); /* set IALC high */
  
  /*
   * verify that it worked
   */
  
  outb(0xff, BaseAddr + 1); /* set IALC high */
  outw(0x0000, BaseAddr + 2); /* start at address 0000 */
  outb(0xf7, BaseAddr + 1); /* set IALC low */
  
  for (i = 0; i < ProgramSize; i++) {
    msw = inw(BaseAddr);
    lsw = inw(BaseAddr);
    code_word = msw << 8 | lsw;
    if (code_word != progCopy[i]) {
      printk("<0>ERROR for code word %d, wrote %x, read %x \n", i,
	     progCopy[i], code_word);
      kfree(progCopy);
      return (-1);
    }
  }

  /*
   * enable interrupts
   */
  
  if (IrqNum >= 0) {
    iret = request_irq(IrqNum, rdas_interrupt, SA_INTERRUPT,
		       "rdas", NULL);
    if (iret) {
      printk("<0>ERROR enabling interrupts for irq %d, iret = %d\n",
	     IrqNum, iret);
      /* kfree(progCopy); */
      /* return (-1); */
    }
    /* do it */
    outb(0xf6, BaseAddr + 1); /* set ALC low, interrupts enabled */ 
  }

  ProgramLoaded = 1;

  /*
   * clean up
   */
  
  kfree(progCopy);

  return (0);
    
}

static void rdas_clear_dav()

{
  inb(BaseAddr + 2);
}

static int rdas_check_dav()

{

  unsigned char status;

  status = inb(BaseAddr + 1);
  if (status & 0x01) {
    /* clear before returning */
    rdas_clear_dav();
    return (0);
  } else {
    return (-1);
  }

}

static loff_t rdas_seek (struct file *filp, loff_t offset, int whence)
{

  long newpos;

  /* printk("<0>Seeking to offset %d, whence %d\n", (int) offset, whence); */

  switch (whence) {

  case 0: /* SEEK_SET */
    newpos = offset;
    break;
  
  case 1: /* SEEK_CUR */
    newpos = filp->f_pos + offset;
    break;
  
  default:
    newpos = -1;
    
  } /* switch */
  
  if (newpos >= 0) {
    filp->f_pos = newpos;
    return (newpos);
  } else {
    return (-EINVAL);
  }

}

static int rdas_ioctl (struct inode *inode, struct file *filp,
		       unsigned int command, unsigned long arg)
     
{

  int iret = 0;

  switch (command) {

  case _RDAS_SET_BASE_ADDR:

    release_region(BaseAddr, 4);
    /* printk("<0>Ioctl, previous base addr is %x\n", BaseAddr); */

    BaseAddr = (int) arg;
    /* printk("<0>Ioctl, setting base addr to %x\n", BaseAddr); */

    iret = check_region(BaseAddr, 4);
    if (iret < 0) {
      printk("<0>Ioctl, ports %x to %x busy\n", BaseAddr, 4);
    }
    request_region(BaseAddr, 4, "rdas");

    break;
    
  case _RDAS_SET_PROGRAM_SIZE:
    ProgramSize = (int) arg;
    /* printk("<0>Ioctl, setting program size to %d\n", ProgramSize); */
    break;
    
  case _RDAS_SET_IRQ_NUM:
    IrqNum = (int) arg;
    printk("<0>Ioctl, setting irq number to %d\n", IrqNum);
    break;
    
  case _RDAS_RESET_DSP:
    rdas_reset_dsp();
    break;
    
  case _RDAS_LOAD_PROGRAM:

    if (ProgramSize == 0) {
      
      printk("<0>Ioctl, ERROR - no program size specified\n");
      iret = -1;

    } else {

      ui32 *progCode = (ui32 *) arg;

      /* printk("<0>Ioctl, loading program from buf %p, size %d\n", progCode, ProgramSize); */

      if (!DspReset) {
	rdas_reset_dsp();
      }
      if (rdas_load_program(progCode)) {
	iret = -1;
      }
      DspReset = 0;
      
    }
    break;

  case _RDAS_SET_ADDR_PROG:
    DataSpaceAddressing = 0;
    break;

  case _RDAS_SET_ADDR_DATA:
    DataSpaceAddressing = 1;
    break;

  case _RDAS_CHECK_DAV:
    if (rdas_check_dav()) {
      return (-1);
    }
    break;

  case _RDAS_CLEAR_DAV:
    rdas_clear_dav();
    break;

  default:
    iret = -1;

  } /* switch */

  if (iret) {
    return (-EINVAL);
  } else {
    return (0);
  }

}

static ssize_t rdas_read(struct file *filp, char *buf,
			 size_t nbytes, loff_t *offset)
{

  int i;
  int nshorts = nbytes / 2;
  ui16 start_address;
  ui16 data_flag = 0x4000;
  ui16 *target = (ui16 *) buf; 

  if (!ProgramLoaded) {
    return (-1);
  }
  
  /* 
   * latch starting address
   */

  outb(0xfe, BaseAddr + 1); /* interrupts disabled */
  start_address = filp->f_pos;
  if (DataSpaceAddressing) {
    start_address |= data_flag;
  }
  outw(start_address, BaseAddr + 2);
  outb(0xf6, BaseAddr + 1); /* set ALC low, interrupts enabled */
  
  /*
   * read 2-byte words in loop
   */
  
  for (i = 0; i < nshorts; i++) {
    /* target[i] = inw(BaseAddr); */
    put_user(inw(BaseAddr), target + i);
    filp->f_pos++;
  }

  return (nshorts * 2);

}


static ssize_t rdas_write(struct file *filp, const char *buf,
			  size_t nbytes, loff_t *offset)
{

  int i;
  int nshorts = nbytes / 2;
  ui16 start_address;
  ui16 data_flag = 0x4000;
  ui16 *source = (ui16 *) buf; 
  ui16 in_data;
  
  if (!ProgramLoaded) {
    return (-1);
  }
  
  /* 
   * latch starting address
   */

  outb(0xfe, BaseAddr + 1); /* set ALC high, interrupts disabled */
  start_address = filp->f_pos;
  if (DataSpaceAddressing) {
    start_address |= data_flag;
  }
  outw(start_address, BaseAddr + 2);
  outb(0xf6, BaseAddr + 1); /* set ALC low, interrupts enabled */

  /*
   * read 2-byte words in loop
   */

  for (i = 0; i < nshorts; i++) {
    get_user(in_data, source + i);
    outw(in_data, BaseAddr);
    filp->f_pos++;
  }
  
  return (nshorts * 2);

}


int init_module(void)
{

  int result;
  int iret;

  printk("<0> ... rdas driver loaded ...\n");

  /*
   * set up file operations struct
   */

  memset(&Fops, 0, sizeof(Fops));

  Fops.open = rdas_open;
  Fops.release = rdas_release;
  Fops.read = rdas_read;
  Fops.write = rdas_write;
  Fops.llseek = rdas_seek;
  Fops.ioctl = rdas_ioctl;

  /*
   * register device
   */

  result = register_chrdev(MajorDev, "rdas_dr", &Fops);
  if (result < 0) {
    printk("<0> - rdas_dr - can't get major %d\n", MajorDev);
    return (result);
  }
  if (MajorDev == 0) {
    /* printk("<0> - rdas_dr - got major number %d\n", result); */
    MajorDev = result;
  }

  /*
   * reserve ports
   */

  iret = check_region(BaseAddr, 4);
  if (iret < 0) {
    printk("<0>Ioctl, ports %x to %x busy\n", BaseAddr, 4);
  }
  request_region(BaseAddr, 4, "rdas");

  return (0);

}

void cleanup_module(void)
{

  /*
   * release ports
   */

  release_region(BaseAddr, 4);

  if (unregister_chrdev(MajorDev, "rdas_dr")) {
    printk("<0>Cannot unregister MajorDev %d, name %s\n",
	   MajorDev, "rdas_dr");
  }

  printk("<0> ... rdas driver removed ...\n");

}


