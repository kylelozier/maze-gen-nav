#include "raylib.h"
#include "stdio.h"
#include "defines.h"
#include "raymath.h"
#include "ezmemory.h"

void mazegenrandom(b8 *issquare, i32 *count);
void mazegenfillenclosedspace(b8 *issquare, i32 *count);
b8 mazenav(b8 *issquare);

static const u32 numsquare = 256;
static const u32 numsquareperside = 16;

int main(){
    //initialize window and associated variables.
    const i32 swidth = 700;
    const i32 sheight = 700;
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

    //variables for the square logic array aka maze.
    b8 issquare[numsquare] = {};
    i32 count = 0;
    mazegenrandom(issquare, &count);//for loop setting squares to true/false for use with draw and pathing logic.
    while(!mazenav(issquare)){
        count = 0;
        mazegenrandom(issquare, &count);
    }
    mazegenfillenclosedspace(issquare, &count);

    //math and variables for squares positioning and sides that account for resolution. 1.0f = 1 pixel.
    f32 squarecalcx = (f32)(swidth / (numsquareperside + 2)); //+2 accounts for border.
    f32 squarecalcy = (f32)(sheight / (numsquareperside +2));
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
            mazegenrandom(issquare, &count);
            while(!mazenav(issquare)){
                count = 0;
                mazegenrandom(issquare, &count);
            }
            mazegenfillenclosedspace(issquare, &count);
        }

        squareposdynamic = squarepos; //resets square pos before drawing all squares in frame.
        {
            floop(numsquare){
                if(!issquare[i]) {DrawRectangleV(squareposdynamic, squareside, LIGHTGRAY);}
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

    printf("\n\n%d squares in array. \n%f pos\n%f side", count, squarecalcx, squarecalcy);

    return 0;
}

void mazegenrandom(b8 *issquare, i32 *count){
    *count = 0;
    floop(numsquare){
        i32 randomnum = GetRandomValue(0, 1);
        if(randomnum == 0) {issquare[i] = true; ++*count;}
        else {issquare[i] = false;}
    }//for loop setting squares to true/false for use with draw and pathing logic.

    if(issquare[numsquareperside]){
        issquare[numsquareperside] = false; //make entrance always have 1 path by it.
        --*count;
    }
    if(issquare[((numsquareperside * numsquareperside) - numsquareperside - 1)]) {
        issquare[((numsquareperside * numsquareperside) - numsquareperside - 1)] = false; //makes exit always have 1 path by it.
        --count;
    }
}

void mazegenfillenclosedspace(b8 *issquare, i32 *count){
    b8 fill = true;
    floop(numsquare){
        fill = true;
        if(((i-1) < numsquare) && ((i & (numsquareperside-1)) != 0)){
            if(!issquare[i-1]) {
                fill = false;
            }
        }

        if(((i+1) < numsquare) && ((i & (numsquareperside-1)) != (numsquareperside - 1))){
            if(!issquare[i+1]) {
                fill = false;
            }
        }

        if(((i+numsquareperside) < numsquare)){
            if(!issquare[i+numsquareperside]) {
                fill = false;
            }
        }

        if(((i-numsquareperside) < numsquare)){
            if(!issquare[i-numsquareperside]) {
                fill = false;
            }
        }
        if(fill){issquare[i] = true; ++*count;}
    }
}

b8 mazenav(b8 *issquare){
    u32 start = numsquareperside;
    u32 end = ((numsquareperside * numsquareperside) - numsquareperside - 1);
    b8 hasbeen[numsquare] = {};//checking off squares the algorithm has passed through so no doubling back.
    b8 issolvable = true; //potentially true until proven false.
    b8 solved = false;    //condition true when solved.

    void *navpaths = darray_create(u32); //dynamic array that resizes as u32 points [i] of issquare[] array are added.
    darray_push(navpaths, start); //add start to array to begin a path finding function.

    while((issolvable == true) && (solved == false)){
        u32 navpathstotal = (u32)darray_length(navpaths); //cast darray_length u64 length to u32 for use in floop.
        floop(navpathstotal){
            u32* currentsquare = navpaths;
            u32 current = currentsquare[i];

            if(((current - 1) < numsquare) && ((current & (numsquareperside-1)) != 0)){
                if((!issquare[current - 1]) && (!hasbeen[current - 1])) {
                    if((current - 1) ==  end) {
                        solved = true;
                    }
                    else{
                        darray_push(navpaths, (current - 1));
                    }
                }
            }

            if(((current + 1) < numsquare) && ((current & (numsquareperside-1)) != (numsquareperside - 1))){
                if((!issquare[current + 1]) && (!hasbeen[current + 1])) {
                    if((current + 1) ==  end) {
                        solved = true;
                    }
                    else{
                        darray_push(navpaths, (current + 1));
                    }
                }
            }

            if(((current + numsquareperside) < numsquare)){
                if((!issquare[current + numsquareperside]) && (!hasbeen[current + numsquareperside])) {
                    if((current + numsquareperside) ==  end) {
                        solved = true;
                    }
                    else{
                        darray_push(navpaths, (current + numsquareperside));
                    }
                }
            }

            if(((current - numsquareperside) < numsquare)){
                if((!issquare[current - numsquareperside]) && (!hasbeen[current - numsquareperside])) {
                    if((current - numsquareperside) ==  end) {
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
