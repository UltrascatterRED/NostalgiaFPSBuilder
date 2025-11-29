#include <stdio.h>
#include <math.h>
#include <GL/glut.h>
#include "Render.h"
#include "GameStructs.h"
#include "LevelFileHandling.h"
#include "Debug.h"

bool drawnPixels[SCREEN_WIDTH][SCREEN_HEIGHT] = {false};
// Resets the drawnPixels bool array to false.
// Should be called after every frame draw.
void resetDrawnPixels()
{
    for(int i = 0; i < SCREEN_WIDTH; i++)
    {
        for(int j = 0; j < SCREEN_HEIGHT; j++)
        {
            drawnPixels[i][j] = false;    
        }
    }
}

// given a parent sector reference and a wall pointer buffer,
// retrieves the child walls for that sector and writes their
// pointers to said buffer.
void retrieveChildWalls(sector* parent, wall* children[])
{
    int returnIdx = 0;
    for(int i = 0; i < numWalls; i++)
    {
        if(Walls[i].parentSector == parent)
        {
            children[returnIdx] = &Walls[i];
            returnIdx++;
        }
        if(returnIdx >= parent->numChildren)
        {
            return;
        }
    }
}
// given the pointers to two sectors, swaps the pointers for their
// respective child walls. Intended for use with Z-order sorting of
// sectors.
void swapWallParentSectors(sector* sec1, sector* sec2)
{

    wall* sec1Children[sec1->numChildren];
    wall* sec2Children[sec2->numChildren];
    
    retrieveChildWalls(sec1, sec1Children);
    retrieveChildWalls(sec2, sec2Children);

    for(int i = 0; i < sec1->numChildren; i++)
    {
        sec1Children[i]->parentSector = sec2;
    }
    for(int i = 0; i < sec2->numChildren; i++)
    {
        sec2Children[i]->parentSector = sec1;
    }
}
// updates all Sectors' distances to player, based
// upon their respective center coordinates.
// Intended to be executed once per frame draw.
void updateSectorsProx()
{
    int a = 0;
    int b = 0;
    int prox = 0;
    for(int i = 0; i < numSectors; i++)
    {
        a = Sectors[i].centerX - Player.x;
        b = Sectors[i].centerY - Player.y;
        // calculate hypotenuse as double, round to nearest int,
        // then truncate via typecast. Theoretically minimizes
        // rounding inaccuracy
        prox = (int)round(sqrt((a*a) + (b*b)));
        Sectors[i].playerProximity = prox;
        // debug
        //printf("Sector %d (%p) prox = %d\n", i, &Sectors[i], Sectors[i].playerProximity);
    }
    // debug
    //printf("-------------------------------------------\n");
}

void updateWallsProx()
{
    int a = 0;
    int b = 0;
    int prox = 0;
    for(int i = 0; i < numWalls; i++)
    {
        a = Walls[i].centerX - Player.x;
        b = Walls[i].centerY - Player.y;
        // calculate hypotenuse as double, round to nearest int,
        // then truncate via typecast. Theoretically minimizes
        // rounding inaccuracy
        prox = (int)round(sqrt((a*a) + (b*b)));
        Walls[i].playerProximity = prox;
        // debug
        //printf("Wall %d prox = %d\n", i, Walls[i].playerProximity);
    }
}
// Given an array of ints and its length, returns the maximum value within
// the array.
int getIntMax(int arr[], int size)
{
    int max = arr[0];
    for(int i = 1; i < size; i++)
    {
        if(arr[i] > max)
        {
            max = arr[i];
        }
    }
    return max;
}
// Given an array of ints and its length, returns the minimum value within
// the array.
int getIntMin(int arr[], int size)
{
    int min = arr[0];
    for(int i = 1; i < size; i++)
    {
        if(arr[i] < min)
        {
            min = arr[i];
        }
    }
    return min;
}
// Given an array of ints and its length, returns the average of the array's
// elements
int avgIntArr(int arr[], int size)
{
    int sum;
    for(int i = 0; i < size; i++)
    {
        sum += arr[i];
    }
    // typecasting for better rounding accuracy
    int avg = (int) roundf((float)sum / (float)size);
    return avg;
}
// the standard implementation of these is annoying so I'm making my own
float getMaxFloat(float f1, float f2)
{
    if(f1 > f2)
    {
        return f1;
    }
    else
    {
        return f2;
    }
}
float getMinFloat(float f1, float f2)
{
    if(f1 < f2)
    {
        return f1;
    }
    else
    {
        return f2;
    }
}
int getMaxInt(int i1, int i2)
{
    if(i1 > i2)
    {
        return i1;
    }
    else
    {
        return i2;
    }
}
int getMinInt(int i1, int i2)
{
    if(i1 < i2)
    {
        return i1;
    }
    else
    {
        return i2;
    }
}

