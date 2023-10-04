#include "iostream"
#include "fstream"

#include "DES.hpp"
using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fstream in;
    in.open(argv[1]);

    DES *des = new DES();
    char line[100];
    while (in.getline(line, 100)) {
        char *token = strtok(line, " ");
        int arr[4], i = 0;
        arr[i++] = stoi(token);
        while (token) {
            token = strtok(nullptr, " ");
            if (token) {
                arr[i++] = stoi(token);
            }
        }

        Process *p = new Process(arr[0], arr[1], arr[2], arr[3]);
        Event *e = new Event(
            p->AT,
            p,
            CREATED,
            READY
        );
        des->put_event(e);
    }

    return 0;
}