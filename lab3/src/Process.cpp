#include "mmu.hpp"

Process::Process() {
    p_stats = {0};
    page_table = std::vector<pte_t>(64);
}

void Process::add_vma(int start_vpage, int end_vpage, bool write_protected, bool file_mapped) {
    vma v{start_vpage, end_vpage, write_protected, file_mapped};
    vmas.push_back(v);
}

std::string Process::to_string() {
    std::string ret = "";
    for (int i = 0; i < vmas.size(); i++) {
        vma v = vmas[i];
        ret += std::to_string(v.start_vpage) + " "
             + std::to_string(v.end_vpage) + " "
             + std::to_string(v.file_mapped) + " "
             + std::to_string(v.write_protected) + "\n";
    }
    return ret;
}

void Process::print_page_table() {
    for (int i = 0; i < page_table.size(); i++) {
        pte_t pte = page_table[i];
        if (pte.present) {
            printf(
                " %d:%c%c%c",
                i,
                (pte.referenced ? 'R' : '-'),
                (pte.modified ? 'M' : '-'),
                (pte.pagedout ? 'S' : '-')
            );
        } else {
            printf(" %c", pte.pagedout ? '#' : '*');
        }
    }
    printf("\n");
}

void Process::print_p_stats() {
    printf("U=%d M=%d I=%d O=%d FI=%d FO=%d Z=%d SV=%d SP=%d\n", 
        p_stats.unmaps, p_stats.maps, p_stats.ins,
        p_stats.outs, p_stats.fins, p_stats.fouts,
        p_stats.zeros, p_stats.segv, p_stats.segprot
    );
}
