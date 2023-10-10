#include "iostream"
#include "fstream"

#include "DES.hpp"
#include "utils.cpp"
using namespace std;

Util *util = new Util("./rfile");
bool CALL_SCHEDULER = false;
Process *CURRENT_RUNNING_PROCESS = nullptr;

void Simulation(DES *des, Scheduler *sched) {
    Event *evt;
    while ((evt = des->get_event())) {
        printf("\n");

        des->print_events();
        Process *process = evt->process;
        int current_time = evt->timestamp;
        STATE old_state = evt->old_state;
        STATE new_state = evt->new_state;
        // TODO: calculate time.
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
                    process->priority = process->static_priority - 1;
                    sched->add_process(process);
                } else if (old_state == RUNNING) {
                    // trans to preemption
                    sched->add_process(process);
                }
                CALL_SCHEDULER = true;
            }
            break;
        
            case RUNNING: {
                // trans to run
                // create event for either preemption or blocking
                int cpu_burst = CURRENT_RUNNING_PROCESS->cpu_burst > 0 ?
                                CURRENT_RUNNING_PROCESS->cpu_burst :
                                util->rand(process->CB);
                cpu_burst = cpu_burst > CURRENT_RUNNING_PROCESS->rem ? CURRENT_RUNNING_PROCESS->rem : cpu_burst;

                // VERBOSE
                printf(" cb=%d rem=%d prio=%d\n", cpu_burst, process->rem, process->priority);

                if (cpu_burst > sched->quantum) {
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
                process->io_time += io_burst;
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
                } else {
                    // finish process
                    sched->finish(process, current_time);
                }
                CURRENT_RUNNING_PROCESS = nullptr;
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
            if (CURRENT_RUNNING_PROCESS == nullptr) {
                sched->print_process_queue();

                CURRENT_RUNNING_PROCESS = sched->get_next_process();
                if (CURRENT_RUNNING_PROCESS == nullptr) {
                    continue;
                }
                CURRENT_RUNNING_PROCESS->cpu_waiting_time += current_time - CURRENT_RUNNING_PROCESS->state_trans_time;
                // create event to make this process runnable for same time.
                Event *e = new Event(current_time, CURRENT_RUNNING_PROCESS, READY, RUNNING);
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