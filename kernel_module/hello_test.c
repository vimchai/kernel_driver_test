
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/mman.h>

int hello_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned char *p = NULL;
    unsigned long pfn = 0;
    unsigned long vmsize = vma->vm_end - vma->vm_start;

    printk("in hello_mmap\n");

    printk(KERN_ALERT"in hello_mmap vma startv%lu\n", vma->vm_start);
    vma->vm_flags |= VM_IO;

    p = kmalloc(PAGE_SIZE, GFP_KERNEL);
    pfn = virt_to_phys((void *)p) >> PAGE_SHIFT;

    if(vmsize > PAGE_SIZE)
        return -EINVAL;
    if(remap_pfn_range(vma, vma->vm_start, pfn, vmsize, vma->vm_page_prot))
        return -EAGAIN;
    return 0;
}