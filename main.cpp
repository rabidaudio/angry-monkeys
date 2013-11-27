/**********************************
 * Author: Charles Julian Knight
 * Institution: Georgia Tech
 *
 * Title: MAIN
 * Class: ECE2035
 * Assignment: Project 2
 **********************************/

//includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <termios.h>

//defines
#define BAUDRATE            9600
#define BUFFSIZE            8192
#define GRAVITY             1.0
#define PHIGH               10
#define PLOW                5
#define PI                  3.141592653589793238462643f
#define ACK                 "ACK"
#define ACK_TIMEOUT         5
#define COM_ADDR            "socks/monkeys_socket"
#define UNIX_PATH_MAX       100

//my defs
#define DEL                 1
#define TICK                .1
#define DEBUG               1//TODO
#define WAIT_TIME           0//20 TODO
#define DIMS                40
#define UP                  0
#define DOWN                2
#define LEFT                1
#define RIGHT               3
#define use                 10



typedef struct Doge{      char
wow                       ;int
         much_code        ;int
    so[use]               ;bool
                 such_fun ;
long long work            ;
             double _wow  ;
} Doge                    ;
//also: http://zachbruggeman.me/dogescript



//self explanatory:
typedef struct Bomb{
    int x;
    int y;
    float t;
    float vx;
    float vy;
} Bomb;

//2D coordinate
typedef struct ord{
    int x;
    int y;
} ord;

//function prototypes
int  invert(int value);
void startGame(void);
int  waitForAck(void);
char get_pb_zxcvqr(void);
void pb1_hit_callback(void);
void pb2_hit_callback(void);
void pb3_hit_callback(void);
void pb4_hit_callback(void);
void getworld (int**world, unsigned char *World);
void updateShot(int row, int column, int del);
void colorTile(int row, int column, int strength);
void deleteTile(int row, int column);
void paaUpdate(int power, int angle);
void hint(int row, int column, int power, int angle);
void run_test_trajectory(int *world);
void egg();



/* I make heavy use of the bool type, even though
   I'm aware it is a C++ (or C99 with a library)
   thing, because it looks prettier and probably
   runs faster than returning a whole 4 byte int
   for a single bit.                           */
   
//Projectile functions
Bomb* launch(int angle, int power);
ord bomb_tick(Bomb* bomb);
bool update_bomb(Bomb* bomb, char** world);
bool handle_collision(int x, int y, char** world);


//Structure destruction functions
void orphan_scan(int x, int y, ord* goodlist, char** world);
bool in_goodlist(ord* goodlist, ord c);
void trash_goodlist(ord* goodlist, char** world);


//Hint functions
void calculate_hint(char** world, bool send_hint);
ord find_branch(int x, int y, char** world, char direction);
int check_guess(int power, int angle, int x, int y);


//Helper functions
void build_fullworld(char** fworld, int *world);
void waitms(float ms);



// Global variables for push buttons
char volatile power=PHIGH, angle=45, fire;
int connection_fd;


int block_count;
ord* blocks;
int monkey_count;

int bl_index=-1;

//TODO char** full_world;


