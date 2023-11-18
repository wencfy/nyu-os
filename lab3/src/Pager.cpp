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
