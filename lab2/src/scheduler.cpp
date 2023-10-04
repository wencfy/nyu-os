#include "scheduler.hpp"

Process::Process(int AT, int TC, int CB, int IO):
    AT(AT), TC(TC), CB(CB), IO(IO) {}



FCFSScheduler::FCFSScheduler() {}

void FCFSScheduler::add_process(Process *p) {
    
}

Process *FCFSScheduler::get_next_process() {

}

bool FCFSScheduler::test_preempt(Process *p) {

}


