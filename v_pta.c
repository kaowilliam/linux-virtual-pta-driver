#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/timer.h>  // 【New】引入計時器標頭檔
#include <linux/jiffies.h> // 【New】引入時間單位 jiffies
#include "v_pta.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kao");
MODULE_DESCRIPTION("Virtual PTA Driver with Timer (Async)");

#define DEVICE_NAME "v_pta"

static int major_num;
static struct pta_stats stats = {0};
static bool bt_active = false;
static spinlock_t state_lock;

// 【New】定義一個計時器結構
static struct timer_list bt_timer;

// 【New】這是計時器時間到時，會被 Kernel 自動呼叫的函式 (Callback)
// 注意：這是在 SoftIRQ Context 執行，絕對不能 sleep！
static void bt_timer_callback(struct timer_list *t) {
    unsigned long flags;

    // 取得鎖 (防止這時候 User 剛好在改狀態)
    spin_lock_irqsave(&state_lock, flags);
    
    bt_active = false; // 時間到，模擬 BT 傳輸結束
    printk(KERN_INFO "Virtual PTA: [Timer] BT Burst Finished. Status -> IDLE\n");
    
    spin_unlock_irqrestore(&state_lock, flags);
}

static long pta_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int val;
    unsigned long flags;

    switch(cmd) {
        case PTA_IOC_WIFI_REQUEST:
            if (copy_from_user(&val, (int*)arg, sizeof(int))) return -EFAULT;
            
            spin_lock_irqsave(&state_lock, flags);
            
            // 仲裁：BT 在忙且 Wi-Fi 優先級低 (<10) 則拒絕
            if (bt_active && val < 10) {
                stats.wifi_rejected++;
                spin_unlock_irqrestore(&state_lock, flags);
                return -EBUSY;
            }
            
            stats.wifi_granted++;
            spin_unlock_irqrestore(&state_lock, flags);
            printk(KERN_INFO "Virtual PTA: Wi-Fi Granted (Priority: %d)\n", val);
            break;

        case PTA_IOC_BT_REQUEST:
            if (copy_from_user(&val, (int*)arg, sizeof(int))) return -EFAULT;

            spin_lock_irqsave(&state_lock, flags);
            
            if (val == 1) { 
                // 模式 1: 永久開啟 (手動)
                bt_active = true;
                printk(KERN_INFO "Virtual PTA: BT Set to ACTIVE (Manual)\n");
            } 
            else if (val == 2) { 
                // 【New】模式 2: 突發傳輸 (Burst 50ms)
                bt_active = true;
                // 設定計時器：目前的 jiffies + 50ms
                mod_timer(&bt_timer, jiffies + msecs_to_jiffies(50));
                printk(KERN_INFO "Virtual PTA: BT Burst Started (50ms timer set)\n");
            }
            else { 
                // 模式 0: 關閉
                bt_active = false;
                del_timer(&bt_timer); // 如果有計時器正在跑，取消它
                printk(KERN_INFO "Virtual PTA: BT Set to IDLE\n");
            }
            
            stats.bt_granted++;
            spin_unlock_irqrestore(&state_lock, flags);
            break;

        case PTA_IOC_GET_STATS:
            if (copy_to_user((struct pta_stats*)arg, &stats, sizeof(stats))) return -EFAULT;
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = pta_ioctl,
};

static int __init pta_init(void) {
    spin_lock_init(&state_lock);
    
    // 【New】初始化計時器，綁定 Callback 函式
    timer_setup(&bt_timer, bt_timer_callback, 0);

    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_num < 0) return major_num;
    printk(KERN_INFO "Virtual PTA: Loaded with Timer support.\n");
    return 0;
}

static void __exit pta_exit(void) {
    // 【非常重要】卸載 Driver 前一定要刪除計時器！
    // 不然 Kernel 之後呼叫一個不存在的記憶體位址，會導致整個 OS 當機 (Kernel Panic)。
    del_timer_sync(&bt_timer);
    
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "Virtual PTA: Unloaded.\n");
}

module_init(pta_init);
module_exit(pta_exit);