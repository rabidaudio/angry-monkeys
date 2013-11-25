/**********************************
 * Author: Seungcheol Baek
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
#define DEBUG               1
#define WAIT_TIME           20
#define DIMS                39
#define use                 10

/*    int dims = floor(sqrt(world[0]))-1;
    printf("dims:%d",dims);*/



typedef struct Doge{      char
wow                       ;int
         much_code        ;int
    so[use]               ;bool
                 such_fun ;
long long work            ;
             double _wow  ;
} Doge                    ;
//also: http://zachbruggeman.me/dogescript




typedef struct Bomb{
    int x;
    int y;
    float t;
    float vx;
    float vy;
} Bomb;


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
//void run_test_trajectory(int *world);

//my functions:
Bomb* launch(int angle, int power);
bool update_bomb(Bomb* bomb, char *world);
void build_fullworld(char *fworld, int *world);
void waitms(float ms);
void handle_collision(int x, int y, char* world);
void egg();

// Global variables for push buttons
char volatile power=PHIGH, angle=45, fire;
int connection_fd;

//main
int main() {

    START:    
    //variables
    //unsigned char World[BUFFSIZE];
    unsigned char* World = (unsigned char*)malloc(BUFFSIZE);
    //World[0]=12;
    if(World == NULL){
    //if(World[0] != 12){
        printf("BAD WORLD ALLOC\n");
        exit(1);
    }

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
  
    /******** Project 2 *********/    
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
        getworld(&world, World);
        printf("world[0]:%d\nworld[3]:%d\n",world[0],world[3]);
        char* fw = (char*)calloc(world[0]*sizeof(char), sizeof(char));
        build_fullworld(fw, world);


        //clear the terminal
        fflush(stdout);
        rewind(stdout);
        printf("\033[2J\033[1;1H");
        printf("Angry Monkeys\n");
        printf("Push the buttons.\n");
        printf("Z - fire cannon\nX - decrease angle    C - increase angle\nV - toggle power\nR - reset    Q - quit\n");
 
        /****   BEGIN - your code goes here for project 2  ****/
    
        int i, num_cannon=10;
        char pb;
        
        updateShot(0,0,DEL); //reset bomb position
        
        //get pb
        //TODO while(num_cannon>0){
        while(1){
            // it's basically impossible to get a keyboard to function in the same way as mbed pushbuttons so...
            // the get_pb_zxcvqr() function returns the character of the next keyboard button pressed
            pb=get_pb_zxcvqr();
            // and then based on that character, you can do something useful!
            //TODO calculate and send hints
            if(pb=='z'){
                printf("Z was pressed: FIRE!!!\n");
                pb4_hit_callback(); 
                Bomb* b = launch(angle,power);
                //Bomb* b = &bomb;
                updateShot(0,0,DEL);
                bool collision = false;
                while( !collision ){
                    printf("Status:%d\n", collision);
                    collision=update_bomb(b, fw);
                    //printf("time:%f\n", b->t);
                    waitms(WAIT_TIME);
                }
                //TODO update world
                num_cannon--;
                free(b);
            //run_test_trajectory(world);
            } else if(pb=='x'){
                    //printf("X was pressed: decreasing angle\n");
                    pb3_hit_callback(); 
                    if(power==PHIGH)
                            printf("Angle:%d PHIGH\n", angle);
                    else
                            printf("Angle:%d PLOW\n", angle);
            } else if(pb=='c'){
                    //printf("C was pressed: increasing angle\n");
                    pb2_hit_callback(); 
                    if(power==PHIGH)
                            printf("Angle:%d PHIGH\n", angle);
                    else
                            printf("Angle:%d PLOW\n", angle); 
            } else if(pb=='v'){
                    //printf("V was pressed: toggling power\n");
                    pb1_hit_callback(); 
                    if(power==PHIGH)
                            printf("Angle:%d PHIGH\n", angle);
                    else
                            printf("Angle:%d PLOW\n", angle); 
            } else if(pb=='q'){
                printf("EXIT\n");
                    free(world);
                    close(socket_fd);
            exit(1);
            }  else if(pb=='r'){
                    printf("RESTART\n");
                    free(world);
                    close(socket_fd);
                    goto START;
            } else if(pb=='d'){ egg();
            }  else if(pb=='t'){//TODO
                     printf("debug....\n");

            }  else {
                printf("testing\n");
            printf("string1: %s\nstring2: %s\n", "hello", "world");
            printf("int: %d, int: %d\n", world[2], world[3]); 
                    printf("Shots left:%d\n", num_cannon);
                    if(power==PHIGH)
                            printf("Angle:%d PHIGH\n", angle);
                    else
                            printf("Angle:%d PLOW\n", angle);
            }
        }
    
        printf("Out of bombs. Goodbye!");
        //have fun... 
        
        /****    END - your code stops here   ****/
        free(world);  
        free(fw);
        close(socket_fd);
    }
    //end loop

}

