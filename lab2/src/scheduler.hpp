#include "list"
#include "deque"
#include "string"

class Process {
public:
    /* number */
    int no;
    /* Arrival Time */
    int AT;
    /* Total CPU Time */
    int TC;
    /* CPU Burst */
    int CB;
    /* IO Burst */
    int IO;
    int cpu_burst;
    int io_burst;
    int rem;
    int priority;
    int static_priority;
    int state_trans_time;

    int io_time;
    int cpu_waiting_time;
    int finish_time;

    Process(int no, int AT, int TC, int CB, int IO, int static_priority);
    std::string to_string();
};

class Scheduler {
private:
    std::list<Process*> finished_queue;

    int total_io_time;
    int total_io_process_count;
    int io_start_time;
protected:
    std::deque<Process*> process_queue;
    
public:
    int quantum;
    virtual void add_process(Process *p) = 0;
    virtual Process *get_next_process() = 0;
    virtual bool test_preempt(Process *p) = 0;

    void print_process_queue();
    void statistics();
    void finish(Process *p, int finish_time);

    // calculate total_io_time functions
    void start_io(int current_time);
    void finish_io(int current_time);
};

class FCFSScheduler: public Scheduler {
public:
    FCFSScheduler();
    void add_process(Process *p);
    Process *get_next_process();
    bool test_preempt(Process *p);
};

class LCFSscheduler: public Scheduler {
public:
    LCFSscheduler();
    void add_process(Process *p);
    Process *get_next_process();
    bool test_preempt(Process *p);
};

class SRTFScheduler: public Scheduler {
public:
    SRTFScheduler();
    void add_process(Process *p);
    Process *get_next_process();
    bool test_preempt(Process *p);
};

