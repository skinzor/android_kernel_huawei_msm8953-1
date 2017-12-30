#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/highmem.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/of.h>
#include <asm/uaccess.h>
#include <linux/of_address.h>
#include <linux/init.h>
#include <linux/module.h>
#include "srecorder_log.h"
#include "srecorder_sahara.h"

#ifdef CONFIG_HUAWEI_KERNEL_DEBUG
struct workqueue_sbl1_lk_log workqueue_get_log;
#endif

extern void get_random_bytes(void *buf, int nbytes);

void srecorder_sahara_clear(struct sahara_boot_log *head)
{
	printk(KERN_ERR "srecorderclear:\n");
	printk(KERN_ERR "head->sbl_log_addr:%x\n",head->sbl_log_addr);
	printk(KERN_ERR "head->lk_log_addr:%x\n",head->lk_log_addr);
	printk(KERN_ERR "head->sbl_log_size:%x\n",head->sbl_log_size);
	printk(KERN_ERR "head->lk_log_size:%x\n",head->lk_log_size);
//	head->sbl_log_addr=0;
//	head->lk_log_addr=0;
//	head->sbl_log_size=0;
//	head->lk_log_size=0;
}

#ifdef CONFIG_HUAWEI_KERNEL_DEBUG
void lk_log_write_to_kernel(struct work_struct *work)
{
    struct workqueue_sbl1_lk_log *sbl1_lk_head=NULL;
    char *buf = NULL;
    unsigned int len=0;
    unsigned int i=0;
    char *p=NULL;
    if(work==NULL)
    {
        return;
    }
    sbl1_lk_head=container_of(work,struct workqueue_sbl1_lk_log,sbl1_lk_log_handle);
    buf = sbl1_lk_head->start_addr;
    len=sbl1_lk_head->size;
    p=buf;
    if(p==NULL)
    {
        return;
    }
    if(len>=SAHARA_BOOT_LOG_SIZE_MAX)
    {
        printk(KERN_ERR "Srecorder sahara bBootloader log size error \n");
        return;
    }
    printk(KERN_ERR "Bootloader log start : %lx\n",(long unsigned int)buf);
    for(i=0;i<len;i++)
    {
        if(buf[i]=='\0')
            buf[i]=' ';
        if(buf[i]=='\n')
        {
            buf[i]='\0';
            printk(KERN_ERR "Bootloader log: %s\n",p);
            buf[i]='\n';
            p=&buf[i+1];
        }
    }

    printk(KERN_ERR "srecorder:i:%d   len:%d\n",i,len);
    printk(KERN_ERR "Bootloader log end\n");

}
#endif


/*===========================================================================
**  Function :  minidump_get_kramdom
* @brief
*   This function get real random number in kernel.
* 
* @param[in] 
*   None
*
* @par Dependencies
*   None
* 
* @retval
*   random number
* 
* @par Side Effects
*   None
** ==========================================================================*/
uint32_t minidump_get_kramdom(void)
{
    uint32_t dumpid_ramdom=0;
    printk(KERN_ERR "Minidump get real random number.\n");
    get_random_bytes(&dumpid_ramdom, sizeof(uint32_t));
    printk(KERN_ERR "Minidump get random number: %ld\n", dumpid_ramdom);
    return dumpid_ramdom;
}

/*===========================================================================
**  Function :  minidump_get_cpuid
* @brief
*   This function get cpuid or BSN number.
* 
* @param[in] 
*   None
*
* @par Dependencies
*   None
* 
* @retval
*   cpu id number
* 
* @par Side Effects
*   None
** ==========================================================================*/
uint32_t minidump_get_cpuid(void)
{
    return 0;
}
/*===========================================================================
**  Function :  minidump_set_dumpid
* @brief
*   This function set dumpid to sahara_mem reserve ddr region include cpuid and random number.
* 
* @param[in] 
*   None
*
* @par Dependencies
*   None
* 
* @retval
*   None
* 
* @par Side Effects
*   None
** ==========================================================================*/
void minidump_set_dumpid()
{
    struct sahara_boot_log *boot_log = NULL;
    unsigned long sahara_log_addr = 0;
    unsigned long sahara_log_size = 0;
    void *head_addr = NULL;
    struct device_node *sahara_mem_dts_node = NULL;
    const u32 *sahara_mem_dts_basep = NULL;
    sahara_mem_dts_node = of_find_compatible_node(NULL, NULL, "sahara_mem");
    if (sahara_mem_dts_node == 0)
    {
        SRECORDER_PRINTK("sahara_log_addr Finding compatible node failed.\n");
        return;
    }
    sahara_mem_dts_basep = of_get_address(sahara_mem_dts_node, 0, (u64*)&sahara_log_size, NULL);
    sahara_log_addr = (unsigned long)of_translate_address(sahara_mem_dts_node, sahara_mem_dts_basep);
    SRECORDER_PRINTK("sahara_log_addr:0x%lx sahara_log_size:0x%lx\n",sahara_log_addr,sahara_log_size);

    if (sahara_log_addr == 0 || sahara_log_size < SAHARA_BOOT_LOG_SIZE_MAX)
    {
         SRECORDER_PRINTK("srecorder_sahara get dts addr error \n");
         return;
    }
#ifdef CONFIG_ARM
    head_addr = (void *)ioremap_nocache(sahara_log_addr,sahara_log_size);
#else
    head_addr = (void *)ioremap_wc(sahara_log_addr, sahara_log_size);
#endif
    boot_log=(struct sahara_boot_log *)head_addr;
	
	//get cpuid
    boot_log->dumpid.cpuid = minidump_get_cpuid();
	
	//write random number as magic number
    boot_log->dumpid.random = minidump_get_kramdom();
    return;    
}

