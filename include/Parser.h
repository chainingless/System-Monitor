#ifndef Parser_h
#define Parser_h

#include <string>
#include <vector>

class Parser {
 public:
  // System
  virtual std::string OperatingSystem() = 0;
  virtual std::string Kernel() = 0;
  virtual float MemoryUtilization() = 0;
  virtual float CpuUtilization() = 0;
  virtual int TotalProcesses() = 0;
  virtual int RunningProcesses() = 0;
  virtual long UpTime() = 0;

  // Processes
  virtual std::vector<int> Pids() = 0;
  virtual std::string User(int pid) = 0;
  virtual float CpuUtilization(int pid) = 0;
  virtual std::string Ram(int pid) = 0;
  virtual long UpTime(int pid) = 0;
  virtual std::string Command(int pid) = 0;
};

#endif /* Parser_h */
