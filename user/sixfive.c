#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

// separator characters
char *seps = " -\r\t\n./,";

int
main(int argc, char *argv[])
{
    if(argc < 2){
        fprintf(2, "Usage: sixfive file...\n");
        exit(1);
    }

    for(int i = 1; i < argc; i++){
        int fd = open(argv[i], O_RDONLY);
        if(fd < 0){
            fprintf(2, "sixfive: cannot open %s\n", argv[i]);
            continue;
        }

        char buf[1];
        char numbuf[32];   // to accumulate digits of a number
        int npos = 0;

        while(read(fd, buf, 1) == 1){
            char c = buf[0];
            if(strchr(seps, c)){
                if(npos > 0){
                    numbuf[npos] = '\0';
                    int val = atoi(numbuf);
                    if(val % 5 == 0 || val % 6 == 0){
                        printf("%d\n", val);
                    }
                    npos = 0;
                }
            } else if(c >= '0' && c <= '9'){
                if(npos < sizeof(numbuf)-1){
                    numbuf[npos++] = c;
                }
            } else {
                // non-digit, non-separator → treat like separator
                if(npos > 0){
                    numbuf[npos] = '\0';
                    int val = atoi(numbuf);
                    if(val % 5 == 0 || val % 6 == 0){
                        printf("%d\n", val);
                    }
                    npos = 0;
                }
            }
        }

        // flush last number if file ended with a digit
        if(npos > 0){
            numbuf[npos] = '\0';
            int val = atoi(numbuf);
            if(val % 5 == 0 || val % 6 == 0){
                printf("%d\n", val);
            }
        }

        close(fd);
    }
    exit(0);
}