void srecorder_save_kernel_log_addr()
{
    struct sahara_boot_log *boot_log = NULL;
    unsigned long sahara_log_addr = 0;
    unsigned long sahara_log_size = 0;
    void *head_addr = NULL;
    struct device_node *sahara_mem_dts_node = NULL;
    const u32 *sahara_mem_dts_basep = NULL;
#ifdef CONFIG_HUAWEI_KERNEL_DEBUG
	struct workqueue_struct *wq = NULL;
	unsigned long delay_time = 0;
#endif

    sahara_mem_dts_node = of_find_compatible_node(NULL, NULL, "sahara_mem");
    if (sahara_mem_dts_node == 0)
    {
        SRECORDER_PRINTK("sahara_log_addr Finding compatible node failed.\n");
        return;
    }

    sahara_mem_dts_basep = of_get_address(sahara_mem_dts_node, 0, (u64*)&sahara_log_size, NULL);
    sahara_log_addr = (unsigned long)of_translate_address(sahara_mem_dts_node, sahara_mem_dts_basep);

    SRECORDER_PRINTK("sahara_log_addr:0x%lx sahara_log_size:0x%lx\n",sahara_log_addr,sahara_log_size);

    if (sahara_log_addr == 0 || sahara_log_size < SAHARA_BOOT_LOG_SIZE_MAX)
    {
         SRECORDER_PRINTK("srecorder_sahara get dts addr error \n");
         return;
    }

#ifdef CONFIG_ARM
    head_addr = (void *)ioremap_nocache(sahara_log_addr,sahara_log_size);
#else
    head_addr = (void *)ioremap_wc(sahara_log_addr, sahara_log_size);
#endif

    boot_log=(struct sahara_boot_log *)head_addr;

#ifdef CONFIG_KALLSYMS
    boot_log->kernel_log_addr = virt_to_phys((void *)kallsyms_lookup_name("__log_buf"));
#else
    boot_log->kernel_log_addr = 0;
#endif

#ifdef CONFIG_LOG_BUF_SHIFT
    boot_log->kernel_log_size = (1 << CONFIG_LOG_BUF_SHIFT);
#else
    boot_log->kernel_log_size = 0;
#endif
#ifdef CONFIG_HUAWEI_KERNEL_DEBUG
    workqueue_get_log.start_addr=(char *)(head_addr+sizeof(struct sahara_boot_log));
    workqueue_get_log.size=(unsigned int)(boot_log->lk_log_size+boot_log->sbl_log_size);
    wq=create_workqueue("srecorder_sbl1_lk_log");
    delay_time = msecs_to_jiffies(SAHARA_BOOT_SBL_LK_DELAY);
    INIT_DELAYED_WORK(&(workqueue_get_log.sbl1_lk_log_handle),lk_log_write_to_kernel);
    queue_delayed_work(wq,&(workqueue_get_log.sbl1_lk_log_handle), delay_time);
//	lk_log_write_to_kernel2((char *)(boot_log2+sizeof(struct sahara_boot_log)),(unsigned int)(boot_log2->lk_log_size+boot_log2->sbl_log_size));
#endif
    SRECORDER_PRINTK("srecorder_sahara get dts addr %lx\n",(long unsigned int)head_addr);
    SRECORDER_PRINTK("srecorder_sahara sbl1_log_addr:%x sbl1_log_size:%x \n",boot_log->sbl_log_addr,boot_log->sbl_log_size);
    SRECORDER_PRINTK("srecorder_sahara lk_log_addr:%x lk_log_size:%x \n",boot_log->lk_log_addr,boot_log->lk_log_size);
    SRECORDER_PRINTK("srecorder_sahara kernel_log_addr:%x kernel_log_size:%x \n",boot_log->kernel_log_addr,boot_log->kernel_log_size);

    srecorder_sahara_clear(boot_log);
}
