__kernel void imgcpy(
        __global const float *IN,
        __global float *OUT,
        const int width,
        const int height,
        const float sin,
        const float cos,
        const int ox,
        const int oy
        ) {
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    float xpos = cos * (x - ox) - sin * (y - oy) + ox - 1; //- 1 for index 0
    float ypos = sin * (x - ox) + cos * (y - oy) + oy - 1;

    /* float xpos = cos * ((float)x) + sin * ((float)y); */
    /* float ypos = cos * ((float)y) - sin * ((float)x); */

    // Bound checking
    if (
            ((int)xpos >= 0 && (int)xpos < width) && 
            ((int)ypos >= 0 && (int)ypos < height)
       )
    {
        OUT[y*width+x] = IN[(int)ypos*width+(int)xpos];
    }
}

