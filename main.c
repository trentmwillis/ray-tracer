#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Determine which version of GLUT to load
#if defined(__APPLE_CC__)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

/*
Compilation command:
gcc -framework GLUT -framework OpenGL main.c -o main
*/

// STRUCTS
// Struct to represent RGB float values
typedef struct {
    float r;
    float g;
    float b;
} RGBf;

// Struct to represent 3-dimensional vectors
typedef struct {
    float x;
    float y;
    float z;
} Vector;

// Struct to represent a sphere
typedef struct {
    float r;
    Vector c;
    RGBf color;
} Sphere;

// Struct to represent a ray
typedef struct {
    Vector direction;
    Vector origin;
} Ray;

// RGB OPERATIONS
// Create an RGBf with the given values
RGBf newRGB(float red, float green, float blue) {
    RGBf result;
    result.r = red;
    result.g = green;
    result.b = blue;
    return result;
}

// Convert int RGB values (on a [0,255] scale) to float RGB values (normalize it)
void setPixelRGB(float red, float green, float blue, RGBf* out) {
    out->r = red / 255;
    out->b = blue / 255;
    out->g = green / 255;
}

// Same as above, but takes in an RGBf that hasn't been normalized
void setPixelColor(RGBf pixelColor, RGBf* pixel) {
    pixel->r = pixelColor.r / 255;
    pixel->g = pixelColor.g / 255;
    pixel->b = pixelColor.b / 255;
}

// Scale a color by a constant float value
RGBf scaleRGB(RGBf rgb, float value) {
    return newRGB(rgb.r * value, rgb.g * value, rgb.b * value);
}

// Add two colors together
RGBf addRGB(RGBf a, RGBf b) {
    return newRGB(a.r + b.r, a.g + b.g, a.b + b.b);
}

