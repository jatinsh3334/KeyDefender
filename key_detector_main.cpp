#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <map>
#include <chrono>
#include <thread>
#include <set>
#include <iomanip>

using namespace std;

// Tracks file writes over time to detect continuous logging
map<string, int> fileWriteCount;

// Stores processes accessing /dev/input/eventX
set<string> devInputProcesses;

// System processes to ignore
set<string> ignoredProcesses = {"systemd", "bash", "sshd", "cron", "init", "kthreadd", "klogd", "journald"};

// Hot zones where keyloggers often store logs
vector<string> hotZones = {"/var/log", "/var/tmp", "/tmp", "/dev/shm"};

// Function to check if a process should be ignored
bool isIgnoredProcess(const string &processName) {
    return ignoredProcesses.find(processName) != ignoredProcesses.end();
}

// Function to check if a file is in a hot zone
bool isInHotZone(const string &filePath) {
    for (const auto &zone : hotZones) {
        if (filePath.find(zone) == 0) {
            return true;
        }
    }
    return false;
}

// Function to detect processes accessing /dev/input/eventX
void detectDevInputAccess() {
    DIR *procDir = opendir("/proc");
    if (!procDir) {
        perror("Failed to open /proc");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(procDir)) != NULL) {
        if (!isdigit(entry->d_name[0])) continue;
        string pid = entry->d_name;
        string fdPath = "/proc/" + pid + "/fd";
        string commPath = "/proc/" + pid + "/comm";

        ifstream commFile(commPath);
        string processName;
        if (commFile.is_open()) {
            getline(commFile, processName);
            commFile.close();
        }

        if (isIgnoredProcess(processName)) continue;

        DIR *fdDir = opendir(fdPath.c_str());
        if (!fdDir) continue;

        struct dirent *fdEntry;
        while ((fdEntry = readdir(fdDir)) != NULL) {
            if (fdEntry->d_name[0] == '.') continue;

            string linkPath = fdPath + "/" + fdEntry->d_name;
            char resolvedPath[PATH_MAX];
            ssize_t len = readlink(linkPath.c_str(), resolvedPath, sizeof(resolvedPath) - 1);
            if (len != -1) {
                resolvedPath[len] = '\0';
                string filePath = resolvedPath;

                if (filePath.find("/dev/input/event") != string::npos) {
                    devInputProcesses.insert(pid);
                }
            }
        }
        closedir(fdDir);
    }
    closedir(procDir);
}

// Function to monitor file writes
void monitorFileWrites() {
    int detectionCount = 0;
    while (true) {
        detectDevInputAccess();  // Update /dev/input/eventX access list

        DIR *procDir = opendir("/proc");
        if (!procDir) {
            perror("Failed to open /proc");
            return;
        }

        struct dirent *entry;
        cout << "\033[1;34m\n============= Real-Time Suspicious Activity Detection =============\033[0m" << endl;
        cout << left << setw(5) << "No." << setw(20) << "Process Name" << setw(10) << "PID" << " File Path" << endl;
        cout << "------------------------------------------------------------" << endl;

        while ((entry = readdir(procDir)) != NULL) {
            if (!isdigit(entry->d_name[0])) continue;

            string pid = entry->d_name;
            string fdPath = "/proc/" + pid + "/fd";
            string commPath = "/proc/" + pid + "/comm";

            ifstream commFile(commPath);
            string processName;
            if (commFile.is_open()) {
                getline(commFile, processName);
                commFile.close();
            }

            if (isIgnoredProcess(processName)) continue;
            DIR *fdDir = opendir(fdPath.c_str());
            if (!fdDir) continue;

            struct dirent *fdEntry;
            while ((fdEntry = readdir(fdDir)) != NULL) {
                if (fdEntry->d_name[0] == '.') continue;

                string linkPath = fdPath + "/" + fdEntry->d_name;
                char resolvedPath[PATH_MAX];
                ssize_t len = readlink(linkPath.c_str(), resolvedPath, sizeof(resolvedPath) - 1);
                if (len != -1) {
                    resolvedPath[len] = '\0';
                    string filePath = resolvedPath;

                    fileWriteCount[filePath]++;

                    if (fileWriteCount[filePath] > 3 && isInHotZone(filePath) && devInputProcesses.find(pid) != devInputProcesses.end()) {
                        detectionCount++;
                        cout  << left << setw(5) << detectionCount 
                             << setw(20) << processName 
                             << setw(10) << pid 
                             << filePath << endl;
                    }
                }
            }
            closedir(fdDir);
        }
        closedir(procDir);
        this_thread::sleep_for(chrono::seconds(2));
    }
}

int main() {
    cout << "\033[1;32mStarting Real-Time Keylogger Detection...\033[0m" << endl;
    monitorFileWrites();
    return 0;
}
