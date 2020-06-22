#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// MARK: System

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ' ', '_');
      replace(line.begin(), line.end(), '=', ' ');
      replace(line.begin(), line.end(), '"', ' ');
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, number;
  string line;
  ifstream filestream(kProcDirectory + kVersionFilename);
  if (filestream.is_open()) {
    getline(filestream, line);
    istringstream linestream(line);
    linestream >> os >> kernel >> number;
  }
  return kernel + " " + number;
}

float LinuxParser::MemoryUtilization() {
  float totalMemory{1};
  float freeMemory{0};
  float buffers{0};
  string token;
  ifstream filestream(kProcDirectory + kMeminfoFilename);

  if (filestream.is_open()) {
    while (filestream >> token) {
      if (token == "MemTotal:") {
        if (filestream >> token) totalMemory = stof(token);
      } else if (token == "MemFree:") {
        if (filestream >> token) freeMemory = stof(token);
      } else if (token == "Buffers:") {
        if (filestream >> token) buffers = stof(token);
      }
    }
  }
  return 1 - freeMemory / (totalMemory - buffers);
}

float LinuxParser::CpuUtilization() {
  float utilization{0};
  long activeTicks = ActiveJiffies();
  long idleTicks = IdleJiffies();
  long durationActive{activeTicks - cachedActiveTicks_};
  long durationIdle{idleTicks - cachedIdleTicks_};
  long duration{durationActive + durationIdle};
  utilization = static_cast<float>(durationActive) / duration;
  cachedActiveTicks_ = activeTicks;
  cachedIdleTicks_ = idleTicks;
  return utilization;
}

int LinuxParser::TotalProcesses() {
  string line;
  string key;
  string value;
  ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      istringstream stream(line);
      while (stream >> key >> value) {
        if (key == "processes") {
          return stoi(value);
        }
      }
    }
  }
  return 0;
}

int LinuxParser::RunningProcesses() {
  string line;
  ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      istringstream linestream(line);
      string key, value;
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          return stoi(value);
        }
      }
    }
  }
  return 0;
}

long LinuxParser::UpTime() {
  string line;
  string token;
  ifstream filestream(kProcDirectory + kUptimeFilename);

  if (filestream.is_open()) {
    getline(filestream, line);
    istringstream linestream(line);
    if (linestream >> token) {
      return stoi(token);
    }
  }
  return 0;
}

// MARK: Processes

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    if (file->d_type == DT_DIR) {
      string filename(file->d_name);
      if (filename.find_first_not_of("0123456789") == string::npos) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

string LinuxParser::User(int pid) {
  ifstream filestream(LinuxParser::kPasswordPath);
  if (filestream.is_open()) {
    string line;
    string token = "x:" + Uid(pid);
    while (getline(filestream, line)) {
      auto marker = line.find(token);
      if (marker != string::npos) {
        return line.substr(0, marker - 1);
      }
    }
  }
  return "0";
}

float LinuxParser::CpuUtilization(int pid) {
  long cachedActiveTicks{0};
  long cachedSystemTicks{0};
  if (pidAndCachedActiveTicksMap_.find(pid) !=
      pidAndCachedActiveTicksMap_.end()) {
    cachedActiveTicks = pidAndCachedActiveTicksMap_[pid];
  }
  if (pidAndCachedSystemTicksMap_.find(pid) !=
      pidAndCachedSystemTicksMap_.end()) {
    cachedSystemTicks = pidAndCachedSystemTicksMap_[pid];
  }

  long activeTicks = ActiveJiffies(pid);
  long systemTicks = Jiffies();

  long durationActive{activeTicks - cachedActiveTicks};
  long duration{systemTicks - cachedSystemTicks};
  cachedCpuUtilization_ = static_cast<float>(durationActive) / duration;
  pidAndCachedActiveTicksMap_[pid] = activeTicks;
  pidAndCachedSystemTicksMap_[pid] = systemTicks;
  return cachedCpuUtilization_;
}

string LinuxParser::Ram(int pid) {
  string token;
  ifstream filestream(LinuxParser::kProcDirectory + to_string(pid) +
                      LinuxParser::kStatusFilename);
  if (filestream.is_open()) {
    while (filestream >> token) {
      if (token == "VmSize:") {
        if (filestream >> token) return to_string(stoi(token) / 1024);
      }
    }
  }
  return string("0");
}

long int LinuxParser::UpTime(int pid) {
  long int time{0};
  string token;
  ifstream filestream(LinuxParser::kProcDirectory + to_string(pid) +
                      LinuxParser::kStatFilename);
  if (filestream.is_open()) {
    for (int i = 0; filestream >> token; ++i)
      if (i == 13) {
        long int time{stol(token)};
        time /= sysconf(_SC_CLK_TCK);
        return time;
      }
  }
  return time;
}

string LinuxParser::Command(int pid) {
  string line;
  ifstream filestream(LinuxParser::kProcDirectory + to_string(pid) +
                      LinuxParser::kCmdlineFilename);
  if (filestream.is_open()) {
    string line;
    getline(filestream, line);
    return line;
  }
  return "";
}

// MARK: Private

long LinuxParser::Jiffies() { return UpTime() * sysconf(_SC_CLK_TCK); }

long LinuxParser::ActiveJiffies(int pid) {
  string line, token;
  vector<string> values;
  ifstream filestream(LinuxParser::kProcDirectory + to_string(pid) +
                      LinuxParser::kStatFilename);
  if (filestream.is_open()) {
    getline(filestream, line);
    istringstream linestream(line);
    while (linestream >> token) {
      values.push_back(token);
    }
  }
  long jiffies{0};
  if (values.size() > 21) {
    long user = stol(values[13]);
    long kernel = stol(values[14]);
    long children_user = stol(values[15]);
    long children_kernel = stol(values[16]);
    jiffies = user + kernel + children_user + children_kernel;
  }
  return jiffies;
}

long LinuxParser::ActiveJiffies() {
  vector<string> time = CpuUtilizationStringList();
  return (stol(time[CPUStates::kUser_]) + stol(time[CPUStates::kNice_]) +
          stol(time[CPUStates::kSystem_]) + stol(time[CPUStates::kIRQ_]) +
          stol(time[CPUStates::kSoftIRQ_]) + stol(time[CPUStates::kSteal_]) +
          stol(time[CPUStates::kGuest_]) + stol(time[CPUStates::kGuestNice_]));
}

long LinuxParser::IdleJiffies() {
  vector<string> time = CpuUtilizationStringList();
  return (stol(time[CPUStates::kIdle_]) + stol(time[CPUStates::kIOwait_]));
}

string LinuxParser::Uid(int pid) {
  string token;
  ifstream filestream(LinuxParser::kProcDirectory + to_string(pid) +
                      LinuxParser::kStatusFilename);
  if (filestream.is_open()) {
    while (filestream >> token) {
      if (token == "Uid:") {
        if (filestream >> token) return token;
      }
    }
  }
  return string("");
}

vector<string> LinuxParser::CpuUtilizationStringList() {
  string line;
  string token;
  vector<string> values;
  ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      istringstream linestream(line);
      while (linestream >> token) {
        if (token == "cpu") {
          while (linestream >> token) values.push_back(token);
          return values;
        }
      }
    }
  }
  return values;
}
