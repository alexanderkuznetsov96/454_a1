// world.cpp


#include "world.h"
#include "lander.h"
#include "ll.h"
#include "gpuProgram.h"
#include "strokefont.h"

#include <sstream>
// defining pi
#define M_PI 3.1415926535897932384626433832795
// Global variables for game use
float gameTime = 0;
float altitude = 0;
int score = 0;
int startfuel = INITIAL_FUEL;
float zoomFactor = 2.0;
bool gameRunning = true;
bool gameWin = false;
int lossReason = 0;

void World::updateState(float elapsedTime)

{	// Checking if the game is currently running
	if (gameRunning) {
		// Increment the time counter
		gameTime += elapsedTime;

		// See if any keys are pressed for thrust

		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) // right arrow
			lander->rotateCW(elapsedTime);

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) // left arrow
			lander->rotateCCW(elapsedTime);

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // down arrow
			lander->addThrust(elapsedTime);

		// Update the position and velocity

		lander->updatePose(elapsedTime);

		// See if the lander has touched the terrain

		vec3 closestTerrainPoint = landscape->findClosestPoint(lander->centrePosition());
		float closestDistance = (closestTerrainPoint - lander->centrePosition()).length();

		// Find if the view should be zoomed

		zoomView = (closestDistance < ZOOM_RADIUS);

		// Check for landing or collision and let the user know
		int segmentIndex = landscape->findSegmentBelow(lander->centrePosition());
		// Getting the altitude for current position
		altitude = landscape->findLanderAltitude(segmentIndex, lander->centrePosition(), lander->getDimensions().y);
		// Check if altitude is close enough to land
		if (abs(altitude) < 10e-2) {
			// check speed
			vec3 v = lander->getVelocity();
			if (abs(v.x) < 0.5 && abs(v.y) < 1) {
				// check segment is flat and lander is contained
				lossReason = landscape->isSegmentGoodToLand(segmentIndex, lander->getOrientation(), lander->centrePosition(), lander->getDimensions().x);
				if (lossReason == 0) {
					lander->stopLander();
					GameWin();
				}
				else {
					// Report why they lost
					switch (lossReason) {
					case 1:
						GameOver("You attempted to land on a segment that was not flat");
						break;
					case 2:
					case 3:
						GameOver("You did not fit on the surface");
						break;
					default:
						GameOver("You crashed");
						break;
					}
				}
			}
			else {
				// game over
				GameOver("You were moving too fast");
			}
		}
		else if (altitude < 0) {
			// game over
			GameOver("You crashed");
		}
	}
	else {		
		// wait for key
		if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
			// start new game
			HardReset();
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			if (startfuel != 0) {
				// continue
				SoftReset();
			}
		}
	}

}

void World::SoftReset() {
	// Set the starting fuel to current fuel
	startfuel = lander->fuel();
	// Reset zoom factor to default
	zoomFactor = 2;
	// Reset game time
	gameTime = 0;
	// reset the lander velocity and position
	lander->reset();
	// set game to run again
	gameRunning = true;
}

void World::HardReset() {
	// Soft reset
	SoftReset();
	// reset the score
	score = 0;
	// reset the fuel
	startfuel = INITIAL_FUEL;
	lander->resetFuel();
}

void World::GameWin() {
	// Game needs to stop
	gameRunning = false;
	gameWin = true;
	// Calculate and add score, score is split 30% for time to land, 30% for fuel used, 40% for size of platform landed on
	score += 300 - gameTime  + 300 * (startfuel - lander->fuel()) / startfuel*10 + 400 * lander->getDimensions().y / landscape->getSegmentWidth(landscape->findSegmentBelow(lander->centrePosition()));
}