Bomb* launch(int angle, int power){
    printf("angle:%d\tpower:%d\n",angle,power);
    Bomb* b = (Bomb *)malloc(sizeof(Bomb));
    b->x=0;
    b->y=0;
    b->t=0;
    b->vx = power*cos(2*PI*angle/360);
    b->vy = power*sin(2*PI*angle/360);
    if(DEBUG){ printf("vx:%f\tvy:%f\n",b->vx,b->vy); }
    return b;
}

bool update_bomb(Bomb* bomb, char *world){
    bomb->t += TICK;
    bomb->y = floor(bomb->vy*bomb->t - 0.5*GRAVITY*bomb->t*bomb->t);
    bomb->x = floor(bomb->vx * bomb->t);
    if(DEBUG){ printf("t:%f\tx:%d\ty:%d\n",bomb->t,bomb->x,bomb->y); }
    updateShot(bomb->y, bomb->x, DEL);
    //check for colisions
    //make colision changes
    if(bomb->x>39 || bomb->x<0 || bomb->y>39 || bomb->y<0){
        return true;
    }else if(world[bomb->x+bomb->y*39]!=0){
        handle_collision(bomb->x, bomb->y, world);
        updateShot(0,0,DEL); //reset bomb position
        return true;
    }else{
        return false;
    }
       
}

void build_fullworld(char* fworld, int* world){

/*
0        Total size of array (#rows *#columns)
1        Number of non-empty squares
i*4+2    Row index of ith non-empty square
i*4+3    Column index of ith non-empty square
i*4+4    Type of object in ith non-empty square
i*4+5    Strength of the ith non-empty square
‘M’, ‘T’, and ‘B’ are ASCII codes 77, 84, and 66. The strength field will be an integer 1-5.
*/

    //use calloc to initalize to 0, then make specified changes
    //air->0, monkey->-1, branch->1..5, tree->6..10 (-5)
    // char is more memory efficient
    int i=0;
    //dims=39;
    /*printf("world[1]: %d\n\n", world[1]);
    for(i=0;i<10;i++){
        printf("%d\n",world[i]);
    }
    printf("\n");*/
    for(i=2;i<world[1]*4+2;i+=4){
        /*printf("%drow:\t%d\n",i,world[i]);
        printf("%dcol:\t%d\n",i,world[i+1]);
        printf("%dtype:\t%d\n",i,world[i+2]);
        printf("%dstrength:\t%d\n",i,world[i+3]);*/
/* -1 means monkey, 0 means nothing, 1-5 means branch, 6-10 means tree*/
        switch( world[i+2] ){
            case 66:
                fworld[ world[i+1]+world[i]*DIMS ]=world[i+3];
                break;
            case 77:
                fworld[ world[i+1]+world[i]*DIMS ]=-1;
                break;
            case 84:
                fworld[ world[i+1]+world[i]*DIMS ]=5+world[i+3];
                break;
            default:
                printf("WARNING! world contained something other than M,T,B\n");
        }
    }
    if(DEBUG){ printf("\n\n27,26: %d\n", fworld[25+27*DIMS]); }
    if (fworld == NULL){
        printf("ohshiz!\n");
        exit(0);
    }
}

    //air->0, monkey->-1, branch->1..5, tree->6..10 (-5)
