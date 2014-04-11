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

float d = 10;

float lightI = 1;   // Light Intensity
Vector lightDir;    // Light direction (unit vector)

Ray ray;

RGBf bgColor;

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
    bgColor = newRGB(100, 100, 100);

    // The viewpoint, e
    e = newVector(5, 0, 0);

    // Calculate basis vectors
    Vector up = newVector(0, 1, 0);                         // Vector representing global up
    Vector viewDirection = newVector(-1, 0, 0);             // The view direction (which way we're looking)
    w = scaleVector(-1/mag(viewDirection), viewDirection);  // The view direction is -w
    Vector upCrossW = cross(up, w);
    u = scaleVector(1/mag(upCrossW), upCrossW);
    v = cross(w, u);

    // Variable for the ray
    ray.origin = e;
    ray.direction = scaleVector(-1, w);

    // Variables for diffuse shading
    lightDir = newVector(0, 1, 1);
    lightDir = scaleVector(1/mag(lightDir),lightDir);

    // Initialize specular color
    specColor = newRGB(250, 250, 250);

    // Create Spheres
    spheres[0].r = 1;
    spheres[0].c = newVector(0, 1, 1);
    spheres[0].color = newRGB(255, 0, 0);

    spheres[1].r = 1;
    spheres[1].c = newVector(0, 1, -1);
    spheres[1].color = newRGB(0, 0, 255);

    spheres[2].r = 1;
    spheres[2].c = newVector(0, -0.75, 0);
    spheres[2].color = newRGB(0, 255, 0);
}

float calcIntersection(Ray ray, Sphere sphere) {
    // Compute discriminate
    // (d . (e - c))^2 - (d.d) * ((e-c).(e-c) - r^2)
    Vector eMinusC = minusVector(ray.origin, sphere.c);
    float d2 = dot(ray.direction, ray.direction);
    float discriminate = dot(ray.direction, eMinusC);
    discriminate *= discriminate;
    discriminate -= (d2 * (dot(eMinusC, eMinusC) - pow(sphere.r, 2.0)));

    if (discriminate >= 0) {
        // Solve quadratic for t
        // t = -d . (e-c) +- discriminate / d.d
        float t = dot(scaleVector(-1, ray.direction), eMinusC);
        float t1 = (t + sqrt(discriminate)) / d2;
        float t2 = (t - sqrt(discriminate)) / d2;

        if (t1 > 0 && t2 > 0) {
            if (t1 < t2) {
                return t1;
            } else {
                return t2;
            }
        } else if (t1 > 0) {
            return t1;
        } else if (t2 > 0) {
            return t2;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}

RGBf castShadowRay(Vector p) {
    // Calculate ray from point to light source
    Ray ray;
    ray.origin = p;
    ray.direction = scaleVector(-1, lightDir);

    float t;

    // See if ray hits any objects
    for (int i=0; i<numSpheres; i++) {
        if ((t = calcIntersection(ray, spheres[i])) > 0.001) {
            return newRGB(-50, -50, -50);
        }
    }

    return newRGB(0, 0, 0);
}

RGBf castReflectRay(Vector p, Vector n, Ray ray) {
    Ray reflectRay;
    reflectRay.origin = p;
    reflectRay.direction = scaleVector(1/mag(ray.direction), ray.direction);
    reflectRay.direction = minusVector(reflectRay.direction, scaleVector(2 * dot(reflectRay.direction,n), n));

    for (int i=0; i<numSpheres; i++) {
        if (calcIntersection(reflectRay, spheres[i]) > 0.001) {
            return spheres[i].color;
        }
    }

    return newRGB(0,0,0);
}

Ray computeViewingRay(int i, int j) {
    Ray viewingRay;

    float us = l + (r-l) * (i+0.5) / window_width;
    float vs = b + (t-b) * (j+0.5) / window_height;

    viewingRay.origin = e;
    viewingRay.direction = addVector(scaleVector(-1*d, w), addVector(scaleVector(us, u), scaleVector(vs, v)));

    return viewingRay;
}

RGBf diffuse(Vector n, RGBf surfaceColor) {
    // Diffuse coloring
    // Calculate pixel color = lightIntensity * surfaceColor * max(0,n.l);
    float nl = dot(n, lightDir);
    float max = (nl > 0) ? nl : 0;
    float scale = lightI * max;
    return scaleRGB(surfaceColor, scale);
}

RGBf specular(Ray ray, Vector n) {
    // h = (v+l) / mag(v+l)
    Vector viewingRay = scaleVector(1/mag(ray.direction), ray.direction);
    Vector h = addVector(viewingRay, lightDir);
    h = scaleVector(1/mag(h),h);

    float nh = dot(n,h);

    float max = (nh > 0) ? nh : 0;
    max = pow(max, specPow);

    float scale = max * lightI;

    printf("%f\n", nh);

    return scaleRGB(specColor, scale);
}

RGBf ambient(float intensity, RGBf color) {
    return scaleRGB(color, ambientLightI);
}

RGBf shading(Ray ray, Vector p, Vector n, Sphere sphere) {
    RGBf pixelColor;
    pixelColor = diffuse(n, sphere.color);
    pixelColor = addRGB(pixelColor, specular(ray, n));
    pixelColor = addRGB(pixelColor, ambient(ambientLightI, sphere.color));

    pixelColor = addRGB(pixelColor, castShadowRay(p));

    pixelColor = addRGB(pixelColor, castReflectRay(p, n, ray));

    return pixelColor;
}

RGBf castRay(Ray ray) {
    RGBf pixelColor = bgColor;
    float t = -1;
    float lastT = 9999;

    for (int i=0; i<numSpheres; i++) {
        t = calcIntersection(ray, spheres[i]);
        if (t > 0 && t < lastT) {
            // Compute point of intersection
            Vector p = addVector(ray.origin, scaleVector(t, ray.direction));

            // Compute surface normal, n = (p-c)/R
            Vector n = scaleVector(1/spheres[i].r, minusVector(p, spheres[i].c));

            //Evaluate shading model
            pixelColor = shading(ray, p, n, spheres[i]);

            // Update last-t value
            lastT = t;
        }
    }

    return pixelColor;
}

// Display method generates the image
void display(void) {
    // Reset drawing window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Ray-tracing loop, for each pixel
    for (int i=0; i<window_height; i++) {
        for (int j=0; j<window_width; j++) {
            // Compute viewing ray
            Ray viewingRay = computeViewingRay(i,j);

            // Get color from casting that ray into scene
            RGBf pixelColor = castRay(viewingRay);

            // Update pixel color to result from ray
            setPixelColor(pixelColor, &pixels[(j*window_width*3) + (i*3)]);
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

    glutCreateWindow("CAP 4730 | Final Project | Advanced Ray-Tracer");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    glutMainLoop();

    return EXIT_SUCCESS;
}
