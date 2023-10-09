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

    Process(int no, int AT, int TC, int CB, int IO);
    std::string to_string();
};

class Scheduler {
public:
    int quantum;

protected:
    std::deque<Process*> process_queue;
    
public:
    virtual void add_process(Process *p) = 0;
    virtual Process *get_next_process() = 0;
    virtual bool test_preempt(Process *p) = 0;

    void print_process_queue();
};

class FCFSScheduler: public Scheduler {
public:
    FCFSScheduler();
    void add_process(Process *p);
    Process *get_next_process();
    bool test_preempt(Process *p);
};
