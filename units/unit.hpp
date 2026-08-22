#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>

class Unit {
  protected:
    std::string read_file(const std::filesystem::path& path) {
        std::ostringstream ss;
        ss << std::ifstream{path}.rdbuf();
        return ss.str();
    }
};