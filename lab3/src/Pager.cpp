#include "utils.cpp"

extern unsigned int num_frames;
extern Utils *util;

extern unsigned long inst_count;

FIFOPager::FIFOPager(): hand(0) {}

unsigned int FIFOPager::select_victim_frame() {
    hand %= num_frames;
    return hand++;
}


RandomPager::RandomPager() {}

unsigned int RandomPager::select_victim_frame() {
    int r = util->rand(num_frames);
    return r;
}


ClockPager::ClockPager(): hand(0) {}

unsigned int ClockPager::select_victim_frame() {
    hand %= num_frames;
    while (true) {
        pte_t *pte = util->get_pte_from_frame(hand);
        if (pte->referenced) {
            pte->referenced = false;
        } else {
            return hand++;
        }
        hand = (hand + 1) % num_frames;
    }
}


NRUPager::NRUPager(): hand(0), last_rst(-1) {}

unsigned int NRUPager::select_victim_frame() {
    int classes[4] = {-1, -1, -1, -1};
    int i = hand;
    do {
        pte_t *pte = util->get_pte_from_frame(i);
        int class_index = 2 * pte->referenced + pte->modified;
        if (classes[class_index] == -1) {
            classes[class_index] = i;
        }
        if (inst_count - last_rst >= reset_cnt) {
            pte->referenced = false;
        } else if (class_index == 0) {
            break;
        }
        i = (i + 1) % num_frames;
    } while (hand != i);

    if (inst_count - last_rst >= reset_cnt) {
        last_rst = inst_count;
    }

    unsigned victim = -1;
    for (int i = 0; i < 4; i++) {
        if (classes[i] != -1) {
            victim = classes[i];
            hand = (victim + 1) % num_frames;
            break;
        }
    }

    return victim;
}


AgingPager::AgingPager(): hand(0) {}

unsigned int AgingPager::select_victim_frame() {
    unsigned int min_idx = hand;
    unsigned int i = hand;

    do {
        frame_table[i].age >>= 1;
        pte_t *pte = util->get_pte_from_frame(i);
        if (pte->referenced) {
            frame_table[i].age = (frame_table[i].age | 0x80000000);
            pte->referenced = false;
        }

        if (frame_table[i].age < frame_table[min_idx].age) {
            min_idx = i;
        }
        i = (i + 1) % num_frames;
    } while (i != hand);

    hand = (min_idx + 1) % num_frames;
    return min_idx;
}


WorkingSetPager::WorkingSetPager(): hand(0) {}

unsigned int WorkingSetPager::select_victim_frame() {
    int i = hand;
    int oldest_idx = hand;
    do {
        pte_t *pte = util->get_pte_from_frame(i);

        if (pte->referenced) {
            frame_table[i].time_last_used = inst_count;
            pte->referenced = false;
        } else if (inst_count - frame_table[i].time_last_used > tau) {
            oldest_idx = i;
            break;
        }

        if (frame_table[i].time_last_used < frame_table[oldest_idx].time_last_used) {
            oldest_idx = i;
        }
        i = (i + 1) % num_frames;
    } while (i != hand);

    hand = (oldest_idx + 1) % num_frames;
    return oldest_idx;
}
