#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(2, "usage: %s <command>\n", argv[0]);
        exit(0);
    }

    char* arg[MAXARG];
    
    for (int i = 1; i < argc; i++) arg[i - 1] = argv[i];
    char argument[1000];
    int finish = 1;
    while (finish) {
        int cnt = 0, lst_arg = 0, argv_cnt = argc - 1;
        char ch = 0;
        while (1) {
            finish = read(0, &ch, 1);
            if (finish == 0) exit(0);
            if (ch == ' ' || ch == '\n') {
                argument[cnt++] = 0;
                arg[argv_cnt++] = &argument[lst_arg];
                lst_arg = cnt;
                if (ch == '\n') break;
            } else argument[cnt++] = ch;
        }
        arg[argv_cnt] = 0;
        if (fork() == 0) {
            exec(arg[0], arg);
        } else {
            wait(0);  
        }
  }

    exit(0);
}