#include "iostream"
#include "fstream"

#include "DES.hpp"
using namespace std;

bool CALL_SCHEDULER = false;
Process *CURRENT_RUNNING_PROCESS = nullptr;

void Simulation(DES *des, Scheduler *sched) {
    Event *evt;
    while ((evt = des->get_event())) {

        Process *process = evt->process;
        int current_time = evt->timestamp;
        STATE old_state = evt->old_state;
        STATE new_state = evt->new_state;
        // TODO: calculate time.
        // int time_in_prev_state = current_time - process.

        switch (new_state) {
            case READY: {
                if (old_state == BLOCKED || old_state == CREATED) {
                    // trans to ready
                    sched->add_process(process);
                    CALL_SCHEDULER = true;
                } else if (old_state == RUNNING) {
                    // preemption
                    CALL_SCHEDULER = true;
                }
            }
            break;
        
            case RUNNING: {
                // trans to run
                // create event for either preemption or blocking

            }
            break;
        
            case BLOCKED: {
                // trans to block
                CALL_SCHEDULER = true;
            }
            break;

            default:
                break;
        }

        if (CALL_SCHEDULER) {
        cout << evt->to_string() << endl;
            if (des->get_next_event_time() == current_time) {
                continue;
            }
            CALL_SCHEDULER = false;
            if (CURRENT_RUNNING_PROCESS == nullptr) {
                CURRENT_RUNNING_PROCESS = sched->get_next_process();
                if (CURRENT_RUNNING_PROCESS == nullptr) {
                    continue;
                }
                // create event to make this process runnable for same time.
                Event *event = new Event(current_time, process, new_state, RUNNING);
                des->put_event(event);
            }
        }
    }
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

        Process *p = new Process(arr[0], arr[1], arr[2], arr[3]);
        Event *e = new Event(
            p->AT,
            p,
            CREATED,
            READY
        );
        des->put_event(e);
    }

    FCFSScheduler *s = new FCFSScheduler();
    Simulation(des, s);

    return 0;
}