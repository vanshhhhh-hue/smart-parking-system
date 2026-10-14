// Smart Parking System - OOP project in C++
// Compile: g++ -std=c++14 smart_parking.cpp -o parking
// Run:     ./parking   (Windows: parking.exe)

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <cctype>
using namespace std;

// ---------------------------------------------------------------
// Abstract base class (ABSTRACTION)
// ---------------------------------------------------------------
class Vehicle {
protected:                      // ENCAPSULATION: data hidden from outside
    string number;
    int slot;
    time_t entryTime;

public:
    Vehicle(string n, int s, time_t t)
        : number(n), slot(s), entryTime(t) {}

    virtual ~Vehicle() {}

    // Pure virtual functions: every vehicle MUST define these
    virtual string getType() const = 0;
    virtual double getRate() const = 0;

    string getNumber() const { return number; }
    int getSlot() const { return slot; }
    time_t getEntryTime() const { return entryTime; }

    // Same code works for all vehicle types (POLYMORPHISM via getRate)
    int billableHours(time_t exitTime) const {
        double seconds = difftime(exitTime, entryTime);
        return max(1, static_cast<int>(ceil(seconds / 3600.0)));
    }

    double calculateFee(time_t exitTime) const {
        return billableHours(exitTime) * getRate();
    }
};

// ---------------------------------------------------------------
// Derived classes (INHERITANCE)
// ---------------------------------------------------------------
class Car : public Vehicle {
public:
    Car(string n, int s, time_t t) : Vehicle(n, s, t) {}
    string getType() const override { return "Car"; }
    double getRate() const override { return 50.0; }
};

class Bike : public Vehicle {
public:
    Bike(string n, int s, time_t t) : Vehicle(n, s, t) {}
    string getType() const override { return "Bike"; }
    double getRate() const override { return 20.0; }
};

class Truck : public Vehicle {
public:
    Truck(string n, int s, time_t t) : Vehicle(n, s, t) {}
    string getType() const override { return "Truck"; }
    double getRate() const override { return 100.0; }
};

// ---------------------------------------------------------------
// Helper functions
// ---------------------------------------------------------------
string toUpper(string s) {
    for (char &c : s)
        c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    return s;
}

// Factory: builds the correct object from a type name
unique_ptr<Vehicle> createVehicle(const string &type, const string &number,
                                  int slot, time_t t) {
    string ty = toUpper(type);
    if (ty == "CAR")   return unique_ptr<Vehicle>(new Car(number, slot, t));
    if (ty == "BIKE")  return unique_ptr<Vehicle>(new Bike(number, slot, t));
    if (ty == "TRUCK") return unique_ptr<Vehicle>(new Truck(number, slot, t));
    return nullptr;  // invalid type
}

// Safe integer input (does not crash on letters)
int readInt() {
    int value;
    while (!(cin >> value)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Enter a number: ";
    }
    return value;
}

// ---------------------------------------------------------------
// Parking management class
// ---------------------------------------------------------------
class ParkingLot {
private:
    static const int TOTAL_SLOTS = 10;
    static const char *const FILE_NAME;

    vector<unique_ptr<Vehicle>> vehicles;

    void saveData() const {
        ofstream file(FILE_NAME);
        for (const auto &v : vehicles) {
            file << v->getNumber() << " " << v->getType() << " "
                 << v->getSlot() << " "
                 << static_cast<long long>(v->getEntryTime()) << "\n";
        }
    }

    void loadData() {
        ifstream file(FILE_NAME);
        string number, type;
        int slot;
        long long timestamp;

        while (file >> number >> type >> slot >> timestamp) {
            if (slot < 1 || slot > TOTAL_SLOTS) continue;   // skip bad data
            auto v = createVehicle(type, number, slot,
                                   static_cast<time_t>(timestamp));
            if (v) vehicles.push_back(move(v));
        }
    }

    int findFreeSlot() const {
        for (int slot = 1; slot <= TOTAL_SLOTS; slot++) {
            bool occupied = any_of(vehicles.begin(), vehicles.end(),
                [slot](const unique_ptr<Vehicle> &v) {
                    return v->getSlot() == slot;
                });
            if (!occupied) return slot;
        }
        return -1;
    }

