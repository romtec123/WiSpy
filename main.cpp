#include <iostream>
#include <filesystem>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <sstream>
#include "exec.h"

using namespace std;

bool isExistingDir(const filesystem::path& p) noexcept;
bool CreateDirectoryRecursive(const std::string & dirName);



int main() {
    cout << "WiSpy starting..." << endl;

    //Get home directory
    string homedir = getenv("HOME"); //stack overflow says this is bad but idk
    if(homedir.empty())
    {
        cout << "Error: Could not get home directory!" << endl;
        return 0;
    }
    const string dataPath = homedir + "/wispy/raw/";

    //Create data directory
    if(!isExistingDir(dataPath))
    {
        CreateDirectoryRecursive(dataPath);
        cout << "Created directories: " << dataPath << endl;
    }

    //Get location
    //This commands works for me, with my Pi. If you want to make it more portable, make a PR.
    string loc = execute("gpspipe -w -x 10 -n 10 | grep -m 1 TPV | jq -r '[.lat, .lon] | @csv'");
    if(loc.empty())
        loc = "NULL"; //yes, i know its a string.


    //Get iwlist output
    const string iwList = execute("sudo iwlist wlan1 scan | grep -E 'Address:|ESSID:|Frequency:|Quality=|Protocol:'");

    //Generate unique name
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
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

    return 0;
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



bool CreateDirectoryRecursive(const std::string & dirName)
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