//main
int main() {

    START:    
    //variables
    //unsigned char World[BUFFSIZE];
    //I put this in the heap instead, trying to fix the world[0]=0 issue
    unsigned char* World = (unsigned char*)malloc(BUFFSIZE);

    //socket connection
    struct sockaddr_un address;
    int socket_fd;
    socklen_t address_length;
    pid_t child;

    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0){
        printf("socket() failed\n");
        return 1;
    }

    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    snprintf(address.sun_path, UNIX_PATH_MAX, COM_ADDR);
    address_length = sizeof(address);
    connect(socket_fd, (struct sockaddr *) &address, address_length);
    connection_fd=socket_fd;

    struct timeval timeout;
    timeout.tv_sec = ACK_TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(connection_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  
   
    //loop
    while(1) {
        //synchronize front end display  
        startGame();                            

        //receive World
        recv(connection_fd,World,BUFFSIZE,0); 
        sleep(1);
        fflush(stdout);    
        printf("received\n");
        
        //get world that will be used for your work
        int *world;
        if(DEBUG) printf("World parts: %d\t%d\t%d\t%d\n", World[0],World[1],World[2],World[3]);
        if((World[0]<<8)+World[1]==0){
            /* This fixes the world[0]==0 bug by reopening the connection and trying again. Thanks to
               Colin Sanders for figuring it out: https://piazza.com/class/hkgysqpwsh325z?cid=237  */
            printf("ERROR: world[0] is 0! must be an allocation or communication error. Trying again...\n");
            free(World);
            close(socket_fd);
            goto START; //I hate to do this...
    }
        
        getworld(&world, World);
        //using if(DEBUG) is great, because the compiler will realize it isn't reachable and drop it from the output
        if(DEBUG) printf("world[0]:%d\nworld[3]:%d\n",world[0],world[3]);
        free(World);
        
        /* The sparse array is great for sending over a socket, but not very
           practical. I create a 2D char array of the world from the sparse
           array and use that instead. It makes the searching algorithms alone
           many times easier (and it's not like it's a huge resource cost). */
        if(DEBUG) printf("allocating fworld\n"); 
        char** fw = (char**)malloc(DIMS*sizeof(char*)); // yo, dawg, I heard you like pointers...
        int f;
        for(f=0;f<DIMS;f++){
            fw[f]=(char*)calloc(DIMS*sizeof(char),sizeof(char)); //calloc makes them all zeros, too
            if(fw[f]==NULL){ printf("ERROR ALLOCATING!\n"); exit(1); }
        }
        if(fw==NULL){ printf("ERROR allocating full world!"); exit(1); }
        if(DEBUG) printf("fworld allocated. filling...\n"); 
        build_fullworld(fw, world);
        if(DEBUG) printf("fworld filled.\n"); 


        //clear the terminal
        fflush(stdout);
        rewind(stdout);
        printf("\033[2J\033[1;1H");
        printf("Angry Monkeys\n");
        printf("Push the buttons.\n");
        printf("Z - fire cannon\nX - decrease angle    C - increase angle\nV - toggle power\nH - give me a hint\nR - reset    Q - quit\n");
 
    
        int i, num_cannon=100;//TODO
        char pb;
        
        updateShot(0,0,DEL); //reset bomb position
        calculate_hint(fw, true);
        //get pb
        while(num_cannon>0 && monkey_count>0){
            //much more efficent print statement
            paaUpdate(power, angle);
            if (!DEBUG) printf("\033[A\033[2K");//clear last line (so only numbers update)
            printf("Angle:%d %s\tBombs left:%d Monkeys left:%d\n", angle, (power==PHIGH) ? "PHIGH" : "PLOW", num_cannon, monkey_count);
            // the get_pb_zxcvqr() function returns the character of the next keyboard button pressed
            pb=get_pb_zxcvqr();//why not use getchar() ???
            
            if(pb=='z'){
                /* Make a bomb object (launch), and then update the position and wait until
                   the shot ends. update_bomb will also handle updating the map after.   */
                if(DEBUG) printf("Z was pressed: FIRE!!!\n");
                pb4_hit_callback(); 
                Bomb* b = launch(angle,power);
                updateShot(0,0,DEL);
                bool collision = false;
                while( !collision ){ //bomb is soaring through the air
                    collision=update_bomb(b, fw);
                    waitms(WAIT_TIME); //sexy
                }
                num_cannon--;
                free(b);
            } else if(pb=='x'){
                    if(DEBUG) printf("X was pressed: decreasing angle\n");
                    pb3_hit_callback(); 
            } else if(pb=='c'){
                    if(DEBUG) printf("C was pressed: increasing angle\n");
                    pb2_hit_callback(); 
            } else if(pb=='v'){
                    if(DEBUG) printf("V was pressed: toggling power\n");
                    pb1_hit_callback(); 
            } else if(pb=='q'){
                printf("EXIT\n");
                    free(world);
                    free(fw);
                    close(socket_fd);
            exit(1);
            }  else if(pb=='r'){
                    printf("RESTART\n");
                    free(world);
                    free(fw);
                    close(socket_fd);
                    goto START;
                    /*"For a number of years I have been familiar with the observation that the quality of programmers
                       is a decreasing function of the density of go to statements in the programs they produce. More
                       recently I discovered why the use of the go to statement has such disastrous effects, and I
                       became convinced that the go to statement should be abolished from all "higher level" programming
                       languages (i.e. everything except, perhaps, plain machine code)."   -- Edsger W. Dijkstra, 1968
                                 http://www.u.arizona.edu/~rubinson/copyright_violations/Go_To_Considered_Harmful.html */
            } else if(pb=='d'){ egg();
            } else if(pb=='h'){
                calculate_hint(fw, false);
            }  else {
                printf("Unknown command.\n");
            }
        }
        
        if(monkey_count<=0) printf("Good Job! You got all the monkeys. ");
        else if(num_cannon<=0) printf("You are out of bombs. ");
        printf("Goodbye!\n");


        free(world);  
        free(fw);
        close(socket_fd);
        exit(1);
    }
    //end loop

}



