#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char** argv) {
    int pid;
    int p[2];
    pipe(p);
    pid = getpid();
    if (fork() == 0) {
        pid = getpid();
        close(0);
        dup(p[0]);
        char ch;
        read(0, &ch, 1);
        printf("%d: received ping\n", pid);
        close(p[0]);
        close(p[1]);
    } else {
        close(p[0]);
        write(p[1], "a", 1);
        close(p[1]);
        wait(0);
        printf("%d: received pong\n", pid);
    }
	exit(0);
}
