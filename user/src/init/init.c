#include "../lib/user.h"
#include "../../../kernel/file/fcntl.h"
#include "../../../kernel/file/file.h"

int main(){
    if(open("console", O_RDWR) < 0){
        mknod("console", CONSOLE, 0);
        open("console", O_RDWR);
    }
    char* s = "\n\nhello, the world.\n\n\n";
    write(0,s,23);
    exit(0);
}