#include "DES.hpp"

std::string state_to_string(STATE s) {
    switch (s) {
    case CREATED:
        return "CREATED";
    case READY:
        return "READY";
    case RUNNING:
        return "RUNNING";
    case BLOCKED:
        return "BLOCKED";
    }
}

Event::Event(int timestamp, Process *process, STATE old_state, STATE new_state) :
    timestamp(timestamp), process(process), old_state(old_state), new_state(new_state) {}

std::string Event::to_string() {
    std::string ret("Event, time: ");
    ret += std::to_string(timestamp) + " process: " + std::to_string(process->no) + " old: " + state_to_string(old_state) + " new: " + state_to_string(new_state);
    return ret;
}

void DES::put_event(Event *event) {
    // chronological
    auto it = event_queue.begin();
    while (it != event_queue.end()) {
        if (event->timestamp < (*it)->timestamp) {
            break;
        }
        it++;
    }
    event_queue.insert(it, event);
}

Event *DES::get_event() {
    Event *e = event_queue.front();
    event_queue.pop_front();
    return e;
}

void DES::print_events() {
    for (auto e: event_queue) {
        printf("%s\n", e->to_string().c_str());
    }
}

int DES::get_next_event_time() {
    Event *e = event_queue.front();
    return e ? e->timestamp : -1;
}