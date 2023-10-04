#include "DES.hpp"

Event::Event(int timestamp, Process *process, STATE old_state, STATE new_state) :
    timestamp(timestamp), process(process), old_state(old_state), new_state(new_state) {}

std::string Event::to_string() {
    std::string ret("event ");
    ret += std::to_string(timestamp);
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