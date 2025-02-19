#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include <sstream>
using namespace std;

///////////////////////////////////////////////////////////
// MachineKind: This enum class identifies the kind of machine.
///////////////////////////////////////////////////////////
enum class MachineKind {
    Bike,
    Car,
    Truck
};

// Helper to convert MachineKind to a readable string.
static string kindToString(MachineKind kind) {
    switch (kind) {
        case MachineKind::Bike: return "Bike";
        case MachineKind::Car: return "Car";
        case MachineKind::Truck: return "Truck";
        default: return "Unknown";
    }
}

///////////////////////////////////////////////////////////
// Machine: Represents a vehicle-like entity.
///////////////////////////////////////////////////////////
class Machine {
public:
    string identifier; // e.g., license plate
    MachineKind kind;

    // Added default constructor
    Machine() : identifier(""), kind(MachineKind::Bike) {}

    // Constructor assigns unique identifier and machine kind.
    Machine(const string& id, MachineKind mkind) : identifier(id), kind(mkind) {}

    // Determines how many slots (spots) this machine needs.
    int slotsNeeded() const {
        return (kind == MachineKind::Truck) ? 2 : 1;
    }
};

///////////////////////////////////////////////////////////
// Slot: Represents an individual parking spot.
///////////////////////////////////////////////////////////
class Slot {
public:
    int levelIndex;   // Which level/floor
    int slotIndex;    // Spot index on that level
    bool isOccupied;  // Whether a machine is present
    string occupantId; // ID of occupant machine

    Slot(int level, int index)
        : levelIndex(level), slotIndex(index), isOccupied(false) {}

    // Marks this slot as occupied by a given machine.
    bool occupySlot(const string& machineId) {
        if (isOccupied) return false;
        occupantId = machineId;
        isOccupied = true;
        return true;
    }

    // Frees up this slot.
    bool vacateSlot() {
        if (!isOccupied) return false;
        occupantId.clear();
        isOccupied = false;
        return true;
    }
};

///////////////////////////////////////////////////////////
// Level: A single floor that contains multiple slots.
///////////////////////////////////////////////////////////
class Level {
public:
    int levelIndex;           // Which level is this?
    vector<Slot> slotList;    // All slots on this level

    Level(int index, int totalSlots) : levelIndex(index) {
        for (int i = 0; i < totalSlots; ++i) {
            slotList.emplace_back(index, i);
        }
    }

    // Find suitable slot(s) for a machine.
    // If only 1 slot is needed, we return the first free slot.
    // If 2 slots are needed (e.g., truck), we look for 2 adjacent free slots.
    vector<int> spotsAvailable(const Machine& machine) {
        int needed = machine.slotsNeeded();
        vector<int> results;

        if (needed == 1) {
            for (auto& s : slotList) {
                if (!s.isOccupied) {
                    results.push_back(s.slotIndex);
                    return results;  // we only need the first
                }
            }
        } else {
            for (size_t i = 0; i + 1 < slotList.size(); ++i) {
                if (!slotList[i].isOccupied && !slotList[i+1].isOccupied) {
                    results.push_back(slotList[i].slotIndex);
                    results.push_back(slotList[i+1].slotIndex);
                    return results;
                }
            }
        }
        return {};
    }

    // Assign the machine to the given slot indices.
    bool assignMachine(const Machine& machine, const vector<int>& slotsToUse) {
        // Check that all required slots are free.
        for (int idx : slotsToUse) {
            if (slotList[idx].isOccupied) return false;
        }
        // Occupy them.
        for (int idx : slotsToUse) {
            slotList[idx].occupySlot(machine.identifier);
        }
        return true;
    }

    // Remove the machine with the given ID (identifier) from this level.
    bool removeMachine(const string& machineId) {
        bool removed = false;
        for (auto& s : slotList) {
            if (s.isOccupied && s.occupantId == machineId) {
                s.vacateSlot();
                removed = true;
            }
        }
        return removed;
    }

    // Count how many slots are currently free.
    int freeSlotsCount() const {
        int count = 0;
        for (const auto& s : slotList) {
            if (!s.isOccupied) count++;
        }
        return count;
    }
};

///////////////////////////////////////////////////////////
// Garage: Oversees all levels and operations.
///////////////////////////////////////////////////////////
class Garage {
private:
    // A listing of levels.
    vector<Level> levels;

    // Store machine location: machine ID -> (levelIndex, vector of slotIndices)
    unordered_map<string, pair<int, vector<int>>> machineLocations;

    // Also keep a record of the entire machine object, so we can retrieve type.
    unordered_map<string, Machine> machineCatalog;

    // We lock this for thread-safe operations.
    mutable mutex garageMutex;

public:
    // Construct a garage with a given number of levels and slots per level.
    Garage(int totalLevels, int slotsEach) {
        for (int i = 0; i < totalLevels; ++i) {
            levels.emplace_back(i, slotsEach);
        }
    }

    // Provide a helpful list of commands for the user.
    void showAllCommands() {
        cout << "\nHere are the commands you can use:" << endl;
        cout << "  add_machine <id> <type>        (e.g. add_machine ABC123 Car)" << endl;
        cout << "  unpark_machine <id>            (e.g. unpark_machine ABC123)" << endl;
        cout << "  check_availability" << endl;
        cout << "  check_full" << endl;
        cout << "  locate_machine <id>            (e.g. locate_machine ABC123)" << endl;
        cout << "  commands                      (Show the list of commands again)" << endl;
        cout << "  quit" << endl;
    }

