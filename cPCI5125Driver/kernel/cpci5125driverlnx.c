/*
* driver.c -- the pci device driver module
*
* Author:
*
* zhaoweihua
*
* Abstract:
*
* Contains routines to initiate and cleanup pci driver module.
*
* Environment:
*
* Kernel mode
*
*/

#include <linux/sched.h>

#include <linux/pci.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/uaccess.h>
#include <linux/device.h>

#include "cpci5125driverlnx.h"

static struct class *cPci5125Class;
static struct device *cPci5125Dev;
#define MINORCOUNT 20
static int pcidev_major   = 0;
static int pcidev_minor   = 0;
static int pcidev_nr_devs = 0;	/* number of plx9056 devices */

static struct pci_device* pci_device_list = NULL;

MODULE_AUTHOR("jinfeicui");
MODULE_LICENSE("Dual BSD/GPL");

/***********************************
*Function name : pcidev_open
*Description : open file
*
*input :
*           inode           node contain pci device descriptor
*           filp            file handle
*
*output :
*return :
*           0              Function successful
*           -EIO           Function failed
*
***********************************/
int intFlag;
int intcount;
//unsigned int pk_cnt_now[4] = { 0 };
//unsigned int pk_cnt_last[4] = { 0 };
//unsigned int indexDiff = 0;
struct pci_device* pHandle = NULL;

int pciDevOpen(struct inode *inode, struct file *filp)
{
    struct pci_device *dev; /* device information */
    LOG("pcidev_open----------IN\n"); //test

    intcount = 0;
    dev = container_of(inode->i_cdev, struct pci_device, cdev);
    filp->private_data = dev; /* for other methods */
    dev->write_offset = 0;
    pHandle = dev;
    dev->intcount = 0;

    try_module_get(THIS_MODULE);

    LOG("pcidev_open----------OUT\n"); //test

    return 0;
}

long pciDevIoctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	static char *me = "cpci429_ioctl()";
	unsigned char unit;
	struct pci_device *dev = filp->private_data;

	switch (cmd)
	{
	case ReadBAR2: //arg: channel int
	{
		void *pDenAddr;
		TY_IOCTL_DATA val;
		if (copy_from_user(&val, (struct ioctl_data *) arg,
				sizeof(struct ioctl_data)))
		{
			LOG("Error:IN:cpci429_ioctl");
			return -1;
		}
		pDenAddr = (void*) (dev->memory.bar[2].vir_address + val.offset);

		val.data = ioread32(pDenAddr);
		if (copy_to_user((void*) arg, &val, 4))
		{
			printk("Error:cpci429_ioctl");
			return -1;
		}
		LOG("IN: val.data = 0x%x val.offset = 0x%x\n", val.data, val.offset);
	}
	break;

	case WRITEBAR2:
	{
		void *pDenAddr;
		TY_IOCTL_DATA val;
		if (copy_from_user(&val, (struct ioctl_data *) arg,
				sizeof(struct ioctl_data)))
		{
			printk("Error:IN:cpci429_ioctl");
			return -1;
		}
		pDenAddr = (void*) (dev->memory.bar[2].vir_address + val.offset);

		printk("OUT: val.data = 0x%x val.offset = 0x%x\n", val.data, val.offset);
		iowrite32(val.data, pDenAddr);
	}
	break;

	default:
	break;
	}
	return 0;
}

int pciDevRelease(struct inode *inode, struct file *filp)
{
    struct pci_device* dev = filp->private_data;

    intFlag = 1;
    free_irq(dev->irq, (void*) dev);

    module_put(THIS_MODULE);

    return 0;
}

struct file_operations pcidev_fops = {
	.owner   =   THIS_MODULE,
	.open = pciDevOpen,
	.unlocked_ioctl = pciDevIoctl,
	.release =   pciDevRelease,
};


