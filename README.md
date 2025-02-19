/*
----------------------------------------------------------------------------------------
# 🚗 Smart Parking Lot Management System
----------------------------------------------------------------------------------------

A sophisticated multi-level parking garage system built in C++ that brings 
intelligence to vehicle parking management.

## 🌟 System Architecture

┌─────────────────────────────────────────┐
│              GARAGE SYSTEM            │
├─────────────────────────────────────────┤
│                                         │
│    Level 3   [□][□][□][□][□][□][□]     │
│                                         │
│    Level 2   [■][■][□][□][□][■][□]     │
│                                         │
│    Level 1   [■][■][□][■][□][□][□]     │
│                                         │
└─────────────────────────────────────────┘
   □ = Empty Slot    ■ = Occupied Slot

## 🚀 Core Features

### Vehicle Types
- 🏍️ Bike  — Requires 1 slot
- 🚗 Car   — Requires 1 slot
- 🚛 Truck — Requires 2 adjacent slots

### Smart Space Management

Level 1: [🚛][ ][🚗][ ]
         └──┘
Truck occupies 2 adjacent slots

## 💻 Command Interface

### Parking Operations
add_machine ABC123 Car     # Parks a car with ID ABC123
unpark_machine ABC123      # Removes the vehicle
locate_machine ABC123      # Finds vehicle location

### System Monitoring
check_availability
   Shows this kind of output:
  - Level 0: 5 slots free
  - Level 1: 3 slots free
  - Level 2: 7 slots free

check_full
  - Tells you if the garage is completely full

### Other Commands
- commands — Display all available commands
- quit — Exit the system

## 🔧 Technical Implementation

Class Hierarchy:
Garage
  └── Level
       └── Slot
            └── Machine

Data Structures Used:
- vector<Level>: Manages multiple parking levels
- unordered_map: Tracks vehicle locations
- mutex: Ensures thread-safe operations

## 🛠️ Building the Project

Prerequisites:
- C++11 compatible compiler
- Make or CMake build system

Compilation Steps:
g++ -std=c++11 main.cpp -o parking_system -pthread
./parking_system

## 🎯 Use Cases

### Mall Parking
- Level 3: Premium Parking  [VIP Section]
- Level 2: Regular Parking  [General Public]
- Level 1: Extended Stay    [Long-term parking]

### Airport Parking
- Level 2: Short-term Parking
- Level 1: Long-term Parking

## 🔍 Features Deep Dive

Space Optimization:
- Smart allocation for trucks needing adjacent spots
- Real-time tracking of available spaces
- Efficient space distribution algorithms

Thread Safety:
std::lock_guard<std::mutex> lock(garageMutex);
  // Automatic locking/unlocking in critical sections

Error Handling:
- Duplicate parking prevention
- Invalid vehicle type detection
- Full garage handling
- Invalid spot allocation prevention

## 📊 Performance
- O(1) vehicle lookup
- O(n) spot allocation (where n is spots per level)
- Thread-safe operations with minimal lock contention

## 🚦 Status Indicators
- ✅ Available Spot
- ❌ Occupied Spot
- 🚧 Reserved Spot

## 🔄 Workflow Example

1. User Input     →  2. Validation    →  3. Spot Assignment
      ↓                   ↓                    ↓
    ABC123            Check Space         Level 2, Spot 5
      ↓                   ↓                    ↓
    Vehicle           Verify Type          Update Maps

## 🎨 Future Enhancements
- Payment Integration
- Mobile App Interface
- License Plate Recognition
- Smart Space Prediction
- EV Charging Spots
*/
