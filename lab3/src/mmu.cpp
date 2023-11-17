#include "iostream"
#include "string"
#include "deque"

#include "utils.cpp"

Utils *util;

std::vector<frame_t> frame_table;
unsigned int num_frames;

std::deque<int> free_list;
std::vector<Process*> process_list;

int current_process = -1;
Pager *pager;

// trace options
bool option_o = false;
bool option_p = false;
bool option_f = false;
bool option_s = false;

// statistics for summary output
unsigned long inst_count;
unsigned long ctx_switches;
unsigned long process_exits;
unsigned long long cost;

// check whether number v_page 
bool check_vma(int v_page) {
    pte_t *pte = &process_list[current_process]->page_table[v_page];
    if (!(pte->vma_valid & 0b10)) {
        pte->vma_valid |= 0b10;
        for (auto v: process_list[current_process]->vmas) {
            if (v_page >= v.start_vpage && v_page <= v.end_vpage) {
                pte->vma_valid |= 0b01;
                pte->write_protect = v.write_protected;
                pte->file_mapped = v.file_mapped;
                break;
            }
        }
    }
    return pte->vma_valid & 0b01;
}

// get frame from free list
unsigned int get_frame(int v_page) {
    unsigned int frame;
    if (!free_list.empty()) {
        frame = free_list.front();
        free_list.pop_front();
    } else {
        frame = pager->select_victim_frame();
        int pid_reversed = frame_table[frame].pid;
        int v_page_reversed = frame_table[frame].v_page;

        if (option_o) {
            printf(" UNMAP %d:%d\n", pid_reversed, v_page_reversed);
        }

        cost += cost_unmaps;
        Process *process = process_list[pid_reversed];
        process->p_stats.unmaps++;
        pte_t *pte = &process->page_table[v_page_reversed];
        pte->present = false;

        if (pte->modified) {
            if (pte->file_mapped) {
                // fout
                if (option_o) {
                    printf(" FOUT\n");
                }
                pte->pagedout = false;
                process->p_stats.fouts++;
                cost += cost_fouts;
            } else {
                // out
                if (option_o) {
                    printf(" OUT\n");
                }
                pte->pagedout = true;
                process->p_stats.outs++;
                cost += cost_outs;
            }
        }
    }

    // maintain the reverse pointers
    frame_table[frame].pid = current_process;
    frame_table[frame].v_page = v_page;
    frame_table[frame].mapped = true;
    return frame;
}

void simulation() {
    char opcode;
    int operand;
    while (util->get_next_instruction(opcode, operand)) {
        if (option_o) {
            printf("%lu: ==> %c %d\n", inst_count++, opcode, operand);
        }

        switch (opcode) {
            case 'c': {
                ctx_switches++;
                current_process = operand;
                cost += cost_context_switches;
                break;
            }
            case 'w':
            case 'r': {
                pte_t *pte = &process_list[current_process]->page_table[operand];
                pte->referenced = true;
                cost += cost_read_write;
                if (!pte->present) {
                    pte->modified = false;

                    // page fault
                    if (check_vma(operand)) {
                        pte->present = true;
                        pte->p_frame = get_frame(operand);

                        if (pte->pagedout) {
                            if (option_o) {
                                printf(" IN\n");
                            }
                            process_list[current_process]->p_stats.ins++;
                            cost += cost_ins;
                        } else if (pte->file_mapped) {
                            if (option_o) {
                                printf(" FIN\n");
                            }
                            process_list[current_process]->p_stats.fins++;
                            cost += cost_fins;
                        } else {
                            if (option_o) {
                                printf(" ZERO\n");
                            }
                            process_list[current_process]->p_stats.zeros++;
                            cost += cost_zeros;
                        }

                        if (option_o) {
                            printf(" MAP %d\n", pte->p_frame);
                        }
                        
                        process_list[current_process]->p_stats.maps++;
                        cost += cost_maps;
                        // age 
                    } else {
                        // vma not valid
                        if (option_o) {
                            printf(" SEGV\n");
                        }
                        process_list[current_process]->p_stats.segv++;
                        cost += cost_segv;
                    }
                }

                if (opcode == 'w') {
                    if (pte->write_protect) {
                        if (option_o) {
                            printf(" SEGPROT\n");
                            process_list[current_process]->p_stats.segprot++;
                            cost += cost_segprot;
                        }
                    } else {
                        pte->modified = true;
                    }
                }
                break;
            }
            case 'e': {
                printf("EXIT current process %d\n", current_process);
                process_exits++;
                cost += cost_process_exits;
                Process *process = process_list[current_process];
                for (int i = 0; i < process->page_table.size(); i++) {
                    pte_t *pte = &process->page_table[i];
                    pte->pagedout = false;
                    if (pte->present) {
                        pte->present = false;
                        if (option_o) {
                            printf(" UNMAP %d:%d\n", current_process, i);
                        }
                        process->p_stats.unmaps++;
                        cost += cost_unmaps;

                        // unmap frame table and maintain free list
                        frame_table[pte->p_frame].mapped = false;
                        free_list.push_back(pte->p_frame);

                        if (pte->file_mapped && pte->modified) {
                            if (option_o) {
                                printf(" FOUT\n");
                            }
                            process->p_stats.fouts++;
                            cost += cost_fouts;
                        }
                    }
                }
            }
        }
    }
}

void init() {
    util->init_process_list();
    frame_table = std::vector<frame_t>(num_frames);
    for (int i = 0; i < num_frames; i++) {
        free_list.push_back(i);
    }
}

void statistics() {
    if (option_p) {
        for (int i = 0; i < process_list.size(); i++) {
            Process *process = process_list[i];
            printf("PT[%d]:", i);
            process->print_page_table();
        }
    }

    if (option_f) {
        printf("FT:");
        for (int i = 0; i < frame_table.size(); i++) {
            if (frame_table[i].mapped) {
                printf(" %d:%d", frame_table[i].pid, frame_table[i].v_page);
            } else {
                printf(" *");
            }
        }
        printf("\n");
    }

    if (option_s) {
        for (int i = 0; i < process_list.size(); i++) {
            printf("PROC[%d]: ", i);
            process_list[i]->print_p_stats();
        }

        printf("TOTALCOST %lu %lu %lu %llu %lu\n",
            inst_count, ctx_switches, process_exits, cost, sizeof(pte_t));
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << "-f<num_frames> -a<algo> [-o<options>] inputfile randomfile" << std::endl;
        return 1;
    }

    std::vector<std::string> files;
    std::string algo;

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]).substr(0, 2) == "-a") {
            algo = std::string(argv[i]).substr(2);

            // f r c e a w
            switch (algo[0]) {
            case 'f': {
                pager = new FIFOPager();
                break;
            }
            
            default:
                break;
            }
        } else if (std::string(argv[i]).substr(0, 2) == "-f") {
            num_frames = std::stoi(std::string(argv[i]).substr(2));
        } else if (std::string(argv[i]).substr(0, 2) == "-o") {
            for (char c: std::string(argv[i]).substr(2)) {
                switch (c) {
                case 'O':
                    option_o = true;
                    break;

                case 'P':
                    option_p = true;
                    break;
                
                case 'F':
                    option_f = true;
                    break;
                
                case 'S':
                    option_s = true;
                    break;
                
                default:
                    break;
                }
            }
        } else {
            files.push_back(std::string(argv[i]));
        }
    }

    util = new Utils(files[0], files[1]);

    init();

    simulation();

    statistics();
}
