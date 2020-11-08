
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char buf[512];
char filename[DIRSIZ + 1];
void find() {
    struct dirent de;
    struct stat st;
    int fd;

    if ((fd = open(buf, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", buf);
        return;
    }
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", buf);
        close(fd);
        return;
    }
    switch(st.type) {
        case T_FILE:
            break;

        case T_DIR:
            if (strlen(buf) + 1 + DIRSIZ + 1 > sizeof buf){
                printf("find: path too long\n");
                break;
            }
            char *p = buf + strlen(buf);
            *p++ = '/';
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                    continue;
                if (de.inum == 0)
                    continue;
                int l = strlen(de.name);
                memmove(p, de.name, strlen(de.name));
                *(p + l) = '\0';
                if (strcmp(de.name, filename) == 0) {
                    printf("%s\n", buf);
                }
                find();
                *p = '\0';
            } 
            *(--p) = '\0';
    }
    close(fd);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("usage: %s <dir> <file>\n", argv[0]);
        exit(1);
    }

    if (strlen(argv[1]) > 511 || strlen(argv[2]) > DIRSIZ) {
        printf("find: path to long.\n");
        exit(1);
    }
    strcpy(buf, argv[1]);
    strcpy(filename, argv[2]);
    find();
    exit(0);
}
