        /*For updating the map, I use what I call a blacklist. I do a depth-first
          search through adjacent blocks, adding each to the blacklist. When the
          deepest node in the branch is reached, it will return true if it's
          surrounded by air and can be deleted or false if it is connected to the
          ground. This trickles back up the branch. If the branch can be deleted,
          we push the blacklist frame up to the blacklist index and move to the
          next direction. If the branch can't be deleted, the blacklist pointer
          jumps back to the blacklist frame, and new nodes are written over the
          previous ones. Whatever is in the blacklist array (from 0 to the frame)
          is then deleted (which could be nothing if the frame remains at -1).
          
          Please note that my algorithm is actually better than required: it can
          delete branches of any shape and could also treat trees as branches (only
          breaking when the full trunk is severed). */
/*        if(update_map){
            ord blacklist[block_count];
            bl_index=-1; bl_frame=-1;
            if(DEBUG) printf("up----------------\n");
            if( check_node(x,y+1,blacklist, world) ){
                if(DEBUG) printf("We can delete this node\n");
                bl_frame=bl_index;//step the frame forward
            }else{
                if(DEBUG) printf("this node is grounded\n");
                bl_index=bl_frame; //rollback blacklist
            }
            
            if(DEBUG) printf("down---------------\n");
            if( check_node(x,y-1,blacklist, world) ){
                if(DEBUG) printf("We can delete this node\n");
                bl_frame=bl_index;
            }else{
                if(DEBUG) printf("this node is grounded\n");
                bl_index=bl_frame;
            }
            
            if(DEBUG) printf("left---------------\n");
            if( check_node(x-1,y,blacklist, world) ){
                if(DEBUG) printf("We can delete this node\n");
                bl_frame=bl_index;
            }else{
                if(DEBUG) printf("this node is grounded\n");
                bl_index=bl_frame;
            }
            
            if(DEBUG) printf("right---------------\n");
            if( check_node(x+1,y,blacklist, world) ){
                if(DEBUG) printf("We can delete this node\n");
                bl_frame=bl_index;
            }else{
                if(DEBUG) printf("this node is grounded\n");
                bl_index=bl_frame;
            }
            trash_blacklist(blacklist, world);
            
            //now remove any orphaned monkeys
            int a,b;
            for(a=0; a<DIMS; a++){
                for(b=0; b<DIMS; b++){
                    if( world[a][b]==-1 && world[a][b-1]==0 ){//orphaned monkey
                        monkey_count--;
                        world[a][b]=0;
                        deleteTile(b,a);
                    }
                }
            }
            
*/ 








/*//=====================OLD WAY==============================
//TODO BUG: two monkeys next to each other
bool check_node(int x, int y, ord* blacklist, char** world){
    if(DEBUG) printf("thing at %d,%d: %d\n", x,y,world[x][y]);
    ord c;
    c.x=x; c.y=y;
    if ( world[x][y] <= 0  ){ //Air or monkey
        //monkeys will be removed after. For now, call them air
        if(DEBUG) printf("%d,%d is empty\n", x,y);
        return true;  //nothing here
    }else if( y==0 ){
        if(DEBUG) printf("%d,%d is grounded\n", x,y);
        return false; //connects to ground
    }else if( in_blacklist(blacklist, c) ){
        return true; //Already checked
    }else{
        if(DEBUG) printf("moving to next node\n");
        blacklist[++bl_index]=c;
        if(        check_node(x,y+1, blacklist, world)      //up
                && check_node(x-1,y, blacklist, world)      //left
                && check_node(x+1,y, blacklist, world)      //right
                && check_node(x,y-1, blacklist, world) ){   //down
            if(DEBUG) printf("%d,%d this branch node goes to air\n",x,y);
            return true;
        }else{
            return false;
        }
    }
}

bool in_blacklist(ord* blacklist, ord c){
    //search to see if the block is already on the list
    if(DEBUG) printf("find in blacklist called\t"); 
    int i=0;
    for(i=0; i<=bl_index; i++){
        ord check = blacklist[i];
        if(check.x==c.x && check.y==c.y){
            if(DEBUG) printf("found\n"); 
            return true;
            break;
        }
    }
    if(DEBUG) printf("not found\n"); 
    return false;
}
void trash_blacklist(ord* blacklist, char** world){
    //go backwards through the blacklist, removing tiles.
    if(DEBUG){
        printf("current blacklist snapshot (%d):\n", bl_index);
        int j;
        for(j=0;j<=bl_index;j++){
            printf("\t\t%d,%d\n", blacklist[j].x,blacklist[j].y);
        }
    }
    int i=bl_index;
    while( i >= 0 ){
        if(DEBUG) printf("\tdeleting %d,%d...\n",blacklist[i].x,blacklist[i].y);
        if( world[ blacklist[i].x ][ blacklist[i].y ]==-1 ) monkey_count--; //decrement monkeylist
        world[ blacklist[i].x ][ blacklist[i].y ]=0;
        deleteTile( blacklist[i].y, blacklist[i].x );
        blacklist[i].y=0;
        blacklist[i].x=0;
        i--;
    }
    if(DEBUG) printf("Done deleteing\n"); 
    bl_index=-1;
}
*/