//TODO make pointer to fullworld global, so we don't have to pass it to EVERY FREAKING FUNCTION




/*//==========================PROJECTILE FUNCTIONS===========================//*/

Bomb* launch(int angle, int power){
    //create a Bomb for a given angle and power
    if(DEBUG) printf("angle:%d\tpower:%d\n",angle,power);
    Bomb* b = (Bomb *)malloc(sizeof(Bomb));
    b->x=0;
    b->y=0;
    b->t=0;
    b->vx = power*cos(2*PI*angle/360);
    b->vy = power*sin(2*PI*angle/360);
    if(DEBUG) printf("vx:%f\tvy:%f\n",b->vx,b->vy); 
    return b;
}

ord bomb_tick(Bomb* bomb){
    //move the bomb to the next position
    int x,y;
    bomb->t += TICK; //discrete time intervals of TICK seconds (0.1s works well)
    y = floor(bomb->vy*bomb->t - 0.5*GRAVITY*bomb->t*bomb->t);
    x = floor(bomb->vx * bomb->t);
    bomb->x=x;
    bomb->y=y;
    if(DEBUG) printf("t:%f\tx:%d\ty:%d\n",bomb->t,x,y);
    ord c;
    c.x=x; c.y=y;
    return c;
}


bool update_bomb(Bomb* bomb, char** world){
    //move the bomb one more tick, and return true if the shot is over
    ord c = bomb_tick(bomb);
    int x=c.x;
    int y=c.y;
    updateShot(y, x, DEL);//stupid update shot wants y,x (row, col)
    //check for collisions
    if( x>DIMS-1 || x<0 || y<0 ){// || y>DIMS-1   // TODO can it go off map from above?
        if(DEBUG) printf("Off-screen\n"); 
        updateShot(0,0,DEL); //reset bomb position
        return true;
    //above:
    }else if( y>DIMS-1 ){
        return false;  //nothing to hit up there, so don't try and read from outside the world
    }else if(world[x][y]!=0){
        if(DEBUG) printf("Collision!\n");
        bool update_map = handle_collision(x, y, world);
        updateShot(0,0,DEL); //reset bomb position
        
        if(update_map){
            /*  I had to rewrite my inital map update algorithm because although it was cool,
                it didn't handle trees correctly. This one is less clever but more elegant
                (albeit a little less efficient). I go through each grounded block group (so
                it only runs once for each tree, no matter how wide), and use a depth-first
                recursive search to add all found blocks to the Good List. Then, using the
                Blocks list (a list of all non-air blocks generated during create_fullworld)
                remove any orphaned blocks or monkeys from the map.                        */
        
            //Search
            ord goodlist[block_count]; //he's making a list...
            bl_index=-1;
            int g;  ord o;
            o.x=0; o.y=0;
            for(g=0; g<DIMS; g++){
                o.x=g;
                if(world[g][0]>0 && !in_goodlist(goodlist, o) ){
                   orphan_scan(g,0,goodlist, world);
                }
            }

            //Destroy
            if(DEBUG) printf("%d (of %d) things put on goodlist\n",bl_index,block_count);
            int k;
            for(k=0;k<block_count;k++){//...he's checking it twice
                o=blocks[k];
                if( ((o.x != -1) && (o.y != -1)) && !in_goodlist(goodlist, o)  ){
                    //if it hasn't already been cleared and isn't on the good list
                    deleteTile(o.y,o.x);
                    if(world[o.x][o.y]==-1) monkey_count--;
                    world[o.x][o.y]=0;
                    o.x=-1; o.y=-1; //TODO define nil==-1
                }
            }
        }
        return true;
    }else{
        return false;
    }
}

