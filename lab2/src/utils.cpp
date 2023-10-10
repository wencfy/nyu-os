#include "fstream"
#include "vector"

class Util {
private:
    std::vector<int> random_values;
    int offset = 0;
public:
    Util(const std::string path) {
        offset = 0;
        std::ifstream ifs(path);
        int n;
        // read the count
        ifs >> n;
        while (ifs >> n) {
            random_values.push_back(n);
        }
    }

    int rand() {
        if (offset == random_values.size()) {
            offset = 0;
        }
        return random_values[offset++];
    }

    int rand(int num) {
        return rand() % num + 1;
    }
};
