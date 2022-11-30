void convolution(
    __read_only image2d_t sourceImage,  //Input image object
    __write_only image2d_t outputImage,  //Output Image object
    int row, int column,
    int rows, int cols,  //Image Dimensions
    __constant float* filter, float filterFactor, int filterWidth,  //Convolution filter object
    sampler_t sampler);

__kernel void sobel(
        __read_only image2d_t pic,
        __write_only image2d_t result,
        sampler_t sampler,
        const int width,
        const int height
)
{
    int row = get_global_id(1);
    int column = get_global_id(0);

    __constant float gaussian_filter[5*5] = {
        2, 4, 5, 4, 2,
        4, 9, 12, 9, 4,
        5, 12, 15, 12, 5,
        4, 9, 12, 9, 4,
        2, 4, 5, 4, 2,
    };
    float filterFactor = 1/159;
    convolution(pic, result, row, column, height, width, gaussian_filter, filterFactor, 5, sampler);
}


void convolution(
    __read_only image2d_t sourceImage,  //Input image object
    __write_only image2d_t outputImage,  //Output Image object
    int row, int column,
    int rows, int cols,  //Image Dimensions
    __constant float* filter, float filterFactor, int filterWidth,  //Convolution filter object
    sampler_t sampler) { //sampler object passed as argument

    int halfWidth = (int)(filterWidth/2);
    float4 sum = {0.0f, 0.0f, 0.0f, 0.0f};

    int filterIdx =  0;
    int2 coords; // Coordinates for accessing the image
    for(int i =  -halfWidth; i <= halfWidth; i++) {
        coords.y =  row + i;
        for(int j = -halfWidth; j <= halfWidth; j++) {
            coords.x = column + j;
            float4 pixel;
            pixel = read_imagef(sourceImage, sampler, coords);
            sum.x += pixel.x * (filter[filterIdx++]);
        }
    }
    sum.x *= 1.0f/200.0f;

    if(row < rows && column < cols) {
        coords.x = column; 
        coords.y = row;
        write_imagef(outputImage, coords, sum);
    }
}