bool handle_collision(int x, int y, char** world){
    //updates a collision block after a collision, and returns
    //  true if a map update is needed (to check for orphaned branches)
    bool removed=false;
    if(DEBUG) printf("thing at %d,%d: %d\n", x,y,world[x][y]);
    char item= world[x][y];
    switch( world[x][y] ){
        case -1: //Monkey
            monkey_count--; //decrement monkeylist
        case 1: //Dead branch
            world[x][y]=0;
            deleteTile(y,x);
            removed=true;
            break;
        case 6: //dead tree
            while(item>5){//while trees
                world[x][y]=0;
                deleteTile(y,x);
                item=world[x][++y];
            }
            removed=true;
            break;
        case 2: //branches
        case 3:
        case 4:
        case 5:
            colorTile(y,x,--world[x][y]);
            break;
        case 7: //trees
        case 8:
        case 9:
        case 10:
            colorTile(y,x,(--world[x][y])-5);
            break;
        default:
            printf("WTF!?!?\n");
    }
    return removed;
}



/*//==========================STRUCTURE DESTRUCTION FUNCTIONS===========================//*/

void orphan_scan(int x, int y, ord* goodlist, char** world){
    if (DEBUG) printf("Scanning %d,%d (%s%s%s)\n", x, y,
            ((world[x][y]<0) ? "monkey" : ""),
            ((world[x][y]>5) ? "tree" : ""),
            ((world[x][y]>0 && world[x][y]<6) ? "branch" : ""));
    ord c;
    c.x=x; c.y=y;
    if( in_goodlist(goodlist, c) ) return;
    //otherwise...
    switch( world[x][y] ){
        case 0://air
            if(DEBUG) printf("\tjust air\n");
            break;
        case -1://monkey //TODO define AIR and MONKEY
            if( !world[x][y-1]==0 ){//a monkey not on solid ground
                if(DEBUG) printf("\tun orphaned monkey\n");
                goodlist[++bl_index]=c;
            }
            break;
        default:
            if(DEBUG) printf("\tadding to goodlist\n");
            goodlist[++bl_index]=c;
            orphan_scan(x,y+1, goodlist, world);
            orphan_scan(x,y-1, goodlist, world);
            orphan_scan(x+1,y, goodlist, world);
            orphan_scan(x-1,y, goodlist, world);
    }
}

bool in_goodlist(ord* goodlist, ord c){
    //search to see if the block is already on the list
    int i=0;
    for(i=0; i<=bl_index; i++){
        ord check = goodlist[i];
        if(check.x==c.x && check.y==c.y){
            if(DEBUG) printf("found in goodlist\n"); 
            return true;
            break;
        }
    }
    if(DEBUG) printf("not found in goodlist\n"); 
    return false;
}
void trash_goodlist(ord* goodlist, char** world){
    //go backwards through the goodlist, removing tiles.
    if(DEBUG){
        printf("current goodlist snapshot (%d):\n", bl_index);
        int j;
        for(j=0;j<=bl_index;j++){
            printf("\t\t%d,%d\n", goodlist[j].x,goodlist[j].y);
        }
    }
    int i=bl_index;
    while( i >= 0 ){
        if(DEBUG) printf("\tdeleting %d,%d...\n",goodlist[i].x,goodlist[i].y);
        if( world[ goodlist[i].x ][ goodlist[i].y ]==-1 ) monkey_count--; //decrement monkeylist
        world[ goodlist[i].x ][ goodlist[i].y ]=0;
        deleteTile( goodlist[i].y, goodlist[i].x );
        goodlist[i].y=-1;
        goodlist[i].x=-1;
        i--;
    }
    if(DEBUG) printf("Done deleteing\n"); 
    bl_index=-1;
}






