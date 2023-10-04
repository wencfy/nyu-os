#include "scheduler.hpp"

enum STATE {
    CREATED,
    READY,
    RUNNING,
    BLOCKED,
};

class Event {
public:
    int timestamp;
    Process *process;
    STATE old_state;
    STATE new_state;

    Event(int timestamp, Process *process, STATE old_state, STATE new_state);
    std::string to_string();
};

class DES {
private:
    std::list<Event*> event_queue;

public:
    Event *get_event();
    void put_event(Event *event);
    void print_events();
};
