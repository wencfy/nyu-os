#include "scheduler.hpp"

Process::Process(int no, int AT, int TC, int CB, int IO):
no(no), AT(AT), TC(TC), CB(CB), IO(IO) {
    rem = TC;
    priority = 1;
}

std::string Process::to_string() {
    return "process: id: " + std::to_string(no) + " rem: " + std::to_string(rem) + " cpub: "
     + std::to_string(cpu_burst) + " iob: " + std::to_string(io_burst);
}

void Scheduler::print_process_queue() {
    for (Process *process: process_queue) {
        printf("%s\n", process->to_string().c_str());
    }
}


/**
 * FCFSScheduler
 */
FCFSScheduler::FCFSScheduler() {
    quantum = 10000;
}

void FCFSScheduler::add_process(Process *p) {
    process_queue.push_back(p);
}

Process *FCFSScheduler::get_next_process() {
    if (!process_queue.empty()) {
        Process *p = process_queue.front();
        process_queue.pop_front();
        return p;
    }
    return nullptr;
}

bool FCFSScheduler::test_preempt(Process *p) {
    
}


