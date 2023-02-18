#include <freeglut.h>
#include <FreeImage.h>
#include <chrono>
#include <thread>

// Mutable global variables
int W = 1920;   // default value, but main function calculates the users screen size anyways
int H = 1080;   
int PARTICLE_SIZE = 6;                      // determines the size of the particle
bool FIREWORKS = false, COLLISION = true, RGB = false;   // booleans for certain toggleable features
int DELAY = 0;                              // number for determining speed of animations
//float red = 1.0, green = 0.0, blue = 0.0;	// starting paint color is always red
int rate = 255;

// Immutable
const int NUM_PARTICLE = 200;               // number of particles per explosion
const float PARTICLE_SPEED_MIN = -50.0;     // minimum particle travelling speed (for going the opposite direction)
const float PARTICLE_SPEED_MAX = 50.0;      // maximum particle travelling speed

// Object representing an individual particle. Has a location, X and Y velocity, RGBA values and an HP bar 
struct Particle {
    GLfloat x, y;       // X and Y locations
    GLfloat vx, vy;     // velocity for X and Y 
    GLfloat r, g, b, a; // RGBA values for randomization and for implementing fade out feature
    int hp;             // play on health points, will use it to determine when the particle should die out
    int counter;
};

Particle p[NUM_PARTICLE];   // create an array of partcles, with a size of NUM_PARTICLE

// Generate a random float between min and max
float randomFloat(float min, float max) {
    float randomValue = (float)rand() / RAND_MAX;  // random value between 0 and 1
    return min + randomValue * (max - min);        // scale to desired range
}       

// Function with my algorithm for creating an RGB (gamer like) pattern
//void colorSequence(int counter, float alpha) {
//    // color values are ++/-- by a very small number to make color shifting much slower/gradual
//    if (0 < counter && counter <= rate) {				  // R --> RG
//        green += (float)1 / rate;
//    }
//    else if (rate < counter && counter <= rate * 2) {	  // RG --> G
//        red -= (float)1 / rate;
//    }
//    else if (rate * 2 < counter && counter <= rate * 3) { // G --> GB
//        blue += (float)1 / rate;
//    }
//    else if (rate * 3 < counter && counter <= rate * 4) { // GB --> B
//        green -= (float)1 / rate;
//    }
//    else if (rate * 4 < counter && counter <= rate * 5) { // B --> R B
//        red += (float)1 / rate;
//    }
//    else if (rate * 5 < counter && counter <= rate * 6) { // R B --> R
//        blue -= (float)1 / rate;
//    }
//    //printf("%f, %f, %f\n", red, green, blue);
//    glColor4f(red, green, blue, alpha);	// updates the current color in used for the next point drawn
//}

// Initialize particles. This function creates the each individual particles with different values for their parameters.
void initParticles() {
    // Using screen dimensions, calculates 1/4 of the total screensize and uses it as valid space for the particles to spawn from
    float forX = (randomFloat(-(W / 4), (W / 4)));
    float forY = (randomFloat(-(H / 4), (H / 4)));

    // generating each individual particle 
    for (int i = 0; i < NUM_PARTICLE; i++) {
        // sets the location of the particle to the screen relative to its center, using forX and forY as offsets (so its not dead center every time)
        p[i].x = (W / 2) + forX;
        p[i].y = (H / 2) + forY;

        // sets the horizontal and vertical velocity/speed of the particle
        p[i].vx = randomFloat(PARTICLE_SPEED_MIN, PARTICLE_SPEED_MAX);
        p[i].vy = randomFloat(PARTICLE_SPEED_MIN, PARTICLE_SPEED_MAX);

        // random color for the particle; will generate a number between 0.0 ... 1.0
        p[i].r = (float)(rand()) / RAND_MAX;
        p[i].g = (float)(rand()) / RAND_MAX;
        p[i].b = (float)(rand()) / RAND_MAX;
        p[i].a = 1.0;   // alpha - completely visible by default

        p[i].hp = 100;  // HP at 100
        p[i].counter = 0;
    }
}

// Function for updating each particle mathematically.
void update(float rate) {
    // updating each individual particle
    for (int i = 0; i < NUM_PARTICLE; i++) {
        // update new position by using its velocity * rate (rate: value to help determine how fast the particles move)
        p[i].x += p[i].vx * rate;
        p[i].y += p[i].vy * rate;

        if (FIREWORKS) {    // if Fireworks mode is toggled
            // velocity slow down idea, using a decreasing rate of 0.99
            p[i].vx *= 0.99;
            p[i].vy *= 0.99;
        }
        
        if (COLLISION) {    // if Collision is toggled
            if (p[i].x < 0) {   // hits left side
                p[i].x = 0;     // particle stops momentarily
                p[i].vx *= -1;  // reflect the direction
            } else if (p[i].x > W) {  // hits right side
                p[i].x = W;
                p[i].vx *= -1;
            }
            // Since glut renders from bottom-left, y < 0 refers to the bottom, and vice versa
            if (p[i].y < 0) {   // hits bottom
                p[i].y = 0;
                p[i].vy *= -1;
            } else if (p[i].y > H) {  // hits top
                p[i].y = H;
                p[i].vy *= -1;
            }
        }
        
        if (FIREWORKS) {                // if Fireworks mode is toggled
            // specific particle fade away system, where instead of fading away the moment the animation starts, it needs to go through its lifespan first (hp bar), THEN slowly fades
            p[i].hp -= 0.0001;          // 0.0001 is the value that determines how long it takes until a particle is THEN able to start fading out
            if (p[i].hp <= 0) {
                p[i].hp = 0;

                p[i].a -= 0.007;        // start to fade out the particle rapidly
                if (p[i].a <= 0) {      // once the particle has completely faded out
                    initParticles();    // reset the animation to give it PERPETUALITY 
                }
            }
        }
    }
}

