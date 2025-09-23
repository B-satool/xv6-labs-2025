#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char *exec_args[32]; // arguments for -exec
int exec_argc = 0;
int do_exec = 0;

void
rfind(char *path, char *target)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    fd = open(path, 0);
    if(fd < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch(st.type){
    case T_FILE: {
        // extract filename
        p = path + strlen(path);
        while(p >= path && *p != '/')
            p--;
        p++;

        if(strcmp(p, target) == 0){
            if(do_exec){
                // build argv list: exec_args + filename
                char *argv[32];
                int i;
                for(i = 0; i < exec_argc; i++)
                    argv[i] = exec_args[i];
                argv[i++] = path;
                argv[i] = 0;

                if(fork() == 0){
                    exec(argv[0], argv);
                    fprintf(2, "exec %s failed\n", argv[0]);
                    exit(1);
                } else {
                    wait(0);
                }
            } else {
                printf("%s\n", path);
            }
        }
        break;
    }

    case T_DIR:
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
            fprintf(2, "find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0)
                continue;
            if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;

            if(stat(buf, &st) < 0){
                fprintf(2, "find: cannot stat %s\n", buf);
                continue;
            }
            if(st.type == T_DIR){
                rfind(buf, target);
            } else {
                if(strcmp(de.name, target) == 0){
                    if(do_exec){
                        char *argv[32];
                        int i;
                        for(i = 0; i < exec_argc; i++)
                            argv[i] = exec_args[i];
                        argv[i++] = buf;
                        argv[i] = 0;

                        if(fork() == 0){
                            exec(argv[0], argv);
                            fprintf(2, "exec %s failed\n", argv[0]);
                            exit(1);
                        } else {
                            wait(0);
                        }
                    } else {
                        printf("%s\n", buf);
                    }
                }
            }
        }
        break;
    }
    close(fd);
}

int
main(int argc, char *argv[])
{
    if(argc < 3){
        fprintf(2, "Usage: find <path> <filename> [-exec cmd ...]\n");
        exit(1);
    }

    // detect -exec
    for(int i = 3; i < argc; i++){
        if(strcmp(argv[i], "-exec") == 0){
            do_exec = 1;
            exec_argc = argc - (i + 1);
            for(int j = 0; j < exec_argc; j++){
                exec_args[j] = argv[i + 1 + j];
            }
            break;
        }
    }

    rfind(argv[1], argv[2]);
    exit(0);
}

