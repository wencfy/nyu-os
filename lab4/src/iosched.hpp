#include "fstream"
#include "sstream"
#include "list"

struct io_request {
    int arr_time;
    int track;
};

class Loader {
private:
    std::ifstream in;
    std::stringstream ss;
    bool __get_next_line();
public:
    Loader(const std::string in);
    bool get_request(io_request &);
    void close();
};


struct io_task {
    int track;

    int request_time;
    int start_time;
    int finish_time;
};

class Scheduler {
protected:
    std::list<io_task*> io_queue;
    std::list<io_task*> finish_queue;

public:
    int current = 0;

    int total_time = 0;
    int total_movement = 0;
    int busy_time = 0;

    io_task *current_running_io_task = nullptr;
    void add_task(io_task *task);
    virtual bool fetch_task() = 0;
    void seek();
    void finish(int timestamp);
    void statistics();
};

class FIFOScheduler: public Scheduler {
public:
    FIFOScheduler();
    bool fetch_task();
};

class SSTFScheduler: public Scheduler {
public:
    SSTFScheduler();
    bool fetch_task();
};
