#include <string>
#include <fstream>

std::string readFile(std::string path) {
    std::ifstream file(path.c_str());

    if (!file.is_open()) {
      printf("Failed to open file: %s", path.c_str());
    }

    std::string full;
    std::string line;

    while (getline(file, line)) {
      full += line + '\n';
    }

    return full;
}

/*
void writeFile(const &std::string path, const &std::string text) {

}
*/