    // Attempt to park (store) a machine.
    bool storeMachine(const Machine& machine) {
        lock_guard<mutex> lock(garageMutex);

        // If it's already stored, let the user know.
        if (machineLocations.count(machine.identifier)) {
            cout << "Machine with ID " << machine.identifier << " is already parked." << endl;
            return false;
        }

        // Otherwise, try to find a level with enough free slots.
        for (auto& lvl : levels) {
            vector<int> slotIndices = lvl.spotsAvailable(machine);
            if (!slotIndices.empty() && lvl.assignMachine(machine, slotIndices)) {
                // Save the location.
                machineLocations[machine.identifier] = {lvl.levelIndex, slotIndices};
                // Also store the machine object so we can retrieve its type later.
                machineCatalog[machine.identifier] = machine;

                cout << "Successfully stored machine '" << machine.identifier << "' on Level "
                     << lvl.levelIndex << " in slot(s): ";
                for (int s : slotIndices) cout << s << " ";
                cout << endl;
                return true;
            }
        }

        // If we couldn't find space.
        cout << "No suitable space found for machine ID: " << machine.identifier << "." << endl;
        return false;
    }

    // Remove an existing machine from the garage.
    bool unparkMachine(const string& machineId) {
        lock_guard<mutex> lock(garageMutex);
        // Check if it's recorded.
        if (!machineLocations.count(machineId)) {
            cout << "Machine with ID " << machineId << " not found in the garage." << endl;
            return false;
        }

        // Identify the level.
        int whichLevel = machineLocations[machineId].first;
        // Let the level remove it.
        if (levels[whichLevel].removeMachine(machineId)) {
            machineLocations.erase(machineId);
            // Remove it from our machineCatalog as well.
            machineCatalog.erase(machineId);

            cout << "Machine '" << machineId << "' has been removed from Level " << whichLevel << "." << endl;
            return true;
        }
        return false;
    }

    // Show how many free slots each level has.
    void checkAvailability() {
        lock_guard<mutex> lock(garageMutex);
        cout << "\n=== Current Availability ===" << endl;
        for (auto& lvl : levels) {
            cout << "Level " << lvl.levelIndex << ": " << lvl.freeSlotsCount() << " slot(s) free." << endl;
        }
    }

    // Verify if the entire garage is full.
    void checkIfFull() {
        lock_guard<mutex> lock(garageMutex);
        for (auto& lvl : levels) {
            if (lvl.freeSlotsCount() > 0) {
                cout << "The garage still has space available." << endl;
                return;
            }
        }
        cout << "The garage is completely full." << endl;
    }

    // Locate a machine by its ID, and display its type as well.
    void locateMachine(const string& machineId) {
        lock_guard<mutex> lock(garageMutex);
        // See if it's recorded.
        if (!machineLocations.count(machineId)) {
            cout << "Could not find machine ID " << machineId << " in the garage." << endl;
            return;
        }
        auto& entry = machineLocations[machineId];
        int lvlIndex = entry.first;
        vector<int> slots = entry.second;

        // Retrieve the machine object from our catalog.
        // This is safe because we only store machineLocations if we also store in machineCatalog.
        auto& theMachine = machineCatalog[machineId];
        string typeName = kindToString(theMachine.kind);

        cout << "Machine '" << machineId << "' (" << typeName << ") is on Level " << lvlIndex << " occupying slot(s): ";
        for (int s : slots) cout << s << " ";
        cout << endl;
    }
};

///////////////////////////////////////////////////////////
// Main function: A simple interface for our "Garage" system.
///////////////////////////////////////////////////////////
int main() {
    // Let's ask the user how many levels and how many slots per level.
    int levelCount, slotsPerLevel;
    cout << "Number of levels inyour parking lot garage: ";
    cin >> levelCount;
    cout << "Number of slots/spots on each level: ";
    cin >> slotsPerLevel;

    // Create the garage with the specified dimensions.
    Garage myGarage(levelCount, slotsPerLevel);

    cout << "\nWelcome to the Garage System!" << endl;
    // Show the user what commands are available.
    myGarage.showAllCommands();

    // We'll read commands in a loop until the user quits.
    while (true) {
        cout << "\nEnter command: ";
        string cmd;
        cin >> cmd;

        if (cmd == "add_machine") {
            // Example usage: add_machine ABC123 Car
            string id, kindStr;
            cin >> id >> kindStr;

            // We'll interpret the second argument as the machine kind.
            MachineKind mk;
            if (kindStr == "Bike")      mk = MachineKind::Bike;
            else if (kindStr == "Car")  mk = MachineKind::Car;
            else                         mk = MachineKind::Truck;

            Machine newMachine(id, mk);
            myGarage.storeMachine(newMachine);
        } else if (cmd == "unpark_machine") {
            // Example usage: unpark_machine ABC123
            string id;
            cin >> id;
            myGarage.unparkMachine(id);
        } else if (cmd == "check_availability") {
            myGarage.checkAvailability();
        } else if (cmd == "check_full") {
            myGarage.checkIfFull();
        } else if (cmd == "locate_machine") {
            // Example usage: locate_machine ABC123
            string id;
            cin >> id;
            myGarage.locateMachine(id);
        } else if (cmd == "commands") {
            // Just show the commands again.
            myGarage.showAllCommands();
        } else if (cmd == "quit") {
            // End of the loop, so end the program.
            cout << "Exiting the Garage System. Have a great day!" << endl;
            break;
        } else {
            // Unknown command.
            cout << "Sorry, I don't recognize that command. Type 'commands' for options." << endl;
        }
    }

    return 0;
}
