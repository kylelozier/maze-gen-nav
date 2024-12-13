#include "raylib.h"
#include "stdio.h"
#include "defines.h"
#include "raymath.h"
#include "ezmemory.h"

#define NUMSQUAREPERSIDE 32
static const u32 numsquareperside = NUMSQUAREPERSIDE;
static const u32 numsquare = NUMSQUAREPERSIDE * NUMSQUAREPERSIDE;

typedef struct a_maze {
    u32 start;
    u32 end;
    i32 count;
    b8 issquare[(NUMSQUAREPERSIDE * NUMSQUAREPERSIDE)];
} a_maze;

void mazegenrandom(a_maze* maze);
void mazegenfillenclosedspace(a_maze* maze);
void mazegenclean(a_maze* maze);
void mazegenaddnoiseunconnected(a_maze* maze);
void mazegenconnecttempnoise(a_maze* maze, u32* temp);
b8 mazenav(a_maze* maze, u32 navstart, u32 navend);

int main(){
    //initialize window and associated variables.
    const i32 swidth = 1000;
    const i32 sheight = 1000;
    InitWindow(swidth, sheight, "A*Mazing.");
    SetTargetFPS(60);
    if(!IsWindowReady()){
        return 0;
    }

    //TODO: make title screen with input for randomseed.

    //initialize random generator.
    i32 randomseed = (i32)GetTime();
    SetRandomSeed(randomseed);

    initialize_memory(); //used with ezmemory to track memory allocation.

    //variables for the square logic array aka maze initialized in struct.
    static a_maze maze = {numsquareperside, ((numsquareperside * numsquareperside) - numsquareperside - 1), 0, {}};

    mazegenrandom(&maze);//for loop setting squares to true/false for use with draw and pathing logic.
    while(!mazenav(&maze, maze.start, maze.end)){
        maze.count = 0;
        mazegenrandom(&maze);
    }
    mazegenclean(&maze);

    //math and variables for squares positioning and sides that account for resolution. 1.0f = 1 pixel.
    f32 squarecalcx = ((f32)swidth / (f32)(numsquareperside + 2)); //+2 accounts for border.
    f32 squarecalcy = ((f32)sheight / (f32)(numsquareperside +2));
    Vector2 squarepos = {squarecalcx, squarecalcy}; //Pos starts at top left from orgin (0.0f, 0.0f) when using Vector2.
    Vector2 squareside = {squarecalcx, squarecalcy};
    Vector2 squareposdynamic = squarepos; //dynamic positioning that gets reset each loop for draw loop.

    Vector2 greensquare = {0, 2 * squarecalcy};
    Vector2 redsquare = {(numsquareperside + 1) * (squarecalcx), (numsquareperside -1) * squarecalcy};

    /*FRAME LOOP START*/
    while(!WindowShouldClose())   {
        BeginDrawing();
        ClearBackground(DARKGRAY);

        if(IsKeyReleased(KEY_N)){
            mazegenrandom(&maze);
            while(!mazenav(&maze, maze.start, maze.end)){
                maze.count = 0;
                mazegenrandom(&maze);
            }
            mazegenclean(&maze);
        }

        if(IsKeyReleased(KEY_M)){
            mazegenaddnoiseunconnected(&maze);
        }

        squareposdynamic = squarepos; //resets square pos before drawing all squares in frame.
        {
            floop(numsquare){
                if(!maze.issquare[i]) {DrawRectangleV(squareposdynamic, squareside, LIGHTGRAY);}
                //else{DrawRectangleV(squareposdynamic, squareside, LIGHTGRAY);}

                Vector2 addthis = {squarecalcx, 0.0f};
                squareposdynamic = Vector2Add(squareposdynamic, addthis);
                if((i + 1) % numsquareperside == 0) {
                    Vector2 addthis2 = {(-squarecalcx * numsquareperside), squarecalcy};
                    squareposdynamic = Vector2Add(squareposdynamic, addthis2);
                }
            }
        } //for loop draw squares all over screen.

        DrawRectangleV(greensquare, squareside, GREEN);
        DrawRectangleV(redsquare, squareside, RED);

        DrawText("Maze Sulver...", 190, 200, 20, RED);
        DrawFPS(30,30);
        EndDrawing();
    }

    i32 array_length = sizeof(maze);
    printf("\n\n%d squares in array. \n%f pos\n%f side\n%d", maze.count, squarecalcx, squarecalcy, array_length);

    return 0;
}

