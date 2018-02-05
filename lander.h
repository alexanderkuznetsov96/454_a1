// lander.h
//
// Unit of length is the meter


#ifndef LANDER_H
#define LANDER_H


#include "headers.h"

#define INITIAL_FUEL 9999

class Lander {

  static float landerVerts[];	// lander segments as vertex pairs
  static float flameVerts[];
  int numSegments;		// number of line segments in the lander model
  int flameNumSegments;
  
  GLuint VAO;			// VAO for lander geometry
  GLuint VAOflame;

  vec3 position;		// position in world coordinates (m)
  vec3 velocity;		// velocity in world coordinates (m/s)

  float orientation;		// orientation (radians CCW)
  float angularVelocity;	// angular velocity (radians/second CCW)

  float worldMaxX, worldMaxY;	// world dimensions

  int fuelLevel;

  vec3 landerDimensions;
  vec3 flameDimensions;

 public:

  Lander( float maxX, float maxY ) {
    worldMaxX = maxX;
    worldMaxY = maxY;
	resetFuel();
    reset();
    setupVAO();
	setupVAOFlame();
  };

  void resetFuel() { fuelLevel = INITIAL_FUEL; }

  void setupVAO();  

  void draw( mat4 &worldToViewTransform );

  void setupVAOFlame();

  void drawFlame(mat4 & worldToViewTransform);

  void updatePose( float deltaT );

  void reset() {
    position = vec3( 0.05 * worldMaxX, 0.7 * worldMaxY, 0.0  );
    velocity = vec3( 30.0f, 0.0f, 0.0f );

    orientation = 0;
    angularVelocity = 0;
  }

  void rotateCW( float deltaT );
  void rotateCCW( float deltaT );
  void addThrust( float deltaT );

  vec3 centrePosition() { return position; }

  float speed() { return velocity.length(); }

  vec3 getVelocity() { return velocity; }

  void stopLander() { velocity = vec3(0, 0, 0); }

  int fuel() { return fuelLevel; }

  vec3 getDimensions() { return landerDimensions; }

  float getOrientation() { return orientation; }
};


#endif
