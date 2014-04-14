typedef struct {
    float r;
    float g;
    float b;
} RGBf;

// Create an RGBf with the given values
RGBf newRGB(float red, float green, float blue) {
    RGBf result;
    result.r = red;
    result.g = green;
    result.b = blue;
    return result;
}

// Scale a color by a constant float value
RGBf scaleRGB(RGBf color, float value) {
    return newRGB(color.r * value, color.g * value, color.b * value);
}

// Add two colors together
RGBf addRGB(RGBf a, RGBf b) {
    return newRGB(a.r + b.r, a.g + b.g, a.b + b.b);
}



typedef struct {
    float x;
    float y;
    float z;
} Vector;

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



typedef struct {
    float r;
    Vector c;
    RGBf color;
    int id;
    float ri;
    int reflective;
} Sphere;



typedef struct {
    Vector direction;
    Vector origin;
} Ray;



typedef struct {
    Sphere* sphere;
    Vector n;
    Vector p;
    float t;
} Hit;



void setPixelColor(RGBf pixelColor, RGBf* pixel) {
    pixel->r = pixelColor.r / 255;
    pixel->g = pixelColor.g / 255;
    pixel->b = pixelColor.b / 255;
}

RGBf shade(Hit hit, Ray ray, int recur);