// Given a sector for comparison, checks whether the player is located inside
// that sector.
// Useful for conditional rendering/culling.
bool isPlayerInsideSector(sector* sec)
{
    // is player between top/bottom Z bounds of sector
    bool insideZBounds = false;
    // is player inside top-down footprint of sector (X/Y plane)
    bool insideFootprint = false;

    insideZBounds = Player.z > sec->bottomZ && Player.z < sec->topZ;
    
    // determine how many walls of this sector are intersected by a
    // raycast from the player's current position & angle.
    // Even result --> outside sector footprint
    // Odd result --> inside sector footprint

    wall* children[sec->numChildren];
    retrieveChildWalls(sec, children);

    // misc values for determining line segment intersections between walls and
    // player raycast
    int raycastLength = 5000; // arbitrary high number exceeding sectors' typical size
    // coords of raycast endpoint; start point is player x and y
    int xDiff; // stores x2-x1 calculations to check against division by 0
    int rayX = TrigVals.cos[Player.angle] * raycastLength;
    int rayY = TrigVals.sin[Player.angle] * raycastLength;
    // slope and y-intercept of player raycast
    xDiff = rayX - Player.x;
    if(xDiff == 0) { xDiff == 1; }
    float mPlayer = (float)(rayY - Player.y) / (float)xDiff;
    int bPlayer = Player.y - (Player.x * mPlayer);

    int wx1, wx2, wy1, wy2; // storage for wall coords, mainly for readability
    float mWall; // slope for wall 
    int bWall; // y-intercept

    int iX, iY; //possible intersection point
    int numIntersections = 0;
    for(int i = 0; i < sec->numChildren; i++)
    {
        wx1 = children[i]->x1;
        wy1 = children[i]->y1;
        wx2 = children[i]->x2;
        wy2 = children[i]->y2;

        xDiff = wx2 - wx1;
        if(xDiff == 0) { xDiff == 1; }
        // calculate slope and y-intercept for this wall
        mWall = (float)(wy2 - wy1) / (float)xDiff;
        bWall = wy1 - (wx1 * mWall);
        // check if intersection interval exists
        if(getMaxInt(Player.x, rayX) < getMinInt(wx1, wx2)) { continue; }
        // parallel lines cannot intersect
        if(mPlayer == mWall) { continue; }

        iX = (int) roundf((float)(bWall - bPlayer) / (mWall - mPlayer));

        // check that x coord of calculated intersection exists on intersection interval
        if(iX < getMaxInt(getMinInt(wx1, wx2), getMinInt(Player.x, rayX)) ||
            iX > getMinInt(getMaxInt(wx1, wx2), getMaxInt(Player.x, rayX)))
        { 
            continue;
        }
        // all checks passed, therefore raycast and wall intersect
        numIntersections++;
    }
    insideFootprint = numIntersections % 2 == 1;
    bool insideSector = insideZBounds && insideFootprint;
    return insideSector;
}

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
//int numDrawCalls = 0; //debug
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
	int x_start = x1; // store initial x1 value, which is needed for lerp calculations 

	// clip x1, x2 to player view in order to still draw wall. Without this
	// operation, the points may end up out-of-view of the screen, and the wall
	// cannot be drawn.
	// NOTE: clipping to 1 and screen width - 1 is optional; this is done as a
	//			 simple debug behavior to confirm clipping by leaving a 1px margin
	//			 on all sides of the screen. Clipping to 0 and screen width is also
	//			 fine.
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
            if(drawnPixels[x][y])
            {
                continue;
            }
			// debug: draw border of wall in different color
			// Helpful for visually differentiating walls, esp. those of same color.
			if(x == x1 || x == x2-1 || y == by || y == ty-1)
			{
                drawPixel(x, y, 9);
                //numDrawCalls++; //debug
                drawnPixels[x][y] = true;
				continue;
			}
			// end debug
			drawPixel(x, y, color);
            //numDrawCalls++; //debug
            drawnPixels[x][y] = true;
		}
	}
}
// Given [input params], draws a single sector cap on the screen.
// Intended to be called at the end of each call to drawSector().
// FIELDS:
//      x1 : leftmost screen-space x
//      x2 : rightmost screen-space x
//      by[] : the bottom screen-space y coords to lerp between.
//      ty[] : the top screen-space y coords to lerp between.
//      numLerps : the number of separate linear interpolation (lerp)
//                 operations to perform when drawing this cap. Will
//                 always equal length of by/ty arrays minus 1.
//      sec : a pointer to the sector whose caps are being drawn. Used
//            to pull out necessary sector data.
void drawCap(int x1, int x2, int by[], int ty[], int numLerps, sector* sec)
{
    // Validation: exit function if sector does not have caps
    if(!sec->hasCaps) { return; }
    //printf("Drawing a cap\n");

    // clip x1, x2 to camera view
	if(x1 < 1) { x1 = 1; }
	if(x1 > SCREEN_WIDTH - 1) { x1 = SCREEN_WIDTH - 1; }
	if(x2 < 1) { x2 = 1; }
	if(x2 > SCREEN_WIDTH - 1) { x2 = SCREEN_WIDTH - 1; }
    // 3 nested loops is profoundly ugly, but we're just drawing pixels here.
    // shouldn't be too slow in practice, it never scales big enough to
    // be problematic
    for(int x = x1; x < x2; x++)
    {
        // DEV NOTE: Lerps are not necessarily vertically aligned, so the below loop
        // is likely flawed. Alternatively, I probably need to lerp in THIS loop,
        // and check if the next ty value is reached. if it is, shift to the next lerp.


        for(int lrp = 0; lrp < numLerps; lrp++)
        {
            // "by" is short for bottom y, aka y coords of bottom edge of cap
            int delta_by = by[lrp+1] - by[lrp];
            // "ty" is likewise short for top y, the y coords of top edge of cap
            int delta_ty = ty[lrp+1] - ty[lrp];
            // set delta_x to 1 if 0 to prevent dividing by zero
            int delta_x = x2 - x1; if(delta_x == 0) { delta_x = 1; }
            // store initial x1 value for this lerp, needed for lerp calculations
            int x_start = x1; 
            // lerp using calculated constants for this loop
            // DEV NOTE: see lerp formulas in drawWall() for ref

            // 0.5 needed for "rounding issues", investigate further.
            // Current conjecture is that this value "nudges" the current coordinates
            // to the correct placement before int truncation
            // These are the lerp'd y  values per x-coord iteration
            int lrp_by = delta_by * (x - x_start + 0.5) / delta_x + by[lrp];
            int lrp_ty = delta_ty * (x - x_start + 0.5) / delta_x + ty[lrp];
            
            // start next lerp operation if end coord for this lerp reached
            if(lrp_by >= by[lrp+1] || lrp_ty >= ty[lrp+1]) { continue; }

            // clip lrp_by, lrp_ty to player view. Same reasoning as x clipping above.
            if(lrp_by < 1) { lrp_by = 1; }
            if(lrp_by > SCREEN_HEIGHT - 1) { lrp_by = SCREEN_HEIGHT - 1; }
            if(lrp_ty < 1) { lrp_ty = 1; }
            if(lrp_ty > SCREEN_HEIGHT - 1) { lrp_ty = SCREEN_HEIGHT - 1; }
            
            for(int y = by[lrp]; y < ty[lrp]; y++)
            {
                // DEV NOTE: caps are currently overdrawn, so this code serves no
                //           purpose at the moment; caps would simply not render lol
                //if(drawnPixels[x][y])
                //{
                //    continue;
                //}

                // debug: draw border of wall in different color
                // Helpful for visually differentiating walls, esp. those of same color.
                if(x == x1 || x == x2-1 || y == by[lrp] || y == ty[lrp]-1)
                {
                    drawPixel(x, y, 9); // 9 hard-coded for white outline
                    //numDrawCalls++; //debug
                    drawnPixels[x][y] = true;
                    continue;
                }
                // end debug

                drawPixel(x, y, sec->capColor);
                //numDrawCalls++; //debug
                drawnPixels[x][y] = true;
            }
        }
    }
}
void drawSector(sector* sec)
{
	float pCos = TrigVals.cos[Player.angle];
	float pSin = TrigVals.sin[Player.angle];
    // render calculation buffer. Each render
    // pipeline stage is executed destructively on
    // these arrays, with the final result being passed
    // to displayFrame
	int wallX[4], wallY[4], wallZ[4];
	int FOV = 200; // DEV NOTE: investigate what units this value is
	int x1, y1, x2, y2;
    // DEV NOTE: array size hard-coded to arbitrary high number for now.
    //           will need to calculate/store number of verts in sector
    //           (i.e. store as a field in struct sector at file read time)
    int ssX[40]; // screen-space x coords of sector; top-bottom coord pairs share same x value
    int ssBottomY[40]; // screen-space bottom y coords of sector
    int ssTopY[40]; // screen-space top y coords of sector
    int by[40]; // bottom-y coords of cap to be drawn
    int ty[40]; // top-y coords of cap to be drawn
    int num_ssCoords = 0; // track number of stored coordinates in ssWallX/ssWallY
    // get all child walls, iterate over them to render
    wall* children[sec->numChildren];
    retrieveChildWalls(sec, children);

    int sectorBottomZ = sec->bottomZ;
    int sectorTopZ = sec->topZ;

    for(int i = 0; i < sec->numChildren; i++)
    {

        // absolute world position of the wall's 4 points; these change as the
        // player moves and rotates
        wallX[0] = children[i]->x1;
        wallY[0] = children[i]->y1;
        wallZ[0] = sectorBottomZ;
        wallX[1] = children[i]->x2;
        wallY[1] = children[i]->y2;
        wallZ[1] = sectorBottomZ;
        // debug
        //printf("  WALL %d : x1=%d, y1=%d, z1=%d, x2=%d, y2=%d, z2=%d\n\tparent=%p\n",
        //      j, wallX[0], wallY[0], wallZ[0], wallX[1], wallY[1], wallZ[1], &Sectors[i]);
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
        wallZ[2] = wallZ[0] + (sectorTopZ-sectorBottomZ);

        wallX[3] = wallX[1];
        wallY[3] = wallY[1];
        wallZ[3] = wallZ[1] + (sectorTopZ-sectorBottomZ);

        // clip offscreen portion of wall (aka skip rendering it)
        // skip drawing wall if entirely offscreen
        if(wallY[0] < 1 && wallY[1] < 1) { continue; }
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
            //printf("[]--> SKIPPED WALL DRAW\n");
            continue; 
        }
        // store screen-space coords for later parsing; used to determine cap rendering
        // X coords
        ssX[i*2] = wallX[0];
        ssX[i*2+1] = wallX[1];
        // Y coords
        ssBottomY[i*2] = wallY[0];
        ssBottomY[i*2+1] = wallY[1];
        ssTopY[i*2] = wallY[2];
        ssTopY[i*2+1] = wallY[3];

        num_ssCoords += 4;

        drawWall(wallX[0], wallX[1], wallY[0], wallY[1], wallY[2], wallY[3], children[i]->color);
    }
     
    // check for caps to be drawn, sort coords & call drawCap() as needed
    //void drawCap(int x1, int x2, int by[], int ty[], int numLerps, sector* sec)
    // Player inside sector? --> draw both caps
    if(isPlayerInsideSector(sec))
    {
        // both caps 
        // sort captured screen-space wall coords into by[] and ty[] arrays
        // to be passed to drawCap()
        
        // average y coord of ssTopY[]
        int tAvg = avgIntArr(ssTopY, num_ssCoords/2);
        // average y coord of ssBottomY[]
        int bAvg = avgIntArr(ssBottomY, num_ssCoords/2);
        int xMin = getIntMax(ssX, num_ssCoords/2);
        int xMax = getIntMin(ssX, num_ssCoords/2);
        for(int i = 0; i < num_ssCoords/2; i++)
        {
            if(ssBottomY[i] < bAvg)
            {
                by[i] = ssBottomY[i];
            }
            else
            {
                ty[i] = ssBottomY[i];
            }
        }
        // draw cap with calculated inputs
        drawCap(xMin, xMax, by, ty, num_ssCoords/2 - 1, sec);

        for(int i = 0; i < num_ssCoords/2; i++)
        {
            if(ssTopY[i] < tAvg)
            {
                by[i] = ssTopY[i];
            }
            else
            {
                ty[i] = ssTopY[i];
            }
        }
        // draw cap with calculated inputs
        drawCap(xMin, xMax, by, ty, num_ssCoords/2 - 1, sec);
    }
    // Player outside sector? --> draw either top or bottom cap (depending on Player z)
    else if(Player.z > sectorTopZ)
    {
        // top cap
        // sort captured screen-space wall coords into by[] and ty[] arrays
        // to be passed to drawCap()
        // average y coord of ssBottomY[]
        int tAvg = avgIntArr(ssTopY, num_ssCoords/2);
        int xMin = getIntMax(ssX, num_ssCoords/2);
        int xMax = getIntMin(ssX, num_ssCoords/2);

        for(int i = 0; i < num_ssCoords/2; i++)
        {
            if(ssTopY[i] < tAvg)
            {
                by[i] = ssTopY[i];
            }
            else
            {
                ty[i] = ssTopY[i];
            }
        }
        // draw cap with calculated inputs
        drawCap(xMin, xMax, by, ty, num_ssCoords/2 - 1, sec);
    }
    else if(Player.z < sectorBottomZ)
    {
        // bottom cap
        // sort captured screen-space wall coords into by[] and ty[] arrays
        // to be passed to drawCap()

        // average y coord of ssBottomY[]
        int bAvg = avgIntArr(ssBottomY, num_ssCoords/2);
        int xMin = getIntMax(ssX, num_ssCoords/2);
        int xMax = getIntMin(ssX, num_ssCoords/2);
        for(int i = 0; i < num_ssCoords/2; i++)
        {
            if(ssBottomY[i] < bAvg)
            {
                by[i] = ssBottomY[i];
            }
            else
            {
                ty[i] = ssBottomY[i];
            }
        }
        // draw cap with calculated inputs
        drawCap(xMin, xMax, by, ty, num_ssCoords/2 - 1, sec);
    }
    
}
// sorts sectors by current distance to player (greatest to least)
// DEV NOTE: refactor least to greatest for overdraw optimization when ready;
//           Must program optimization first
void sortSectorsZOrder()
{
	// sort sectors here
	// bubble sort probably sucks for this, but easier to implement quickly
    updateSectorsProx();
	sector secSwp;
	for (int i = 0; i < numSectors; i++)
	{
		for (int j = 0; j < numSectors-1; j++)
		{
			if(Sectors[j].playerProximity > Sectors[j+1].playerProximity)
			{
                // swap corresponding sectors' positions in global Sectors
                // array
                sector* sec1 = &Sectors[j];
                sector* sec2 = &Sectors[j+1];
                //printSectorInfo(sec1);
                //printSectorInfo(sec2);
                // propagate swapped sector locations to child walls
                // DEV NOTE (11-13-25): This use of the 
                //      updateWallParentSector() function is flawed
                //      because there is no swap buffer for the sector
                //      pointers. Must devise something like that to
                //      fix visual bugs.
                swapWallParentSectors(sec1, sec2);

                secSwp = Sectors[j];
                Sectors[j] = Sectors[j+1];
                Sectors[j+1] = secSwp;
			}
		}
	}
}
// Sorts walls by current distance to player, closest to farthest.
// Walls are sorted this way to accommodate overdraw optimizations.
void sortWallsZOrder()
{
    updateWallsProx(); 
	// sort walls here
	// bubble sort probably sucks for this, but easier to implement quickly
	wall wallSwp;
	for (int i = 0; i < numWalls; i++)
	{
		for (int j = 0; j < numWalls-1; j++)
		{
			if(Walls[j].playerProximity > Walls[j+1].playerProximity)
			{
                wallSwp = Walls[j];
                Walls[j] = Walls[j+1];
                Walls[j+1] = wallSwp;
			}
		}
	}
}
// Renders the current view of the 3D environment
void drawView()
{
    //numDrawCalls = 0;
    resetDrawnPixels();

	// sort sectors and walls for correct draw order
    sortSectorsZOrder();
	sortWallsZOrder();

    for(int i = 0; i < numSectors; i++)
    {
        drawSector(&Sectors[i]);
    }

    //printf("Draw calls this frame: %d\n", numDrawCalls);
}
