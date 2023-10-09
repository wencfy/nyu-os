#include "fstream"

class Util {
private:
    const char path[8] = "./rfile";
    std::fstream in;
public:
    Util() {
        in.open(path);
    }

    int rand() {
        char line[128] = "";
        in.getline(line, 128);
        return std::stoi(line);
    }
};
