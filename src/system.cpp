#include "system.h"

#include <unistd.h>

#include <algorithm>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "process.h"
#include "processor.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;
using namespace std;

// TODO: Return the system's CPU
Processor& System::Cpu() {
  cpu_.Utilization(parser_.CpuUtilization());
  return cpu_;
}

// TODO: Return a container composed of the system's processes
vector<Process>& System::Processes() {
  vector<int> pids = parser_.Pids();
  vector<Process> processes{};
  for (auto pid : pids) {
    processes.push_back(Process(pid, parser_.User(pid),
                                parser_.CpuUtilization(pid), parser_.Ram(pid),
                                parser_.UpTime(pid), parser_.Command(pid)));
  }
  sort(processes.begin(), processes.end(), greater<Process>());

  processes_ = processes;
  return processes_;
}

// TODO: Return the system's kernel identifier (string)
std::string System::Kernel() { return parser_.Kernel(); }

// TODO: Return the system's memory utilization
float System::MemoryUtilization() { return parser_.MemoryUtilization(); }

// TODO: Return the operating system name
std::string System::OperatingSystem() { return parser_.OperatingSystem(); }

// TODO: Return the number of processes actively running on the system
int System::RunningProcesses() { return parser_.RunningProcesses(); }

// TODO: Return the total number of processes on the system
int System::TotalProcesses() { return parser_.TotalProcesses(); }

// TODO: Return the number of seconds since the system started running
long int System::UpTime() { return parser_.UpTime(); }
