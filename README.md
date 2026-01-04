# Autonomous and Energy-Efficient Tram System  
![Logo](images/logo.jpg)
## Overview
This project is a fully autonomous tram system prototype developed individually
as an embedded systems engineering project.

The system demonstrates autonomous vehicle control, infrastructure automation,
and energy efficiency solutions for urban rail transport under real-time and
hardware constraints.

The prototype includes automated tram control, traffic lights, switches, stops,
and an energy recovery concept implemented using embedded systems.

## Objectives
- Develop autonomous and remote tram control
- Automate tram infrastructure (switches, traffic lights, stops)
- Build a working physical prototype
- Analyze energy consumption and recovery in tram systems

## Hardware
- Arduino Nano (multiple controllers)
- Motor driver: TB6612FNG
- Wireless communication: NRF24L01
- Laser rangefinder: VL53L0X
- Reed switches, potentiometers
- Servo motors (switches)
- DC motor with gearbox
- Custom PCB (designed in EasyEDA)
- 3D-printed tram body, rails, infrastructure

## Software
- Language: C++
- Platform: Arduino
- Modular control logic for tram and infrastructure
- Real-time loop-based control without external frameworks

## System Architecture
The system uses multiple Arduino controllers responsible for:
- Tram movement and obstacle detection
- Switch and traffic light control
- Stop management and intervals
- Emergency and manual override

Wireless communication ensures coordination between tram and infrastructure.

## Key Algorithms and Logic
- Autonomous navigation along rails
- Obstacle detection and emergency stop using laser rangefinder
- Fixed-interval tram scheduling independent of entry point
- Fail-safe logic with manual control fallback
- Anti-jam door control system

## Energy Efficiency Concepts
- Regenerative braking with flywheel energy storage (FES)
- Energy harvesting at stops using piezoelectric elements
- Solar panels on stops for auxiliary power
- Analysis of braking energy reuse for tram acceleration

## Challenges and Engineering Decisions
- Sensor noise handled with thresholding and filtering
- Real-time constraints addressed by optimized control loops
- Safety prioritized through manual override and system isolation
- Cost optimization using low-cost embedded components

## Results
- Fully working autonomous tram prototype
- Automated infrastructure interaction
- Demonstrated feasibility of energy recovery

## What I Would Improve Today
- Transition to a formal state machine architecture
- More robust sensor fusion
- CAN or RS485 instead of NRF for industrial reliability
- Improved modularity and scalability

## Background
This project was originally developed as an individual robotics competition project.
It won the national World Robot Olympiad (WRO) final in Russia and was later presented
at the international WRO 2021 final, where the author represented Russia.
