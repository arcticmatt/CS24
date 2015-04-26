#include "shapes.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>


/*============================================================================
 * Global State
 *
 * These are the instances of class-information used by objects of the various
 * types.  Pass a pointer to these to the various object-init functions.
 */

static Shape_Class  Shape;
static Box_Class    Box;
static Sphere_Class Sphere;
static Cone_Class   Cone;


/*============================================================================
 * General Functions
 */


/*!
 * This function performs static initialization of all classes.  It must be
 * called before any of the classes can be used.
 */
void static_init() {
    Shape_class_init(&Shape);
    Box_class_init(&Box);
    Sphere_class_init(&Sphere);
    Cone_class_init(&Cone);
}


/*============================================================================
 * Shape base class
 */


/*! Static initialization for the Shape class. */
void Shape_class_init(Shape_Class *class) {
    // This is a virtual function pointer, so set it to be null
    class->getVolume = NULL;
}


/*!
 * Object initialization (i.e. the constructor) for the Shape class.  This
 * function initializes the density of the shape, as well as the class info.
 */
void Shape_init(Shape_Data *this, Shape_Class *class, float D) {
    /* Initialize Shape_Class and density */
    this->class = class;
    this->density = D;
}


/*! Sets the density of this shape.  The argument must be nonnegative! */
void Shape_setDensity(Shape_Data *this, float D) {
    // Make sure argument is nonnegative
    assert(D > 0);
    // Set density of shape
    this->density = D;
}


/*! Returns the mass of this shape, computed from the density and volume. */
float Shape_getMass(Shape_Data *this) {
    // Calculate mass = density x volume
    float mass = this->density * this->class->getVolume(this);
    return mass;
}


/*
 * THERE IS NO Shape_getVolume() FUNCTION, because Shape doesn't provide an
 * implementation!  In the class initialization, set the function-pointer to
 * NULL to signify this.
 */

/*
 * There is also no new_Shape() function, since Shape is abstract (missing
 * some of its implementation), and therefore is not instantiable.
 */


/*============================================================================
 * Box subclass
 */


/*! Static initialization for the Box class. */
void Box_class_init(Box_Class *class) {
    // Set the function pointer to the Box's getVolume function
    class->getVolume = Box_getVolume;
}


/*!
 * Object initialization (i.e. the constructor) for the Box class.  This
 * function first calls the Shape constructor to initialize the class info and
 * density, and then it initializes its data members with the specified values.
 */
void Box_init(Box_Data *this, Box_Class *class,
    float L, float W, float H, float D) {
    // Call superconstructor to initialize class info and density
    Shape_init((Shape_Data *) this, (Shape_Class *) class, D);
    // Initialize box specific data members
    this->length = L;
    this->width = W;
    this->height = H;
}


/*!
 * This function implements the operation corresponding to the C++ code
 * "new Box(L, W, H, D)", performing both heap-allocation and initialization.
 */
Box_Data * new_Box(float L, float W, float H, float D) {
    // Allocate memory for the box_data pointer
    Box_Data *box_data = malloc(sizeof(Box_Data));
    // Initialize box_data
    Box_init(box_data, &Box, L, W, H, D);
    // Return it
    return box_data;
}


/*!
 * Sets the dimensions of the box.  The arguments are asserted to be positive.
 */
void Box_setSize(Box_Data *this, float L, float W, float H) {
    // Make sure arguments are positive
    assert(L > 0);
    assert(W > 0);
    assert(H > 0);
    // Set the dimensions
    this->length = L;
    this->width = W;
    this->height = H;
}


/*!
 * Computes the volume of the box.  This function provides the implementation
 * of Shape::getVolume(), which is abstract (i.e. pure-virtual).
 */
float Box_getVolume(Box_Data *this) {
    // For a box, volume = length x width x height
    return this->length * this->width * this->height;
}


/*============================================================================
 * Sphere subclass
 */


/*! Static initialization for the Sphere class. */
void Sphere_class_init(Sphere_Class *class) {
    // Set the function pointer to the Sphere's getVolume function
    class->getVolume = Sphere_getVolume;
}


/*!
 * Object initialization (i.e. the constructor) for the Sphere class.  This
 * function first calls the Shape constructor to initialize the class info and
 * density, and then it initializes its data members with the specified values.
 */
void Sphere_init(Sphere_Data *this, Sphere_Class *class, float R, float D) {
    // Call superconstructor to initialize class info and density
    Shape_init((Shape_Data *) this, (Shape_Class *) class, D);
    // Initialize sphere specific data members
    this->radius = R;
}


/*!
 * This function implements the operation corresponding to the C++ code
 * "new Sphere(R, D)", performing both heap-allocation and initialization.
 */
Sphere_Data * new_Sphere(float R, float D) {
    // Allocate memory for the sphere_data pointer
    Sphere_Data *sphere_data = malloc(sizeof(Sphere_Data));
    // Initialize sphere_data
    Sphere_init(sphere_data, &Sphere, R, D);
    // Return it
    return sphere_data;
}


/*! Sets the radius of the sphere.  The argument is asserted to be positive. */
void Sphere_setRadius(Sphere_Data *this, float R) {
    // Make sure the radius is positive
    assert(R > 0);
    // Set radius to some positive value
    this->radius = R;
}


/*!
 * Computes the volume of the sphere.  This function provides the implementation
 * of Shape::getVolume(), which is abstract (i.e. pure-virtual).
 */
float Sphere_getVolume(Sphere_Data *this) {
    // For a sphere, volume = 4/3 * pi * r^3
    float volume = (4.0 / 3.0) * M_PI * pow(this->radius, 3.0);
    return volume;
}


/*============================================================================
 * Cone subclass
 */


/*! Static initialization for the Cone class. */
void Cone_class_init(Cone_Class *class) {
    // Set the function pointer to the Cone's getVolume function
    class->getVolume = Cone_getVolume;
}


/*!
 * Object initialization (i.e. the constructor) for the Cone class.  This
 * function first calls the Shape constructor to initialize the class info and
 * density, and then it initializes its data members with the specified values.
 */
void Cone_init(Cone_Data *this, Cone_Class *class, float BR, float H, float D) {
    // Call superconstructor to initialize class info and density
    Shape_init((Shape_Data *) this, (Shape_Class *) class, D);
    // Initialize cone specific data members
    this->base_radius = BR;
    this->height = H;
}


/*!
 * This function implements the operation corresponding to the C++ code
 * "new Cone(BR, H, D)", performing both heap-allocation and initialization.
 */
Cone_Data * new_Cone(float BR, float H, float D) {
    // Allocate memory for the cone_data pointer
    Cone_Data *cone_data = malloc(sizeof(Cone_Data));
    // Initialize cone_data
    Cone_init(cone_data, &Cone, BR, H, D);
    // Return it
    return cone_data;
}


/*!
 * Sets the dimensions of the cone.  The arguments are asserted to be positive.
 */
void Cone_setBaseHeight(Cone_Data *this, float BR, float H) {
    // Make sure arguments are positive
    assert(BR > 0);
    assert(H > 0);
    // Set dimensions
    this->base_radius = BR;
    this->height = H;
}


/*!
 * Computes the volume of the cone.  This function provides the implementation
 * of Shape::getVolume(), which is abstract (i.e. pure-virtual).
 */
float Cone_getVolume(Cone_Data *this) {
    // For a cone, volume = pi * br^2 * h/3
    float volume = M_PI * pow(this->base_radius, 2.0) * (this->height / 3.0);
    return volume;
}
