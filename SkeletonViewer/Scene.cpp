#include <iostream>
#include <vector>

#include <GL/freeglut.h>
#include <GL/glu.h>

#include "definitions.hpp"

using namespace std;

void initScene() {

    State& state = *static_cast<State*>(glutGetWindowData());

    //select clearing (background) color
    glClearColor(0.0, 0.0, 0.0, 0.0);

    // initialize projection matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    int w = glutGet( GLUT_WINDOW_WIDTH );
    int h = glutGet( GLUT_WINDOW_HEIGHT );
    gluPerspective( 60, w / h, 0.1, 100 );

    glEnable(GL_DEPTH_TEST);

    // creation of floor texture
    state.texture = createTexture();
}

void quad(int a, int b, int c, int d) {
    State& state = *static_cast<State*>(glutGetWindowData());
    glBegin(GL_QUADS);
    glTexCoord2d(0., 0.);
    glVertex3fv(ver[a]);
    glTexCoord2d(0., 1.);
    glVertex3fv(ver[b]);
    glTexCoord2d(1., 1.);
    glVertex3fv(ver[c]);
    glTexCoord2d(1., 0.);
    glVertex3fv(ver[d]);
    glEnd();
}


void texturedcube() {
    quad(0,3,2,1);
    quad(2,3,7,6);
    quad(0,4,7,3);
    quad(1,2,6,5);
    quad(4,5,6,7);
    quad(0,1,5,4);
}


// https://stackoverflow.com/questions/12518111/how-to-load-a-bmp-on-glut-to-use-it-as-a-texture
GLuint createTexture() {

    State& state = *static_cast<State*>(glutGetWindowData());

    unsigned int width = 20, height = 20; 
    std::vector<unsigned char> data(width*height*3, 0);
    for(unsigned int y = 0; y < height; y ++) {
        for(unsigned int x = 0; x < width; x ++) {
            int c1 = (x%width)/(width/2);
            int c2 = (y%height)/(height/2);
            if(c1 != c2) {
                data[(y*width+x)*3] = 255;
                data[(y*width+x)*3+1] = 255;
                data[(y*width+x)*3+2] = 255;
            }
        }
    }
    
    glGenTextures( 1, &state.texture );
    glBindTexture( GL_TEXTURE_2D, state.texture );
    glActiveTexture(GL_TEXTURE0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data() );

    return state.texture;
}

void drawAxes() {
    glBegin(GL_LINES);
    glColor4f(1., 0., 0., 1.);
    glVertex3f(0., 0., 0.);
    glVertex3f(1., 0., 0.);
    glColor3f(0., 1., 0.);
    glVertex3f(0., 0., 0.);
    glVertex3f(0., 1., 0.);
    glColor3f(0., 0., 1.);
    glVertex3f(0., 0., 0.);
    glVertex3f(0., 0., 1.);
    glEnd();
}

// the display function, where to draw
void displayScene() {

    State& state = *static_cast<State*>(glutGetWindowData());

    //Clear all pixels
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(3, 3, 3, 0, 0, 0, 0, 0, 1);

    // floor
    glPushMatrix();
    glTranslatef(0., 0., 1.); // floor z is now at 0
    glScalef(2., 2., 1.);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, state.texture);
    glBegin(GL_QUADS);
    glColor3f(1., 1., 1.);
    glTexCoord2d(0., 0.);
    glVertex3fv(ver[4]);
    glTexCoord2d(0., 3.);
    glVertex3fv(ver[5]);
    glTexCoord2d(3., 3.);
    glVertex3fv(ver[6]);
    glTexCoord2d(3., 0.);
    glVertex3fv(ver[7]);
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    // draw axis
    glPushMatrix();
    glTranslatef(0., 0., 0.5);
    glScalef(0.5, 0.5, 0.5);
    glLineWidth(4.);
    drawAxes();
    glPopMatrix();

    // cube: https://stackoverflow.com/questions/20106258/drawing-cube-3d-using-opengl
    glPushMatrix();
    glTranslatef(1., -1., 0.3); // on the floor
    glScalef(0.3, 0.3, 0.3);
    glRotatef( state.rotate_z, 0.0, 0.0, 1.0 );
    glColor3f(1., 0., 0.);
    glEnable(GL_TEXTURE_2D);
    texturedcube();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    if(state.skel.tracking) {

    // skeleton drawing
    glPushMatrix();
    glTranslatef(1., 4., 2.);
    glScalef(0.0015, -0.0015, 0.0015);
    glColor3f(0.8, 0.8, 0.8);
    glLineWidth(7.);
    glBegin(GL_LINES);
    for(auto& limb: limbs) {
        auto& joint1 = state.skel.skeleton[limb.first]; 
        auto& joint2 = state.skel.skeleton[limb.second]; 
        if(joint1.position.fConfidence <= 0.3) {
            continue;
        }
        if(joint2.position.fConfidence <= 0.3) {
            continue;
        }
        glVertex3f(joint1.position.position.X, joint1.position.position.Z, joint1.position.position.Y);
        glVertex3f(joint2.position.position.X, joint2.position.position.Z, joint2.position.position.Y);
    }
    glEnd();
    glPopMatrix();

    }


    // Don't wait start processing buffered OpenGL routines
    glFlush();

    glutSwapBuffers();
}
