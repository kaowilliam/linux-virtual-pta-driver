# Linux Virtual PTA (Packet Traffic Arbitration) Driver

## 📖 Project Overview
This project implements a **Linux Character Device Driver** to simulate the **Packet Traffic Arbitration (PTA)** mechanism used in wireless coexistence (Wi-Fi/Bluetooth) systems.

The goal is to demonstrate **Kernel Space programming**, **Concurrency Control**, and **Asynchronous Hardware Simulation** within a Linux environment. It mimics a real-world scenario where a Bluetooth transmission (high priority) must preempt a Wi-Fi request to avoid radio interference.

## 🚀 Key Features
* **Character Device Interface:** Implements `open`, `close`, and `ioctl` system calls for User/Kernel communication.
* **Concurrency Control:** Uses **Spinlocks (`spinlock_t`)** to protect critical sections and shared state variables against Race Conditions.
* **Arbitration Logic:** Enforces priority-based resource granting (Bluetooth > Wi-Fi) with rejection handling (`-EBUSY`).
* **Asynchronous Event Handling:** Utilizes **Kernel Timers (`struct timer_list`)** and **Jiffies** to simulate non-blocking hardware transmission bursts (SoftIRQ context).
* **User Space Tool:** A C++ CLI tool to interact with the driver and perform stress tests.

## 🛠️ Tech Stack
* **Language:** C (Kernel Module), C++ (User Space App)
* **Kernel APIs:** `linux/module.h`, `linux/fs.h`, `linux/spinlock.h`, `linux/timer.h`
* **Build System:** GNU Make (Kbuild)
* **Platform:** Linux (tested on Ubuntu 24.04, Kernel 6.8+)

## 🏗️ Architecture
The system consists of a User Space application sending requests via `ioctl`, and a Kernel Module acting as the Arbiter.

```mermaid
sequenceDiagram
    participant User as C++ App
    participant Driver as Kernel Driver (Arbiter)
    participant Timer as Kernel Timer (SoftIRQ)

    Note over User, Driver: Scenario: BT Burst vs Wi-Fi
    
    User->>Driver: ioctl(BT_BURST)
    activate Driver
    Driver->>Driver: spin_lock_irqsave()
    Driver->>Driver: Set bt_active = true
    Driver->>Timer: mod_timer(+50ms)
    Driver->>Driver: spin_unlock_irqrestore()
    Driver-->>User: return 0 (Non-blocking)
    deactivate Driver

    User->>Driver: ioctl(WIFI_REQUEST)
    activate Driver
    Driver->>Driver: Check bt_active == true
    Driver-->>User: return -EBUSY (REJECTED)
    deactivate Driver

    Note over Timer: 50ms later...
    Timer->>Driver: Callback Triggered
    Driver->>Driver: Set bt_active = false
```

## ⚡ How to Build & Run

### 1. Build the Driver and App
```bash
make
g++ main.cpp -o pta_app
```

### 2. Load the Module
```bash
sudo insmod v_pta.ko
# If device node is not created automatically:
sudo mknod /dev/v_pta c 240 0
sudo chmod 666 /dev/v_pta
```

### 3. Test Scenarios
**Scenario A: BT Burst & Conflict**
```bash
./pta_app bt burst
./pta_app wifi 5
# Output: [REJECTED] Wi-Fi Request Denied! (BT is active)
```

**Scenario B: Check Stats**
```bash
./pta_app stats
```

## 👨‍💻 About Me
I am a Firmware Engineer specializing in Wi-Fi connectivity and embedded systems. This project was created to deepen my understanding of Linux Kernel internals and resource management in multi-core environments.