#include "iosched.hpp"

bool Loader::__get_next_line() {
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

Loader::Loader(const std::string in) {
    this->in.open(in);
}

bool Loader::get_request(io_request &iorequest) {
    int arr_time, track;
    if (__get_next_line()) {
        ss >> arr_time >> track;
        iorequest = {arr_time, track};
        return true;
    }
    return false;
}

void Loader::close() {
    this->in.close();
}