void mazegenrandom(a_maze *maze){
    maze->count = 0;
    floop(numsquare){
        i32 randomnum = GetRandomValue(0, 1);
        if(randomnum == 0) {maze->issquare[i] = true; ++maze->count;}
        else {maze->issquare[i] = false;}
    }//for loop setting squares to true/false for use with draw and pathing logic.

    if(maze->issquare[numsquareperside]){
        maze->issquare[numsquareperside] = false; //make entrance always have 1 path by it.
        --maze->count;
    }
    if(maze->issquare[((numsquareperside * numsquareperside) - numsquareperside - 1)]) {
        maze->issquare[((numsquareperside * numsquareperside) - numsquareperside - 1)] = false; //makes exit always have 1 path by it.
        --maze->count;
    }
}

void mazegenfillenclosedspace(a_maze *maze){
    b8 fill = true;
    floop(numsquare){
        fill = true;
        if(((i-1) < numsquare) && ((i & (numsquareperside-1)) != 0)){
            if(!maze->issquare[i-1]) {
                fill = false;
            }
        }

        if(((i+1) < numsquare) && ((i & (numsquareperside-1)) != (numsquareperside - 1))){
            if(!maze->issquare[i+1]) {
                fill = false;
            }
        }

        if(((i+numsquareperside) < numsquare)){
            if(!maze->issquare[i+numsquareperside]) {
                fill = false;
            }
        }

        if(((i-numsquareperside) < numsquare)){
            if(!maze->issquare[i-numsquareperside]) {
                fill = false;
            }
        }
        if(fill){ ++maze->count;}
    }
}

void mazegenclean(a_maze *maze){
   mazegenfillenclosedspace(maze);

   floop(numsquare){
       if((!maze->issquare[i]) && (i != maze->end)){
           if(!mazenav(maze, i, maze->end)){
               maze->issquare[i] = true;
               ++maze->count;
           }
       }
   }
}

void mazegenaddnoiseunconnected(a_maze *maze){
    b8 temp[numsquare] = {}; //initialize a temp to store random noise and apply to unconnected areas on issquare array.
    void* outtemp = darray_create(u32); //created to pass array into a function that connects these unconnected noise created tiles with the main path.
    {
        floop(numsquare){
            temp[i] = maze->issquare[i];
        }
    }
    {
        b8 unconnected = true;
        floop(numsquare){
            unconnected = true;
            if(((i-1) < numsquare) && ((i & (numsquareperside-1)) != 0)){
                if(!maze->issquare[i-1]) {
                    unconnected = false;
                }
            }

            if(((i+1) < numsquare) && ((i & (numsquareperside-1)) != (numsquareperside - 1))){
                if(!maze->issquare[i+1]) {
                    unconnected = false;
                }
            }

            if(((i+numsquareperside) < numsquare)){
                if(!maze->issquare[i+numsquareperside]) {
                    unconnected = false;
                }
            }

            if(((i-numsquareperside) < numsquare)){
                if(!maze->issquare[i-numsquareperside]) {
                    unconnected = false;
                }
            }
            if(unconnected){
                i32 randomnum = GetRandomValue(0, 1);
                if(randomnum == 0) {temp[i] = true; --maze->count;}
                else {temp[i] = false; darray_push(outtemp, i)};
            }
        }

        floop(numsquare){
            maze->issquare[i] = temp[i];
        }

        mazegenconnecttempnoise(maze, outtemp);
        mazegenclean(maze);
    }
}

