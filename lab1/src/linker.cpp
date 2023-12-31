#include "iostream"

#include "helper.hpp"

using namespace std;

void pass1(Parser *parser) {
    int total_ins_count = 0;
    int module_count = 0;

    while (!parser->eof()) {
        int def_count = parser->read_int(true);
        if (def_count == -1) {
            break;
        }
        module_count++;

        if (def_count > 16) {
            // TOO_MANY_DEF_IN_MODULE
            parser->__parse_error(4);
        }

        for (int i = 0; i < def_count; i++) {
            string symbol = parser->read_symbol();
            int val = parser->read_int(false);

            // printf("%s: %d\n", symbol.c_str(), val);
            parser->create_symbol(symbol, total_ins_count + val, module_count);
        }

        int use_count = parser->read_int(false);

        if (use_count > 16) {
            // TOO_MANY_DEF_IN_MODULE
            parser->__parse_error(5);
        }

        for (int i = 0; i < use_count; i++) {
            string symbol = parser->read_symbol();

            // printf("%s\n", symbol.c_str());
        }

        int ins_count = parser->read_int(false);
        total_ins_count += ins_count;
        if (total_ins_count > 512) {
            // TOO_MANY_INSTR
            parser->__parse_error(6);
        }

        for (int i = 0; i < ins_count; i++) {
            char address_mode = parser->read_MARIE();
            int oprand = parser->read_int(false);

            // printf("%c: %d\n", address_mode, oprand);
        }
        // Store the base address for next module
        parser->module_base[module_count] = total_ins_count;
        parser->module_count = module_count;
        parser->check_symbol_address(module_count);
    }
    
    parser->print_symbol_table();
}

void pass2(Parser *parser) {
    int module_count = 0;

    printf("Memory Map\n");

    while (!parser->eof()) {
        int def_count = parser->read_int(true);
        if (def_count == -1) {
            break;
        }
        module_count++;
        for (int i = 0; i < def_count; i++) {
            string symbol = parser->read_symbol();
            int val = parser->read_int(false);
        }
        
        std::vector<pair<std::string, bool>> use_list;
        int use_count = parser->read_int(false);
        for (int i = 0; i < use_count; i++) {
            string symbol = parser->read_symbol();

            use_list.push_back({symbol, false});
        }

        int ins_count = parser->read_int(false);
        for (int i = 0; i < ins_count; i++) {
            char address_mode = parser->read_MARIE();
            int ins_code = parser->read_int(false);

            int opcode = ins_code / 1000;
            int operand = ins_code % 1000;
            
            int cur_no = parser->module_base[module_count - 1] + i;
            std::string message = "";

            if (opcode >= 10) {
                message = " Error: Illegal opcode; treated as 9999";
                opcode = 9;
                operand = 999;
            } else {
                switch (address_mode) {
                    case 'M': {
                        int module_count = parser->module_count;
                        if (operand < module_count) {
                            operand = parser->module_base[operand];
                        } else {
                            operand = 0;
                            message = " Error: Illegal module operand ; treated as module=0";
                        }
                    }
                    break;

                    case 'A': {
                        if (operand < 512) {
                            // do nothing
                        } else {
                            operand = 0;
                            message = " Error: Absolute address exceeds machine size; zero used";
                        }
                    }
                    break;

                    case 'R': {
                        int module_size = parser->module_base[module_count] - parser->module_base[module_count - 1];
                        if (operand < module_size) {
                            operand += parser->module_base[module_count - 1];
                        } else {
                            operand = parser->module_base[module_count - 1];
                            message = " Error: Relative address exceeds module size; relative zero used";
                        }
                    }
                    break;
                    
                    case 'I': {
                        if (operand < 900) {
                            // do nothing
                        } else {
                            operand = 999;
                            message = " Error: Illegal immediate operand; treated as 999";
                        }
                    }
                    break;

                    case 'E': {
                        if (operand < use_list.size()) {
                            std::string symbol = use_list[operand].first;
                            use_list[operand].second = true;
                            int address = parser->get_symbol_address(symbol);
                            if (address != -1) {
                                operand = address;
                            } else {
                                operand = 0;
                                message = " Error: " + symbol + " is not defined; zero used";
                            }
                        } else {
                            operand = parser->module_base[module_count - 1];
                            message = " Error: External operand exceeds length of uselist; treated as relative=0";
                        }
                    }
                    break;
                }
            }

            printf("%03d: %d%03d%s\n", cur_no, opcode, operand, message.c_str());
        }

        for (int i = 0; i < use_list.size(); i++) {
            auto p = use_list[i];
            if (!p.second) {
                printf("Warning: Module %d: uselist[%d]=%s was not used\n", module_count, i, p.first.c_str());
            }
        }
    }

    printf("\n");
    parser->check_used();
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Parser *parser = new Parser(argv[1]);

    pass1(parser);

    parser->refresh();
    printf("\n");

    pass2(parser);

    parser->close();
    
    return 0;
}