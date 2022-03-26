#include "../lib/user.h"
#include "../../../kernel/file/fcntl.h"
#include "../../../kernel/file/file.h"

int main(){
    if(open("console", O_RDWR) < 0){
        mknod("console", CONSOLE, 0);
        open("console", O_RDWR);
    }
    char s[] = "\n\nhello, the world.\n\n\n";
    int pid = fork();
    if(pid == 0){
        char child_s[] = "from child\n";
        write(0,s,sizeof(s));
        write(0,child_s,sizeof(child_s));
    }else{
        char parent_s[] = "from parent\n";
        write(0,s,sizeof(s));
        write(0,parent_s,sizeof(parent_s));
        wait(0);
    }
    
    
    exit(0);
}