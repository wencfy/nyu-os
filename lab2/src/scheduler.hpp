#include "list"
#include "deque"
#include "string"
#include "vector"

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

    Process *current_running_process = nullptr;

    int total_io_time;
    int total_io_process_count;
    int io_start_time;

protected:
    std::string type;
    std::deque<Process*> process_queue;
    
public:
    int quantum;
    bool prio_preempt = false;
    virtual void add_process(Process *p) = 0;
    virtual Process *get_next_process() = 0;

    Process *get_current_process();
    void set_current_process(Process *process);
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
    FCFSScheduler(int quantum);
    void add_process(Process *p);
    Process *get_next_process();
};

class LCFSscheduler: public Scheduler {
public:
    LCFSscheduler();
    void add_process(Process *p);
    Process *get_next_process();
};

class SRTFScheduler: public Scheduler {
public:
    SRTFScheduler();
    void add_process(Process *p);
    Process *get_next_process();
};

class PRIOScheduler: public Scheduler {
private:
    std::vector<std::deque<Process*>> active;
    std::vector<std::deque<Process*>> expired;
public:
    PRIOScheduler(int quantum, int maxprio, bool prio_preempt);
    void add_process(Process *p);
    Process *get_next_process();
};
