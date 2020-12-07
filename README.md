# Pseudo Hilbert Curve Generator

A simple program to generate (x, y) coordinates in the interval [0, 1] for pseudo-hilbert curves of a specified order.
Does not take any input, therefore you must modify code to apply options you want.

## WARNING

This is my own implementation and it can probably be optimized a lot (in terms of memory and CPU usage).
Currently, 15th order curves can use up to 24GB of RAM and, depending on the computer, use a lot of CPU.
Use this code at your own risk, as this was a side project and was never intended to be of actual use.

## Functions, Structs, & Macros

### Geometry

- `space_pos_t` - The datatype of the coordinate plane
- `space_vec2` - A structure defining an (x, y) position
- `SPACE_SWAP_POINT` - Swap the (x, y) values in a `space_vec2`
- `SPACE_OP_POINTS` - Performs an operation between two `space_vec2` points
- `SPACE_REFLECT_POINT` - Reflects a point in the coordinate plane
    - **Warn**: Do not use, instead use `space_reflect_y` and `space_reflect_x`
- `space_reflect_y` - Reflects all points in a `space_vec2` array across the y axis
- `space_reflect_x` - Reflects all points in a `space_vec2` array across the x axis
- `SPACE_ROTATE_POINT_ABOUT_ORIGIN` - Rotates a point around an origin point.
    - **Warn**: Do not use, instead use `space_rotate_c` and `space_rotate_cc`
- `space_rotate_c` - Rotates all points in a `space_vec2` array around an origin point clockwise
- `space_rotate_cc` - Rotates all points in a `space_vec2` array around an origin point counter-clockwise
- `space_scale` - Scales all points in a `space_vec2` array towards or away from an origin point

### Hilbert Curve

- `HILBERT_NUM_POINTS` - Defines the number of points a pseudo-hilbert curve of a certain order will have
- `hilbert_create` - Recursively creates a pseudo-hilbert curve, defined as an array of `space_vec2`

### Main

- `write_hilbert_curve` - Writes the binary representation of a `space_vec2` array into a stream
- `write_hilbert_curve_txt` - Writes a text representation of a `space_vec2` array into a stream
- `main` - Entry point for the program, creates a hilbert curve up to the 15th order and binary writes into files

## License

This work is unlicensed and available to the public domain. Use for example only, please read `WARNING`.
