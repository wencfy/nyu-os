#include "scheduler.hpp"

Process::Process(int AT, int TC, int CB, int IO):
    AT(AT), TC(TC), CB(CB), IO(IO) {}


/**
 * FCFSScheduler
 */
FCFSScheduler::FCFSScheduler() {}

void FCFSScheduler::add_process(Process *p) {
    process_queue.push_back(p);
}

Process *FCFSScheduler::get_next_process() {
    Process *p = process_queue.front();
    return p;
}

bool FCFSScheduler::test_preempt(Process *p) {

}


