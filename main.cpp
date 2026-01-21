#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cerrno> // 為了讀取 EBUSY 錯誤碼
#include "v_pta.h"

int main(int argc, char* argv[]) {
    // 檢查參數夠不夠
    if (argc < 2) {
        std::cout << "Usage:\n";
        std::cout << "  ./pta_app wifi <priority>\n";
        std::cout << "  ./pta_app bt <1|0>        (Manual On/Off)\n";
        std::cout << "  ./pta_app bt burst        (Auto 50ms Pulse)  <-- NEW!\n";
        std::cout << "  ./pta_app stats\n";
        return 1;
    }

    int fd = open("/dev/v_pta", O_RDWR);
    if (fd < 0) {
        perror("開啟裝置失敗"); // perror 會自動印出錯誤原因
        return -1;
    }

    std::string cmd = argv[1];

    if (cmd == "wifi") {
        if (argc < 3) {
            std::cerr << "缺少優先級參數！範例: ./pta_app wifi 5\n";
            return 1;
        }
        int priority = std::stoi(argv[2]); // 字串轉整數
        
        // 發送請求
        if (ioctl(fd, PTA_IOC_WIFI_REQUEST, &priority) < 0) {
            // 如果 Driver 回傳 -EBUSY，這裡會變成 errno = 16
            if (errno == EBUSY) {
                std::cout << "❌ [REJECTED] 請求被拒！(因為 BT 正在使用中)\n";
            } else {
                perror("Ioctl Error");
            }
        } else {
            std::cout << "✅ [GRANTED] Wi-Fi 請求通過。\n";
        }
    } 
    else if (cmd == "bt") {
        if (argc < 3) return 1;
        std::string subcmd = argv[2];
        int state = 0;

        if (subcmd == "burst") {
            state = 2; // 我們定義 2 為 Burst 模式
            std::cout << "⚡ 發送 BT 突發傳輸 (50ms)... \n";
        } else {
            state = std::stoi(subcmd);
            std::cout << "🔵 設定 BT 狀態: " << state << "\n";
        }

        ioctl(fd, PTA_IOC_BT_REQUEST, &state);
    }
    
    else if (cmd == "stats") {
        struct pta_stats s;
        ioctl(fd, PTA_IOC_GET_STATS, &s);
        std::cout << "--- 統計報表 ---\n";
        std::cout << "Wi-Fi 成功: " << s.wifi_granted << "\n";
        std::cout << "Wi-Fi 被拒: " << s.wifi_rejected << "\n";
        std::cout << "BT    成功: " << s.bt_granted << "\n";
    }

    close(fd);
    return 0;
}