void mazegenconnecttempnoise(a_maze *maze, u32 *temp){
//connects b8 temp[] i values from a passed dynamic array to issquare array.  This carves out more maze in otherwise empty space.
    i32 randomnum = 0;

    floop(darray_length(temp)){
        u32 mazepos = temp[i];
        if(((mazepos - 2) < numsquare) && (((temp[i] - 1) & (numsquareperside-1)) != 0)){
            if(!maze->issquare[mazepos - 2]) {
                randomnum = GetRandomValue(0, 4);
                if(randomnum == 0) {
                    --maze->count;
                    maze->issquare[mazepos - 1] = false;
                }
            }
        }

        if(((mazepos + 2) < numsquare) && (((temp[i] + 1) & (numsquareperside-1)) != (numsquareperside - 1))){
            if(!maze->issquare[mazepos + 2]) {
                randomnum = GetRandomValue(0, 4);
                if(randomnum == 0) {
                    --maze->count;
                    maze->issquare[mazepos + 1] = false;
                }
            }
        }

        if(((mazepos + (numsquareperside * 2)) < numsquare)){
            if(!maze->issquare[mazepos + (numsquareperside * 2)]) {
                randomnum = GetRandomValue(0, 4);
                if(randomnum == 0) {
                    --maze->count;
                    maze->issquare[mazepos + numsquareperside] = false;
                }
            }
        }

        if(((mazepos - (numsquareperside * 2)) < numsquare)){
            if(!maze->issquare[mazepos - (numsquareperside * 2)]) {
                randomnum = GetRandomValue(0, 4);
                if(randomnum == 0) {
                    --maze->count;
                    maze->issquare[mazepos - numsquareperside] = false;
                }
            }
        }
    }

    darray_destroy(temp);
}

b8 mazenav(a_maze *maze, u32 navstart, u32 navend){
    b8 hasbeen[numsquare] = {};//checking off squares the algorithm has passed through so no doubling back.
    b8 issolvable = true; //potentially true until proven false.
    b8 solved = false;    //condition true when solved.

    void *navpaths = darray_create(u32); //dynamic array that resizes as u32 points [i] of issquare[] array are added.
    darray_push(navpaths, navstart); //add start to array to begin a path finding function.

    while((issolvable == true) && (solved == false)){
        u32 navpathstotal = (u32)darray_length(navpaths); //cast darray_length u64 length to u32 for use in floop.
        floop(navpathstotal){
            u32* currentsquare = navpaths;
            u32 current = currentsquare[i];

            if(((current - 1) < numsquare) && ((current & (numsquareperside-1)) != 0)){
                if((!maze->issquare[current - 1]) && (!hasbeen[current - 1])) {
                    hasbeen[current - 1] = true;
                    if((current - 1) ==  navend) {
                        solved = true;
                    }
                    else{
                        darray_push(navpaths, (current - 1));
                    }
                }
            }

            if(((current + 1) < numsquare) && ((current & (numsquareperside-1)) != (numsquareperside - 1))){
                if((!maze->issquare[current + 1]) && (!hasbeen[current + 1])) {
                    hasbeen[current + 1] = true;
                    if((current + 1) ==  navend) {
                        solved = true;
                    }
                    else{
                        darray_push(navpaths, (current + 1));
                    }
                }
            }

            if(((current + numsquareperside) < numsquare)){
                if((!maze->issquare[current + numsquareperside]) && (!hasbeen[current + numsquareperside])) {
                    hasbeen[current + numsquareperside] = true;
                    if((current + numsquareperside) ==  navend) {
                        solved = true;
                    }
                    else{
                        darray_push(navpaths, (current + numsquareperside));
                    }
                }
            }

            if(((current - numsquareperside) < numsquare)){
                if((!maze->issquare[current - numsquareperside]) && (!hasbeen[current - numsquareperside])) {
                    hasbeen[current - numsquareperside] = true;
                    if((current - numsquareperside) ==  navend) {
                        solved = true;
                    }
                    else{
                        darray_push(navpaths, (current - numsquareperside));
                    }
                }
            }

            hasbeen[current] = true;
            darray_pop_at(navpaths, i, navpaths);
            --i;
            --navpathstotal;
            if(darray_length(navpaths) < 1){
                issolvable = false;
            }
        }
    }

    darray_destroy(navpaths);
    return solved;
}
