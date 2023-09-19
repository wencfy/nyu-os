#include "fstream"

#include "helper.hpp"
using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Parser *parser = new Parser(argv[1]);
    while (char *token = parser->get_token()) {
        printf("token=<%s>\tposition=%d:%d\n", token, parser->get_line_num(), parser->get_line_offset());
    }

    // string symbol;
    // while ((symbol = parser->read_symbol()) != "") {
    //     printf("%s\n", symbol.c_str());
    // }
    
    return 0;
}