/* prim.c make a maze using prim's algo.
(c) 2018 Sean Brennan
Public Domain
*/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

struct VertexNode;
struct VertexNode {
  int x;
  int y;
  struct VertexNode *next;
};

struct EdgeNode;
struct EdgeNode {
  int x1;
  int y1;
  int x2;
  int y2;
  long int cost;
  struct EdgeNode *next;
};

struct Maze {
  struct EdgeNode *maze_edges;
  struct EdgeNode *consider;
  struct VertexNode *seen;
  int xdim;
  int ydim;
  int xstart;
  int ystart;
};

void PrintVerts(struct Maze *maze);
void ClearMaze(char *buffer, int xr, int yr);
void MakeHallway(char *buffer, struct EdgeNode *e, int xr);
void PrintMaze(char *buffer, int xr, int yr);
void FreeMaze(struct Maze *maze);

// better served by a hashmap.
int IsSeen(struct VertexNode *seen, int x, int y) {
  while (seen != NULL) {
   if (seen->x == x && seen->y == y) return 1;
   seen = seen->next;
  }
  return 0;
}

// for given vertex, add all neighbors not seen yet.
void AddAll(struct Maze *maze, struct VertexNode *node) {
  int xi;
  int yi;
  int xt;
  int yt;
  int seen_it;
  struct EdgeNode *tmpedge;
  for (int yi = -1; yi < 2; yi++) { 
    for (int xi = -1; xi < 2; xi++) { 
      if (!((yi != 0) ^ (xi != 0))) continue;  // orto directions only.
      xt = xi + node->x;
      yt = yi + node->y;
      if (xt < 0 || yt < 0 || xt >= maze->xdim || yt >= maze->ydim) continue;
      // if the other vertex(xt,yt) is already in the seen list, skip.
      seen_it = IsSeen(maze->seen, xt, yt);
      if (seen_it == 0) {  // finally we can add the edge to be considered.
        tmpedge = (struct EdgeNode *) malloc(sizeof(struct EdgeNode));
        tmpedge->x1 = node->x;
        tmpedge->y1 = node->y;
        tmpedge->x2 = xt;
        tmpedge->y2 = yt;
        tmpedge->cost = random();
        tmpedge->next = maze->consider;
        maze->consider = tmpedge;
      }
    }
  }
}

void AddCheapest(struct Maze *maze) {
  struct EdgeNode *tmpedge = NULL, *minedge = NULL;
  struct VertexNode *tmpvert = NULL, *seen = NULL;
  int seen1 = 0;
  int seen2 = 0;
  long int mincost = maze->consider->cost;
  minedge = maze->consider;
  tmpedge = maze->consider;
  while (tmpedge != NULL) {
    if (tmpedge->cost < mincost) {
      seen1 = IsSeen(maze->seen, tmpedge->x1, tmpedge->y1); 
      seen2 = IsSeen(maze->seen, tmpedge->x2, tmpedge->y2); 
      if (seen1 + seen2 == 2) {
        tmpedge = tmpedge->next;
        continue;  // better to remove from consideration.
      }
      minedge = tmpedge;
    }
    tmpedge = tmpedge->next;
  }

  if (minedge != NULL) {  // add the unseen to seen, move edge to maze.
    // append new vertex.
    tmpvert = (struct VertexNode *) malloc(sizeof(struct VertexNode));
    seen1 = IsSeen(maze->seen, minedge->x1, minedge->y1); 
    if (seen1 == 1) {
       tmpvert->x = minedge->x2;
       tmpvert->y = minedge->y2;
    } else {
       tmpvert->x = minedge->x1;
       tmpvert->y = minedge->y1;
    }
    tmpvert->next = NULL;
    seen = maze->seen;
    while (seen->next != NULL) {
       seen = seen->next;
    }
    seen->next = tmpvert;
    AddAll(maze, tmpvert);   // <-- Maze grows here.
    // done append new vertex.
    // move min edge to maze.
    tmpedge = maze->consider;
    if (tmpedge == minedge) {
      maze->consider = maze->consider->next;
    } else {
      while (tmpedge->next != minedge) {
        tmpedge = tmpedge->next;
      }
      tmpedge->next = tmpedge->next->next;
    }
    if (maze->maze_edges == NULL) {
      maze->maze_edges = minedge;
    } else {
      tmpedge = maze->maze_edges;
      while (tmpedge->next != NULL) {
        tmpedge = tmpedge->next;
      }
      tmpedge->next = minedge;
    }
    minedge->next = NULL;
    // end move min edge to maze.
  }
}
  
