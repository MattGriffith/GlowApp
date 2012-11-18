
// Easy way to read .ini configuration files
// Written by Matthew Griffith

#include "config.h"
#include <fstream>
#include <stdlib.h>

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
        // Remove surrounding whitespace or semi-colons
        size_t startPos = line.find_first_not_of(" \t\r");
        if (startPos == std::string::npos) continue;
        size_t endPos = line.find_last_not_of(" \t\r;");
        if (endPos == std::string::npos) continue;
        line = line.substr(startPos,endPos+1-startPos);

        // Check if it's a category change
        unsigned int length = line.length();
        if (length >= 3 && line[0] == '[' && line[length-1] == ']') {
            category = line.substr(1,length-2);
            continue;
        }
        // Otherwise check if it's a variable
        size_t equalsPos = line.find_first_of('=');
        if (equalsPos == std::string::npos || equalsPos == 0) continue;

        // Get the variable name
        size_t varEnd = line.find_last_not_of(" \t\r",equalsPos-1);
        if (varEnd == std::string::npos) continue;
        std::string var = line.substr(0,varEnd+1);
        if (var == "") continue;

        // Get its value
        std::string val = "";
        size_t valStart = line.find_first_not_of(" \t\"\r",equalsPos+1);
        size_t valEnd = line.find_last_not_of("\"");
        if (valStart != std::string::npos && valEnd >= valStart)
            val = line.substr(valStart,valEnd+1-valStart);

        // Add the lookup entry
        lookup[category+":"+var] = val;
    } while (!file.eof()); // End if there are no more lines

    file.close();
    return true;
}


std::string ConfigFile::GetString(const char* key, std::string sample) {
    std::map<std::string,std::string>::iterator index;
    index = lookup.find(key);
    if (index != lookup.end()) return index->second;
    return sample;
}

bool ConfigFile::GetBool(const char* key, bool sample) {
    std::map<std::string,std::string>::iterator index;
    index = lookup.find(key);
    if (index != lookup.end()) {
        if (index->second == "true" || index->second == "TRUE") return true;
        if (index->second == "false" || index->second == "FALSE") return false;
        return atoi(index->second.c_str());
    }
    return sample;
}

int ConfigFile::GetInt(const char* key, int sample) {
    std::map<std::string,std::string>::iterator index;
    index = lookup.find(key);
    if (index != lookup.end()) {
        return atoi(index->second.c_str());
    }
    return sample;
}

float ConfigFile::GetFloat(const char* key, float sample) {
    std::map<std::string,std::string>::iterator index;
    index = lookup.find(key);
    if (index != lookup.end()) {
        return atof(index->second.c_str());
    }
    return sample;
}


