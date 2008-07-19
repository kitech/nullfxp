#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(int argc, char **argv)
{
    char *vn = 0, *ep = 0;
    FILE *fp = 0;
    char line[1000];
    if(argc == 2) {
        vn = argv[1];
        fp = fopen(vn, "r");
        assert(fp != 0);
        memset(line, 0, sizeof(line));
        while(!feof(fp)) {
            fgets(line, sizeof(line) - 1, fp);
            if(strlen(line) == 0) break;
            vn = strstr(line, "\"NullFXP ");
            if(vn != 0) {
                vn = strchr(vn, ' ');
                assert(vn != 0);
                ep = strstr(vn, " \"");
                assert(ep != 0);
                *ep = '\0';
                printf(vn+1);
                break;
            }
        }
        fclose(fp);
    }else{
        printf("8.8.8");
    }
    return 0;
}
