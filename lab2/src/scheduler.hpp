#include "list"
#include "deque"
#include "string"

class Process {
public:
    /* Arrival Time */
    int AT;
    /* Total CPU Time */
    int TC;
    /* CPU Burst */
    int CB;
    /* IO Burst */
    int IO;

    Process(int AT, int TC, int CB, int IO);
};

class Scheduler {
private:
    std::deque<Process*> process_queue;
    
public:
    virtual void add_process(Process *p) = 0;
    virtual Process *get_next_process() = 0;
    virtual bool test_preempt(Process *p) = 0;
};

class FCFSScheduler: public Scheduler {
public:
    FCFSScheduler();
    void add_process(Process *p);
    Process *get_next_process();
    bool test_preempt(Process *p);
};
