#include <iostream>
#include <filesystem>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <functional>
#include "exec.h"

using namespace std;

struct Coordinates {
    double latitude;
    double longitude;
};
bool isExistingDir(const filesystem::path& p) noexcept;
bool CreateDirectoryRecursive(const string & dirName);
void timer_start(function<void(void)> func, unsigned int interval);
double calculateDistance(double lat1, double lon1, double lat2, double lon2);
Coordinates parseCoordinates(const string& inputString);
void scan();
string dataPath;
string lastPos = "";


int main() {
    cout << "WiSpy starting..." << endl;

    //Get home directory
    string homedir = getenv("HOME"); //stack overflow says this is bad but idk
    if (homedir.empty()) {
        cout << "Error: Could not get home directory!" << endl;
        return 0;
    }
    dataPath = homedir + "/wifidata/raw/";

    //Create data directory
    if (!isExistingDir(dataPath)) {
        CreateDirectoryRecursive(dataPath);
        cout << "Created directories: " << dataPath << endl;
    }

    //Start timer
    timer_start(scan, 20000);

    while(true);


}

void scan() {

    cout << "Starting scan..." << endl;

    //Get location
    //These commands work for me, with my Pi. If you want to make it more portable, make a PR.
    string loc = execute("gpspipe -w -x 10 -n 10 | grep -m 1 TPV | jq -r '[.lat, .lon] | @csv'");
    if(loc.empty() || loc.size() < 5)
    {
        cout << "Location was not found. Skipping..." << endl;
        return;
    }

    if(!lastPos.empty())
    {
        Coordinates last = parseCoordinates(lastPos);
        Coordinates current = parseCoordinates(loc);

        double distance = calculateDistance(last.latitude, last.longitude, current.latitude, current.longitude);

        cout << "Scan Distance: " << distance << endl;
        if(distance < 50)
        {
            cout << "To close to last scan pos! Skipping..." << endl;
            return;
        }
    }
    lastPos = loc;


    //Get iwlist output
    const string iwList = execute("sudo iwlist wlan1 scan | grep -E 'Address:|ESSID:|Frequency:|Quality=|Protocol:'");

    if(iwList.empty() || iwList.size() < 16)
    {
        cout << "Couldn't find any networks. Skipping..." << endl;
        return;
    }

    //Generate unique name
    auto t = time(nullptr);
    auto tm = *localtime(&t);
    ostringstream oss;
    oss << put_time(&tm, "%d-%m-%Y_%H-%M-%S");
    auto time = oss.str();

    const string filename = "scan_" + time + ".txt";

    //Generate output & save it.
    const string fullPath = dataPath+filename;

    ofstream output(fullPath.c_str());

    output << "loc=" << loc << "\n" << iwList << endl;
    output.close();

    cout << "Scan saved to: " << fullPath << endl;

    return;
    //ngl this function prolly leaks memory
}


bool isExistingDir(const filesystem::path& p) noexcept
{
    try
    {
        return filesystem::is_directory(p);
    }
    catch (exception& e)
    {
        // Output the error message.
        const auto theError = string{ e.what() };
        cerr << theError;

        return false;
    }
}



bool CreateDirectoryRecursive(const string & dirName)
{
    error_code err;
    if (!filesystem::create_directories(dirName, err))
    {
        if (filesystem::exists(dirName))
        {
            return true;    // the folder probably already existed
        }

        printf("CreateDirectoryRecursive: FAILED to create [%s], err:%s\n", dirName.c_str(), err.message().c_str());
        return false;
    }
    return true;
}


void timer_start(function<void(void)> func, unsigned int interval)
{
    thread([func, interval]() {
        while (true)
        {
            func();
            this_thread::sleep_for(chrono::milliseconds(interval));
        }
    }).detach();
}


Coordinates parseCoordinates(const string& inputString) {
    Coordinates coords;

    stringstream ss(inputString);
    ss >> coords.latitude;
    ss.ignore();  // Ignore the comma separator
    ss >> coords.longitude;

    return coords;
}


double calculateDistance(double lat1, double lon1, double lat2, double lon2)
{
    double earthRadius = 20902231.98757;  // Earth's radius in feet
    double dLat = (lat2 - lat1) * M_PI / 180.0;  // Latitude difference in radians
    double dLon = (lon2 - lon1) * M_PI / 180.0;  // Longitude difference in radians

    // Haversine formula to calculate distance
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
               sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    double distance = earthRadius * c;

    return distance;
}



