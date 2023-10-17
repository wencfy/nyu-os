#include "iostream"
#include "fstream"

#include "DES.hpp"
#include "utils.cpp"
using namespace std;

Util *util = new Util("./rfile");
bool CALL_SCHEDULER = false;

void Simulation(DES *des, Scheduler *sched) {
    Event *evt;
    while ((evt = des->get_event())) {
        printf("\n");

        des->print_events();
        Process *process = evt->process;
        int current_time = evt->timestamp;
        STATE old_state = evt->old_state;
        STATE new_state = evt->new_state;

        int time_in_prev_state = current_time - process->state_trans_time;
        process->state_trans_time = current_time;

        printf("%d %d %d: %s -> %s", current_time, process->no, time_in_prev_state, state_to_string(old_state).c_str(), state_to_string(new_state).c_str());

        delete evt;
        evt = nullptr;

        switch (new_state) {
            case READY: {
                printf("\n");
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
                CALL_SCHEDULER = true;
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
                printf(" cb=%d rem=%d prio=%d\n", cpu_burst, process->rem, process->priority);

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
                printf(" ib=%d rem=%d\n", io_burst, process->rem);

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
                CALL_SCHEDULER = true;
            }
            break;

            default:
                break;
        }

        if (CALL_SCHEDULER) {
            if (des->get_next_event_time() == current_time) {
                continue;
            }
            CALL_SCHEDULER = false;
            if (sched->get_current_process() == nullptr) {
                sched->print_process_queue();

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
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fstream in;
    in.open(argv[1]);

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

        Process *p = new Process(no++, arr[0], arr[1], arr[2], arr[3], util->rand(4));
        Event *e = new Event(
            p->AT,
            p,
            CREATED,
            READY
        );
        des->put_event(e);
    }
    in.close();

    Scheduler *s = new SRTFScheduler();
    Simulation(des, s);

    return 0;
}