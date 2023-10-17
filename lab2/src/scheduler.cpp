#include "scheduler.hpp"

Process::Process(int no, int AT, int TC, int CB, int IO, int static_priority):
no(no), AT(AT), TC(TC), CB(CB), IO(IO), static_priority(static_priority) {
    rem = TC;
    priority = static_priority - 1;
    state_trans_time = 0;
    io_time = 0;
    cpu_waiting_time = 0;
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

void Scheduler::statistics() {
    int total_count = finished_queue.size();
    int max_finish_time = 0;
    double cpu_util = 0;
    double io_util = 0;
    double avg_turn_around = 0;
    double avg_wait_time = 0;
    double throughput;

    while (!finished_queue.empty()) {
        Process *p = finished_queue.front();
        finished_queue.pop_front();
        printf(
            "%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
            p->no, p->AT, p->TC, p->CB, p->IO, p->static_priority,
            p->finish_time, p->finish_time - p->AT, p->io_time, p->cpu_waiting_time
        );

        if (max_finish_time < p->finish_time) {
            max_finish_time = p->finish_time;
        }

        cpu_util += p->TC;
        avg_turn_around += p->finish_time - p->AT;
        avg_wait_time += p->cpu_waiting_time;
    }
    cpu_util = cpu_util * 100 / max_finish_time;
    avg_turn_around /= total_count;
    avg_wait_time /= total_count;
    throughput = 100.0 * total_count / max_finish_time;

    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
	       max_finish_time,
	       cpu_util,
	       io_util,
	       avg_turn_around,
	       avg_wait_time,
	       throughput);
}

void Scheduler::finish(Process *p, int finish_time) {
    p->finish_time = finish_time;
    auto it = finished_queue.begin();
    while (it != finished_queue.end()) {
        if ((*it)->no <= p->no) {
            it++;
        } else {
            break;
        }
    }
    finished_queue.insert(it, p);
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


/**
 * LCFSScheduler
 */
LCFSscheduler::LCFSscheduler() {
    quantum = 10000;
}

void LCFSscheduler::add_process(Process *p) {
    process_queue.push_front(p);
}

Process *LCFSscheduler::get_next_process() {
    if (!process_queue.empty()) {
        Process *p = process_queue.front();
        process_queue.pop_front();
        return p;
    }
    return nullptr;
}

bool LCFSscheduler::test_preempt(Process *p) {
    
}


/**
 * SRTFScheduler
 */
SRTFScheduler::SRTFScheduler() {
    quantum = 10000;
}

void SRTFScheduler::add_process(Process *p) {
    auto it = process_queue.begin();
    while (it != process_queue.end()) {
        if ((*it)->rem <= p->rem) {
            it++;
        } else {
            break;
        }
    }
    process_queue.insert(it, p);
}

Process *SRTFScheduler::get_next_process() {
    if (!process_queue.empty()) {
        Process *p = process_queue.front();
        process_queue.pop_front();
        return p;
    }
    return nullptr;
}

bool SRTFScheduler::test_preempt(Process *p) {
    
}