// Toggles ON/OFF for collision mode
void toggle_collision() {
    if (COLLISION == false) {
        COLLISION = true;
        printf("Collision: ON\n");
    }
    else {
        COLLISION = false;
        printf("Collision: OFF\n");
    }
}

void rgb_mode() {
    if (!RGB) {
        RGB = true;
        FIREWORKS = false;
    }
    else {
        RGB = false;
    }
}

// Toggles ON/OFF for Fireworks mode
void fireworks() {
    if (!FIREWORKS) {
        FIREWORKS = true;
        RGB = false;
        printf("Fireworks: ON\n");
    }
    else {
        FIREWORKS = false;
        printf("Fireworks: OFF\n");
    }
}

// Toggles either 0ms delay or 10ms delay
void time_warp() {
    if (DELAY == 0) {
        printf("Slooooowing Doooown....\n");
        DELAY = 10;
    }
    else {
        DELAY = 0;
        printf("Normal Speed\n");
    }
}

// Increase size of the particles
void increase_size() {
    PARTICLE_SIZE++;
}

// Decrease size of the particles
void decrease_size() {
    PARTICLE_SIZE--;

    // Ensures that the particles are ridicously small, but also making sure they never reach 0 or below
    if (PARTICLE_SIZE <= 2) {
        printf("Smallest particle sized reached (min: 2)\n");
        PARTICLE_SIZE = 2;
    }
}

// Tells us the current size of the particle
void check_size() {
    printf("Current particle size: %i", PARTICLE_SIZE);
}

// Function for rendering the particles onto the screen
void animate() {
    // loop will produce each of the particles and show them on screen
    for (int i = 0; i < NUM_PARTICLE; i++) {

        glPointSize(PARTICLE_SIZE);  // set the size of the particle
        glBegin(GL_POINTS);          // determines the parameters for drawing the shape

        /*if (RGB) {
            colorSequence(p[i].counter, p[i].a);
            p[i].counter++;
            printf("%i\n", p[i].counter);

            if (p[i].counter == (rate*6)+1) {
                p[i].counter = 0;
                
            }
        }*/
        //else {
            glColor4f(p[i].r, p[i].g, p[i].b, p[i].a);  // describe the colors + opacity level
        //}
        
        glVertex2f(p[i].x, p[i].y);  // drawing their shape onto the screen
    }
    glEnd();
}

// Central function for performing all calculations, then animations in sequence. 
void display() {
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY)); // time warp: only solution I could think of for implementing a slower time warp 

    glClear(GL_COLOR_BUFFER_BIT);

    animate();      // draw the particles
    update(0.1);    // update the particles 

    glutSwapBuffers(); // switches back buffer with front buffer to give smooth animations
}

// Function for when the user moves the glut window around. 
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);    // allows us to arrange how particles will appear while in this mode
    glLoadIdentity();

    glOrtho(0, w, 0, h, -1, 1);     // creates the projection plane based off the dimensions of the window
    glMatrixMode(GL_MODELVIEW);     // allows us to perform all types of shape manipulation in this mode
}

// Explanation of commands
void show_keys() {
    printf("1: Fireworks Mode (works best with Collision ON) (when switching between modes, reset using 'R' for intended effects)\n");
    //printf("2: RGB Mode (Disables Fireworks Mode)\n");
    printf("2: Toggle Collision (default ON)\n");
    printf("Z: Decrease particle size by 1\n");
    printf("X: Increase particle size by 1\n");
    printf("T: Time Warp (slow down). Press again to toggle ON/OFF.\n");
    printf("R: Reset Animation\n");
    printf("Esc/Q: Quit Program\n\n");
}

// Function of all usable keyboard keys
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 0x1B:
    case 'q':
    case 'Q':
        exit(0);
        break;
    case 'r':
    case 'R':
        initParticles();
        break;
    case 't':
    case 'T':
        time_warp();
        break;
    case 'z':
    case 'Z':
        decrease_size();
        break;
    case 'x':
    case 'X':
        increase_size();
        break;

    case '1':
        fireworks();
        break;
    case '2':
        toggle_collision();
        break;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    // retrieve the user's screen dimensions
    W = glutGet(GLUT_SCREEN_WIDTH);
    H = glutGet(GLUT_SCREEN_HEIGHT);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(W, H);   // make the window size fit 
    glutCreateWindow("Particle Burst by Julian Ellis Geronimo");
    glutPositionWindow(0, 0);   // used the fullscreen "workaround" from the email sent by the professor

    // glut functions
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);

    show_keys();        // display the keyboard controls on the console

    glClearColor(0.1, 0.1, 0.1, 1); // makes the background a dark gray background
    initParticles();    // prepare the particles to be animated

    // allows for opacity
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glutMainLoop();

    return 0;
}