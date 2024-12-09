#include "raylib.h"
#include "stdio.h"
#include "defines.h"
#include "raymath.h"


int main(){
    //initialize window and associated variables.
    const i32 swidth = 850;
    const i32 sheight = 850;
    InitWindow(swidth, sheight, "A*Mazing.");
    SetTargetFPS(60);
    if(!IsWindowReady()){
        return 0;
    }

    //variables for the square logic array.
    const i32 numsquare = 1024;
    const i32 numsquareperside = 32;
    b8 issquare[numsquare] = {};
    i32 count = 0;
    {
        floop(numsquare){
            if(i %2 == 0) {issquare[i] = true; count++;}
            else {issquare[i] = false;}
        }
    } //for loop setting squares to true/false for use with draw and pathing logic.

    //math and variables for squares positioning and sides that account for resolution. 1.0f = 1 pixel.
    f32 squarecalcx = (f32)(swidth / (numsquareperside + 2)); //+2 accounts for border.
    f32 squarecalcy = (f32)(sheight / (numsquareperside +2));
    Vector2 squarepos = {squarecalcx, squarecalcy}; //Pos starts at top left from orgin (0.0f, 0.0f) when using Vector2.
    Vector2 squareside = {squarecalcx, squarecalcy};
    Vector2 squareposdynamic = squarepos; //dynamic positioning that gets reset each loop for draw loop.

    while(!WindowShouldClose())   {
        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        squareposdynamic = squarepos; //resets square pos before drawing all squares in frame.
        {
            floop(numsquare){
                if(issquare[i]) {DrawRectangleV(squareposdynamic, squareside, DARKGRAY);}
                Vector2 addthis = {squarecalcx, 0.0f};
                squareposdynamic = Vector2Add(squareposdynamic, addthis);
                if((i + 1) % numsquareperside == 0) {
                    Vector2 addthis2 = {(-squarecalcx * 32), squarecalcy};
                    squareposdynamic = Vector2Add(squareposdynamic, addthis2);
                }
            }
        } //for loop draw squares all over screen.

        DrawText("Maze Sulver...", 190, 200, 20, RED);
        DrawFPS(30,30);
        EndDrawing();
    }

    printf("\n\n%d squares in array. \n%f pos\n%f side", count, squarecalcx, squarecalcy);

    return 0;
}
