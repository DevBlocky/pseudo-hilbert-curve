/*
 * main.c - Program for generating pseudo-hilbert curves
 * Copyright (C) 2020 Jacob Parker
 * Unlicensed - Public Domain work
 * This piece of work is unlicensed, and can be used commercially
 *
 * This code will generate a psuedo-hilbert curve of a specified order in a space between (0) and (1)
 * It uses long doubles as stored integers, so the larger the order, the less precision you're going to get
 * In terms of how space is layed out, (0, 0) is top left and (1, 1) is bottom right
 *
 * SEGMENTS:
 *  Geometry - Contains basic geometric functions revolving around points in space and translating their position
 *  Hilbert Curves - Code for generation of pseudo-hilbert curves, built on-top of the geometric functions
 *  Main - Entry point of program, generates the hilbert curve and writes the file with all points
 *
 * WARNING:
 *  This is code that I wrote myself as a challenge and most likely isn't the most efficient in performance OR memory
 *  It is extremely recommended not to use this anywhere commercially, and only as an example piece
*/

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>

/* GEOMETRY */

typedef double space_pos_t;
// Simple structure for a point in 2d space
struct space_vec2 {
    space_pos_t x, y;
};
// macro for easily assigning space_vec2 variables
#define POINT_AT(px, py) { .x = px, .y = py }

// swap the x and y values in a point
#define SPACE_SWAP_POINT(point) { \
    space_pos_t tmp = point.x; \
    point.x = point.y; \
    point.y = tmp; \
}

// macro for defining an operation between two points that is the same for the x and y values
#define SPACE_OP_POINTS(pointA, op, pointB) \
    pointA.x op pointB.x; \
    pointA.y op pointB.y

// macro for reflecting a single value across an origin value
#define SPACE_REFLECT_POINT(val, origin) {\
    val -= origin; \
    val *= -1; \
    val += origin; \
}

// reflects all points in a space_vec2 array across a given vertical line at 'origin'
void space_reflect_y(struct space_vec2 *arr, size_t len, space_pos_t origin) {
    for (size_t i = 0; i < len; i++) SPACE_REFLECT_POINT(arr[i].x, origin);
}

// reflects all points in a space_vec2 array across a given horizontal line at 'origin'
void space_reflect_x(struct space_vec2 *arr, size_t len, space_pos_t origin) {
    for (size_t i = 0; i < len; i++) SPACE_REFLECT_POINT(arr[i].y, origin);
}

/*
 * Little explanation on how rotations work:
 * 90 degree counter-clockwise rotation is (A, B) -> (-y, x)
 * 270 degree counter-clockwise rotation (essentially 90 degree clockwise) is (x, y) -> (y, -x)
 *
 * This is what the macro and functions below are doing in essence.
 * It also takes into account a point of origin so it doesn't just rotate around (0, 0)
 *
 * P.S. x/y negates below are swapped because positive y's are down instead of up
 * which flips the rotation order (cc becomes c, vice versa)
*/

// helper macro for the rotate functions
#define SPACE_ROTATE_POINT_ABOUT_ORIGIN(point, neg_mult, origin) { \
    SPACE_OP_POINTS(point, -=, origin); \
    SPACE_SWAP_POINT(point); \
    point.neg_mult *= -1; \
    SPACE_OP_POINTS(point, +=, origin); \
}

// rotates all points in a space_vec2 array 90 degress around an origin point clockwise
void space_rotate_c(struct space_vec2 *arr, size_t len, struct space_vec2 origin) {
    for (size_t i = 0; i < len; i++) SPACE_ROTATE_POINT_ABOUT_ORIGIN(arr[i], x, origin);
}

// rotates all points in a space_vec2 array 90 degress around an origin point counter-clockwise
void space_rotate_cc(struct space_vec2 *arr, size_t len, struct space_vec2 origin) {
    for (size_t i = 0; i < len; i++) SPACE_ROTATE_POINT_ABOUT_ORIGIN(arr[i], y, origin);
}

// scales all point sin a space_vec2 array a multiplier around an origin point
void space_scale(struct space_vec2 *arr, size_t len, space_pos_t scale, struct space_vec2 origin) {
    // define a space_vec2 for the scale so that we can easily multiply the values
    struct space_vec2 scale_p = POINT_AT(scale, scale);
    for (size_t i = 0; i < len; i++) {
        SPACE_OP_POINTS(arr[i], -=, origin);
        SPACE_OP_POINTS(arr[i], *=, scale_p);
        SPACE_OP_POINTS(arr[i], +=, origin);
    }
}

/* HILBERT CURVE */

// every pseudo-hilbert curve has 4^order points
// this essentially returns that, but uses bitshifting instead
#define HILBERT_NUM_POINTS(order) (size_t) (1 << ((size_t) order * 2))

