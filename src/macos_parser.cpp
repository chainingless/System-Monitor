#include "macos_parser.h"

#include <libproc.h>
#include <mach/mach_host.h>
#include <pwd.h>
#include <sys/sysctl.h>
#include <time.h>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

// MARK: System

string MacosParser::OperatingSystem() {
  if (operatingSystem_.empty()) {
    string fileName = "operating_system_info.txt";
    system(("sw_vers > " + fileName).c_str());
    ifstream filestream(fileName);

    string line;
    string key;
    string value;
    if (filestream.is_open()) {
      while (getline(filestream, line)) {
        replace(line.begin(), line.end(), ' ', '_');
        replace(line.begin(), line.end(), ':', ' ');
        istringstream linestream(line);
        while (linestream >> key >> value) {
          replace(value.begin(), value.end(), '_', ' ');
          value.erase(value.begin(),
                      find_if(value.begin(), value.end(),
                              [](int ch) { return !isspace(ch); }));
          if (key == "ProductName") {
            operatingSystem_ += value;
          } else if (key == "ProductVersion") {
            operatingSystem_ += " " + value;
          } else if (key == "BuildVersion") {
            operatingSystem_ += " (" + value + ")";
          }
        }
        linestream.clear();
      }
      filestream.close();
    }
  }

  return operatingSystem_;
}

string MacosParser::Kernel() {
  if (kernel_.empty()) {
    string fileName = "kernal_info.txt";
    system(("uname -sr > " + fileName).c_str());
    ifstream filestream(fileName);
    if (filestream.is_open()) {
      string line;
      if (getline(filestream, line)) {
        kernel_ = line;
      }
      filestream.close();
    }
  }
  return kernel_;
}

float MacosParser::MemoryUtilization() {
  int mib[2];
  mib[0] = CTL_HW;
  mib[1] = HW_MEMSIZE;
  uint64_t totalMemory;
  size_t sizeOfTotalMemoryBuffer = sizeof(totalMemory);
  float rate{0.0};

  if (sysctl(mib, 2, &totalMemory, &sizeOfTotalMemoryBuffer, NULL, 0) != -1) {
    vm_size_t pageSize;
    mach_port_t machPort = mach_host_self();
    vm_statistics vmStats;
    mach_msg_type_number_t count = sizeof(vmStats) / sizeof(natural_t);

    if (KERN_SUCCESS == host_page_size(machPort, &pageSize) &&
        KERN_SUCCESS == host_statistics(machPort, HOST_VM_INFO,
                                        (host_info_t)&vmStats, &count)) {
      uint64_t usedMemory =
          (vmStats.active_count + vmStats.inactive_count + vmStats.wire_count) *
          pageSize;
      rate = (float)usedMemory / totalMemory;
    }
  }
  return rate;
}

float MacosParser::CpuUtilization() {
  host_cpu_load_info_data_t cpuinfo;
  mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
  if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO,
                      (host_info_t)&cpuinfo, &count) == KERN_SUCCESS) {
    unsigned long long totalTicks = 0;
    for (int i = 0; i < CPU_STATE_MAX; i++) totalTicks += cpuinfo.cpu_ticks[i];
    unsigned long long idleTicks = cpuinfo.cpu_ticks[CPU_STATE_IDLE];

    unsigned long long totalTicksSinceLastTime =
        totalTicks - previousTotalTicks_;
    unsigned long long idleTicksSinceLastTime = idleTicks - previousIdleTicks_;
    previousTotalTicks_ = totalTicks;
    previousIdleTicks_ = idleTicks;
    cachedCpuUtilization_ =
        (totalTicksSinceLastTime > 0 &&
         totalTicksSinceLastTime > idleTicksSinceLastTime)
            ? ((float)(totalTicksSinceLastTime - idleTicksSinceLastTime) /
               totalTicksSinceLastTime)
            : 0;
  }
  return cachedCpuUtilization_;
}

int MacosParser::TotalProcesses() { return cachedTotalProcesses_; }

int MacosParser::RunningProcesses() { return cachedRunningProcesses_; }

long MacosParser::UpTime() {
  struct timeval boottime;
  size_t len = sizeof(boottime);
  int mib[2] = {CTL_KERN, KERN_BOOTTIME};
  if (sysctl(mib, 2, &boottime, &len, NULL, 0) < 0) {
    return 0;
  }
  time_t bootTime = boottime.tv_sec, currentTime = time(NULL);
  long diff = (long)difftime(currentTime, bootTime);
  return diff;
}

// MARK: Processes

vector<int> MacosParser::Pids() {
  updateProcessInfo();
  return cachedPids_;
}

string MacosParser::User(int pid) {
  struct proc_bsdshortinfo info;
  string user;
  if (proc_pidinfo(pid, PROC_PIDT_SHORTBSDINFO, 0, &info,
                   PROC_PIDT_SHORTBSDINFO_SIZE) != -1) {
    user = user_from_uid(info.pbsi_uid, 0);
  }
  return user;
}

float MacosParser::CpuUtilization(int pid) {
  auto itr = pidAndCpuUsageMap_.find(pid);
  if (itr != pidAndCpuUsageMap_.end()) {
    return itr->second;
  }
  return 0;
}

string MacosParser::Ram(int pid) {
  rusage_info_current info;
  string ram;
  if (proc_pid_rusage(pid, RUSAGE_INFO_CURRENT, (void **)&info) == 0) {
    ram = to_string(info.ri_resident_size >> 20);
  }
  if (ram.empty()) {
    ram = "0";
  }
  return ram;
}

long MacosParser::UpTime(int pid) {
  rusage_info_current info;
  uint64_t uptime{0};
  if (proc_pid_rusage(pid, RUSAGE_INFO_CURRENT, (void **)&info) == 0) {
    uptime = info.ri_user_time + info.ri_system_time;
    chrono::nanoseconds ns{uptime};
    auto s = chrono::duration_cast<chrono::seconds>(ns);
    uptime = s.count();
  }
  return (long)uptime;
}

string MacosParser::Command(int pid) {
  string command;
  char path[PROC_PIDPATHINFO_MAXSIZE];
  if (proc_pidpath(pid, path, sizeof(path)) != -1) {
    command = path;
  }
  return command;
}

// MARK: Private

void MacosParser::updateProcessInfo() {
  int numberOfProcesses = proc_listallpids(nullptr, 0);
  if (numberOfProcesses == -1) return;

  pid_t pids[numberOfProcesses];
  numberOfProcesses = proc_listallpids(pids, sizeof(pids));
  if (numberOfProcesses == -1) return;

  int numberOfRunning{0};
  struct proc_taskinfo info;
  for (int i = 0; i < numberOfProcesses; i++) {
    if (proc_pidinfo(pids[i], PROC_PIDTASKINFO, 0, &info,
                     PROC_PIDTASKINFO_SIZE) != -1) {
      if (info.pti_numrunning > 0) numberOfRunning++;
    }
  }

  cachedPids_ = {pids, pids + numberOfProcesses};
  cachedTotalProcesses_ = numberOfProcesses;
  cachedRunningProcesses_ = numberOfRunning;

  string fileName = "pid_and_cpu_usage_map_info.txt";
  system(("ps -A -o pid= -o %cpu= > " + fileName).c_str());
  ifstream filestream(fileName);
  if (filestream.is_open()) {
    string line;
    string key;
    string value;

    while (getline(filestream, line)) {
      istringstream linestream(line);
      while (linestream >> key >> value) {
        pidAndCpuUsageMap_[stoi(key)] = stof(value) / 100.0;
      }
      linestream.clear();
    }
    filestream.close();
  }
}
