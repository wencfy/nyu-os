#include "helper.hpp"
#include "string.h"

void Parser::__parse_error(int errcode) {
    static const char* errstr[] = {
        "NUM_EXPECTED",             // Number expect, anything >= 2^30 is not a number either
        "SYM_EXPECTED",             // Symbol Expected
        "MARIE_EXPECTED",           // Addressing Expected which is M/A/R/I/E
        "SYM_TOO_LONG",             // Symbol Name is too long
        "TOO_MANY_DEF_IN_MODULE",   // > 16
        "TOO_MANY_USE_IN_MODULE",   // > 16
        "TOO_MANY_INSTR",           // total num_instr exceeds memory size (512)
    };
    printf("Parse Error line %d offset %d: %s\n", line_num, line_offset, errstr[errcode]);
    exit(0);
}

int isMARIE(char token) {
    return token == 'M' ||
           token == 'A' ||
           token == 'R' ||
           token == 'I' ||
           token == 'E';
}

Parser::Parser(char *path) {
    this->path = (char *) malloc((strlen(path) + 1) * sizeof(char));
    strcpy(this->path, path);
    in.open(path);

    line_num = 0;
    line_offset = 0;
    save = (char**) malloc(sizeof(char*));
    module_base[0] = 0;
    module_count = 0;
};

char* Parser::get_token() {
    const char sep[] = " \t\n";

    if (*save == nullptr || !strlen(*save)) {
        if (in) {
            std::string line;
            do {
                if (getline(in, line)) {
                    if (!in.eof()) {
                        line_num++;
                        line_length = 0;
                    } else {
                        line_length = line.length();
                    }
                }
                line.erase(line.find_last_not_of(sep) + 1);
            } while (!line.length() && in);

            *save = (char*) malloc((line.length() + 1) * sizeof(char));

            strcpy(*save, line.c_str());

            line_start = *save;
        }

    }

    char *token = strtok_r(*save, sep, save);
    if (token) {
        line_offset = token - line_start + 1;
    } else {
        line_offset = line_length + 1;
        line_num++;
    }

    // printf("%s --- save: %s\n", token, *save);
    return token;
}

int Parser::read_int(bool nullable) {
    char *token = get_token();

    if (!token) {
        if (nullable) {
            return -1;
        }
        // NUM_EXPECTED: nothing provided
        __parse_error(0);
    }

    int ret = 0;
    for (int i = 0; *(token + i); i++) {
        if (!isdigit(token[i])) {
            // NUM_EXPECTED: not a digit
            __parse_error(0);
        }
        ret = ret * 10 + (*(token + i) - '0');
    }
    return ret;
}

std::string Parser::read_symbol() {
    char *token = get_token();

    if (!token || !isalpha(*(token))) {
        // SYM_EXPECTED: nothing provided
        __parse_error(1);
    }

    for (int i = 0; *(token + i); i++) {
        if (!isalnum(token[i])) {
            // SYM_EXPECTED: not alnum
            __parse_error(1);
        }
    }

    char* token_cp = (char*) malloc((strlen(token) + 1) * sizeof(char));
    strcpy(token_cp, token);
    std::string symbol = token_cp;
    if (symbol.length() > 16) {
        // SYM_TOO_LONG
        __parse_error(3);
    }
    return symbol;
}

char Parser::read_MARIE() {
    char *token = get_token();

    if (!token || !isMARIE(*(token)) || *(token + 1)) {
        // MARIE_EXPECTED: not MARIE
        __parse_error(2);
    }
    char instruction = *token;
    return instruction;
}

Symbol::Symbol(std::string symbol, int address, int module) {
    this->value = symbol;
    this->address = address;
    this->module = module;
    this->redef = false;
    this->used = false;
}

void Parser::create_symbol(std::string symbol, int address, int module) {
    auto it = symbol_table.begin();
    for (; it != symbol_table.end(); it++) {
        if ((*it).value == symbol) {
            break;
        }
    }
    if (it != symbol_table.end()) {
        (*it).redef = true;

        printf("Warning: Module %d: %s redefinition ignored\n", module, symbol.c_str());
    } else {
        symbol_table.push_back(*new Symbol(symbol, address, module));
    }
}

int Parser::get_line_num() {
    return line_num;
}

int Parser::get_line_offset() {
    return line_offset;
}

bool Parser::eof() {
    return in.eof();
}

void Parser::refresh() {
    // this->in.close();
    // this->in.open(this->path);
    this->in.clear();
    this->in.seekg(0, std::ios::beg);
}

void Parser::print_symbol_table() {
    printf("Symbol Table\n");

    for (Symbol symbol: symbol_table) {
        if (symbol.redef) {
            printf("%s=%d Error: This variable is multiple times defined; first value used\n", symbol.value.c_str(), symbol.address);
        } else {
            printf("%s=%d\n", symbol.value.c_str(), symbol.address);
        }
    }
}

void Parser::check_symbol_address(int module) {
    int ins_count = module_base[module] - module_base[module - 1] - 1;
    for (auto it = symbol_table.begin(); it != symbol_table.end(); it++) {
        if ((*it).module == module) {
            int r_address = (*it).address - module_base[module - 1];
            if (r_address > ins_count) {
                printf("Warning: Module %d: %s too big %d (max=%d) assume zero relative\n", module, (*it).value.c_str(), r_address, ins_count);
                (*it).address = module_base[module - 1];
            }
        }
    }
}

int Parser::get_symbol_address(std::string symbol) {
    for (int i = 0; i < symbol_table.size(); i++) {
        if (symbol_table[i].value == symbol) {
            symbol_table[i].used = true;
            return symbol_table[i].address;
        }
    }
    return -1;
}

void Parser::check_used() {
    for (Symbol s: symbol_table) {
        if (!s.used) {
            printf("Warning: Module %d: %s was defined but never used\n", s.module, s.value.c_str());
        }
    }
}

void Parser::close() {
    in.close();
}

void Parser::print_save() {
    printf("print save --- %s\n", *save);
}
