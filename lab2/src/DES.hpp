#include "scheduler.hpp"

enum STATE {
    CREATED,
    READY,
    RUNNING,
    BLOCKED,
};

std::string state_to_string(STATE s);

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

    int get_next_event_time();
};
