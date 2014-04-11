#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "raytrace.h"

#if defined(__APPLE_CC__)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

// GLOBAL VARIABLES
unsigned int window_width = 512, window_height = 512;
float pixels[512*512*3];

// Scene information
unsigned int numSpheres = 4;
Sphere spheres[4];

// Viewpoint information
Vector e;
float d = 10;

// Camera basis vectors, must be unit vectors
Vector w;
Vector u;
Vector v;

// Variable to represent image plane
float l = -10, r = 10;
float b = -10, t = 10;

// Global light information
float lightI = 1;
Vector lightDir;

// Default background color
RGBf bgColor;


void init() {
    bgColor = newRGB(100, 100, 100);

    // The viewpoint, e
    e = newVector(5, 0, 0);

    // Calculate basis vectors
    Vector up = newVector(0, 0, 1);
    Vector viewDirection = newVector(-1, 0, 0);
    w = scaleVector(-1/mag(viewDirection), viewDirection);
    Vector upCrossW = cross(up, w);
    u = scaleVector(1/mag(upCrossW), upCrossW);
    v = cross(w, u);

    // Variables for diffuse shading
    lightDir = newVector(0, -1, -1);
    lightDir = scaleVector(1/mag(lightDir),lightDir);

    // Create spheres in scene
    spheres[0].r = 1;
    spheres[0].c = newVector(0, 1, 1);
    spheres[0].color = newRGB(255, 0, 0);
    spheres[0].id = 0;

    spheres[1].r = 1;
    spheres[1].c = newVector(0, 1, -1);
    spheres[1].color = newRGB(0, 0, 255);
    spheres[1].id = 1;

    spheres[2].r = 1;
    spheres[2].c = newVector(0, -0.75, 0);
    spheres[2].color = newRGB(0, 255, 0);
    spheres[2].id = 2;

    spheres[3].r = 5;
    spheres[3].c = newVector(0, -5, -5);
    spheres[3].color = newRGB(255, 255, 0);
    spheres[3].id = 3;
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

RGBf castShadowRay(Vector p, int sphereId) {
    // Calculate ray from point to light source
    Ray ray;
    ray.origin = p;
    ray.direction = scaleVector(-1, lightDir);

    float t;

    // See if ray hits any objects
    for (int i=0; i<numSpheres; i++) {
        if ((t = calcIntersection(ray, spheres[i])) > 0.01 && i != sphereId) {
            return newRGB(-100, -100, -100);
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
            return scaleRGB(spheres[i].color, 0.5);
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
    RGBf specColor = newRGB(250, 250, 250);
    unsigned int specPow = 20;

    // h = (v+l) / mag(v+l)
    Vector viewingRay = scaleVector(1/mag(ray.direction), ray.direction);
    Vector h = addVector(viewingRay, lightDir);
    h = scaleVector(1/mag(h),h);

    float nh = dot(n,h);

    float max = (nh > 0) ? nh : 0;
    max = pow(max, specPow);

    float scale = max * lightI;

    return scaleRGB(specColor, scale);
}

RGBf ambient(RGBf color) {
    float intensity = 0.2;
    return scaleRGB(color, intensity);
}

RGBf shading(Ray ray, Vector p, Vector n, Sphere sphere) {
    RGBf pixelColor;
    pixelColor = diffuse(n, sphere.color);
    pixelColor = addRGB(pixelColor, specular(ray, n));
    pixelColor = addRGB(pixelColor, castReflectRay(p, n, ray));
    pixelColor = addRGB(pixelColor, castShadowRay(p, sphere.id));
    pixelColor = addRGB(pixelColor, ambient(sphere.color));



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
            Vector n = scaleVector(-1/spheres[i].r, minusVector(p, spheres[i].c));

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