    vector<unique_ptr<Vehicle>>::iterator findByNumber(const string &number) {
        return find_if(vehicles.begin(), vehicles.end(),
            [&number](const unique_ptr<Vehicle> &v) {
                return v->getNumber() == number;
            });
    }

public:
    ParkingLot() { loadData(); }   // constructor loads saved data

    void parkVehicle() {
        if (static_cast<int>(vehicles.size()) >= TOTAL_SLOTS) {
            cout << "Parking is full!\n";
            return;
        }

        string number, type;
        cout << "Enter vehicle number: ";
        cin >> number;
        number = toUpper(number);

        if (findByNumber(number) != vehicles.end()) {
            cout << "This vehicle is already parked!\n";
            return;
        }

        cout << "Enter type (Car/Bike/Truck): ";
        cin >> type;

        int slot = findFreeSlot();
        auto v = createVehicle(type, number, slot, time(nullptr));

        if (!v) {
            cout << "Invalid vehicle type!\n";
            return;
        }

        vehicles.push_back(move(v));
        saveData();

        cout << "Vehicle parked successfully!\n";
        cout << "Allocated slot: " << slot << "\n";
    }

    void showStatus() const {
        int occupied = static_cast<int>(vehicles.size());

        cout << "\nTotal slots: " << TOTAL_SLOTS << "\n";
        cout << "Occupied slots: " << occupied << "\n";
        cout << "Available slots: " << TOTAL_SLOTS - occupied << "\n";

        cout << "\n--- Parked Vehicles ---\n";
        if (vehicles.empty())
            cout << "No vehicles parked.\n";

        for (const auto &v : vehicles) {
            cout << "Slot: " << v->getSlot()
                 << " | Number: " << v->getNumber()
                 << " | Type: " << v->getType()
                 << " | Rate/hour: Rs. " << v->getRate() << "\n";
        }
    }

    void exitVehicle() {
        string number;
        cout << "Enter vehicle number to exit: ";
        cin >> number;
        number = toUpper(number);

        auto it = findByNumber(number);
        if (it == vehicles.end()) {
            cout << "Vehicle not found!\n";
            return;
        }

        time_t now = time(nullptr);
        const Vehicle &v = **it;

        cout << fixed << setprecision(2);
        cout << "\n----- PARKING RECEIPT -----\n";
        cout << "Vehicle number : " << v.getNumber() << "\n";
        cout << "Vehicle type   : " << v.getType() << "\n";
        cout << "Slot           : " << v.getSlot() << "\n";
        cout << "Duration       : " << v.billableHours(now) << " hour(s)\n";
        cout << "Rate per hour  : Rs. " << v.getRate() << "\n";
        cout << "Total fee      : Rs. " << v.calculateFee(now) << "\n";
        cout << "---------------------------\n";

        vehicles.erase(it);   // unique_ptr frees the memory automatically
        saveData();
        cout << "Vehicle exited successfully!\n";
    }

    void searchVehicle() {
        string number;
        cout << "Enter vehicle number to search: ";
        cin >> number;
        number = toUpper(number);

        auto it = findByNumber(number);
        if (it == vehicles.end()) {
            cout << "Vehicle not found!\n";
            return;
        }

        cout << "Vehicle found!\n";
        cout << "Type: " << (*it)->getType() << "\n";
        cout << "Slot: " << (*it)->getSlot() << "\n";
    }

    void showMenu() const {
        cout << "\n===== SMART PARKING SYSTEM =====\n";
        cout << "1. Park Vehicle\n";
        cout << "2. View Parking Status\n";
        cout << "3. Exit Vehicle and Calculate Fee\n";
        cout << "4. Search Vehicle\n";
        cout << "5. Close Program\n";
        cout << "Enter choice: ";
    }
};

const char *const ParkingLot::FILE_NAME = "parking_data.txt";

// ---------------------------------------------------------------
int main() {
    ParkingLot parking;
    int choice;

    do {
        parking.showMenu();
        choice = readInt();

        switch (choice) {
            case 1: parking.parkVehicle();   break;
            case 2: parking.showStatus();    break;
            case 3: parking.exitVehicle();   break;
            case 4: parking.searchVehicle(); break;
            case 5: cout << "Thank you for using Smart Parking!\n"; break;
            default: cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 5);

    return 0;
}