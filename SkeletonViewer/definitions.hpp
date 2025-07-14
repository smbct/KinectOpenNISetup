#pragma once

#include <map>
#include <ctime>

#include <GL/glu.h>
#include <XnCppWrapper.h> // only OpenNI header necessary

// Skeleton attributes
struct SkeletonState {

    xn::Context g_Context;
    xn::ScriptNode g_scriptNode;
    xn::UserGenerator g_UserGenerator;

    XnBool g_bNeedPose = FALSE;
    XnChar g_strPose[20] = "";

    std::map<XnSkeletonJoint, XnSkeletonJointTransformation> skeleton;

    bool tracking;
};

// global attributes
struct State {

    std::clock_t lastTimeMeasure;
    double rotate_z = 0.;
    GLuint texture;

    SkeletonState skel;
    
};

// vertices of a cube
const float ver[8][3] = {
    {-1.0,-1.0,1.0},
    {-1.0,1.0,1.0},
    {1.0,1.0,1.0},
    {1.0,-1.0,1.0},
    {-1.0,-1.0,-1.0},
    {-1.0,1.0,-1.0},
    {1.0,1.0,-1.0},
    {1.0,-1.0,-1.0}};

const XnSkeletonJoint joints[] = {XN_SKEL_HEAD,
	                            XN_SKEL_NECK,
	                            XN_SKEL_TORSO,
	                            XN_SKEL_WAIST,
	                            XN_SKEL_LEFT_COLLAR,
                                XN_SKEL_LEFT_SHOULDER,
                                XN_SKEL_LEFT_ELBOW,
                                XN_SKEL_LEFT_WRIST,
                                XN_SKEL_LEFT_HAND,
                                XN_SKEL_LEFT_FINGERTIP,
                                XN_SKEL_RIGHT_COLLAR,
                                XN_SKEL_RIGHT_SHOULDER,
                                XN_SKEL_RIGHT_ELBOW,
                                XN_SKEL_RIGHT_WRIST,
                                XN_SKEL_RIGHT_HAND,
                                XN_SKEL_RIGHT_FINGERTIP,
                                XN_SKEL_LEFT_HIP,
                                XN_SKEL_LEFT_KNEE,
                                XN_SKEL_LEFT_ANKLE,
                                XN_SKEL_LEFT_FOOT,
                                XN_SKEL_RIGHT_HIP,
                                XN_SKEL_RIGHT_KNEE,
                                XN_SKEL_RIGHT_ANKLE,
                                XN_SKEL_RIGHT_FOOT};

const std::pair<XnSkeletonJoint, XnSkeletonJoint> limbs[] = {
    std::make_pair(XN_SKEL_HEAD, XN_SKEL_NECK),
    std::make_pair(XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER),
    std::make_pair(XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW),
    std::make_pair(XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND),
    std::make_pair(XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER),
    std::make_pair(XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW),
    std::make_pair(XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND),
    std::make_pair(XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO),
    std::make_pair(XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO),
    std::make_pair(XN_SKEL_TORSO, XN_SKEL_LEFT_HIP),
    std::make_pair(XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE),
    std::make_pair(XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT),
    std::make_pair(XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP),
    std::make_pair(XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE),
    std::make_pair(XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT),
    std::make_pair(XN_SKEL_LEFT_HIP, XN_SKEL_RIGHT_HIP)
};


//----------------------------------------
// Scene functions
void initScene();

void displayScene();

void quad(int a, int b, int c, int d);

void texturedcube();

// thanks to https://www.codeproject.com/Articles/23365/A-Simple-OpenGL-Window-with-GLUT-Library for the GLUT introduction
GLuint createTexture();

void drawAxes();


//----------------------------------------
// Skeleton tracking
#define MAX_NUM_USERS 1 // only one user in this scene

XnStatus initializeTracking(SkeletonState& skel_state);

void updateTracking(SkeletonState& skel_state);

void cleanTracking(SkeletonState& skel_state);