#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <sys/stat.h>

namespace fs = std::filesystem;

// Function to read all log files and combine their content
std::vector<std::string> readAndCombineLogs(const fs::path& logFolderPath) {
    std::vector<std::string> combinedLogs;
    
    for (const auto& entry : fs::directory_iterator(logFolderPath)) {
        std::ifstream file(entry.path());
        if (file) {
            std::string line;
            while (std::getline(file, line)) {
                combinedLogs.push_back(line);
            }
        }
    }
    
    // Sort the log entries by timestamp
    std::sort(combinedLogs.begin(), combinedLogs.end());
    
    return combinedLogs;
}

// Function to write the combined logs to the actions.txt file
void writeCombinedLogs(const fs::path& logFolderPath, const std::vector<std::string>& combinedLogs) {
    std::ofstream outFile(logFolderPath / "actions.txt");
    for (const auto& log : combinedLogs) {
        outFile << log << std::endl;
    }
}

int main() {
    // Define the path to the logFolder
    fs::path logFolderPath = "./logFolder";
    
    // Create logFolder if it does not exist
    if (!fs::exists(logFolderPath)) {
        fs::create_directory(logFolderPath);
        chmod(logFolderPath.c_str(), 0755);
    }

    // Read and combine all log files
    std::vector<std::string> combinedLogs = readAndCombineLogs(logFolderPath);

    // Write the combined logs to actions.txt
    writeCombinedLogs(logFolderPath, combinedLogs);

    return 0;
}