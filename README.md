<div align="center">
  <img src="images/logo.jpg" alt="Autonomous Tram System Banner" width="100%" />
  
  <h1>Autonomous Tram System with Kinetic Energy Recovery (KERS)</h1>
  
  <p>
    <b>An autonomous urban rail prototype featuring Vehicle-to-Infrastructure (V2I) communication, smart traffic priority algorithms, and a mechanical regenerative braking system.</b>
  </p>

  <p>
    <img src="https://img.shields.io/badge/Language-C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B" />
    <img src="https://img.shields.io/badge/Hardware-Arduino_%7C_Custom_PCB-00979D?style=for-the-badge&logo=arduino" />
    <img src="https://img.shields.io/badge/Mechanism-Flywheel_KERS-orange?style=for-the-badge&logo=mechanical-keyboard" />
    <img src="https://img.shields.io/badge/Award-WRO_2021_International_Finalist-gold?style=for-the-badge&logo=google-scholar" />
  </p>
</div>

---

## 📖 Overview
This project addresses the energy inefficiency of urban stop-and-go transport. It is a fully functional **1:20 scale prototype** of an autonomous tram network that integrates **mechanical energy recovery** and **smart traffic management** to reduce power consumption.

Unlike standard line-followers, this system uses **V2I (Vehicle-to-Infrastructure)** communication to actively control traffic lights, creating a "Green Wave" for the tram to minimize braking events.

> 🏆 **Achievements:**
> * **International Finalist:** World Robot Olympiad (WRO) 2021
> * **2nd Place:** Baltic Science and Engineering Competition
> * **1st Place:** All-Russian Festival "Vector"

## ⚙️ Engineering Architecture

### 1. The "Green Wave" Algorithm (V2I)
The core software innovation is the **Predictive Priority System**.
* **Logic:** The tram transmits its velocity and distance to upcoming traffic nodes via **NRF24L01** radio modules.
* **Action:** The traffic light controller calculates the tram's arrival time and preemptively switches the signal to GREEN.
* **Result:** Eliminates unnecessary stops at intersections, preserving momentum and reducing energy spikes by up to **30%**.

### 2. Kinetic Energy Recovery System (KERS)
To address energy loss during mandatory stops (stations), I engineered a hybrid recovery system:
* **Mechanical Flywheel:** A custom-designed **mechanical variator** connects a heavy flywheel to the drivetrain. During braking, the flywheel spins up (storing kinetic energy). During acceleration, this energy is mechanically transferred back to the wheels.
* **Electrical Buffer:** Supercapacitors were analyzed to handle voltage spikes during regenerative braking phases.

### 3. Distributed Hardware Control
The system operates on a decentralized architecture with custom PCBs:
* **Main Unit:** Arduino Nano (Tram Controller) handling PID speed control and sensor fusion.
* **Infrastructure Nodes:** Independent MCUs for Traffic Lights and Track Switches.
* **Sensors:**
    * **VL53L0X (ToF LiDAR):** High-precision distance measurement for station docking (~2mm accuracy).
    * **Reed Switches:** Magnetic positioning for track segmentation.
    * **Encoders:** Real-time odometry for speed calculation.

## 🔋 Energy & Efficiency Analysis
Research conducted during the project demonstrated that the combination of **Smart Traffic Logic** and **KERS** could theoretically reduce total network energy consumption significantly compared to standard non-connected transport systems.

* **Auxiliary Power:** The station models feature piezoelectric elements under the platform to harvest energy from passenger foot traffic, powering local lighting.

## 🧠 Technical Challenges Solved

| Challenge | Engineering Solution |
| :--- | :--- |
| **Momentum Conservation** | Replaced standard worm gears with a custom **low-friction gearbox** to maximize coasting distance. |
| **Communication Conflicts** | Implemented a custom packet structure over NRF24L01 to prevent data collisions between multiple infrastructure nodes. |
| **Precision Docking** | Used a closed-loop control system (LiDAR + PID) to ensure the tram aligns perfectly with station doors. |

## 🛠️ Retrospective: v2.0 Roadmap
*Writing this as a Computer Engineering student looking back:*
* **Upgrade Comms:** Transition from NRF24L01 to **ESP32/Wi-Fi (MQTT)** for IoT integration and telemetry logging.
* **Refine KERS:** Replace the mechanical variator with an **Electronic KERS** (Motor as Generator -> Supercapacitor Bank -> Buck-Boost Converter) for higher efficiency and fewer moving parts.
* **State Management:** Implement a formal **FreeRTOS** architecture to handle multiple real-time tasks (comms, motor control, sensing) more robustly.

## 📂 Repository Structure
```bash
├── firmware/
│   ├── tram_main/             # PID Logic & NRF Comms
│   ├── infrastructure/        # Smart Traffic Light Logic
│   └── libraries/             # Custom VL53L0X wrapper
├── hardware/
│   ├── pcb/                   # EasyEDA Gerber files
│   └── mechanical/            # STL files for Flywheel & Chassis
└── docs/
    ├── research_paper.pdf     # Full WRO Report
    └── schematics/            # Circuit Diagrams
