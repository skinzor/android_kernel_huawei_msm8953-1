#ifndef SRECORDER_SAHARA_H
#define SRECORDER_SAHARA_H


#ifdef __cplusplus
extern "C" {
#endif
#define SAHARA_BOOT_LOG_ADDR  0x92000000
#define SAHARA_BOOT_LOG_SIZE  0x00050000
#define SAHARA_BOOT_LOG_SIZE_MAX  0x00050000

#ifdef CONFIG_HUAWEI_KERNEL_DEBUG
#define SAHARA_BOOT_SBL_LK_DELAY 20000
#endif
struct minidump_dumpid
{
    uint32_t cpuid;
    uint32_t random;
};

struct sahara_boot_log
{
    uint32_t sbl_log_addr;
    uint32_t lk_log_addr;
    uint32_t kernel_log_addr;
    uint32_t sbl_log_size;
    uint32_t lk_log_size;
    uint32_t kernel_log_size;
	struct minidump_dumpid dumpid;
};

#ifdef CONFIG_HUAWEI_KERNEL_DEBUG
struct workqueue_sbl1_lk_log
{
    char *start_addr;
    unsigned int size;
    struct delayed_work  sbl1_lk_log_handle;
};
#endif
void srecorder_save_kernel_log_addr(void);
void minidump_set_dumpid(void);
#ifdef __cplusplus
}
#endif

#endif /* SRECORDER_SAHARA_H*/
