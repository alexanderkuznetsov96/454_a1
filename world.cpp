// world.cpp


#include "world.h"
#include "lander.h"
#include "ll.h"
#include "gpuProgram.h"
#include "strokefont.h"

#include <sstream>

#define M_PI 3.1415926535897932384626433832795

float gameTime = 0;
float altitude = 0;
void World::updateState( float elapsedTime )

{
	gameTime += elapsedTime;

  // See if any keys are pressed for thrust

  if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS) // right arrow
    lander->rotateCW( elapsedTime );

  if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS) // left arrow
    lander->rotateCCW( elapsedTime );

  if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS) // down arrow
    lander->addThrust( elapsedTime );

  // Update the position and velocity

  lander->updatePose( elapsedTime );

  // See if the lander has touched the terrain

  vec3 closestTerrainPoint = landscape->findClosestPoint( lander->centrePosition() );
  float closestDistance = ( closestTerrainPoint - lander->centrePosition() ).length();

  // Find if the view should be zoomed

  zoomView = (closestDistance < ZOOM_RADIUS);

  // Check for landing or collision and let the user know
  int segmentIndex = landscape->findSegmentBelow(lander->centrePosition());
  altitude = landscape->findLanderAltitude(segmentIndex, lander->centrePosition(), lander->getDimensions().y);
  if (abs(altitude) < 10e-4) {
	  // check speed
	  vec3 v = lander->getVelocity();
	  if (v.x < 0.5 && v.y < 1) {
		  // check segment is flat and lander is contained
		  if (landscape->isSegmentGoodToLand(segmentIndex, lander->getOrientation(), lander->centrePosition(), lander->getDimensions().x)) {
			  cout << "Game win" << std::endl;
			  lander->stopLander();
		  }
		  else {
			  cout << "Game Over" << std::endl;
		  }
	  }
	  else {
		  // game over
		  cout << "Game Over" << std::endl;
	  }
  }
  else if (altitude < 0) {
	  // game over
	  cout << "Game Over" << std::endl;
  }
  // YOUR CODE HERE
}

float zoomFactor = 2.0;

void World::draw()

{
  mat4 worldToViewTransform;
  //zoomView = true;
  if (!zoomView) {

    // Find the world-to-view transform that transforms the world
    // to the [-1,1]x[-1,1] viewing coordinate system, with the
    // left edge of the landscape at the left edge of the screen, and
    // the bottom of the landscape BOTTOM_SPACE above the bottom edge
    // of the screen (BOTTOM_SPACE is in viewing coordinates).

    float s = zoomFactor / (landscape->maxX() - landscape->minX());

	if (zoomFactor > 2) {
		zoomFactor -= 0.05;
		worldToViewTransform
			= translate(0, BOTTOM_SPACE, 0)
			* scale(s, s, 1)
			* translate(-lander->centrePosition().x, -lander->centrePosition().y, 0);
	}
	else {
		worldToViewTransform
			= translate(-1, -1 + BOTTOM_SPACE, 0)
			* scale(s, s, 1)
			* translate(-landscape->minX(), -landscape->minY(), 0);
	}
  } else {

    // Find the world-to-view transform that is centred on the lander
    // and is ZOOM_WIDTH wide (in world coordinates).

    // YOUR CODE HERE

	  // Adjusting 2 will "Zoom" in and out
	  if (zoomFactor < landscape->maxX()/ZOOM_RADIUS) {
		  zoomFactor += 0.05;
	  }
	  float s = zoomFactor / (landscape->maxX() - landscape->minX());

	// Need to adjust the translate fucntions to translate around the lander
	  worldToViewTransform
		  = translate(0, BOTTOM_SPACE, 0)
		  * scale(s, s, 1)
		  * translate(-lander->centrePosition().x, -lander->centrePosition().y, 0);

  }

  // Draw the landscape and lander, passing in the worldToViewTransform
  // so that they can append their own transforms before passing the
  // complete transform to the vertex shader.
  landscape->draw( worldToViewTransform);
  lander->draw(worldToViewTransform);
  

  // Draw the heads-up display (i.e. all text).

  stringstream ss;

  drawStrokeString( "LUNAR LANDER", -0.2, 0.85, 0.06, glGetUniformLocation( myGPUProgram->id(), "MVP") );

  ss.setf( ios::fixed, ios::floatfield );
  ss.precision(1);

  ss << "SPEED " << lander->speed() << " m/s";
  drawStrokeString( ss.str(), -0.95, 0.75, 0.06, glGetUniformLocation( myGPUProgram->id(), "MVP") );

  int m = (int)(gameTime / 60);
  int s = (int)gameTime % 60;
  stringstream time;
  time << "TIME ";
  if (m < 10) {
	  time << "0";
  }
  time << m << ":";
  if (s < 10) {
	  time << "0";
  }
  time << s;
  drawStrokeString(time.str(), -0.95, 0.65, 0.06, glGetUniformLocation(myGPUProgram->id(), "MVP"));

  ss.str(std::string());
  ss << "FUEL ";
  for (int i = 1000; i >= 1; i /= 10) {
	  ss << (lander->fuel() % (i*10)) / i;
  }
  drawStrokeString(ss.str(), -0.95, 0.55, 0.06, glGetUniformLocation(myGPUProgram->id(), "MVP"));

  ss.str(std::string());
  ss.precision(0);
  ss << "ALTIDUDE " << altitude;
  drawStrokeString(ss.str(), -0.95, 0.45, 0.06, glGetUniformLocation(myGPUProgram->id(), "MVP"));

  ss.str(std::string());
  int vx = lander->getVelocity().x;
  ss << "HORIZONTAL SPEED " << abs(vx);
  drawStrokeString(ss.str(), -0.95, 0.35, 0.06, glGetUniformLocation(myGPUProgram->id(), "MVP"));
  ss.str("\a");
  float theta = 0;
  if (vx > 0) {
	  // draw right arrow
	  theta = -M_PI/2;
  }
  else if (vx < 0) {
	// draw left arrow
	  theta = M_PI/2;
  }
  else {
	  ss.str(std::string());
  }
  drawStrokeString(ss.str(), 0, 0.35, 0.06, glGetUniformLocation(myGPUProgram->id(), "MVP"), theta);

  ss.str(std::string());
  int vy = lander->getVelocity().y;
  ss << "VERTICAL SPEED " << abs(vy);
  drawStrokeString(ss.str(), -0.95, 0.25, 0.06, glGetUniformLocation(myGPUProgram->id(), "MVP"));
  ss.str("\a");
  if (vy > 0) {
	  // draw down arrow
	  theta = 0;
  }
  else if (vy < 0) {
	  // draw up arrow
	  theta = M_PI;
  }
  else {
	  ss.str(std::string());
  }
  drawStrokeString(ss.str(), 0, 0.25, 0.06, glGetUniformLocation(myGPUProgram->id(), "MVP"), theta);
  // YOUR CODE HERE (modify the above code, too)
}
