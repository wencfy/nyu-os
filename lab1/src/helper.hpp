#include "fstream"
#include "vector"
#include "map"


class Symbol {
public:
    std::string value;
    int address;
    int module;
    bool redef;
    bool used;

    Symbol(std::string symbol, int address, int module);
};

class Parser {
private:
    char *path;
    std::fstream in;
    char *line_start;
    char **save;
    int line_num;
    int line_offset;
    int line_length;

    // symbol => <absolute address, is redefined>
    std::vector<Symbol> symbol_table;

public:
    int module_base[512];
    int module_count;

    Parser(char*);

    void __parse_error(int errcode);

    char *get_token();
    int read_int(bool nullable);
    std::string read_symbol();
    char read_MARIE();

    void create_symbol(std::string symbol, int address, int module);

    int get_line_num();
    int get_line_offset();
    bool eof();
    void refresh();

    void print_symbol_table();
    void check_symbol_address(int module);
    int get_symbol_address(std::string);
    void check_used();

    void close();
    
    // test func
    void print_save();
};