// recursively creates an pseudo-hilbert curve of a certain order
size_t hilbert_create(int order, struct space_vec2 **out) {
    // statically defined order 1 pseudo-hilbert curve
    // this is upside-down to how you usually see it
    const static struct space_vec2 o1_hilbert[] = {
            POINT_AT(0.25, 0.75), // bottom left
            POINT_AT(0.25, 0.25), // top left
            POINT_AT(0.75, 0.25), // top right
            POINT_AT(0.75, 0.75), // bottom right
    };
    // space_vec2 origins for where to scale lower order pseudo-hilbert curves
    // must be same order as order 1 pseudo-hilbert curve
    const static struct space_vec2 scale_origins[] = {
            POINT_AT(0.0, 1.0), // bottom left
            POINT_AT(0.0, 0.0), // top left
            POINT_AT(1.0, 0.0), // top right
            POINT_AT(1.0, 1.0), // bottom right
    };

    // make sure we got valid input
    if (out == NULL && order >= 1) goto fail;

    // find the number of points this hilber curve requires, then allocate the space
    size_t num_points = HILBERT_NUM_POINTS(order);
    struct space_vec2 *arr = (struct space_vec2 *) malloc(num_points * sizeof(struct space_vec2));
    assert(arr != NULL);

    // if it's an order 1 hilbert curve, just use the statically allocated o1_hilbert
    // this will break the process of recursion
    if (order == 1) {
        // with order 1 hilbert curves, num_points will always be 4 so this is safe
        memcpy(arr, o1_hilbert, num_points * sizeof(struct space_vec2));
        *out = arr;
        return num_points;
    }

    // find the pseudo-hilbert curve of the order below this and store it as memoization
    // lo = lower order
    struct space_vec2 *lo = NULL;
    size_t lo_points = hilbert_create(order - 1, &lo);
    size_t lo_size = lo_points * sizeof(struct space_vec2);

    // variables used in the loop below
    struct space_vec2 center_point = POINT_AT(0.5, 0.5); // point defining the center of space
    struct space_vec2 *work; // working copy of lo

    // create this version of the pseudo-hilbert curve from the others
    for (size_t i = 0; i < sizeof(scale_origins) / sizeof(struct space_vec2); i++) {
        // create another allocation for the pseudo-hilbert curve we're going to work with in this loop
        work = (struct space_vec2 *) malloc(lo_size);
        assert(work != NULL);
        memcpy(work, lo, lo_size);

        // perform transformations based on current i value
        if (i == 0) { // bottom left
            space_reflect_y(work, lo_points, 0.5);
            space_rotate_c(work, lo_points, center_point);
        } else if (i == 4) { // bottom right
            space_reflect_y(work, lo_points, 0.5);
            space_rotate_cc(work, lo_points, center_point);
        }
        // scale it to 1/4 total area, also moves it in the quadrant we want (because of origin)
        space_scale(work, lo_points, 0.5, scale_origins[i]);

        // copy our one quadrant into the total list of hilbert curve coordinates
        memcpy(&arr[lo_points * i], work, lo_points * sizeof(struct space_vec2));

        // cleanup working hilbert-curve
        free(work);
    }
    // cleanup lower order hilbert-curve memoization
    free(lo);

    // finally return this value
    *out = arr;
    return num_points;

    fail:
    *out = NULL;
    return -1;
}

/* MAIN */

// writes coordinates of a hilbert curve to a stream in binary format
void write_hilbert_curve(struct space_vec2 *hc, size_t len, FILE *fp) {
    static size_t max_write = 65536; // amount of element to write in one go

    size_t write_left; // length of array that's left to write
    size_t to_write; // the length of array to write to the stream
    for (size_t i = 0; i < len; i += max_write) {
        // how much length left to write from the hilbert curve
        write_left = (len - i);
        // figure out how much we should write this go (branchless)
        to_write = (write_left > max_write) * max_write +
                   (write_left <= max_write) * write_left;

        // use the to_write as the length of how much to write
        fwrite(&hc[i], sizeof(struct space_vec2), to_write, fp);
        fflush(fp);
    }
}

// writes coordinates of a hilbert curve to a stream in a txt format
void write_hilbert_curve_txt(struct space_vec2 *hc, size_t len, FILE *fp) {
    for (size_t i = 0; i < len; i++) {
        fprintf(fp, "(%.15lf,%.15lf)\n", (double) hc[i].x, (double) hc[i].y);
    }
    fflush(fp);
}

int main(void) {
    // generate 1-15 pseudo-hilbert curves
    struct space_vec2 *hilbert_curve;
    for (size_t order = 1; order < 16; order++) {
        size_t len = hilbert_create(order, &hilbert_curve);
        assert(len != -1);

        char file_name[] = "oxx_hilbert";
        sprintf(file_name, "o%02lli_hilbert", order);

        // write the contents of the hilbert curve to a file
        FILE *fp = fopen(file_name, "wb+");
        assert(fp != NULL);
        write_hilbert_curve(hilbert_curve, len, fp);
        fclose(fp);

        free(hilbert_curve);

        printf("order %lli pseudo-hilbert curve written\n", order);
    }

    return EXIT_SUCCESS;
}
