#include <linux/ctype.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/string.h>

#include "encdec.h"

#define MODULE_NAME "encdec"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ahmad & Yazan");

int encdec_open(struct inode *inode, struct file *filp);
int encdec_release(struct inode *inode, struct file *filp);
int encdec_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

ssize_t encdec_read_caesar(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t encdec_write_caesar(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

ssize_t encdec_read_xor(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t encdec_write_xor(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

int memory_size = 0;
static char *CaesarBuffer;
static char *XORBuffer;

MODULE_PARM(memory_size, "i");

int major = 0;

struct file_operations fops_caesar = {
	.open = encdec_open,
	.release = encdec_release,
	.read = encdec_read_caesar,
	.write = encdec_write_caesar,
	.llseek = NULL,
	.ioctl = encdec_ioctl,
	.owner = THIS_MODULE
};

struct file_operations fops_xor = {
	.open = encdec_open,
	.release = encdec_release,
	.read = encdec_read_xor,
	.write = encdec_write_xor,
	.llseek = NULL,
	.ioctl = encdec_ioctl,
	.owner = THIS_MODULE
};

// Implemetation suggestion:
// -------------------------
// Use this structure as your file-object's private data structure
typedef struct {
	unsigned char key;
	int read_state;
} encdec_private_date;

int init_module(void)
{
	major = register_chrdev(major, MODULE_NAME, &fops_caesar);
	if (major < 0)
	{
		printk(KERN_WARNING "Bad dynamic major\n");
		return major;
	}
	else
	{
		CaesarBuffer = (char*)kmalloc(memory_size * sizeof(char), GFP_KERNEL);
		if (!CaesarBuffer)
		{
			printk(KERN_WARNING "CaesarBuffer memory not allocated!\n");
			return -ENOMEM;
		}
		XORBuffer = (char*)kmalloc(memory_size * sizeof(char), GFP_KERNEL);
		if (!XORBuffer)
		{
			printk(KERN_WARNING "XORBuffer memory not allocated!\n");
			return -ENOMEM;
		}

		memset(CaesarBuffer, 0, memory_size);
		memset(XORBuffer, 0, memory_size);
	}
	return 0;
}

void cleanup_module(void)
{


	if (unregister_chrdev(major, MODULE_NAME) < 0)	// add base minor and count
	{
		printk(KERN_WARNING "unregister_chrdev failed!\n");
	}

	kfree(CaesarBuffer);
	kfree(XORBuffer);


}

int encdec_open(struct inode *inode, struct file *filp)
{

    int minor = MINOR(inode->i_rdev);

    // Implemetation suggestion:
    // -------------------------
    // 1. Set 'filp->f_op' to the correct file-operations structure (use the minor value to determine which)
    // 2. Allocate memory for 'filp->private_data' as needed (using kmalloc)

    if (minor < 0 || minor > 1)        // Invalid argument
    {
        return -ENODEV;
    }
    else if (minor == 0)
    {
        filp->f_op = &fops_caesar;
    }
    else if (minor == 1)
    {
        filp->f_op = &fops_xor;
    }
    else
    {
        return -ENODEV;
    }

    filp->private_data = (struct encdec_private_date*)kmalloc(sizeof(encdec_private_date), GFP_KERNEL);
    ((encdec_private_date*)filp->private_data)->key = 0;
    ((encdec_private_date*)filp->private_data)->read_state = ENCDEC_READ_STATE_DECRYPT;


	return 0;
}

int encdec_release(struct inode *inode, struct file *filp)
{

	// Implemetation suggestion:
	// -------------------------
	// 1. Free the allocated memory for 'filp->private_data' (using kfree)

	kfree(filp->private_data);

	return 0;
}

int encdec_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	encdec_private_date *myPrivateData = filp->private_data;
	int minor = MINOR(inode->i_rdev);
	int i = 0;

	// Implemetation suggestion:
	// -------------------------
	// 1. Update the relevant fields in 'filp->private_data' according to the values of 'cmd' and 'arg'

	switch (cmd)
	{
	case ENCDEC_CMD_CHANGE_KEY:

		myPrivateData->key = arg;
		break;

	case ENCDEC_CMD_SET_READ_STATE:

		myPrivateData->read_state = arg;
		break;

	case ENCDEC_CMD_ZERO:

		if (minor < 0 || minor > 255)        
		{
			return -ENODEV;
		}
		else if (minor == 0) 
		{
			for (i = 0; i < memory_size; i++)
			{
				CaesarBuffer[i] = 0;
			}
		}
		else if (minor == 1)  
		{
			for (i = 0; i < memory_size; i++)
			{
				XORBuffer[i] = 0;
			}
		}
		break;


	default:
		return -ENOTTY;

	}

	return 0;
}

// Add implementations for:
// ------------------------
// 1. ssize_t encdec_read_caesar( struct file *filp, char *buf, size_t count, loff_t *f_pos );
// 2. ssize_t encdec_write_caesar(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
// 3. ssize_t encdec_read_xor( struct file *filp, char *buf, size_t count, loff_t *f_pos );
// 4. ssize_t encdec_write_xor(struct file *filp, const char *buf, size_t count, loff_t *f_pos);


void caeserEncrypt(size_t count, loff_t *f_pos, int key)
{
	int i = 0;

while(i<count)
	{
		CaesarBuffer[i + *f_pos] = (CaesarBuffer[i + *f_pos] + key) % 128; 
		i++;
	}
}

void xorEncrypt(size_t count, loff_t *f_pos, int key)
{
	int i = 0;

while(i<count)
	{
		XORBuffer[i + *f_pos] = XORBuffer[i + *f_pos] ^ key; 
		i++;
	}
}

void caeserDecrypt(size_t count, char *buf, int key )
{
	int i = 0;

while(i<count)
	{
		buf[i] = (buf[i] - key + 128) % 128; 
		i++;
	}
}

void xorDycrypt(size_t count, char *buf, int key)
{
	int i = 0;

while(i<count)
	{
		buf[i] = buf[i] ^ key; 
		i++;
	}
}


ssize_t encdec_write_caesar(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{

	encdec_private_date  *myPrivateData = filp->private_data;

	if (filp == NULL)
	{
		return -ENOENT;
	}
	else if (buf == NULL)
	{
		return -EINVAL;
	}
	else if (*f_pos >= memory_size)    
	{
		return -ENOSPC;
	}
	else if (*f_pos + count > memory_size)  
	{

		count=memory_size-*(f_pos);
	}
	else
	{
		if (copy_from_user(CaesarBuffer + *f_pos, buf, count) != 0) 
		{
			*f_pos = *f_pos + count;
			return -EFAULT;
		}

		else
		{
			caeserEncrypt(count, f_pos, myPrivateData->key); 
			*f_pos = *f_pos + count;
		}

	}
	return count;
}

ssize_t encdec_write_xor(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	encdec_private_date  *myPrivateData = filp->private_data;

	if (filp == NULL)
	{
		return -ENOENT;
	}
	else if (buf == NULL)
	{
		return -EINVAL;
	}
	else if (*f_pos >= memory_size)   
	{

		return -ENOSPC;

	}
	else if (*f_pos + count > memory_size)  
	{

		count=memory_size-*(f_pos);
	}
	else
	{
		if (copy_from_user(XORBuffer + *f_pos, buf, count) != 0) 
		{
			*f_pos = *f_pos + count;
			return -EFAULT;
		}
		else
		{
			xorEncrypt(count, f_pos, myPrivateData->key);
			*f_pos = *f_pos + count;
		}
	}

	return count;
}

ssize_t encdec_read_caesar(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	encdec_private_date *myPrivateData = (encdec_private_date*)(filp->private_data);
	int available = memory_size - *f_pos;

	if (count > available)
	{
		count = available;
	}
	if (filp == NULL || buf == NULL)
	{
		return -ENOENT;
	}
	else if (*f_pos >= memory_size)    
	{
		return -EINVAL;
	}
	else if (*f_pos + count > memory_size)  
	{

		count=memory_size-*(f_pos);
	}
	else
	{
	    int result = copy_to_user(buf, CaesarBuffer + *f_pos, count);
		if (myPrivateData->read_state == ENCDEC_READ_STATE_RAW)
		{
			if (result)			
			{
				return -EFAULT;
			}
		}
		else if (myPrivateData->read_state == ENCDEC_READ_STATE_DECRYPT)
		{
			if (result)			
			{
				return -EFAULT;
			}
			else
			{
				caeserDecrypt(count, buf, myPrivateData->key); 
            }
        }
	}
    *f_pos = *f_pos + count;

	return count;

}


ssize_t encdec_read_xor(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	encdec_private_date  *myPrivateData = filp->private_data;
	int available = memory_size - *f_pos;

	if (count > available)
	{
		count = available;
	}
	else if (*f_pos >= memory_size)    
	{
		return -EINVAL;
	}
	else if (*f_pos + count > memory_size)  
	{

		count=memory_size-*(f_pos);
	}
	else
	{
		if (myPrivateData->read_state == ENCDEC_READ_STATE_RAW)
		{
			if (copy_to_user(buf, XORBuffer + *f_pos, count) != 0)			 
			{
				*f_pos = *f_pos + count;
				return -EFAULT;
			}
			else
			{
				*f_pos = *f_pos + count;
			}
		}
		else if (myPrivateData->read_state == ENCDEC_READ_STATE_DECRYPT)
		{
			if (copy_to_user(buf, XORBuffer + *f_pos, count) != 0)			
			{
				*f_pos = *f_pos + count;
				return -EFAULT;
			}
			else
			{
				xorDycrypt(count, buf, myPrivateData->key); 
				*f_pos = *f_pos + count;
			}
		}
	}

	return count;
}




