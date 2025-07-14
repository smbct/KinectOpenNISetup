#include <iostream>
#include <vector>

#include <ctime>

#include <GL/freeglut.h>
#include <GL/glu.h>

#include "definitions.hpp"

using namespace std;

void processKeys(unsigned char key, int x, int y) {
    if(key == 27) {
        glutLeaveMainLoop();
    }
}

void onIdle() {  

    State& state = *static_cast<State*>(glutGetWindowData());

    // update skeleton tracking
    updateTracking(state.skel);

    // display coordinates for one joint
    auto joint = state.skel.skeleton[XN_SKEL_TORSO];

    // update rotation
    std::clock_t current_time = std::clock();
    double elapsedTime = static_cast<double>(current_time - state.lastTimeMeasure)/CLOCKS_PER_SEC;
    state.lastTimeMeasure = current_time;
    state.rotate_z += elapsedTime*800.;

    // update display (glut)
    glutPostRedisplay();
}


int main(int argc, char** argv) {

    //Initialise GLUT with command-line parameters. 
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );

    //Set the window size
    glutInitWindowSize(800, 600);

    //Set the window position
    glutInitWindowPosition(100,100);
    
    //Create the window
    glutCreateWindow("SkeletonViewer");

    // State object
    State state;
    glutSetWindowData(&state);

    state.lastTimeMeasure = clock();

    // Initialization of 3d scene
    initScene();

    // Initialization of Skeleton tracking
    XnStatus status = initializeTracking(state.skel);
    if(status != XN_STATUS_OK) {
        return -1;
    }

    glutIdleFunc(onIdle);

    // drawing callback function
    glutDisplayFunc(displayScene);

    glutKeyboardFunc(processKeys);

    // run the display loop
    glutMainLoop();

    // cleaning
    glDeleteTextures(1, &state.texture);

    cleanTracking(state.skel);  

    return 0;
}