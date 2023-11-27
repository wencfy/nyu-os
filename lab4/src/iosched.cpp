#include "iostream"

#include "iosched.hpp"

std::list<io_request> request_queue;

Scheduler *sched;
int timestamp = 0;
void simulation() {
    while (true) {
        if (!request_queue.empty() && request_queue.front().arr_time == timestamp) {
            // if a new I/O arrived at the system at this current time
            // -> add request to IO-queue
            io_request r = request_queue.front();
            request_queue.pop_front();

            io_task *task = (io_task*) malloc(sizeof(io_task));
            task->track = r.track;
            task->request_time = timestamp;
            sched->add_task(task);
        }

        if (sched->current_running_io_task && sched->current_running_io_task->track == sched->current) {
            // if an IO is active and completed at this time
            sched->finish(timestamp);
            continue;
        }

        if (!sched->current_running_io_task) {
            if (sched->fetch_task()) {
                sched->current_running_io_task->start_time = timestamp;
                continue;
            } else if (request_queue.empty()) {
                sched->total_time = timestamp;
                break;
            }
        }

        if (sched->current_running_io_task) {
            // if an IO is active
            sched->seek();
            sched->busy_time++;
        }
        timestamp++;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << "[ -s<schedalgo> | -v | -q | -f ] <inputfile>" << std::endl;
        return 1;
    }

    std::string file;
    std::string algo;

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]).substr(0, 2) == "-s") {
            algo = std::string(argv[i]).substr(2);

            // "N S L C F"
            switch (algo[0]) {
                case 'N': {
                    sched = new FIFOScheduler();
                    break;
                }

                case 'S': {
                    sched = new SSTFScheduler();
                    break;
                }

                case 'L': {
                    sched = new LookScheduler();
                    break;
                }
            }
        } else if (std::string(argv[i]).substr(0, 2) == "-v") {

        } else if (std::string(argv[i]).substr(0, 2) == "-q") {

        } else {
            file = std::string(argv[i]);
        }
    }

    Loader *loader = new Loader(file);

    io_request r;
    while (loader->get_request(r)) {
        request_queue.push_back(r);
    }
    loader->close();

    simulation();

    sched->statistics();
    
}
