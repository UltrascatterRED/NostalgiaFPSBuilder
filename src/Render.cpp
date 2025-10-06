#include <stdio.h>
#include <math.h>
#include <GL/glut.h>
#include "Render.h"

void drawPixel(int x, int y, int color)
{
  // rgb[0] is red
  // rgb[1] is green
  // rgb[2] is blue
  int rgb[3];
  switch (color) 
  {
    case 0:
      // pure red
      rgb[0]=255; rgb[1]=0; rgb[2]=0;
      break;
    case 1:
      // dark red
      rgb[0]=160; rgb[1]=0; rgb[2]=0;
      break;
    case 2:
      // pure green
      rgb[0]=0; rgb[1]=255; rgb[2]=0;
      break;
    case 3:
      // dark green
      rgb[0]=0; rgb[1]=160; rgb[2]=0;
      break;
    case 4:
      // pure blue
      rgb[0]=0; rgb[1]=0; rgb[2]=255;
      break;
    case 5:
      // dark blue
      rgb[0]=0; rgb[1]=0; rgb[2]=160;
      break;
    case 6:
      // pure yellow
      rgb[0]=255; rgb[1]=255; rgb[2]=0;
      break;
    case 7:
      // dark yellow
      rgb[0]=160; rgb[1]=160; rgb[2]=0;
      break;
    case 8:
      // background color of choice (default: dark gray)
      rgb[0]=75; rgb[1]=75; rgb[2]=75;
      break;
    default:
      // defaults to white
      rgb[0]=255; rgb[1]=255; rgb[2]=255;
      break;
  }
  glColor3ub(rgb[0], rgb[1], rgb[2]);
  glBegin(GL_POINTS);
  glVertex2i(x*PIXEL_SCALE+2, y*PIXEL_SCALE+2);
  glEnd();
}

// draws singular, flat color to the screen
void clearBackground() 
{
  int x,y;
  for(y=0;y<SCREEN_HEIGHT;y++)
  { 
    for(x=0;x<SCREEN_WIDTH;x++)
    { 
      drawPixel(x,y,8);
    } 
  }	
}

// Clips the 3D points of a partially visible wall to the current view
// of the camera. This prevents unwanted rendering errors.
// Only the first point is passed by reference because these are the only
// values being altered.
// Params: wall points in 3D space
void clipBehindCamera(int *x1, int *y1, int *z1, int x2, int y2, int z2)
{
  // store starting values of y1, y2 for use in calculation of instersect factor 
  // distance plane is difference in y coords, therefore perpendicular to plane of
  // camera view, which always faces positive y
  float distPlane_a = *y1; // always <=0
  float distPlane_b = y2;
  float distPlane = distPlane_a - distPlane_b; // always negative
  if(distPlane == 0) { distPlane = 1; } //prevent divide by 0; 
  // represents proportion of visible wall; always between 0 and 1.
  // Used to calculate x1, y1, z1 transforms
  float intersectFactor = distPlane_a / distPlane;
  // transform x1, y1, z1 to position corresponding to camera view boundary 
  *x1 = *x1 + intersectFactor * (x2-(*x1));
  *y1 = *y1 + intersectFactor * (y2-(*y1));
  if(*y1 == 0) { *y1 = 1; } // prevent divide by 0 in transformation to screen space
  *z1 = *z1 + intersectFactor * (z2-(*z1));
}

