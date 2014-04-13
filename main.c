#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "raytrace.h"
#include <time.h>

#if defined(__APPLE_CC__)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

// GLOBAL VARIABLES
unsigned int window_width = 512, window_height = 512;
float pixels[512*512*3];

// Scene information
unsigned int numSpheres = 5;
Sphere spheres[5];

// Viewpoint information
Vector e;
float d = 10;

// Camera basis vectors, must be unit vectors
Vector w;
Vector u;
Vector v;

// Variable to represent image plane
float l = -4, r = 4;
float b = -4, t = 4;

// Global light information
float lightI = .7;
Vector lightDir;

// Default background color
RGBf bgColor;


void init() {
    bgColor = newRGB(0, 0, 0);

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
    lightDir = newVector(-1,-1,-1);
    lightDir = scaleVector(1/mag(lightDir),lightDir);

    // Create spheres in scene
    spheres[4].r = 1;
    spheres[4].c = newVector(0, 1, 1);
    spheres[4].color = newRGB(180,180,180);
    spheres[4].id = 0;
    spheres[4].ri = 1;

    spheres[1].r = 1;
    spheres[1].c = newVector(0, -1, -1);
    spheres[1].color = newRGB(180,180,180);
    spheres[1].id = 1;
    spheres[1].ri = 1;

    spheres[2].r = 1;
    spheres[2].c = newVector(0, -1, 1);
    spheres[2].color = newRGB(180,180,180);
    spheres[2].id = 2;
    spheres[2].ri = 2.4;

    spheres[3].r = 1;
    spheres[3].c = newVector(0, 1, -1);
    spheres[3].color = newRGB(180,180,180);
    spheres[3].id = 3;
    spheres[3].ri = 1;

    spheres[0].r = 1;
    spheres[0].c = newVector(-5, 0, 0);
    spheres[0].color = newRGB(180,180,180);
    spheres[0].id = 3;
    spheres[0].ri = 1;
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

float sceneHit(Ray ray, Hit* hit) {
    float t = 0;
    float lastT = 9999;
    float result = -1;

    for (int i=0; i<numSpheres; i++) {
        t = calcIntersection(ray, spheres[i]);
        if (t > 0 && t < lastT) {
            result = t;
            hit->sphere = &spheres[i];
        }
    }

    hit->t = result;
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
    unsigned int specPow = 40;

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

// P. 305
GLboolean refract(Vector d, Vector n, float ri, Vector* t) {
    Vector t1 = scaleVector(dot(d,n),n);
    t1 = minusVector(d,t1);
    t1 = scaleVector(1/ri,t1);

    float root = 1-(1-pow(dot(d,n),2.0))/pow(ri,2.0);
    if (root < 0) {
        return GL_FALSE;
    }

    root = sqrt(root);
    Vector t2 = scaleVector(root, t2);

    *t = minusVector(t1,t2);

    return GL_TRUE;
}

RGBf castRay(Ray ray, int recur) {
    RGBf pixelColor = bgColor;
    Hit hit, hit2;

    sceneHit(ray,&hit);

    if (hit.t > 0.001) {
        // Compute point of intersection
        Vector p = addVector(ray.origin, scaleVector(hit.t-0.0001, ray.direction));

        // Add base ambient color
        pixelColor = ambient(hit.sphere->color);

        // Compute surface normal, n = (p-c)/R
        Vector n = scaleVector(-1/hit.sphere->r, minusVector(p, hit.sphere->c));

        Ray shadowRay;
        shadowRay.origin = p;
        shadowRay.direction = scaleVector(-1, lightDir);
        sceneHit(shadowRay,&hit2);
        if (hit2.t < 0) {
            pixelColor = addRGB(pixelColor,diffuse(n, hit.sphere->color));
            pixelColor = addRGB(pixelColor, specular(ray, n));
        }

        if (recur > 0) {
            Ray reflectRay;
            reflectRay.origin = p;
            reflectRay.direction = scaleVector(1/mag(ray.direction), ray.direction);
            reflectRay.direction = minusVector(reflectRay.direction, scaleVector(2 * dot(reflectRay.direction,n), n));
            // pixelColor = addRGB(pixelColor, scaleRGB(castRay(reflectRay, recur-1), 0.25));

            if (hit.sphere->ri != 1) {
                float kr, kg, kb, c;
                Vector t;
                if (dot(ray.direction,n) < 0) {
                    refract(ray.direction,n,hit.sphere->ri,&t);
                    c = dot(scaleVector(-1,ray.direction),n);
                    kr = kg = kb = 1;
                } else {
                    if (refract(ray.direction,scaleVector(-1,n),1/hit.sphere->ri,&t)) {
                        c = dot(t,n);
                    } else {
                        return addRGB(pixelColor, scaleRGB(castRay(reflectRay, recur-1), 0.25));
                    }
                }

                float n = hit.sphere->ri;
                float r0 = pow((n-1),2.0) / pow((n+1),2.0);
                float r1 = r0 + (1-r0) * pow(1-c, 5.0);

                pixelColor = addRGB(pixelColor, scaleRGB(castRay(reflectRay, recur-1), r));
                reflectRay.direction = t;
                pixelColor = addRGB(pixelColor, scaleRGB(castRay(reflectRay, recur-1), 1-r));
            }
        }
    }

    return pixelColor;
}


// Display method generates the image
void display(void) {
    // Reset drawing window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float x,y;

    srand(time(NULL));
    float r;
    int samples = 4;

    // Ray-tracing loop, for each pixel
    for (int i=0; i<window_height; i++) {
        for (int j=0; j<window_width; j++) {
            // RGBf pixelColor = newRGB(0,0,0);


            // for (int p=0; p<samples; p++) {
            //     for (int q=0; q<samples; q++) {
            //         r = (rand() % 100)/100;

            //         x = i + (p+r) / samples;
            //         y = j + (q+r) / samples;
            //         // Compute viewing ray
            //         Ray viewingRay = computeViewingRay(x,y);

            //         pixelColor = addRGB(pixelColor, castRay(viewingRay,3));
            //     }
            // }
            
            // pixelColor = scaleRGB(pixelColor, 1/pow(samples,2.0));

            Ray viewingRay = computeViewingRay(i,j);

            RGBf pixelColor = castRay(viewingRay, 3);

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
