#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <fstream>
#include <map>
#include <string>

#include "Parser.h"

class LinuxParser : public Parser {
  // Paths
  const std::string kProcDirectory{"/proc/"};
  const std::string kCmdlineFilename{"/cmdline"};
  const std::string kCpuinfoFilename{"/cpuinfo"};
  const std::string kStatusFilename{"/status"};
  const std::string kStatFilename{"/stat"};
  const std::string kUptimeFilename{"/uptime"};
  const std::string kMeminfoFilename{"/meminfo"};
  const std::string kVersionFilename{"/version"};
  const std::string kOSPath{"/etc/os-release"};
  const std::string kPasswordPath{"/etc/passwd"};

  // CPU
  enum CPUStates {
    kUser_ = 0,
    kNice_,
    kSystem_,
    kIdle_,
    kIOwait_,
    kIRQ_,
    kSoftIRQ_,
    kSteal_,
    kGuest_,
    kGuestNice_
  };

  // System

  std::string OperatingSystem();
  std::string Kernel();
  float MemoryUtilization();
  float CpuUtilization();
  int TotalProcesses();
  int RunningProcesses();
  long UpTime();

  // Processes

  std::vector<int> Pids();
  std::string User(int pid);
  float CpuUtilization(int pid);
  std::string Ram(int pid);
  long UpTime(int pid);
  std::string Command(int pid);

 private:
  long Jiffies();
  long ActiveJiffies();
  long ActiveJiffies(int pid);
  long IdleJiffies();
  std::string Uid(int pid);
  std::vector<std::string> CpuUtilizationStringList();
  long cachedActiveTicks_{0};
  long cachedIdleTicks_{0};
  std::map<int, long> pidAndCachedSystemTicksMap_{};
  std::map<int, long> pidAndCachedActiveTicksMap_{};
  float cachedCpuUtilization_;
};

#endif