int32 pcidev_setup_cdev(struct pci_device* dev, dev_t devno)
{
	cdev_init(&dev->cdev, &pcidev_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops   = &pcidev_fops;

	if (cdev_add (&dev->cdev, devno, 1))
	{
		LOG("failed to add device\n");
		return -1;
	}

    //cPci5125Class = class_create(THIS_MODULE, DRIVER_NAME);
    //cPci5125Dev = device_create(cPci5125Class, NULL, MKDEV(pcidev_major, pcidev_minor), NULL, DRIVER_NAME);
	return 0;
}
/***********************************
*Function name : pci_map
*Description : get resources of the device and map
*
*input :
*           dev           device handle
*
*output :
*
*return :
*
***********************************/
void pci_map(struct pci_device* dev)
{
	u_int32 i;

	for (i = 0; i < _countof(dev->memory.bar); i++)
	{
		unsigned long flags;

		dev->memory.bar[i].phy_address = pci_resource_start(dev->pci_dev, i);
		dev->memory.bar[i].size = pci_resource_len(dev->pci_dev, i);

		if (0 == dev->memory.bar[i].phy_address || 0 == dev->memory.bar[i].size)
			continue;

		flags = pci_resource_flags(dev->pci_dev, i);
		if (flags & IORESOURCE_MEM)
		{
			dev->memory.bar[i].vir_address = ioremap_nocache(dev->memory.bar[i].phy_address, dev->memory.bar[i].size);
			dev->memory.bar[i].flag = IO_TYPE_MEMORY;
		}
		else if (flags & IORESOURCE_IO)
		{
			dev->memory.bar[i].vir_address = ioport_map(dev->memory.bar[i].phy_address, dev->memory.bar[i].size);
			dev->memory.bar[i].flag = IO_TYPE_PORT;
		}
		LOG("pci_map:i = %d,phy_address = 0x%x,vir_address = 0x%x,size = 0x%x\n",i,
			dev->memory.bar[i].phy_address,
			dev->memory.bar[i].vir_address,
			dev->memory.bar[i].size);
	}
}

/***********************************
*Function name : pci_unmap
*Description : unmap the device resources
*
*input :
*           dev           device handle
*
*output :
*
*return :
*
***********************************/
void pci_unmap(struct pci_device* dev)
{
	u_int32 i;

	for (i = 0; i < _countof(dev->memory.bar); i++)
	{
		if (NULL == dev->memory.bar[i].vir_address)
			continue;

		if (IO_TYPE_MEMORY == dev->memory.bar[i].flag)
			iounmap(dev->memory.bar[i].vir_address);
		else if (IO_TYPE_PORT == dev->memory.bar[i].flag)
			ioport_unmap(dev->memory.bar[i].vir_address);
	}
}

/***********************************
*Function name : pcidev_init
*Description : initialize device,call pci_map and alloc_dma
*
*input :
*           dev           device handle
*
*output :
*return :
*           0              Function successful
*           -1             Function not successful
*
***********************************/
int32 pcidev_init(struct pci_device* dev)
{
	int32 result;
	pci_map(dev);

	dev->read_offset = 0;
	dev->write_offset = 0;

	//hw_init(dev);
	return 0;
}

/***********************************
*Function name : pcidev_deinit
*Description : free device's resources
*
*input :
*           dev           device handle
*
*output :
*return :
*
***********************************/
void pcidev_deinit(struct pci_device* dev)
{
	pci_unmap(dev);
	pci_disable_device(dev->pci_dev);
}

static int __devinit pcidev_probe(struct pci_dev *dev, const struct pci_device_id *ent)
{

    LOG("vendor 0x%x device id 0x%x\n", dev->vendor, dev->device);
    static char *me = "pcidev_probe()";
    struct pci_device* pcidev_node = NULL;
    dev_t devno;
    int retval;

    if (pci_enable_device(dev))
	{
		LOG("failed to enable device: %p\n", dev);
		return -1;
	}

	if (NULL == (pcidev_node = kmalloc(sizeof(struct pci_device), GFP_KERNEL)))
	{
		LOG("failed to alloc memory!\n");
		pci_disable_device(dev);
		return -2;
	}

	memset(pcidev_node, 0, sizeof(struct pci_device));
	pcidev_node->pci_dev = dev;
	pcidev_node->irq = dev->irq;
	pcidev_node->location.bus = dev->bus->number;
	pcidev_node->location.function = dev->devfn;
	pcidev_node->location.device = dev->device;
	LOG("pcidev_probe irq=%d \n", pcidev_node->irq);

	if (pcidev_init(pcidev_node) < 0)
	{
		pcidev_deinit(pcidev_node);
		kfree(pcidev_node);
		return -3;
	}
	/*LOG("iowrite32 0x2000\n");
	iowrite32(0x2000,pcidev_node->memory.bar[2].vir_address + 0x1000);
	LOG("ioread32bar2 %x",ioread32(pcidev_node->memory.bar[2].vir_address + 0x1000));*/
	if(pcidev_major)
	{
		devno = MKDEV(pcidev_major, 0);
		retval = register_chrdev_region(devno, MINORCOUNT, DRIVER_NAME);
		if(retval < 0)
		{

			pcidev_deinit(pcidev_node);
			kfree(pcidev_node);
			LOG("register_chrdev failed.\n");
			return retval;
		}
	}
    else
    {
        retval = alloc_chrdev_region(&devno, 0, 1, DRIVER_NAME);
        if(retval < 0)
        {
			pcidev_deinit(pcidev_node);
			kfree(pcidev_node);
            LOG("register_chrdev failed.\n");
            return retval;
        }
        else
        {
            pcidev_major = MAJOR(devno);
			pcidev_minor = MINOR(devno);
            LOG("Device register success : %d(devno %d)\n", pcidev_major, devno);
        }
    }

	LOG("pcidev_init_module:major = %d,minor = %d!\n",pcidev_major,pcidev_minor);
	devno = MKDEV(pcidev_major, pcidev_nr_devs);
	pcidev_node->minor = pcidev_nr_devs;
	if (pcidev_setup_cdev(pcidev_node, devno) < 0)
	{
		unregister_chrdev_region(devno, 1);
		pcidev_deinit(pcidev_node);
		kfree(pcidev_node);
		LOG("pcidev_setup_cdev failed\n");
	}

	pcidev_nr_devs++;
	if (NULL == pci_device_list)
	{
		pci_device_list = pcidev_node;
	}
	else
	{
		struct pci_device* rear = pci_device_list;

		while (rear->next)
		{
			rear = rear->next;
		}
		rear->next = pcidev_node;
	}

	if (0 == pcidev_nr_devs || NULL == pci_device_list)
	{
		LOG("pci device not found!\n");
		return -4;
	}

	LOG("step out of probe\n");
    return 0;
}


static void __devexit pcidev_remove(struct pci_dev *dev)
{
    dev_t devno = 0;
    struct pci_device* pcidev_node = NULL;

    pci_disable_device(dev);
    while (pci_device_list)
        {
            pcidev_node = pci_device_list;
            pci_device_list = pci_device_list->next;
            if(pcidev_node->pci_dev == dev)
            {
                LOG("pcidev_remove:major = %d,minor = %d!\n",pcidev_major,pcidev_node->minor);
                devno = MKDEV(pcidev_major, pcidev_node->minor);
                cdev_del(&(pcidev_node->cdev));
                pcidev_deinit(pcidev_node);
                kfree(pcidev_node);
                break;
            }
        }
    unregister_chrdev_region(devno, 1);

    //device_destroy(cPci5125Class, MKDEV(pcidev_major, pcidev_minor));
    //class_destroy(cPci5125Class);
}

static struct pci_device_id pcidev_table[] =
  {
    { PLX_VENDOR_ID, PLX_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { 0, }, };

MODULE_DEVICE_TABLE( pci, pcidev_table);

static struct pci_driver can_pci_driver_ops =
  { .name = DRIVER_NAME, .id_table = pcidev_table, .probe = pcidev_probe,
      .remove = __devexit_p(pcidev_remove), };

int __init pcidev_init_module(void)
{
    int status = 0;

    LOG("----------init-----------\n");

    status = pci_register_driver(&can_pci_driver_ops);

    return status;
}

static void __exit pcidev_exit_module(void)
{
    LOG("-----------exit----------");
    pci_unregister_driver(&can_pci_driver_ops);
}

module_init(pcidev_init_module);
module_exit(pcidev_exit_module);