void MakeMaze(int xdim, int ydim, int xstart, int ystart, unsigned int seed) {
  struct Maze maze;
  maze.maze_edges = NULL;
  maze.consider = NULL;
  maze.seen = NULL;
  maze.xdim = xdim;
  maze.ydim = ydim;
  maze.xstart = xstart;
  maze.ystart = ystart;
  struct EdgeNode *tmpedge;
  int pixel_count = 0;
  int num_pixels = xdim * ydim;
  int picturesize = (xdim * 2 + 1) * (ydim * 2 + 1);
  char *buffer = (char *) malloc(picturesize * sizeof(char));
  /* using prim to generate a 2D maze as follows:
  // o edges connect orthogonal neighbors like cells in a maze.
  // o vertices are the cells in a maze.
  // o edge costs are random.
  // o the terminating condition is all cells visited.
  // WARNING: Modern containers (in particular a priority queue
  // for the edge list and a hash set for the seen list should be used.
  // The linear search here is simply to show how prim works.
  */
  srandom(seed);
  maze.seen = (struct VertexNode *) malloc(sizeof(struct VertexNode));
  maze.seen->x = xstart;
  maze.seen->y = ystart;
  maze.seen->next = NULL;
  AddAll(&maze, maze.seen);
  while (pixel_count < num_pixels) {
    AddCheapest(&maze);
    pixel_count++;
    // PrintVerts(&maze);
  }
  // done making maze edges.
  // print it.
  ClearMaze(buffer, xdim * 2 + 1, ydim * 2 + 1);
  tmpedge = maze.maze_edges;
  while (tmpedge != NULL) {
     MakeHallway(buffer, tmpedge, xdim * 2 + 1);
     tmpedge = tmpedge->next;
  }
  PrintMaze(buffer, xdim * 2 + 1, ydim * 2 + 1);
  FreeMaze(&maze);
  free(buffer);
}

//////////// MAIN //////////////////
int main(int argc, char **argv) {
  int xdim = 20;
  int ydim = 15;
  int xstart = 1;
  int ystart = 1;
  unsigned int seed = 1;
  MakeMaze(xdim, ydim, xstart, ystart, seed);
} 

/////////////// Utilities /////////////////////////

void PrintVerts(struct Maze *maze) {
  struct VertexNode *tmpvert = NULL;
  struct EdgeNode *tmpedge = NULL;
  tmpvert = maze->seen;
  printf(" Seen:\n");
  while (tmpvert != NULL) {
    printf(" (%d, %d)\n", tmpvert->x, tmpvert->y);
    tmpvert = tmpvert->next;
  }
  printf(" Consider:\n");
  tmpedge = maze->consider;
  while (tmpedge != NULL) {
    printf(" (%d, %d)-(%d, %d) [%ld]\n",
        tmpedge->x1,
        tmpedge->y1,
        tmpedge->x2,
        tmpedge->y2,
        tmpedge->cost);
    tmpedge = tmpedge->next;
  }
}

void ClearMaze(char *buffer, int xr, int yr) {
  int x, y, i;
  for (y  = 0; y < yr; y++) {
    for (x  = 0; x < xr; x++) {
      i = y * xr + x;
      if (x % 2 == 1 && y % 2 == 1)
        buffer[i] = ',';
      else 
        buffer[i] = 'X';
    }
  }
}

void MakeHallway(char *buffer, struct EdgeNode *e, int xr) {
  int x, y, i;
  x = e->x1 + e->x2 + 1;
  y = e->y1 + e->y2 + 1;
  i = y * xr + x;
  buffer[i] = '.';
}

void PrintMaze(char *buffer, int xr, int yr) {
  int x, y, i;
  for (y = 0; y < yr; y++) {
    for (x = 0; x < xr; x++) {
      i = y * xr + x;
      char c = buffer[i];
      printf("%c", c);
    }
    printf("\n");
  }
  printf("\n");
}

void FreeMaze(struct Maze *maze) {
  struct VertexNode *tmpvert, *nextvert = NULL;
  struct EdgeNode *tmpedge, *nextedge = NULL;
  tmpvert = maze->seen;
  do {
    nextvert = tmpvert->next;
    free(tmpvert);
    tmpvert = nextvert;
  } while (nextvert);
  tmpedge = maze->maze_edges;
  do {
    nextedge = tmpedge->next;
    free(tmpedge);
    tmpedge = nextedge;
  } while (nextedge);
  tmpedge = maze->consider;
  do {
    nextedge = tmpedge->next;
    free(tmpedge);
    tmpedge = nextedge;
  } while (nextedge);
}

/////////////// End Utilities /////////////////////////
