#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define true 1
#define false 0
#define DIMS 16
typedef int bool;


//M should be 9,7 => 9+16*7=121
char world[] = "        c         \n        c         \n        c         \n        c         \n        c         \n        c         \n       Mcc        \n       c c        \n       c c        \n       c c        \n       c c        \n                  \n                  \n                  \n                  \n                  \n                 ";

bool check_node(int index, char direction, int* blacklist){
    if ( world[index]==' ' ){
        printf("%d is empty\n", index);
        return true;  //nothing here
    }else if( index<40 ){
        printf("%d is grounded\n", index);
        return false; //connects to ground
    }else{
        printf("do next\n");
        /*int left = 
        int right = 
        int forward =
        if( check_node(forward) && check_node(left) && check_node(right) ){
            blacklist[0]=index;//blacklist the current node
            return true; //deletable
        }*/
    }
}

/*     ... 3, 2, 1, 0
       ...14,15,16,17
                 ....
                 */

int main(){
    int index = 26;//121;
    printf("%s\n", world);
    //printf("->%c<-\n\n", world[index]);
    
    int* blacklist = (int*)malloc(DIMS*DIMS*sizeof(int));//this can probably be smaller, but meh
    if(blacklist == NULL){ printf("OH NOES!\n"); exit(1); }
    //DIRECTIONS: 0-up,1-left,2-down,3-right
    printf("->%c<-\n\n", world[index]);
    world[index]=' ';
    printf("->%c<-\n\n", world[index]);
    bool up,down,left,right;
    printf("up----------------\n");
    up=check_node(index+DIMS,0,blacklist);
    printf("down---------------\n");
    down=check_node(index-DIMS,2,blacklist);
    printf("left--------------\n");
    left=check_node(index-1,1,blacklist);
    printf("right-------------\n");
    right=check_node(index+1,3,blacklist);
    printf("results:%d,%d,%d,%d.\n",up,down,left,right);
}
