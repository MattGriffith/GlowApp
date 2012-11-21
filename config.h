/*

    Easy way to read .ini configuration files

    Copyright 2007, 2012 Matthew Griffith

    This file is part of TroidGlow.

    TroidGlow is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TroidGlow is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TroidGlow.  If not, see <http://www.gnu.org/licenses/>.

*/

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