/*//==============================HINT FUNCTIONS=============================//*/
void calculate_hint(char** world, bool send_hint){
    //find leftmost monkey
    int x=-1;
    int y=-1;
    int i,j;
    for(i=0; i<DIMS; i++){
        for(j=0;j<DIMS; j++){
            if(world[i][j]==-1){
                x=i; y=j;
                break;
            }
        }
        if(x!=-1 && y!=-1) break;
    }
    if(x<0 || y<0){ printf("Oh man, there aren't no monkeys here! badness 10000\n"); exit(1); }
    if(DEBUG) printf("leftmost monkey: %d,%d\n",x,y);
    
    //find his/her branch
    if(send_hint && world[x][y-1] < 6){
        //if monkey is NOT on a tree, find the branch and use that instead.
        ord branch = find_branch(x, y-1, world, DOWN);
        x=branch.x; y=branch.y;
        if(DEBUG) printf("branch: %d,%d\n",x,y);
    }
    /* Here is a pretty simple, trig-light guessing scheme: The angle is always going to be
       a little higher than the angle to the monkey. We can calculate this pretty quickly
       and then compensate. I ran a lot of data and used linear models to find compensation
       based on the x and y values of the monkey position. These numbers are incredibly
       accurate; it usually isn't off by more than a single degree which is fixed in the
       next phase. The raw data is in hint-linear-model-data.gnumeric                     */
       
    int power=PHIGH; //might as well always use the long shot. The short shot is useless
    int angle=round(atan((double)y / (double)x)*360/(2*PI) + (0.097*y+8.375) + (0.2*x - 5));
    if(DEBUG) printf("Guessing angle %d...\n",angle);
    
    /* I'm not sure why we are guessing. The code can try every case lightning fast.
       If we have to confirm any guess anyway, why waste the effort? Will you actually 
       notice the handful milliseconds you'd save? Nope.                           */
    int guess = check_guess(power, angle, x, y);
    int guess_count=0;
    while( guess != 0 ){
        guess_count++;
        if( guess>0 ) guess=check_guess(power, --angle, x, y);
        if( guess<0 ) guess=check_guess(power, ++angle, x, y);
    }
    if(DEBUG) printf("after %d correction(s), we decided p:%d,a:%d\n",guess_count, power, angle);
    if(send_hint){
        hint(y,x,power,angle);
    }else{
        printf("Try angle %d, power %s\n", angle, power==PHIGH ? "PHIGH" : "PLOW");
    }
}

ord find_branch(int x, int y, char** world, char direction){
    if(DEBUG) printf("checking %d,%d (%s%s%s)\t",x,y,(world[x][y]==0) ? "AIR" : "", (world[x][y]>5) ? "TREE" : "", (world[x][y]<6 && world[x][y]>0) ? "BRANCH" : "");
    ord c;
    c.x=x; c.y=y;
    if( world[x][y]==0 ){
        c.x=-1; c.y=-1; return c;
    }else if( world[x][y]>5 ){ //if a tree, return the caller's ord
        switch( direction ){
            case UP:
                c.y--; break;
            case DOWN:
                c.y++; break;
            case LEFT:
                c.x++; break;
            case RIGHT:
                c.x--; break;
        }
        if(DEBUG) printf("Tree at %d,%d\n",c.x,c.y);
        return c; //I guess that this must be the place (sing into my mouth!)
    }
    if(DEBUG) printf("\n");

    ord up,down,left,right;
    if( direction!=UP ){//down
        down=find_branch(x, y-1, world, DOWN);
        if( down.x>-1 && down.y>-1 ) return down; 
    }
    if( direction!=LEFT ){//right
        right=find_branch(x+1, y, world, RIGHT);
        if( right.x>-1 && right.y>-1 ) return right; 
    }
    if( direction!=RIGHT ){//left
        left=find_branch(x-1, y, world, LEFT);
        if( left.x>-1 && left.y>-1 ) return left; 
    }
    if( direction!=DOWN ){//up
        up=find_branch(x, y+1, world, UP);
        if( up.x>-1 && up.y>-1 ) return up; 
    }
}

int check_guess(int power, int angle, int x, int y){
    Bomb* hint = launch(angle, power);
    int hx = hint->x;
    ord c;
    while( hx<x ){
        c = bomb_tick(hint);
        hx=c.x;
    }
    if (c.y == y){
        if(DEBUG) printf("Yay! Match\n");
        return 0;
    } else if( c.y < y ){
        if(DEBUG) printf("try bigger angle\n");
        return -1;
    } else{
        if(DEBUG) printf("try smaller angle\n");
        return 1;
    }
    free(hint);
}


/*//==============================HELPER FUNCTIONS=============================//*/

