#include <iostream>
#include <string>
using namespace std;

char *test(char **bk, bool first) {
    char *token;
    if (first) {
        string s = "hello world hello nihao shijie";
        strcpy(*bk, s.c_str());
        token = strtok_r(*bk, " ", bk);
    } else {
        cout << " token: " << (*bk == nullptr) << endl;
        token = strtok_r(nullptr, " ", bk);
    }
    return token;
}

int main() {
    char *bk;
    char *token = test(&bk, true);
    while (token) {
        cout << token << endl;
        token = test(&bk, false);
    }
}
