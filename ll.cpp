// Lunar lander game


#include <sys/timeb.h>

#include "headers.h"
#include "gpuProgram.h"
#include "world.h"
#include "ll.h"


GPUProgram *myGPUProgram;	// pointer to GPU program object

World *world;			// the world, including landscape and lander

bool pauseGame = false;


// Handle a keypress


void keyCallback( GLFWwindow *w, int key, int scancode, int action, int mods )

{
  if (action == GLFW_PRESS)
    
    if (key == GLFW_KEY_ESCAPE)	// quit upon ESC
      exit(0);

    else if (key == 'p')	// p = pause
      pauseGame = !pauseGame;

    else if (key == 'r')	// r = reset lander
      world->resetLander();

    else if (key == '?') 	// ? = output help
      cout << "help" << endl;
}


// Error callback

void errorCallback( int error, const char* description )

{
  cerr << "Error: " << description << endl;
}


  
// Main program


int main( int argc, char **argv )

{
  // Set up GLFW

  GLFWwindow* window;

  // glfwSetErrorCallback( errorCallback );
  
  if (!glfwInit())
    return 1;
  
  // Open window (ask for OpenGL ES 3.0 or better)

  glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );

  window = glfwCreateWindow( SCREEN_ASPECT * SCREEN_WIDTH, SCREEN_WIDTH, "Lunar Lander", NULL, NULL);
  
  if (!window) {
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent( window );
  
  glfwSwapInterval( 1 ); // redraw at most every 1 screen scan
  
  // Set OpenGL function bindings

  gladLoadGLLoader( (GLADloadproc) glfwGetProcAddress );

  // Set up callbacks

  glfwSetKeyCallback( window, keyCallback );
  
  // Set up shaders

  myGPUProgram = new GPUProgram( "ll.vert", "ll.frag" );
  myGPUProgram->activate();

  // Set up world

  world = new World( window );

  // Run

  struct timeb prevTime, thisTime;
  ftime( &prevTime );

  while (!glfwWindowShouldClose( window )) {

    // Find elapsed time since last render

    ftime( &thisTime );
    float elapsedSeconds = (thisTime.time + thisTime.millitm / 1000.0) - (prevTime.time + prevTime.millitm / 1000.0);
    prevTime = thisTime;

    // Update the world state

    if (!pauseGame)
      world->updateState( elapsedSeconds );

    // Display the world

    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    glClear( GL_COLOR_BUFFER_BIT );

    world->draw();

    glfwSwapBuffers( window );
    
    // Check for new events

    glfwPollEvents();
  }

  glfwDestroyWindow( window );
  glfwTerminate();
  return 0;
}


// A bug in some GL* library in Ubuntu 14.04 requires that libpthread
// be forced to load.  The code below accomplishes this (from MikeMx7f
// http://stackoverflow.com/questions/31579243/segmentation-fault-before-main-when-using-glut-and-stdstring).
//
// You'll also need to modify the Makefile to uncomment the pthread parts

// #include <pthread.h>

// void* simpleFunc(void*) { return NULL; }

// void forcePThreadLink() { pthread_t t1; pthread_create(&t1, NULL, &simpleFunc, NULL); }