void build_fullworld(char** fworld, int* world){
/*
0        Total size of array (#rows *#columns)
1        Number of non-empty squares
i*4+2    Row index of ith non-empty square
i*4+3    Column index of ith non-empty square
i*4+4    Type of object in ith non-empty square
i*4+5    Strength of the ith non-empty square
‘M’, ‘T’, and ‘B’ are ASCII codes 77, 84, and 66. The strength field will be an integer 1-5.
*/
    int i=0, j=0;
    ord o;
    block_count=world[1];
    blocks=(ord*)malloc(block_count*sizeof(ord));
    for(i=2;i<world[1]*4+2;i+=4){
        if(DEBUG){
            printf("%drow:\t%d\n",i,world[i]);
            printf("%dcol:\t%d\n",i,world[i+1]);
            printf("%dtype:\t%d\n",i,world[i+2]);
            printf("%dstrength:\t%d\n",i,world[i+3]);
        }
/* -1 means monkey, 0 means nothing, 1-5 means branch, 6-10 means tree*/
        switch( world[i+2] ){
            case 66:
                fworld[world[i+1]][world[i]]= world[i+3];
                break;
            case 77:
                monkey_count++; //add one to the monkey list
                fworld[world[i+1]][world[i]]= -1;
                break;
            case 84:
                fworld[world[i+1]][world[i]]= 5+world[i+3];
                break;
            default:
                printf("WARNING! world contained something other than M,T,B\n");
        }
        if(DEBUG) updateShot(world[i+1],world[i],0);//visual debugging
        //add to blocks list
        o.x=world[i+1]; o.y=world[i];
        blocks[j++]=o;
    }
    if(DEBUG){ printf("\n\n17,26: %d\n", fworld[26][17]); }
}

