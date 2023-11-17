#include "fstream"
#include "sstream"
#include "vector"

#include "mmu.hpp"

extern std::vector<Process*> process_list;

class Utils {
private:
    std::vector<int> random_values;
    std::ifstream in;
    std::stringstream ss;
    int offset = 0;

    int rand() {
        if (offset == random_values.size()) {
            offset = 0;
        }
        return random_values[offset++];
    }

    bool __get_next_line() {
        std::string line;
        while (getline(in, line)) {
            if (line[0] == '#') {
                continue;
            } else {
                ss.clear();
                ss.str(line);
                return true;
            }
        }
        return false;
    }

    Process *__get_process() {
        int vma_cnt;
        __get_next_line();
        ss >> vma_cnt;

        Process *process = new Process();
        int start_vpage;
        int end_vpage;
        bool write_protected;
        bool file_mapped;
        for (int i = 0; i < vma_cnt; i++) {
            __get_next_line();
        // printf("%s\n", process->to_string().c_str());
            ss >> start_vpage >> end_vpage >> write_protected >> file_mapped;
            process->add_vma(start_vpage, end_vpage, write_protected, file_mapped);
        }

        return process;
    }

public:
    Utils(const std::string iPath, const std::string rPath) {
        offset = 0;
        std::ifstream ifs(rPath);
        int n;
        // read the count
        ifs >> n;
        while (ifs >> n) {
            random_values.push_back(n);
        }
        ifs.close();

        in.open(iPath);
    }

    int rand(int num) {
        return rand() % num + 1;
    }

    void init_process_list() {
        int process_num;
        __get_next_line();
        ss >> process_num;
        for (int i = 0; i < process_num; i++) {
            Process *process = __get_process();
            process_list.push_back(process);
        }
    }

    bool get_next_instruction(char &opcode, int &operand) {
        if (__get_next_line()) {
            ss >> opcode >> operand;
            return true;
        }
        return false;
    }

};