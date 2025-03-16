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

map<string, int> fileWriteCount;

set<string> ignoredProcesses = {"systemd", "bash", "sshd", "cron", "init", "kthreadd", "klogd", "journald"};

vector<string> hotZones = {"/var/log", "/var/tmp", "/tmp", "/dev/shm"};


bool isIgnoredProcess(const string &processName) {
    return ignoredProcesses.find(processName) != ignoredProcesses.end();
}


bool isInHotZone(const string &filePath) {
    for (const auto &zone : hotZones) {
        if (filePath.find(zone) == 0) {
            return true;
        }
    }
    return false;
}


void detectKeystrokeInterception() {
    DIR *procDir = opendir("/proc");
    if (!procDir) {
        perror("Failed to open /proc");
        return;
    }

    struct dirent *entry;
    cout << "\033[1;31m[Keystroke Capture Detection]\033[0m" << endl;
    cout << left << setw(20) << "Process Name" << setw(10) << "PID" << " File Descriptor" << endl;
    cout << "---------------------------------------------------" << endl;

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
                if (filePath.find("/dev/input/event") == 0) {
                    cout << "\033[1;31m" << left << setw(20) << processName << setw(10) << pid << filePath << "\033[0m" << endl;
                }
            }
        }
        closedir(fdDir);
    }
    closedir(procDir);
}


void detectSuspiciousLogWriters() {
    DIR *procDir = opendir("/proc");
    if (!procDir) {
        perror("Failed to open /proc");
        return;
    }

    struct dirent *entry;
    cout << "\n\033[1;34m[Log-Writing Process Detection]\033[0m" << endl;
    cout << left << setw(20) << "Process Name" << setw(10) << "PID" << " File Path" << endl;
    cout << "---------------------------------------------------" << endl;

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
                if (fileWriteCount[filePath] > 3) {
                    if (isInHotZone(filePath)) {
                        cout << "\033[1;31m"; 
                    }
                    cout << left << setw(20) << processName << setw(10) << pid << filePath << "\033[0m" << endl;
                }
            }
        }
        closedir(fdDir);
    }
    closedir(procDir);
}

int main() {
    cout << "\033[1;32mStarting Advanced Keylogger Detection...\033[0m\n" << endl;
    while (true) {
        detectKeystrokeInterception();
        detectSuspiciousLogWriters();
        this_thread::sleep_for(chrono::seconds(10));
    }
    return 0;
}
