![i2CAT Logo](Fundacio-Privada-I2CAT.png)

# Project Status: 
Complete

# Description
This repository contains the **Ns-3 simulator Wifi module** with modifications to support **Wi-Fi Multi-Link Operation (MLO) with Simultaneous Transmission and Reception (STR)**. This project executes the SLA-MLO scheduler proposed in [Paper](https://ieeexplore.ieee.org/document/10454738).


# Pre-requisites
- Ubuntu 20.04 / 22.04 (recommended)
- Python 3.6 +
- GCC 9.0+ or Clang 10.0+
- For more on how to install NS-3, check official site ([NS3.41](https://www.nsnam.org/))


##  Precautions

- NS-3 tracers and other measurement methods may report different delay, throughput, and similar metrics due to variations in how the calculations are performed. Therefore, it is recommended to use the trace files generated directly from the simulation for analysis.


# How to install it 
No pre-built binaries are shipped. Follow the official ns-3 installation instructions from the project website.

# How to Build and Run

### Step 1: Integrate the MLO Files

You have two options to add the MLO implementation:

1. **Full folder approach**  
   - Download the entire ns-3 folder with MLO modifications.  
   - Delete the `CMakeCache.txt` file and the `build/` folder.  
   - Rebuild ns-3 from scratch using:
     ```bash
     ./ns3 configure --enable-examples --enable-tests
     ./ns3 build
     ```

2. **Replace Wi-Fi module**  
   - Replace the original Wi-Fi module in your existing ns-3 installation with the modified files.  
   - ⚠️ Warning: This may cause issues with the latest ns-3 versions.

### Step 2: Configure Log/Trace File Locations

By default, trace files are generated in: "ns-allinone-3.36.1_MLO/ns-allinone-3.36.1/ns-3.36.1/traces".

You can change the output location by modifying the following files:

- `Work.cc`  
- `qos-txop.cc`  
- `sta-wifi-mac.cc`  
- `wifi-mac.cc`  
- `ht-frame-exchange-manager.cc`  

> Recommendation: Use a `traces/` folder inside your ns-3 root directory for easy management.

### Step 3: Run File
```bash
'./ns3 run Work'
```

- The code will generate per-link delay, average delay, and per-STA delay files for different STAs.
- Currently, the setup supports one flow per STA, but it can be extended to multiple flows if required.
- Read the code comments for detailed explanations for configurations.



### Step 4: Analyze Results

- Use the generated trace files in the traces/ folder for plotting or calculating metrics like throughput, delay, and packet loss.
- Avoid relying on external packet captures (e.g., Wireshark), as they may report slightly different metrics due to differences in measurement methods.





# Technical description 

## Wi-Fi MLO Implementation in ns-3

This section describes the implementation of **Wi-Fi Multi-Link Operation (MLO)** with **Simultaneous Transmission and Reception (STR)** capability in ns-3.

---

### Overview

The implementation enables a Wi-Fi station (STA) to transmit and receive simultaneously across multiple links.  
Each link has its own:

- **EDCA (Enhanced Distributed Channel Access)**
- **Frame Exchange Mechanism**
- **Channel Access Manager**
- **Physical Layer (PHY)**
- **Independent MAC queue** (default size: 500 packets per link, configurable)

This setup supports multi-link configurations, starting from a two-link case.

---

### Important Modified Files and Classes in Wi-Fi Module

| File | Description |
|------|--------------|
| `sta-wifi-mac.cc` | SLA-based flow handling, background STAs handling and call to scheduler|
| `wifi-mac.cc` | Core scheduler implementation |
| `txop.cc` | Link-specific queue management and channel access |
| `wifi-mac-queue.cc` | Queue search and packet retrieval for specific link |
| `ht-frame-exchange-manager.cc` | on packet reception, writing data into files|


---

### Scheduler Design

#### `sta-wifi-mac.cc`

- Sets background STAs on one link (`link1` or `link2`).
- Configures **SLA-based flows** that distribute traffic over multiple links based on delay bounds and error thresholds.
- Calls the **scheduler**, which decides which link to use for transmission.
- Adds **link ID** in the packet header before enqueueing.

#### `wifi-mac.cc`

- Implements the **core scheduler logic** for managing two-link operation.
- Coordinates between links to ensure STR capability and SLA compliance.
- Interfaces with per-link channel access and frame exchange managers.

---

### Queue and Channel Access Management

#### `txop.cc`

- Modified to manage **per-link queues** (currently supports 2 links, but configurable).  
- Calls the particular **Channel Access Manager** for the selected link during transmission.

#### `wifi-mac-queue.cc`

- The `SearchIItem()` function is updated to **find packets for a specific link** in the MAC queue.

---

### Frame Exchange Management

#### `ht-frame-exchange-manager.cc`

- Once acknowledgment of sucessfull reception is received, it log the file with current delay values for MLD STAs.
- It can also be used to gather information on throughput or any other statics

---

### Notes

- Current version supports **2-link STR operation**.
- The architecture can be easily extended for more links.
- Each link maintains full independence for contention, transmission, and reception, while being coordinated by the central scheduler.

---

# Source
This code has been developed within the research / innovation project 5GSmartFact. This project has received funding from the European Union’s Horizon 2020 research and innovation programme under the Marie Skłodowska-Curie grant agreement ID 956670. This project started on 1 March 2021 and will end on 31 December 2025, funded under H2020-EU.1.3.1 of the Horizon 2020 Framework Programme.  
More information about the grant at https://cordis.europa.eu/project/id/956670

# Copyright

This repository contains a modified version of the ns-3 network simulator.

- **Original ns-3 code**  
  Portions of this code are from the ns-3 simulator (https://www.nsnam.org/),  
  which is licensed under the **GNU General Public License v2 (GPLv2)**.  
  All original copyright notices and license headers in ns-3 files are preserved.

- **Modifications (Wi-Fi MLO STR)**  
  The modifications and additional code for Wi-Fi Multi-Link Operation (MLO) with Simultaneous Transmission and Reception (STR) have been developed by **Fundació Privada Internet i Innovació Digital a Catalunya (i2CAT)**.  
  i2CAT is a *non-profit research and innovation centre* that promotes mission-driven knowledge to solve business challenges, co-create solutions with transformative impact, empower citizens through open and participative digital social innovation with territorial capillarity, and promote pioneering and strategic initiatives.  
  i2CAT *aims to transfer* research project results to private companies in order to create social and economic impact via out-licensing of intellectual property and the creation of spin-offs.  
  This code is licensed under the **Affero GPL v3 (AGPLv3)**.  
  More information about i2CAT projects and IP rights can be found at: [https://i2cat.net/tech-transfer/](https://i2cat.net/tech-transfer/)




# License
- **Original ns-3 code:**  
  This work is based on the ns-3 network simulator (https://www.nsnam.org/),  
  which is licensed under the **GNU General Public License v2 (GPLv2)**.  
  All original ns-3 files retain their license headers. Users must comply with GPLv2
  when using or redistributing the original ns-3 code.

- **Modifications (Wi-Fi MLO STR):**  
  This code is licensed under the terms of the **Affero GPL v3 (AGPLv3)**.  
  Information about the license can be located at:  
  [https://www.gnu.org/licenses/agpl-3.0.en.html](https://www.gnu.org/licenses/agpl-3.0.en.html)

This repository is intended to be used in conjunction with the official ns-3 simulator.
All original copyright notices and license terms from ns-3 are preserved.

# Attribution Notice
This work uses third-party components that are attributed as follows:
* Modified version of NS-3.41 — Copyright © 2006–2025 The ns-3 project — licensed under GPL v2

