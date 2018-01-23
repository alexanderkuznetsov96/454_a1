// ll.h

#ifndef LL_H
#define LL_H


class GPUProgram;
extern GPUProgram *myGPUProgram;

class World;
extern World *world;

typedef enum { UP, DOWN } KeyState;
extern KeyState upKey, downKey, leftKey, rightKey;


#define SCREEN_WIDTH 800	// width in pixels
#define SCREEN_ASPECT 1.3333	// 4:3 aspect ratio of the original screen


#endif