void waitms(float ms){
    //can actually only wait intervals of 10ms (so it will round up) :/
    // still better than TIME(NULL) or sleep(), which only update on seconds
    clock_t start = clock();
    float wait = (ms/1000) * CLOCKS_PER_SEC;
    start = clock();
    clock_t now = start;
    while( (float)(now-start) < wait ){
        now=clock();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


/*void run_test_trajectory(int *world){
  //Just for test...
  int i;
  for(i=1;i<10;i++){
    updateShot(i-1,i,0);
    sleep(1);
    hint(i+2, i+3,2,1);
  } 
  
  for(i=0;i<5;i++){       
    deleteTile(world[4*i+2],world[4*i+3]);
    updateShot(i-1,i,1);
  }
}*/

//fcn to send update
void updateShot(int row, int column, int del){
    //temp variables
    char buffer[BUFFSIZE];
    
    //construct message
    sprintf(buffer, "%s-%d-%d-%d;", "update", row, column, del);

    //send message
    send(connection_fd, buffer, strlen(buffer),0);
    waitForAck();
}

//fcn to send color
void colorTile(int row, int column, int strength){
    //temp variables
    char buffer[BUFFSIZE];
    
    //construct message
    sprintf(buffer, "%s-%d-%d-%d;", "color", row, column, strength);

    //send message
    send(connection_fd, buffer, strlen(buffer),0);
    waitForAck();
}    
    
//fcn to send delete
void deleteTile(int row, int column){
    //temp variables
    char buffer[BUFFSIZE];
    
    //construct message
    sprintf(buffer, "%s-%d-%d;", "delete", row, column);

    //send message
    send(connection_fd, buffer, strlen(buffer),0);
    waitForAck();
} 

//fcn to send power and angle
void paaUpdate(int power, int angle){
    //temp variables
    char buffer[BUFFSIZE];
    
    //construct message
    sprintf(buffer, "%s-%d-%d;", "paa", power, angle);

    //send message
    send(connection_fd, buffer, strlen(buffer),0);
    waitForAck();
} 

//fcn to send hint
void hint(int row, int column, int power, int angle){
    //temp variables
    char buffer[BUFFSIZE];
    
    //construct message
    sprintf(buffer, "%s-%d-%d-%d-%d;", "hint", row, column, power, angle);

    //send message
    send(connection_fd, buffer, strlen(buffer),0);
    waitForAck();
}

//fcn to get acknowledgement from serial peripheral
int waitForAck(void) {
    //get acknowlegement
    char buffer[BUFFSIZE];
    double elapsed;
    time_t start;
    time_t now;
    time(&start);
    while(1) {
        memset(&buffer[0],0,strlen(buffer));  
        recv(connection_fd,buffer,BUFFSIZE-1,0);
        if(strncmp(ACK, buffer, strlen(ACK)) == 0) {
            break;
        }
        memset(&buffer[0],0,strlen(buffer));     
        time(&now);
        elapsed = difftime(now, start);
        //printf("%.f, ", elapsed);
        fflush(stdout);
        if(elapsed >= ACK_TIMEOUT)
            return 1;
    }
    return 0;
}

//fcn to initialize the frontend display
void startGame(void) {
    //temp variables
    char buffer[BUFFSIZE];

    //construct message
    sprintf(buffer, "start");

    //send message
    send(connection_fd, buffer, strlen(buffer),0);

    //wait for acknowledgement
    waitForAck();
}

//function to perform bitwise inversion
int invert(int value) {
    if (value == 0) {
        return 1;
    } else {
        return 0;
    }
}

char get_pb_zxcvqr(void) { 
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
            perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
            perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
            perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
            perror ("tcsetattr ~ICANON");
    return (buf);
}


// Callback routine is interrupt activated by a debounced pb hit
void pb1_hit_callback (void) {
    if(power==PHIGH)
        power=PLOW;
    else 
        power=PHIGH;
}
void pb2_hit_callback (void) {
    if(angle<90)
        angle++;
    else if(angle>=90) 
        angle=0;
}
void pb3_hit_callback (void) {
    if(angle>0)
        angle--;
    else if(angle<=0)
        angle=90;
}
void pb4_hit_callback (void) {
    fire++;
}

//func. to get world
void getworld (int**world, unsigned char *World){
    int i;
    char temp[3];
    
    //allocate world
    *world = (int*)malloc(sizeof(int)*(((World[2]<<8)+World[3])*4+2));
    if(*world == NULL){
        printf("ERROR allocating world\n");
        exit(1);
    }
    
    //get it
    (*world)[0]=(World[0]<<8)+World[1];
    (*world)[1]=(World[2]<<8)+World[3];
    for(i=0;i<((*world)[1]*4);i++){
        temp[0] = World[(2*i)+4];
        temp[1] = World[(2*i)+5];
        temp[2] = '\0';   
        sscanf(temp, "%d", &((*world)[i+2]));            
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void egg(){ char d[] = {226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150, 145,226,150,145,226,150,132,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150, 145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,132,226,150, 145,226,150,145,226,150,145,226,150,145,10,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226, 150,145,226,150,145,226,150,145,226,150,140,226,150,146,226,150,136,226,150,145,226,150,145,226,150,145,226, 150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,132,226, 150,128,226,150,146,226,150,140,226,150,145,226,150,145,226,150,145,10,226,150,145,226,150,145,226,150,145, 226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,140,226,150,146,226,150,146,226,150,136, 226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,132, 226,150,128,226,150,146,226,150,146,226,150,146,226,150,144,226,150,145,226,150,145,226,150,145,10,226,150, 145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,144,226,150,132,226,150, 128,226,150,146,226,150,146,226,150,128,226,150,128,226,150,128,226,150,128,226,150,132,226,150,132,226,150, 132,226,150,128,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,144,226,150,145,226,150, 145,226,150,145,10,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,132,226,150,132,226, 150,128,226,150,146,226,150,145,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226, 150,146,226,150,146,226,150,146,226,150,136,226,150,146,226,150,146,226,150,132,226,150,136,226,150,146,226, 150,144,226,150,145,226,150,145,226,150,145,10,226,150,145,226,150,145,226,150,145,226,150,132,226,150,128, 226,150,146,226,150,146,226,150,146,226,150,145,226,150,145,226,150,145,226,150,146,226,150,146,226,150,146, 226,150,145,226,150,145,226,150,145,226,150,146,226,150,146,226,150,146,226,150,128,226,150,136,226,150,136, 226,150,128,226,150,146,226,150,140,226,150,145,226,150,145,226,150,145,10,226,150,145,226,150,145,226,150, 144,226,150,146,226,150,146,226,150,146,226,150,132,226,150,132,226,150,146,226,150,146,226,150,146,226,150, 146,226,150,145,226,150,145,226,150,145,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150, 146,226,150,146,226,150,128,226,150,132,226,150,146,226,150,146,226,150,140,226,150,145,226,150,145,10,226, 150,145,226,150,145,226,150,140,226,150,145,226,150,145,226,150,140,226,150,136,226,150,128,226,150,146,226, 150,146,226,150,146,226,150,146,226,150,146,226,150,132,226,150,128,226,150,136,226,150,132,226,150,146,226, 150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,136,226,150,146,226,150,144,226, 150,145,226,150,145,10,226,150,145,226,150,144,226,150,145,226,150,145,226,150,145,226,150,146,226,150,146, 226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,140,226,150,136,226,150,136, 226,150,128,226,150,146,226,150,146,226,150,145,226,150,145,226,150,145,226,150,146,226,150,146,226,150,146, 226,150,128,226,150,132,226,150,140,226,150,145,10,226,150,145,226,150,140,226,150,145,226,150,146,226,150, 132,226,150,136,226,150,136,226,150,132,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150, 146,226,150,146,226,150,146,226,150,146,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150, 145,226,150,146,226,150,146,226,150,146,226,150,146,226,150,140,226,150,145,10,226,150,128,226,150,146,226, 150,128,226,150,144,226,150,132,226,150,136,226,150,132,226,150,136,226,150,140,226,150,132,226,150,145,226, 150,128,226,150,146,226,150,146,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226, 150,145,226,150,145,226,150,145,226,150,145,226,150,146,226,150,146,226,150,146,226,150,144,226,150,145,10, 226,150,144,226,150,146,226,150,146,226,150,144,226,150,128,226,150,144,226,150,128,226,150,146,226,150,145, 226,150,132,226,150,132,226,150,146,226,150,132,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146, 226,150,146,226,150,145,226,150,146,226,150,145,226,150,146,226,150,145,226,150,146,226,150,146,226,150,146, 226,150,146,226,150,140,10,226,150,144,226,150,146,226,150,146,226,150,146,226,150,128,226,150,128,226,150, 132,226,150,132,226,150,146,226,150,146,226,150,146,226,150,132,226,150,146,226,150,146,226,150,146,226,150, 146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,145,226,150,146,226,150,145,226,150,146,226,150, 145,226,150,146,226,150,146,226,150,144,226,150,145,10,226,150,145,226,150,140,226,150,146,226,150,146,226, 150,146,226,150,146,226,150,146,226,150,146,226,150,128,226,150,128,226,150,128,226,150,146,226,150,146,226, 150,146,226,150,146,226,150,146,226,150,146,226,150,145,226,150,146,226,150,145,226,150,146,226,150,145,226, 150,146,226,150,145,226,150,146,226,150,146,226,150,146,226,150,140,226,150,145,10,226,150,145,226,150,144, 226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146, 226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,145,226,150,146,226,150,145,226,150,146, 226,150,145,226,150,146,226,150,146,226,150,132,226,150,146,226,150,146,226,150,144,226,150,145,226,150,145, 10,226,150,145,226,150,145,226,150,128,226,150,132,226,150,146,226,150,146,226,150,146,226,150,146,226,150, 146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,145,226,150,146,226,150, 145,226,150,146,226,150,145,226,150,146,226,150,132,226,150,146,226,150,146,226,150,146,226,150,146,226,150, 140,226,150,145,226,150,145,10,226,150,145,226,150,145,226,150,145,226,150,145,226,150,128,226,150,132,226, 150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226, 150,146,226,150,132,226,150,132,226,150,132,226,150,128,226,150,146,226,150,146,226,150,146,226,150,146,226, 150,132,226,150,128,226,150,145,226,150,145,226,150,145,10,226,150,145,226,150,145,226,150,145,226,150,145, 226,150,145,226,150,145,226,150,128,226,150,132,226,150,132,226,150,132,226,150,132,226,150,132,226,150,132, 226,150,128,226,150,128,226,150,128,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,132, 226,150,132,226,150,128,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,10,226,150,145,226,150, 145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,146,226,150, 146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150, 128,226,150,128,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150, 145,10,116,121,112,101,100,101,102,32,115,116,114,117,99,116,32,68,111,103,101,123,32,32,32,32,32,32,99,104,97, 114,10,119,111,119,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,59,105,110,116,10,32, 32,32,32,32,32,32,32,32,109,117,99,104,95,99,111,100,101,32,32,32,32,32,32,32,32,59,105,110,116,10,32,32,32, 32,115,111,91,117,115,101,93,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,59,98,111,111,108,10,32,32,32,32,32, 32,32,32,32,32,32,32,32,32,32,32,32,115,117,99,104,95,102,117,110,32,59,10,108,111,110,103,32,108,111,110,103, 32,119,111,114,107,32,32,32,32,32,32,32,32,32,32,32,32,59,10,32,32,32,32,32,32,32,32,32,32,32,32,32,100,111, 117,98,108,101,32,95,119,111,119,32,32,59,10,125,32,68,111,103,101,32,32,32,32,32,32,32,32,32,32,32,32,32,32, 32,32,32,32,32,32,59,0}; printf("\n%s\n\n\n",d);}
