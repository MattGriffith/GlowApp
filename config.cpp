
// Easy way to read .ini configuration files
// Written by Matthew Griffith

#include "config.h"
#include <fstream>

// Parses the config file, stores pairs into a std::map
// Returns true if successful, false if not
bool ConfigFile::Open(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    std::string category;

    // For each line in the file...
    do {
        std::string line;
        getline(file,line);

        // Remove comments
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos) {
            line = line.substr(0,commentPos);
        }

        // Remove surrounding whitespace
        size_t startPos = line.find_first_not_of(" \t");
        if (startPos == std::string::npos) continue;
        size_t endPos = line.find_last_not_of(" \t");
        line = line.substr(startPos,endPos-startPos);

        // Check if it's a category change
        if (line.length() < 3) continue;
        if (line[0] == '[' && line[line.length()-1] == ']') {
            category = line.substr(1,line.length()-2);
        }
        // Otherwise check if it's a variable
        else {
            size_t equalsPos = line.find_first_of('=');
            if (equalsPos == std::string::npos) continue;

            // Get the variable name
            size_t varEnd = line.find_last_not_of(" \t",equalsPos-1);
            if (varEnd == std::string::npos) continue;
            std::string var = line.substr(0,varEnd);

            // Get its value
            std::string val = "";
            size_t valStart = line.find_first_not_of(" \t",equalsPos+1);
            if (valStart != std::string::npos)
                val = line.substr(valStart,line.length()-valStart);

            // Add the lookup entry
            lookup[category+":"+var] = val;
        }
    } while (!file.eof()); // End if there are no more lines

    file.close();
    return true;
}


std::string ConfigFile::GetString(const char* key, std::string sample) {

    return sample;
}

bool ConfigFile::GetBool(const char* key, bool sample) {

    return sample;
}

int ConfigFile::GetInt(const char* key, int sample) {

    return sample;
}

float ConfigFile::GetFloat(const char* key, float sample) {

    return sample;
}