// draws a singular wall on the screen. All coordinate values handled
// by this function are in SCREEN SPACE, not 3D space.
// DEV NOTE: Refactor this function to handle negative delta values.
//           This will make all walls drawable in one pass, regardless
//           of which side is facing the player camera (aka no more culling)
void drawWall(int x1, int x2, int by1, int by2, int ty1, int ty2, int color)
{
  // debug
  // printf("DRAWING A WALL\n");
  // printf("***************************************\n");
  // printf("DrawWall params: x1 = %d, x2 = %d\n", x1, x2);
  // end debug
  int swp;
  // swap each coord with its counterpart if x2 is bigger.
  // this is done to always draw the wall regardless of orientation
  // (in other words, never cull walls)
  if(x1 > x2)
  {
    swp = x1; x1 = x2; x2 = swp;
    swp = by1; by1 = by2; by2 = swp;
    swp = ty1; ty1 = ty2; ty2 = swp;
  }
  // "by" is short for bottom y, aka y coords of bottom edge of wall
  int delta_by = by2 - by1;
  // "ty" is likewise short for top y, the y coords of top edge of wall
  int delta_ty = ty2 - ty1;
  // set delta_x to 1 if 0 to prevent dividing by zero
  int delta_x = x2 - x1; if(delta_x == 0) { delta_x = 1; }
  int x_start = x1; // store initial x1 value, which is needed for wall drawing

  // clip x1, x2 to player view in order to still draw wall. Without this
  // operation, the points may end up out-of-view of the screen, and the wall
  // cannot be drawn.
  // NOTE: clipping to 1 and screen width - 1 is optional; this is done as a
  //       simple debug behavior to confirm clipping by leaving a 1px margin
  //       on all sides of the screen. Clipping to 0 and screen width is also
  //       fine.
  if(x1 < 1) { x1 = 1; }
  if(x1 > SCREEN_WIDTH - 1) { x1 = SCREEN_WIDTH - 1; }
  if(x2 < 1) { x2 = 1; }
  if(x2 > SCREEN_WIDTH - 1) { x2 = SCREEN_WIDTH - 1; }

  for(int x = x1; x < x2; x++)
  {
    // 0.5 needed for "rounding issues", investigate further.
    // Current conjecture is that this value "nudges" the current coordinates
    // to the correct placement before int truncation
    // by represents current bottom y coord for the vertical line to be drawn
    int by = delta_by * (x - x_start + 0.5) / delta_x + by1;
    int ty = delta_ty * (x - x_start + 0.5) / delta_x + ty1;
    // printf("Bottom Y = %d, Top Y = %d\n", by, ty); // debug 
    // clip by, ty to player view. Same reasoning as x clipping above.
    if(by < 1) { by = 1; }
    if(by > SCREEN_HEIGHT - 1) { by = SCREEN_HEIGHT - 1; }
    if(ty < 1) { ty = 1; }
    if(ty > SCREEN_HEIGHT - 1) { ty = SCREEN_HEIGHT - 1; }
    
    // draw between current bottom point and top point, creating a vertical line of
    // pixels 
    for(int y = by; y < ty; y++)
    {
      // debug: draw border of wall in different color
      // Helpful for visually differentiating walls, esp. those of same color.
      // Also looks kind of cool. Maybe turn into a shader effect or something
      if(x == x1 || x == x2-1 || y == by || y == ty-1)
      {
        drawPixel(x, y, 9);
        continue;
      }
      // end debug
      drawPixel(x, y, color);
    }
  }
}
int Walls[][7] = 
{
	{-20, 100, 0, -20, 350, 0, 3},
	{20, 100, 0, 20, 350, 0, 3},
	{-80, 410, 0, -20, 350, 0, 3},
	{20, 350, 0, 80, 410, 0, 3},
	{-80, 490, 0, -180, 490, 0, 6},
	{-80, 410, 0, -180, 410, 0, 6},
	{-180, 490, 0, -180, 590, 0, 6},
	{-180, 410, 0, -180, 310, 0, 6},
	{-180, 590, 0, -380, 590, 0, 6},
	{-180, 310, 0, -380, 310, 0, 6},
	{-380, 590, 0, -380, 310, 0, 6},
	{80, 410, 0, 80, 490, 0, 3},
	{-80, 490, 0, -20, 550, 0, 3},
	{80, 490, 0, 150, 560, 0, 1},
	{20, 550, 0, 90, 620, 0, 1},
	{150, 560, 0, 250, 560, 0, 1},
	{90, 620, 0, 90, 720, 0, 1},
	{250, 560, 0, 250, 720, 0, 1},
	{130, 720, 0, 300, 720, 0, 5},
	{300, 720, 0, 300, 870, 0, 5},
	{90, 720, 0, 90, 870, 0, 5},
	{90, 870, 0, 300, 870, 0, 5},
	{-20, 550, 0, 20, 550, 0, 3}
};
// Each sub-array in Walls is one wall
// FIELDS:
// x1, y1, z1, x2, y2, z2, color
int numFields = 7; // hardcoded for now; must update this manually
// size in bytes / size of int / number of fields per wall = number of walls
int numWalls = 23; // hardcoded for now; must update this manually
void sortWallsZOrder()
{
  // Proximity of each wall to player. Calculated with Pythagorean Theorem.
  // Used to sort walls by draw order.
  double proximity[numWalls];
  for (int i = 0; i < numWalls; i++)
  {
    // center point of wall
    int wallX = (Walls[i][0] + Walls[i][3])/2;
    int wallY = (Walls[i][1] + Walls[i][4])/2;
    // distance to player = sqrt(a squared + b squared)
    int a = wallX - Player.x;
    int b = wallY - Player.y;
    proximity[i] = sqrt((a*a) + (b*b));
  }
  // NOTE: do not implement this until walls are more portable. Sorting an int array
  //       in chunks of 6 elements is a mainainability nightmare
  // sort walls here
  // bubble sort probably sucks for this, but easier to implement quickly
  double proxSwp;
  int wallSwp;
  for (int i = 0; i < numWalls; i++)
  {
    for (int j = 0; j < numWalls-1; j++)
    {
      if(proximity[j] < proximity[j+1])
      {
	proxSwp = proximity[j];
	proximity[j] = proximity[j+1];
	proximity[j+1] = proxSwp;
	for (int k = 0; k < numFields; k++)
	{
          wallSwp = Walls[j][k];
	  Walls[j][k] = Walls[j+1][k];
	  Walls[j+1][k] = wallSwp;
	}	
      }
    }
  }
  // debug
  /*for (int i = 0; i < numWalls; i++)
  {
    printf("%f\n", proximity[i]);
  }
  for (int i = 0; i < numWalls; i++)
  {
    printf("Wall %d:\t", i);
    for (int j = 0; j < numFields; j++)
    {
      printf("%d  ", Walls[i][j]);
    }
    printf("\n");
  } 
  printf("-----------\n");*/
}
// Renders the current view of the 3D environment
void drawView()
{
  //end debug
  float pCos = TrigVals.cos[Player.angle];
  float pSin = TrigVals.sin[Player.angle];
  int wallX[4], wallY[4], wallZ[4];
  int FOV = 200; // DEV NOTE: investigate what units this value is
  int x1, y1, x2, y2;
  int wall_height = 40;
  // sort walls for correct draw order
  sortWallsZOrder();
  for (int i = 0; i < numWalls; i++) 
  {
    wallX[0] = Walls[i][0];
    wallY[0] = Walls[i][1];
    wallZ[0] = Walls[i][2];
    wallX[1] = Walls[i][3];
    wallY[1] = Walls[i][4];
    wallZ[1] = Walls[i][5];
  
    // absolute world position of the wall's 4 points; these change as the
    // player moves and rotates

    // offset wall points by player location
      x1 = wallX[0] - Player.x;
      y1 = wallY[0] - Player.y;
      x2 = wallX[1] - Player.x;
      y2 = wallY[1] - Player.y;
    // offset wall points by player rotation
    wallX[0] = (x1 * pCos) - (y1 * pSin);
    wallY[0] = (y1 * pCos) + (x1 * pSin);
    // z coords are unaffected by looking angle
    wallZ[0] = wallZ[0] - Player.z;

    wallX[1] = (x2 * pCos) - (y2 * pSin);
    wallY[1] = (y2 * pCos) + (x2 * pSin);
    // z coords are unaffected by looking angle
    wallZ[1] = wallZ[1] - Player.z;

    // set upper 2 points' coords. X and Y are the same for vertically aligned points,
    // but z coords will be offset, defining wall height

    wallX[2] = wallX[0];
    wallY[2] = wallY[0];
    wallZ[2] = wallZ[0] + wall_height;

    wallX[3] = wallX[1];
    wallY[3] = wallY[1];
    wallZ[3] = wallZ[1] + wall_height;

    // clip offscreen portion of wall (aka skip rendering it)
    // skip drawing wall if entirely offscreen
    if(wallY[0] < 1 && wallY[1] < 1) { printf("[]--> SKIPPED WALL DRAW\n"); continue; }
    // clip point 1 of wall if offscreen
    if(wallY[0] < 1)
    {
      // clip bottom line of wall
      clipBehindCamera(&wallX[0], &wallY[0], &wallZ[0], wallX[1], wallY[1], wallZ[1]);
      // clip top line of wall
      clipBehindCamera(&wallX[2], &wallY[2], &wallZ[2], wallX[3], wallY[3], wallZ[3]);
    }
    // clip point 2 of wall if offscreen
    if(wallY[1] < 1)
    {
      // clip bottom line of wall
      clipBehindCamera(&wallX[1], &wallY[1], &wallZ[1], wallX[0], wallY[0], wallZ[0]);
      // clip top line of wall
      clipBehindCamera(&wallX[3], &wallY[3], &wallZ[3], wallX[2], wallY[2], wallZ[2]);
    }

    // transform 3D wall points into 2D screen positions
    wallX[0] = (wallX[0] * FOV) / wallY[0] + (SCREEN_WIDTH / 2); 
    wallY[0] = (wallZ[0] * FOV) / wallY[0] + (SCREEN_HEIGHT / 2);

    wallX[1] = (wallX[1] * FOV) / wallY[1] + (SCREEN_WIDTH / 2);
    wallY[1] = (wallZ[1] * FOV) / wallY[1] + (SCREEN_HEIGHT / 2);

    wallX[2] = (wallX[2] * FOV) / wallY[2] + (SCREEN_WIDTH / 2);
    wallY[2] = (wallZ[2] * FOV) / wallY[2] + (SCREEN_HEIGHT / 2);

    wallX[3] = (wallX[3] * FOV) / wallY[3] + (SCREEN_WIDTH / 2);
    wallY[3] = (wallZ[3] * FOV) / wallY[3] + (SCREEN_HEIGHT / 2);
    
    //debug
    //printf("Screen Wall %d: delta x=%d\n", i, wallX[1] - wallX[0]);
    //end debug

    // skip if wall outside of camera view
    if((wallX[0] < 1 && wallX[1] < 1) || (wallX[0] > SCREEN_WIDTH-1 && wallX[1] > SCREEN_WIDTH-1)) 
    {
      printf("[]--> SKIPPED WALL DRAW\n");
      continue; 
    }
    drawWall(wallX[0], wallX[1], wallY[0], wallY[1], wallY[2], wallY[3], Walls[i][6]);
  }

}