//colorTile(int row, int column, int strength){
void handle_collision(int x, int y, char* world){
    if(DEBUG){
        //printf("handle_collision called\n");
        printf("thing at %d,%d: %d\n", x,y,world[x+DIMS*y]);
    }
    switch( world[x+DIMS*y] ){
        case -1: //Monkey
        case 1: //Dead branch
        case 6: //dead tree
            world[x+DIMS*y]=0;
            deleteTile(y,x);
            break;
        case 2: //branches
        case 3:
        case 4:
        case 5:
            colorTile(y,x,--world[x+DIMS*y]);
            break;
        case 7: //trees
        case 8:
        case 9:
        case 10:
            colorTile(y,x,(--world[x+DIMS*y])-5);
            break;
        default:
            printf("WTF!?!?");
    }
    
}

void waitms(float ms){
    //can actually only wait intervals of 10ms (so it will round up) :/
    // still better than TIME(NULL), which only updates on seconds
    clock_t start = clock();
    //printf("%f\n",(float)(start));
    float wait = (ms/1000) * CLOCKS_PER_SEC;
    //printf("waiting: %f\n", wait );
    start = clock();
    clock_t now = start;
    while( (float)(now-start) < wait ){
        now=clock();
        //printf("%f\n",(float)(time(NULL)-start));
    }
    //printf("%f\n",(float)(now-start));
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
    printf("World parts: %d\t%d\t%d\t%d\n", World[0],World[1],World[2],World[3]);
    if((World[0]<<8)+World[1]==0){
        printf("ERROR worldsize is 0!\n");
        exit(1);
    }
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
void egg(){ char d[] = {226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150, 145,226,150,145,226,150,132,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150, 145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,132,226,150, 145,226,150,145,226,150,145,226,150,145,10,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226, 150,145,226,150,145,226,150,145,226,150,140,226,150,146,226,150,136,226,150,145,226,150,145,226,150,145,226, 150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,132,226, 150,128,226,150,146,226,150,140,226,150,145,226,150,145,226,150,145,10,226,150,145,226,150,145,226,150,145, 226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,140,226,150,146,226,150,146,226,150,136, 226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,132, 226,150,128,226,150,146,226,150,146,226,150,146,226,150,144,226,150,145,226,150,145,226,150,145,10,226,150, 145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,144,226,150,132,226,150, 128,226,150,146,226,150,146,226,150,128,226,150,128,226,150,128,226,150,128,226,150,132,226,150,132,226,150, 132,226,150,128,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,144,226,150,145,226,150, 145,226,150,145,10,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,132,226,150,132,226, 150,128,226,150,146,226,150,145,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226, 150,146,226,150,146,226,150,146,226,150,136,226,150,146,226,150,146,226,150,132,226,150,136,226,150,146,226, 150,144,226,150,145,226,150,145,226,150,145,10,226,150,145,226,150,145,226,150,145,226,150,132,226,150,128, 226,150,146,226,150,146,226,150,146,226,150,145,226,150,145,226,150,145,226,150,146,226,150,146,226,150,146, 226,150,145,226,150,145,226,150,145,226,150,146,226,150,146,226,150,146,226,150,128,226,150,136,226,150,136, 226,150,128,226,150,146,226,150,140,226,150,145,226,150,145,226,150,145,10,226,150,145,226,150,145,226,150, 144,226,150,146,226,150,146,226,150,146,226,150,132,226,150,132,226,150,146,226,150,146,226,150,146,226,150, 146,226,150,145,226,150,145,226,150,145,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150, 146,226,150,146,226,150,128,226,150,132,226,150,146,226,150,146,226,150,140,226,150,145,226,150,145,10,226, 150,145,226,150,145,226,150,140,226,150,145,226,150,145,226,150,140,226,150,136,226,150,128,226,150,146,226, 150,146,226,150,146,226,150,146,226,150,146,226,150,132,226,150,128,226,150,136,226,150,132,226,150,146,226, 150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,136,226,150,146,226,150,144,226, 150,145,226,150,145,10,226,150,145,226,150,144,226,150,145,226,150,145,226,150,145,226,150,146,226,150,146, 226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,140,226,150,136,226,150,136, 226,150,128,226,150,146,226,150,146,226,150,145,226,150,145,226,150,145,226,150,146,226,150,146,226,150,146, 226,150,128,226,150,132,226,150,140,226,150,145,10,226,150,145,226,150,140,226,150,145,226,150,146,226,150, 132,226,150,136,226,150,136,226,150,132,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150, 146,226,150,146,226,150,146,226,150,146,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150, 145,226,150,146,226,150,146,226,150,146,226,150,146,226,150,140,226,150,145,10,226,150,128,226,150,146,226, 150,128,226,150,144,226,150,132,226,150,136,226,150,132,226,150,136,226,150,140,226,150,132,226,150,145,226, 150,128,226,150,146,226,150,146,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226, 150,145,226,150,145,226,150,145,226,150,145,226,150,146,226,150,146,226,150,146,226,150,144,226,150,145,10, 226,150,144,226,150,146,226,150,146,226,150,144,226,150,128,226,150,144,226,150,128,226,150,146,226,150,145, 226,150,132,226,150,132,226,150,146,226,150,132,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146, 226,150,146,226,150,145,226,150,146,226,150,145,226,150,146,226,150,145,226,150,146,226,150,146,226,150,146, 226,150,146,226,150,140,10,226,150,144,226,150,146,226,150,146,226,150,146,226,150,128,226,150,128,226,150, 132,226,150,132,226,150,146,226,150,146,226,150,146,226,150,132,226,150,146,226,150,146,226,150,146,226,150, 146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,145,226,150,146,226,150,145,226,150,146,226,150, 145,226,150,146,226,150,146,226,150,144,226,150,145,10,226,150,145,226,150,140,226,150,146,226,150,146,226, 150,146,226,150,146,226,150,146,226,150,146,226,150,128,226,150,128,226,150,128,226,150,146,226,150,146,226, 150,146,226,150,146,226,150,146,226,150,146,226,150,145,226,150,146,226,150,145,226,150,146,226,150,145,226, 150,146,226,150,145,226,150,146,226,150,146,226,150,146,226,150,140,226,150,145,10,226,150,145,226,150,144, 226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146, 226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,145,226,150,146,226,150,145,226,150,146, 226,150,145,226,150,146,226,150,146,226,150,132,226,150,146,226,150,146,226,150,144,226,150,145,226,150,145, 10,226,150,145,226,150,145,226,150,128,226,150,132,226,150,146,226,150,146,226,150,146,226,150,146,226,150, 146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,145,226,150,146,226,150, 145,226,150,146,226,150,145,226,150,146,226,150,132,226,150,146,226,150,146,226,150,146,226,150,146,226,150, 140,226,150,145,226,150,145,10,226,150,145,226,150,145,226,150,145,226,150,145,226,150,128,226,150,132,226, 150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226, 150,146,226,150,132,226,150,132,226,150,132,226,150,128,226,150,146,226,150,146,226,150,146,226,150,146,226, 150,132,226,150,128,226,150,145,226,150,145,226,150,145,10,226,150,145,226,150,145,226,150,145,226,150,145, 226,150,145,226,150,145,226,150,128,226,150,132,226,150,132,226,150,132,226,150,132,226,150,132,226,150,132, 226,150,128,226,150,128,226,150,128,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,132, 226,150,132,226,150,128,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,10,226,150,145,226,150, 145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,146,226,150, 146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150,146,226,150, 128,226,150,128,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,226,150,145,0}; printf("\n%s\n",d);}
