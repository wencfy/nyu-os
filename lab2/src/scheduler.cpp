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

Process *Scheduler::get_current_process() {
    return current_running_process;
}

void Scheduler::set_current_process(Process *process) {
    current_running_process = process;
}

void Scheduler::print_process_queue() {
    for (Process *process: process_queue) {
        printf("%s\n", process->to_string().c_str());
    }
}

void Scheduler::statistics() {
    printf("%s\n", type.c_str());

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
    io_util = total_io_time * 100.0 / max_finish_time;
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

void Scheduler::start_io(int current_time) {
    total_io_process_count++;
    if (total_io_process_count == 1) {
        // just started io
        io_start_time = current_time;
    }
}

void Scheduler::finish_io(int current_time) {
    total_io_process_count--;
    if (total_io_process_count == 0) {
        // just finished io
        total_io_time += current_time - io_start_time;
    }
}


/**
 * FCFSScheduler
 */
FCFSScheduler::FCFSScheduler() {
    quantum = 10000;
    this->type = "FCFS";
}

FCFSScheduler::FCFSScheduler(int quantum) {
    this->quantum = quantum;
    this->type = "RR " + std::to_string(quantum);
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


/**
 * LCFSScheduler
 */
LCFSscheduler::LCFSscheduler() {
    quantum = 10000;
    this->type = "LCFS";
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


/**
 * SRTFScheduler
 */
SRTFScheduler::SRTFScheduler() {
    quantum = 10000;
    this->type = "SRTF";
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


/**
 * PRIOScheduler
 */
PRIOScheduler::PRIOScheduler(int quantum, int maxprio, bool prio_preempt) {
    this->quantum = quantum;
    this->prio_preempt = prio_preempt;
    active.resize(maxprio);
    expired.resize(maxprio);
    if (prio_preempt) {
        this->type = "PREPRIO " + std::to_string(quantum);
    } else {
        this->type = "PRIO " + std::to_string(quantum);
    }
}

void PRIOScheduler::add_process(Process *p) {
    if (p->priority < 0) {
        p->priority = p->static_priority - 1;
        auto idx = expired.size() - p->priority - 1;
        expired[idx].push_back(p);
    } else {
        auto idx = active.size() - p->priority - 1;
        active[idx].push_back(p);
    }
}

Process *PRIOScheduler::get_next_process() {
    auto it = active.begin();
    while (it != active.end()) {
        if (!(it->empty())) {
            break;
        }
        it++;
    }
    if (it != active.end()) {
        Process *p = it->front();
        it->pop_front();
        return p;
    }

    active.swap(expired);
    
    it = active.begin();
    while (it != active.end()) {
        if (!(it->empty())) {
            break;
        }
        it++;
    }
    if (it != active.end()) {
        Process *p = it->front();
        it->pop_front();
        return p;
    }

    return nullptr;
}
