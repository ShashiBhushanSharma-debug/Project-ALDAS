<div align="center">

<img src="./assets/aldas.png" alt="ALDAS Logo" width="180"/>

# ALDAS — Autonomous Launching and Docking Assistance System

**Autonomous drone launch, docking, and charging via a mobile ground rover**

[![ROS2](https://img.shields.io/badge/ROS2-Humble-blue)](https://docs.ros.org/en/humble/)
[![PX4](https://img.shields.io/badge/PX4-Autopilot-1a1a1a)](https://px4.io/)
[![Gazebo](https://img.shields.io/badge/Gazebo-Harmonic-orange)](https://gazebosim.org/)
[![Status](https://img.shields.io/badge/status-in--progress-yellow)]()

</div>

---

## Overview

ALDAS enables a drone to autonomously launch from and dock onto a moving ground rover — which stores and charges it — allowing continuous, human-free operation cycles without fixed infrastructure.

**Highlights**
- Autonomous docking/launch coordination between a UAV and a mobile ground rover
- Built on ROS2, with PX4 integration and Gazebo simulation
- Custom control and path planning for precision approach/docking
- Computer vision–based tracking for real-time alignment during docking
- In-progress development with core modules functional

---

# PX4 + Gazebo Harmonic + ROS 2 Humble Development Environment Setup Guide

> **Author's Note**
>
> This document records the exact steps followed while setting up a PX4
> Software-In-The-Loop (SITL) development environment on Ubuntu 22.04.
> The intention is to create a reproducible setup that can later be
> converted into a Docker-based environment for collaborative
> development.

------------------------------------------------------------------------

# 1. Objective

The objective of this setup is to create a native PX4 simulation
environment consisting of:

-   Ubuntu 22.04.5 LTS
-   ROS 2 Humble
-   Gazebo Harmonic
-   PX4 Autopilot
-   QGroundControl (to be built from source)
-   Docker (planned after native setup)

At the end of this phase, the X500 drone should successfully spawn
inside Gazebo Harmonic.

------------------------------------------------------------------------

# 2. Software Architecture

                    Gazebo Harmonic
                (Physics + Sensors)
                         ▲
                         │
                  Simulated Sensors
                         │
                         ▼
                    PX4 SITL
              (Flight Controller)
                         │
              MAVLink / uXRCE-DDS
                │                │
                ▼                ▼
     QGroundControl          ROS 2
          (Later)            (Later)

------------------------------------------------------------------------

# 3. System Requirements

## Operating System

Ubuntu 22.04.5 LTS

## ROS

ROS 2 Humble

## Compiler

-   GCC 11+      
-   CMake 3.22+
-   Python 3.10+
-   Ninja
-   CCache

------------------------------------------------------------------------

# 4. Installation Procedure

## Step 1 --- Verify Existing Environment

### Purpose

Before installing new software, verify the current system.

### Commands

``` bash
lsb_release -a
uname -r
echo $XDG_SESSION_TYPE
printenv | grep ROS
gcc --version
cmake --version
python3 --version
make --version
```

### Expected Result

-   Ubuntu 22.04
-   X11 session
-   ROS 2 Humble
-   GCC available
-   Python available
-   CMake available

Reference:
https://docs.px4.io/main/en/dev_setup/dev_env_linux_ubuntu.html

------------------------------------------------------------------------

## Step 2 --- Remove Gazebo Classic

### Why?

Gazebo Classic is no longer the recommended simulator for current PX4
development. To avoid plugin conflicts and confusion, it was removed
completely before installing Gazebo Harmonic.



### Check for gazebo

```bash
which gz
gz sim --version
```
*if any of the above gives an output then follow the below commands.

### Commands

``` bash
sudo apt remove --purge \
gazebo \
gazebo-common \
gazebo-plugin-base \
libgazebo11 \
libgazebo-dev \
ros-humble-gazebo-dev \
ros-humble-gazebo-msgs \
ros-humble-gazebo-plugins \
ros-humble-gazebo-ros \
ros-humble-gazebo-ros-pkgs \
ros-humble-turtlebot3-gazebo

sudo apt autoremove
```

### Verification

``` bash
which gazebo
which gz
dpkg -l | grep gazebo
```

Expected:

No Gazebo Classic packages should remain.

Reference: https://gazebosim.org/docs

------------------------------------------------------------------------

## Step 3 --- Install Gazebo Harmonic

### Purpose

Install the simulator officially supported by modern PX4.

### Commands

``` bash
sudo apt update
sudo apt install -y curl lsb-release gnupg

sudo curl https://packages.osrfoundation.org/gazebo.gpg \
-o /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg

echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] http://packages.osrfoundation.org/gazebo/ubuntu-stable $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/gazebo-stable.list > /dev/null

sudo apt update
sudo apt install -y gz-harmonic
```

### Verification

``` bash
gz sim
gz sim shapes.sdf
gz sim --version
```

Expected:

-   Gazebo opens successfully.
-   Shapes world loads.
-   Camera movement works.
-   Version reports Gazebo Sim 8.x.

Reference: https://gazebosim.org/docs/harmonic/install

------------------------------------------------------------------------

## Step 4 --- Create Workspace

``` bash
mkdir -p ~/robotics
cd ~/robotics
```

Purpose:

Store all robotics projects under a single directory.

------------------------------------------------------------------------

## Step 5 --- Clone PX4

``` bash
git clone --recursive https://github.com/PX4/PX4-Autopilot.git

cd PX4-Autopilot

git branch --show-current

git submodule status
```

Expected:

-   Branch should be `main`
-   All submodules initialized

Reference: https://docs.px4.io/main/en/dev_setup/building_px4.html

------------------------------------------------------------------------

## Step 6 --- Install PX4 Dependencies

### Command

``` bash
bash Tools/setup/ubuntu.sh
```

### Purpose

Installs:

-   Python dependencies
-   Ninja
-   CCache
-   GeographicLib
-   Build dependencies

Reference:
https://docs.px4.io/main/en/dev_setup/dev_env_linux_ubuntu.html

------------------------------------------------------------------------

## Step 7 --- Verify Build Environment

``` bash
python3 -m pip --version
python3 -m pip list | grep empy
ninja --version
ccache --version
```

Expected:

All commands should execute successfully.

------------------------------------------------------------------------

## Step 8 --- Build PX4

``` bash
make px4_sitl
```

Purpose:

Compile the PX4 flight controller.

Expected:

Compilation completes without errors.

------------------------------------------------------------------------

## Step 9 --- Launch Simulation

``` bash
make px4_sitl gz_x500
```

Expected:

-   Gazebo launches.
-   X500 appears.
-   PX4 terminal starts.
-   Physics simulation begins.

Normal warnings include:

    Preflight Fail: No connection to the GCS
    LED open failed

These are expected before QGroundControl is connected.

------------------------------------------------------------------------

# 5. Verification Checklist

-   Ubuntu verified
-   ROS 2 verified
-   Gazebo Classic removed
-   Gazebo Harmonic installed
-   Gazebo verified
-   PX4 cloned
-   PX4 dependencies installed
-   PX4 built successfully
-   X500 simulation launched

------------------------------------------------------------------------

# 6. Problems Encountered During Setup

## 1. Gazebo Classic already installed

Issue:

ROS 2 Humble installation had already installed Gazebo Classic packages.

Solution:

Removed Gazebo Classic completely before installing Gazebo Harmonic.

------------------------------------------------------------------------

## 2. Gazebo version confusion

Issue:

`gz` command existed before Harmonic installation.

Cause:

Different Gazebo generations use different command-line tools.

Solution:

Verified installed packages using:

``` bash
dpkg -l | grep gazebo
dpkg -l | grep ignition
```

------------------------------------------------------------------------

## 3. QGroundControl download URL

Issue:

Older CloudFront download URL returned HTTP 403.

Solution:

Use the official QGroundControl documentation or GitHub releases
instead.

Reference:

https://docs.qgroundcontrol.com/

------------------------------------------------------------------------

## 4. QGroundControl AppImage failed

Observed error:

-   GLIBC_2.36 not found
-   GLIBC_2.38 not found
-   GLIBCXX_3.4.32 not found

Reason:

Latest AppImage targets newer Linux distributions than Ubuntu 22.04.

Planned solution:

Build QGroundControl from source so it links against Ubuntu 22.04 system
libraries.

------------------------------------------------------------------------

## 5. Docker

Why not start directly with Docker?

Decision:

Build everything natively first.

Reason:

Understanding the installation process makes debugging easier and
provides a validated environment before containerization.

Docker will be introduced only after the native setup is fully
functional.

------------------------------------------------------------------------

# 7. Future Work

-   Build QGroundControl from source
-   Connect QGroundControl to PX4
-   Understand MAVLink communication
-   Connect ROS 2 through Micro XRCE-DDS
-   Develop Offboard Control
-   Containerize the environment using Docker
-   Integrate Wokwi ESP32 simulations
-   Deploy on real PX4-compatible hardware

------------------------------------------------------------------------

# 8. References

PX4 Documentation

https://docs.px4.io/

Gazebo Documentation

https://gazebosim.org/docs

ROS 2 Documentation

https://docs.ros.org/en/humble/

QGroundControl Documentation

https://docs.qgroundcontrol.com/

QGroundControl GitHub

https://github.com/mavlink/qgroundcontrol

PX4 GitHub

https://github.com/PX4/PX4-Autopilot
