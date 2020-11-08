#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void primes() {
    int prime = 0;
    read(0, &prime, 4);
    if (prime == 0) exit(0);

    printf("prime %d\n", prime);
    int p[2];
    int cp = prime;
    pipe(p);
    if (fork() == 0) {
        close(0);
        dup(p[0]);
        close(p[1]);
        primes();
        close(p[0]);
    } else {
        close(p[0]);
        while (read(0, &prime, 4)) {
            if (prime % cp)
                write(p[1], &prime, 4);
        }
        close(p[1]);
        close(0);
        wait(0);
    }
}

int main(int argc, char** argv) {
    int p[2];
    pipe(p);
    printf("prime 2\n");
    if (fork() == 0) {
        close(0);
        dup(p[0]);
        close(p[1]);
        primes();
        close(p[0]);
    } else {
        close(p[0]);
        for (int i = 3; i < 36; i += 2) {
            write(p[1], &i, 4);
        }
        close(p[1]);
        wait(0);
    }
    exit(0);
}