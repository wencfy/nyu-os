#include "iosched.hpp"
#include "algorithm"

void Scheduler::add_task(io_task *task) {
    io_queue.push_back(task);
}

void Scheduler::finish(int timestamp) {
    current_running_io_task->finish_time = timestamp;

    auto iter = finish_queue.begin();
    while (iter != finish_queue.end()) {
        if (current_running_io_task->request_time < (*iter)->request_time) {
            break;
        }
        iter++;
    }
    finish_queue.insert(iter, current_running_io_task);
    current_running_io_task = nullptr;
}

void Scheduler::seek() {
    total_movement++;
    if (current < current_running_io_task->track) {
        current++;
    } else {
        current--;
    }
}

void Scheduler::statistics() {
    double avg_turn_around = 0.;
    double avg_wait_time = 0.;
    int max_wait_time = 0;
    int i = 0;
    for (io_task *task: finish_queue) {
        avg_turn_around += task->finish_time - task->request_time;
        int wait_time = task->start_time - task->request_time;
        avg_wait_time += wait_time;
        if (max_wait_time < wait_time) {
            max_wait_time = wait_time;
        }

        printf("%5d: %5d %5d %5d\n", i++, task->request_time, task->start_time, task->finish_time);
    }
    avg_turn_around /= finish_queue.size();
    avg_wait_time /= finish_queue.size();

    printf("SUM: %d %d %.4lf %.2lf %.2lf %d\n",
        total_time, total_movement, 1. * busy_time / total_time,
        avg_turn_around, avg_wait_time, max_wait_time
    );
}


FIFOScheduler::FIFOScheduler() {}

bool FIFOScheduler::fetch_task() {
    if (io_queue.empty()) {
        return false;
    }
    current_running_io_task = io_queue.front();
    io_queue.pop_front();
    return true;
}


SSTFScheduler::SSTFScheduler() {}

bool SSTFScheduler::fetch_task() {
    if (io_queue.empty()) {
        return false;
    }

    auto it = io_queue.begin();
    auto min_it = io_queue.begin();
    int min_dis = current > (*min_it)->track ?
        current - (*min_it)->track : (*min_it)->track - current;
    while (it != io_queue.end()) {
        int cur_dis = current > (*it)->track ?
            current - (*it)->track : (*it)->track - current;
        if (cur_dis < min_dis) {
            min_dis = cur_dis;
            min_it = it;
        }
        it++;
    }
    current_running_io_task = *min_it;
    io_queue.erase(min_it);
    return true;
}


LookScheduler::LookScheduler() {}

void LookScheduler::add_task(io_task *task) {
    Scheduler::add_task(task);
}

bool LookScheduler::fetch_task() {
    if (io_queue.empty()) {
        return false;
    }

    auto l = io_queue.end();
    auto r = io_queue.end();
    for (auto it = io_queue.begin(); it != io_queue.end(); it++) {
        if (current <= (*it)->track) {
            if (r == io_queue.end() || (*r)->track > (*it)->track) {
                r = it;
            }
        }
        if (current >= (*it)->track) {
            if (l == io_queue.end() || (*l)->track < (*it)->track) {
                l = it;
            }
        }
    }
    std::list<io_task *>::iterator it;
    if (dir) {
        if (r != io_queue.end()) {
            it = r;
        } else {
            dir = !dir;
            it = l;
        }
    } else {
        if (l != io_queue.end()) {
            it = l;
        } else {
            dir = !dir;
            it = r;
        }
    }
    current_running_io_task = *it;
    io_queue.erase(it);

    return true;
}


CLookScheduler::CLookScheduler() {}

bool CLookScheduler::fetch_task() {
    if (io_queue.empty()) {
        return false;
    }

    auto start = io_queue.begin();
    auto r = io_queue.end();
    for (auto it = io_queue.begin(); it != io_queue.end(); it++) {
        if (current <= (*it)->track) {
            if (r == io_queue.end() || (*r)->track > (*it)->track) {
                r = it;
            }
        }
        if ((*start)->track > (*it)->track) {
            start = it;
        }
    }

    if (r != io_queue.end()) {
        current_running_io_task = *r;
        io_queue.erase(r);
    } else {
        current_running_io_task = *start;
        io_queue.erase(start);
    }
    return true;
}


FLookScheduler::FLookScheduler() {}

void FLookScheduler::add_task(io_task *task) {
    add_queue.push_back(task);
}

bool FLookScheduler::fetch_task() {
    if (io_queue.empty()) {
        io_queue.swap(add_queue);
    }

    return LookScheduler::fetch_task();
}