// VECTOR OPERATIONS
// Create a new Vector with the given values
Vector newVector(float x, float y, float z) {
    Vector result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

// Compute the magnitude of a vector
float mag(Vector v) {
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

// Compute the dot product of two vectors
float dot(Vector a, Vector b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

// Scale a vector by a constant value
Vector scaleVector(float value, Vector v) {
    return newVector(v.x * value, v.y * value, v.z * value);
}

// Compute the cross-product of two vectors
Vector cross(Vector a, Vector b) {
    return newVector(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

// Compute the addition of two vectors
Vector addVector(Vector a, Vector b) {
    return newVector(a.x + b.x, a.y + b.y, a.z + b.z);
}

// Compute a vector produced by substracting a vector from another
Vector minusVector(Vector a, Vector b) {
    return addVector(a, scaleVector(-1,b));
}

// GLOBAL VARIABLES
unsigned int window_width = 512, window_height = 512;
unsigned int numSpheres = 3;
float pixels[512*512*3];
Sphere spheres[3];
Vector e;
Vector viewDirection;
Vector up;
Vector w;

Vector n;           // Surface normal

float lightI = 1;   // Light Intensity
Vector lightDir;    // Light direction (unit vector)

Ray ray;

RGBf specColor; // The color of the specular highlight
unsigned int specPow = 20;

float ambientLightI = 0.2;  // Ambient light intensity

// Variable to represent image plane
float l = -10, r = 10;
float b = -10, t = 10;

// Basis vectors, must be unit vectors
Vector w;
Vector u;
Vector v;


void init() {
    // The viewpoint, e
    e = newVector(10, 0, 0);

    // The view direction (which way we're looking)
    viewDirection = newVector(-1, 0, 0);

    // Vector representing global up
    up = newVector(0, 1, 0);

    // Calculate basis vectors
    w = scaleVector(-1/mag(viewDirection), viewDirection);   // The view direction is -w
    Vector upCrossW = cross(up, w);
    u = scaleVector(1/mag(upCrossW), upCrossW);
    v = cross(w, u);

    // Variable for the ray
    ray.origin = e;
    ray.direction = scaleVector(-1, w);

    // Variables for diffuse shading
    lightDir = newVector(-1, 1, 0);
    lightDir = scaleVector(1/mag(lightDir),lightDir);

    // Initialize specular color
    specColor = newRGB(150, 150, 150);

    // Create Spheres
    spheres[0].r = 1;
    spheres[0].c = newVector(3, 3, 0);
    spheres[0].color = newRGB(255, 255, 0);

    spheres[1].r = 1.5;
    spheres[1].c = newVector(0, 0, 0);
    spheres[1].color = newRGB(0, 0, 255);

    spheres[2].r = 100;
    spheres[2].c = newVector(0, -100, 0);
    spheres[2].color = newRGB(0, 255, 0);
}

Vector intersect(Ray ray, Sphere sphere) {
        // Compute discriminate
        // (d . (e - c))^2 - (d.d) * ((e-c).(e-c) - r^2)
        Vector eMinusC = minusVector(ray.origin, sphere.c);
        float d2 = dot(ray.direction, ray.direction);
        float discriminate = dot(ray.direction, eMinusC);
        discriminate *= discriminate;
        discriminate -= (d2 * (dot(eMinusC, eMinusC) - pow(sphere.r, 2.0)));

        if (discriminate > 0) {
            // Calculate p, point of intersection, p = e+td
            // Solve quadratic for t
            // t = -d . (e-c) +- discriminate / d.d
            float t = dot(scaleVector(-1, ray.direction), eMinusC);
            t += sqrt(discriminate);
            if (t < 0) {
                t = t - (2*sqrt(discriminate));
            }
            if (t < 0) {
                return pixelColor;
                break;
            }
            t = t / d2;

            // Compute p
            return addVector(ray.origin, scaleVector(t, ray.direction));
        } else {
            return &NULL;
        }
}

void castShadowRay(Vector p) {
    // Calculate ray from point to light source

    // See if ray hits any objects

}

RGBf castRay(int i, int j) {
    RGBf pixelColor = newRGB(100,100,100);

    // Compute viewing ray
    float vs = l + (r-l) * (i+0.5) / window_height;      // Vertical displacement
    float us = b + (t-b) * (j+0.5) / window_width;       // Horizontal displacement

    ray.origin = addVector(e, addVector(scaleVector(us, u), scaleVector(vs, v)));

    //Check for hits with Spheres
    for (int k=0; k<numSpheres; k++) {
        // Check if ray hits sphere and color accordingly
        Vector p = intersect(ray, spheres[k]);

        if (p != NULL) {                // Hit
            // Send out reflection, refraction, and shadow rays
            castShadowRay(p);

            // Calculate surface normal n = (p-c)/R
            Vector n = scaleVector(1/spheres[k].r, minusVector(p,spheres[k].c));

            // Calculate pixel color = lightIntensity * surfaceColor * max(0,n.l);
            float nl = dot(n, lightDir);
            float max = (nl > 0) ? nl : 0;
            float scale = lightI * max;
            
            pixelColor = scaleRGB(spheres[k].color, scale);

            // SPECULAR SHADING
            // h = (v+l) / mag(v+l)
            Vector viewingRay = scaleVector(1/mag(ray.direction), ray.direction);

            Vector h = addVector(viewingRay, lightDir);
            h = scaleVector(1/mag(h),h);

            float nh = dot(n,h);

            max = (nh > 0) ? nh : 0;
            max = pow(max, specPow);

            scale = max * lightI;

            pixelColor = addRGB(pixelColor, scaleRGB(specColor, scale));

            // AMBIENT SHADING
            pixelColor = addRGB(pixelColor, scaleRGB(spheres[k].color, ambientLightI));

            return pixelColor;
        }
    }

    return pixelColor;
}

// Display method generates the image
void display(void) {
    // Reset drawing window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Ray-tracing loop
    for (int i=0; i<window_height; i++) {
        for (int j=0; j<window_width; j++) {
            setPixelColor(castRay(i,j), &pixels[(i*window_width*3) + (j*3)]);
        }
    }

    // Draw the pixel array
    glDrawPixels(window_width, window_height, GL_RGB, GL_FLOAT, pixels);

    // Reset buffer for next frame
    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
}

void idle(void) {
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    init();

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(window_width, window_height);

    (void)glutCreateWindow("CAP 4730 | Final Project | Advanced Ray-Tracer");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    glutMainLoop();

    return EXIT_SUCCESS;
}
