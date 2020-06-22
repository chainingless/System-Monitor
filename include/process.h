#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid, std::string user, float cpuUtilization, std::string ram,
          long uptime, std::string command)
      : pid_(pid),
        user_(user),
        cpuUtilization_(cpuUtilization),
        ram_(ram),
        uptime_(uptime),
        command_(command) {}
  int Pid();                               // TODO: See src/process.cpp
  std::string User();                      // TODO: See src/process.cpp
  std::string Command();                   // TODO: See src/process.cpp
  float CpuUtilization() const;            // TODO: See src/process.cpp
  std::string Ram();                       // TODO: See src/process.cpp
  long int UpTime();                       // TODO: See src/process.cpp
  bool operator<(const Process& a) const;  // TODO: See src/process.cpp
  bool operator>(const Process& a) const;

  // TODO: Declare any necessary private members
 private:
  int pid_;
  std::string user_;
  float cpuUtilization_;
  std::string ram_;
  long uptime_;
  std::string command_;
};

#endif
