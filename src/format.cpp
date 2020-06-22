#include "format.h"

#include <string>

using std::string;
using std::to_string;

string ZeroPaddedNumberString(string string) {
  if (string.size() < 2) {
    string.insert(string.begin(), 2 - string.size(), '0');
  }
  return string;
}

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
  string hoursString = ZeroPaddedNumberString(to_string(seconds / (60 * 60)));
  string minutesString = ZeroPaddedNumberString(to_string((seconds / 60) % 60));
  string secondsString = ZeroPaddedNumberString(to_string(seconds % 60));
  return hoursString + ":" + minutesString + ":" + secondsString;
}
