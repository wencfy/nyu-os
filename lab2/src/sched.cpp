#include "iostream"
#include "fstream"

#include "DES.hpp"
#include "utils.cpp"
using namespace std;

Util *util;
bool _call_scheduler = false;

bool _verbose = false;
bool _verbose_trace = false;
bool _verbose_event_queue = false;
bool _verbose_preempt = false;


void Simulation(DES *des, Scheduler *sched) {
    Event *evt;
    while ((evt = des->get_event())) {
        if (_verbose) {
            des->print_events();
        }

        Process *process = evt->process;
        int current_time = evt->timestamp;
        STATE old_state = evt->old_state;
        STATE new_state = evt->new_state;

        int time_in_prev_state = current_time - process->state_trans_time;
        process->state_trans_time = current_time;

        if (_verbose) {
            printf("%d %d %d: %s -> %s", current_time, process->no, time_in_prev_state, state_to_string(old_state).c_str(), state_to_string(new_state).c_str());
        }

        delete evt;
        evt = nullptr;

        switch (new_state) {
            case READY: {
                if (old_state == BLOCKED || old_state == CREATED) {
                    // trans to ready
                    if (old_state == BLOCKED) {
                        process->io_time += time_in_prev_state;
                        sched->finish_io(current_time);
                    }

                    process->priority = process->static_priority - 1;
                    sched->add_process(process);

                    Process *current_running_process = sched->get_current_process();
                    if (sched->prio_preempt && current_running_process) {
                        // priority preemption
                        Event *pending_event = des->get_event(current_running_process->no);
                        bool preempt = pending_event->timestamp > current_time && process->priority > current_running_process->priority;

                        // VERBOSE

                        if (preempt) {
                            current_running_process->rem += pending_event->timestamp - current_time;
                            current_running_process->cpu_burst += pending_event->timestamp - current_time;
                            des->remove_event(current_running_process->no);

                            Event *event = new Event(
                                current_time,
                                current_running_process,
                                RUNNING,
                                READY
                            );
                            des->put_event(event);
                        }
                    }
                } else if (old_state == RUNNING) {
                    // trans to preemption
                    process->priority--;
                    sched->set_current_process(nullptr);
                    sched->add_process(process);
                }
                _call_scheduler = true;
            }
            break;
        
            case RUNNING: {
                // trans to run
                // create event for either preemption or blocking
                int cpu_burst = sched->get_current_process()->cpu_burst > 0 ?
                                sched->get_current_process()->cpu_burst :
                                util->rand(process->CB);
                cpu_burst = cpu_burst > sched->get_current_process()->rem ? sched->get_current_process()->rem : cpu_burst;
                process->cpu_waiting_time += time_in_prev_state;
                // VERBOSE
                if (_verbose) {
                    printf(" cb=%d rem=%d prio=%d\n", cpu_burst, process->rem, process->priority);
                }

                if (cpu_burst > sched->quantum) {
                    // quantum preemption
                    process->rem -= sched->quantum;
                    process->cpu_burst = cpu_burst - sched->quantum;
                    int until = current_time + sched->quantum;
                    Event *e = new Event(
                        until,
                        process,
                        new_state,
                        READY
                    );
                    des->put_event(e);
                } else {
                    process->rem -= cpu_burst;
                    process->cpu_burst = 0;
                    int until = current_time + cpu_burst;
                    Event *e = new Event(
                        until,
                        process,
                        new_state,
                        BLOCKED
                    );
                    des->put_event(e);
                }
            }
            break;
        
            case BLOCKED: {
                // trans to block
                // create an event for when process becomes READY again
                int io_burst = process->rem > 0 ?
                               util->rand(process->IO) : 0;
                // VERBOSE
                if (_verbose) {
                    printf(" ib=%d rem=%d\n", io_burst, process->rem);
                }

                if (process->rem > 0) {
                    Event *e = new Event(
                        current_time + io_burst,
                        process,
                        new_state,
                        READY
                    );
                    des->put_event(e);
                    sched->start_io(current_time);
                } else {
                    // finish process
                    sched->finish(process, current_time);
                }
                sched->set_current_process(nullptr);
                _call_scheduler = true;
            }
            break;

            default:
                break;
        }

        if (_call_scheduler) {
            if (des->get_next_event_time() == current_time) {
                continue;
            }
            _call_scheduler = false;
            if (sched->get_current_process() == nullptr) {
                if (_verbose) {
                    sched->print_process_queue();
                }

                sched->set_current_process(sched->get_next_process());
                if (sched->get_current_process() == nullptr) {
                    continue;
                }
                // create event to make this process runnable for same time.
                Event *e = new Event(current_time, sched->get_current_process(), READY, RUNNING);
                des->put_event(e);
            }
        }
    }
    sched->statistics();
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << "[-v] [-t] [-e] [-p] [-s<schedspec>] inputfile randfile" << std::endl;
        return 1;
    }

    std::vector<string> files;
    std::string sched;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]).substr(0, 2) == "-s") {
            sched = std::string(argv[i]).substr(2);
        } else if (std::string(argv[i]) == "-v") {
            _verbose = true;
        } else if (std::string(argv[i]) == "-t") {
            _verbose_trace = true;
        } else if (std::string(argv[i]) == "-e") {
            _verbose_event_queue = true;
        } else if (std::string(argv[i]) == "-p") {
            _verbose_preempt = true;
        } else {
            files.push_back(std::string(argv[i]));
        }
    }
    fstream in;
    in.open(files[0].c_str());
    util = new Util(files[1]);

    int quantum;
    int maxprio = 4;
    Scheduler *s;
    if (sched == "F") {
        s = new FCFSScheduler();
    } else if (sched == "L") {
        s = new LCFSscheduler();
    } else if (sched == "S") {
        s = new SRTFScheduler();
    } else if (sched.substr(0, 1) == "R") {
        quantum = stoi(sched.substr(1).c_str());
        s = new FCFSScheduler(quantum);
    } else if (sched.substr(0, 1) == "P") {
        if (int idx = sched.find(':') != -1) {
            quantum = stoi(sched.substr(1, idx).c_str());
            maxprio = stoi(sched.substr(idx + 2).c_str());
        } else {
            quantum = stoi(sched.substr(1).c_str());
        }
        s = new PRIOScheduler(quantum, maxprio, false);
    } else if (sched.substr(0, 1) == "E") {
        if (int idx = sched.find(':') != -1) {
            quantum = stoi(sched.substr(1, idx).c_str());
            maxprio = stoi(sched.substr(idx + 2).c_str());
        } else {
            quantum = stoi(sched.substr(1).c_str());
        }
        s = new PRIOScheduler(quantum, maxprio, true);
    }

    DES *des = new DES();
    char line[100];
    int no = 0;
    while (in.getline(line, 100)) {
        char *token = strtok(line, " ");
        int arr[4], i = 0;
        arr[i++] = stoi(token);
        while (token) {
            token = strtok(nullptr, " ");
            if (token) {
                arr[i++] = stoi(token);
            }
        }

        Process *p = new Process(no++, arr[0], arr[1], arr[2], arr[3], util->rand(maxprio));
        Event *e = new Event(
            p->AT,
            p,
            CREATED,
            READY
        );
        des->put_event(e);
    }
    in.close();

    Simulation(des, s);

    return 0;
}