#include <string>
#include <mutex>

extern std::mutex logMutex;

void logCommands(const std::string& command);
