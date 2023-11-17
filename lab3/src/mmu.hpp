#include "vector"
#include "string"

// page table entrie
typedef struct {
    unsigned present:1;
    unsigned referenced:1;
    unsigned modified:1;
    unsigned write_protect:1;
    unsigned pagedout:1;
    unsigned p_frame:7;

    // self defined keys
    // first bit means checked, second means valid
    unsigned vma_valid:2;
    unsigned file_mapped:1;
} pte_t;

typedef struct {
    int pid;
    int v_page;
    int age;
    bool mapped;
} frame_t;

enum cost {
    cost_read_write          = 1,
    cost_context_switches    = 130,
    cost_process_exits       = 1230,
    cost_maps                = 350,
    cost_unmaps              = 410,
    cost_ins                 = 3200,
    cost_outs                = 2750,
    cost_fins                = 2350,
    cost_fouts               = 2800,
    cost_zeros               = 150,
    cost_segv                = 440,
    cost_segprot             = 410,
};

class Process {
    // virtual memory manager
    typedef struct {
        int start_vpage;
        int end_vpage;
        bool write_protected;
        bool file_mapped;
    } vma;

    typedef struct {
        int unmaps;
        int maps;
        int ins;
        int outs;
        int fins;
        int fouts;
        int zeros;
        int segv;
        int segprot;
    } stats;

public:
    // a per process array of fixed size=64 of pte_t
    std::vector<pte_t> page_table;
    std::vector<vma> vmas;
    stats p_stats;

    Process();
    void add_vma(int start_vpage, int end_vpage, bool write_protected, bool file_mapped);

    std::string to_string();
    void print_page_table();
    void print_p_stats();
};

// virtual base class
class Pager {
public:
    // return the index of the frame to be replaced in the frame_table
    virtual unsigned int select_victim_frame() = 0;
};

class FIFOPager: public Pager {
private:
    int hand = 0;
public:
    FIFOPager();
    unsigned int select_victim_frame();
};

