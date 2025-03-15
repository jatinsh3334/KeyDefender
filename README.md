# KeyDefender  
🚨 **DISCLAIMER:** This project is strictly for educational and ethical cybersecurity research. **Unauthorized use for malicious intent is illegal and against ethical norms.** Any misuse of keyloggers violates privacy laws. Proceed responsibly. 🚨

## Table of Contents  
- [Ethical Perspective & Deep Dive into Keylogger Development](#ethical-perspective--deep-dive-into-keylogger-development)  
- [How the Keylogger Was Built](#how-the-keylogger-was-built)  
- [How the Detection System Works](#how-the-detection-system-works)  
- [Setup & Installation Guide](#setup--installation-guide)  
  - [Cloning the Repository](#cloning-the-repository)  
  - [Setting Up the Keylogger](#setting-up-the-keylogger)  
  - [Setting the Keylogger to Run on Boot](#setting-the-keylogger-to-run-on-boot)  
  - [Setting Up the Keylogger Detection System](#setting-up-the-keylogger-detection-system)  
- [Future Improvements](#future-improvements)  
- [Screenshots & Diagrams](#screenshots--diagrams)  

---  



# Ethical Perspective & Deep Dive into Keylogger Development  

**Keyloggers pose a significant security threat by stealthily recording keystrokes, potentially compromising sensitive information.**  
This project not only demonstrates how a keylogger operates but also provides a robust detection mechanism to counter them.  
The problem of unauthorized keystroke logging is **universal**, affecting various operating systems, but the solution presented here is specifically designed for **Linux-based systems**.  
By leveraging **real-time monitoring** of `/dev/input/eventX` access and **suspicious file-writing activity**, this project offers an effective method to identify potential keyloggers running on a Linux machine.


## Ethical Usage of Keyloggers  
Keyloggers are powerful tools that can be used for both **legitimate** and **malicious** purposes. Ethical developers and security researchers leverage them to:  
✅ **Test System Security** – Simulating real-world attacks to strengthen system defenses.  
✅ **Parental Control & Monitoring** – Tracking device activity responsibly.  
✅ **Corporate IT Security Audits** – Ensuring employees adhere to security policies.  
✅ **Forensic Investigations** – Recovering lost data or analyzing security breaches.  

---  

# How the Keylogger Was Built  
1️⃣ **Capturing Keystrokes** – The keylogger directly reads keystrokes from `/dev/input/eventX`, which registers every keypress event on Linux.  
2️⃣ **Stealth Mode** – Runs in the background, without visible UI, making detection harder.  
3️⃣ **Auto-Start on Boot** – A system service (`.service` file) ensures the keylogger persists after reboot.  
4️⃣ **Hidden Storage** – Logs keystrokes in `/var/tmp/.hidden_keylog/` to avoid easy detection.  
5️⃣ **Minimal System Impact** – Uses lightweight system calls to avoid high resource usage.  

---  

# How the Detection System Works  
This detection system is divided into **two C++ files** for a better user experience:  
- 🛡️ **`key_detector_main.cpp`** – Detects both log-writing and `/dev/input` access in a single process.  
- 🔍 **`secondary_detector.cpp`** – Separates the detection into two steps:  
  - Processes accessing `/dev/input/eventX`  
  - Processes writing logs (without accessing input events)  
  
## Detection Mechanisms  
1️⃣ **Real-Time File Write Tracking** – Monitors processes writing to suspicious locations (`/var/log`, `/tmp`, `/var/tmp`).  
2️⃣ **Scanning for /dev/input Access** – Flags processes reading keyboard events.  
3️⃣ **Dynamic Scanning & Process Matching** – Ensures only real keyloggers are detected.  
4️⃣ **Background Monitoring** – Runs continuously, requiring no manual intervention.  
5️⃣ **Blacklist Avoidance** – Excludes legitimate system processes to prevent false positives.  

---  

# Setup & Installation Guide  
## Cloning the Repository  
```sh  
git clone https://github.com/jatinsh3334/KeyDefender 
cd KeyDefender
```  

## Setting Up the Keylogger  
### Compiling & Running the Keylogger  
```sh  
g++ keylogger_code.cpp -o keylogger_code  
sudo ./keylogger_code  
```  

### Check if logs are recorded correctly  
```sh  
cat /var/tmp/.hidden_keylog/keylog.txt  
```  

## Setting the Keylogger to Run on Boot  
### Copy the Service File to systemd Directory  
```sh  
sudo cp keylogger_code.service /etc/systemd/system/  
```  

### Enable and Start the Service  
```sh  
sudo systemctl enable keylogger_code.service  
sudo systemctl start keylogger_code.service  
```  

### To Disable the Service (if needed)  
```sh  
sudo systemctl disable keylogger_code.service  
```  

### Check the Status of Service  
```sh  
sudo systemctl status keylogger_code.service  
```  

## Setting Up the Keylogger Detection System  
### Compile & Run the Keylogger Detector  
```sh  
g++ key_detector_main.cpp -o key_detector_main  
./key_detector_main  
```  

### Running the Secondary Detector (Optional)  
```sh  
g++ secondary_detector.cpp -o secondary_detector  
./secondary_detector  
```  

### Interpret the Output  
- If a process accessing `/dev/input/eventX` **AND** writing logs is found → **Likely a keylogger.**  
- If a process **only** accesses `/dev/input` → Could be a legitimate system process or spyware.  
- If a process **only** writes logs → Could be normal logging activity or a custom logging tool.  

---  

# Future Improvements 🚀  
📌 **Network-based Keylogger Detection** – Track data exfiltration over TCP/UDP.  
📌 **AI-Based Anomaly Detection** – Use ML to distinguish between legit and malicious loggers.  
📌 **Real-time GUI Dashboard** – Create a UI for detection results.  
📌 **Cross-Platform Support** – Extend support to macOS and Windows.  

---  


# Licensing & Legal Disclaimer  
📜 **MIT License** – This project is open-source and can be modified for ethical purposes.  
⚠️ **Warning:** This tool must **never** be used for unauthorized access or malicious activities. The author is **not responsible** for any misuse.  