void World::GameOver(string reason) {
	// Game needs to stop
	gameRunning = false;
	gameWin = false;
}

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
	// Increase the zoom factor until it is 2 while staying centered on the lander
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

	  // Adjusting increase the zoom view until it reaches the set max
	  if (zoomFactor < landscape->maxX()/ZOOM_RADIUS) {
		  zoomFactor += 0.05;
	  }
	  float s = zoomFactor / (landscape->maxX() - landscape->minX());

	// Set the transform for the lander into the world
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
  // Draw the title
  drawStrokeString( "LUNAR LANDER", -0.4, 0.85, 0.1, glGetUniformLocation( myGPUProgram->id(), "MVP") );

  ss.setf( ios::fixed, ios::floatfield );
  ss.precision(1);
  // Draw the score with placeholder 0's
  ss << "SCORE ";
  for (int i = 1000; i >= 1; i /= 10) {
	  ss << (score % (i * 10)) / i;
  };
  drawStrokeString( ss.str(), -0.95, 0.75, 0.05, glGetUniformLocation( myGPUProgram->id(), "MVP") );
  // finding the number of minutes and seconds
  int m = (int)(gameTime / 60);
  int s = (int)gameTime % 60;
  stringstream time;
  // Draw the time with place holder 0's and split for minutes
  time << "TIME ";
  if (m < 10) {
	  time << "0";
  }
  time << m << ":";
  if (s < 10) {
	  time << "0";
  }
  time << s;
  drawStrokeString(time.str(), -0.95, 0.65, 0.05, glGetUniformLocation(myGPUProgram->id(), "MVP"));

  ss.str(std::string());
  // Draw the fuel level with placeholder 0's
  ss << "FUEL ";
  for (int i = 1000; i >= 1; i /= 10) {
	  ss << (lander->fuel() % (i*10)) / i;
  }
  drawStrokeString(ss.str(), -0.95, 0.55, 0.05, glGetUniformLocation(myGPUProgram->id(), "MVP"));

  ss.str(std::string());
  ss.precision(2);
  // Draw the altitude with percision 2 as it helps player see how close they are
  ss << "ALTITUDE " << altitude;
  drawStrokeString(ss.str(), 0.1, 0.75, 0.05, glGetUniformLocation(myGPUProgram->id(), "MVP"));

  ss.str(std::string());
  ss.precision(1);
  float vx = lander->getVelocity().x;
  // Display the horizontal speed
  ss << "HORIZONTAL SPEED " << abs(vx);
  drawStrokeString(ss.str(), 0.1, 0.65, 0.05, glGetUniformLocation(myGPUProgram->id(), "MVP"));
  // Draw the arrow which is \a overwritten in fg_stroke
  ss.str("\a");
  float theta = 0;
  // Adjust the angle it points at by the direction its going
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
  drawStrokeString(ss.str(), 0.90, 0.67, 0.05, glGetUniformLocation(myGPUProgram->id(), "MVP"), theta);

  ss.str(std::string());
  float vy = lander->getVelocity().y;
  // Display the vertical speed
  ss << "VERTICAL SPEED " << abs(vy);
  drawStrokeString(ss.str(), 0.1, 0.55, 0.05, glGetUniformLocation(myGPUProgram->id(), "MVP"));
  // Draw the arrow which is \a overwritten in fg_stroke
  ss.str("\a");
  // Adjust the angle it points at by the direction its going
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
  drawStrokeString(ss.str(), 0.90, 0.57, 0.05, glGetUniformLocation(myGPUProgram->id(), "MVP"), theta);
  // Check if the game is in running mode
  float pos = -0.4;
  float size = 0.05;
  if (!gameRunning) {
	  ss.str(std::string());
	  // display win screen
	  if (gameWin) {
		  ss << "Game Win";
		  pos = -0.3;
		  size = 0.1;
	  }
	  // display loss screen
	  else {
		  ss << "Game Loss:";
		  switch (lossReason) {
		  case 1:
			  ss << "You attempted to land on a segment that was not flat";
			  break;
		  case 2:
		  case 3:
			  ss << "You did not fit on the surface";
			  break;
		  default:
			  ss << "You crashed";
			  break;
		  }
	  }
	  drawStrokeString(ss.str(), pos, 0.35, size, glGetUniformLocation(myGPUProgram->id(), "MVP"));
	  // Print the game options for continue game or new game
	  ss.str(std::string());
	  if (startfuel == 0) {
		  ss << "Out of fuel. Press 'n' to start new game.";
	  }
	  else {
		  ss << "Press 's' to continue game. Press 'n' to start new game.";
	  }
	  drawStrokeString(ss.str(), -0.75, 0.25, 0.04, glGetUniformLocation(myGPUProgram->id(), "MVP"));
  }
}
