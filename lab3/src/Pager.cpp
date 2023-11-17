#include "mmu.hpp"

extern unsigned int num_frames;

FIFOPager::FIFOPager(): hand(0) {}

unsigned int FIFOPager::select_victim_frame() {
    hand %= num_frames;
    return hand++;
}
