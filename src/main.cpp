#include "linux_parser.h"
#include "macos_parser.h"
#include "ncurses_display.h"
#include "system.h"

int main() {
#ifdef __linux__
  LinuxParser parser = LinuxParser();
#elif __APPLE__
  MacosParser parser = MacosParser();
#endif
  System system = System(parser);
  NCursesDisplay::Display(system);
}
