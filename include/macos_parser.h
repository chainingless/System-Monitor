#ifndef macos_parser_h
#define macos_parser_h

#include <map>

#include "Parser.h"

class MacosParser : public Parser {
 public:
  MacosParser() { updateProcessInfo(); }

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
  void updateProcessInfo();
  std::string operatingSystem_;
  std::string kernel_;
  float cachedCpuUtilization_;
  unsigned long long previousTotalTicks_;
  unsigned long long previousIdleTicks_;
  int cachedTotalProcesses_{0};
  int cachedRunningProcesses_{0};
  std::vector<int> cachedPids_{};
  std::map<int, float> pidAndCpuUsageMap_{};
};
#endif /* macos_parser_h */
