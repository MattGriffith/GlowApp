
// Easy way to read .ini configuration files
// Written by Matthew Griffith

#include <string>
#include <map>

class ConfigFile {
    public:
    bool Open(const char* filename);

    std::string GetString(const char* key, std::string sample);
    bool GetBool(const char* key, bool sample);
    int GetInt(const char* key, int sample);
    float GetFloat(const char* key, float sample);

    private:
    std::map<std::string,std::string> lookup;
};

