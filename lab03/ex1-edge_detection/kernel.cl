__kernel void sobel(
        __global const uchar *pic,
        __global uchar *result,
        const int xsize
)
{

int magnitude,sum1,sum2;
int i = get_global_id(0);
int j = get_global_id(1);



sum1 =  pic[ xsize * (i-1) + j+1 ] -     pic[ xsize*(i-1) + j-1 ] 
        + 2 * pic[ xsize * (i)   + j+1 ] - 2 * pic[ xsize*(i)   + j-1 ]
        +     pic[ xsize * (i+1) + j+1 ] -     pic[ xsize*(i+1) + j-1 ];
      
sum2 = pic[ xsize * (i-1) + j-1 ] + 2 * pic[ xsize * (i-1) + j ]  + pic[ xsize * (i-1) + j+1 ]
            - pic[xsize * (i+1) + j-1 ] - 2 * pic[ xsize * (i+1) + j ] - pic[ xsize * (i+1) + j+1 ];

int offset = i*xsize + j;
      
magnitude =  sum1*sum1 + sum2*sum2;

      if (magnitude > 4000)
        result[offset] = 255;
      else 
        result[offset] = 0;
}

__kernel void imgcpy(
        __global const uchar *IN,
        __global uchar *OUT,
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

/* __kernel void convolution( */
/*     __read_only image2d_t sourceImage,  //Input image object */
/*     __write_only image2d_t outputImage,  //Output Image object */
/*     int rows, int cols,  //Image Dimensions */
/*     __constant float* filter, int filterWidth,  //Convolution filter object */
/*     sampler_t sampler) { //sampler object passed as argument */
/*   */
/*     // Store each work-item’s unique row and column */
/*     int column = get_global_id(0);  */
/*     int row = get_global_id(1); */
/*     // Half the width of the filter is needed for indexing memory later */
/*     int halfWidth = (int)(filterWidth/2); */
/*     // Accesses to images return data as four-element vector (i.e., float4), although  */
/*     // only the ’x’ component will contain meaningful data in this code */
/*     float4 sum = {0.0f, 0.0f, 0.0f, 0.0f}; */
/**/
/*     // Iterator for the filter */
/*     int filterIdx =  0; */
/*     // Each work-item iterates around its local area based on the size of the filter */
/*     int2 coords; // Coordinates for accessing the image */
/*     // Iterate over the rows */
/*     for(int i =  -halfWidth; i <= halfWidth; i++) { */
/*         coords.y =  row + i; */
/*         // Iterate over the columns */
/*         for(int j = -halfWidth; j <= halfWidth; j++) { */
/*             coords.x = column + j; */
/*             float4 pixel; */
/*             // Read a pixel from the image. A single channel image  stores the pixel */
/*             //in the ’x’  coordinate of the returned  vector. */
/*             pixel = read_imagef(sourceImage, sampler, coords); */
/*             sum.x += pixel.x * filter[filterIdx++]; */
/*         } */
/*     } */
/**/
/*      //Copy the data to the output image if the  work-item is in bounds */
/*     if(row < rows && col < cols) { */
/*         coords.x = column;  */
/*         coords.y = row; */
/*         write_imagef(outputImage, coords, sum); */
/*     } */
/*     //End OpenCL Kernel */
/* } */
