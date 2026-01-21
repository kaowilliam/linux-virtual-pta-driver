#ifndef V_PTA_H
#define V_PTA_H

#include <linux/ioctl.h>

// 定義 Magic Number，避免跟系統其他 Driver 衝突
#define PTA_IOC_MAGIC 'p'

// 定義指令：模擬 Wi-Fi 請求傳送 (帶有優先級參數)
// _IOW 巨集的意思是：Input (從 User 寫入 Kernel), Magic Number, 指令編號, 數據型態
#define PTA_IOC_WIFI_REQUEST  _IOW(PTA_IOC_MAGIC, 1, int)

// 定義指令：模擬 BT 請求傳送
#define PTA_IOC_BT_REQUEST    _IOW(PTA_IOC_MAGIC, 2, int)

// 定義指令：讀取統計數據 (給 BMC Monitor 用)
#define PTA_IOC_GET_STATS     _IOR(PTA_IOC_MAGIC, 3, struct pta_stats)

// 定義統計數據的結構
struct pta_stats {
    unsigned long wifi_granted;
    unsigned long wifi_rejected;
    unsigned long bt_granted;
    unsigned long bt_rejected;
};

#endif