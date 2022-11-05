__kernel void average(
        __global const float *A,
        __global const float *B,
        __global float *C
        ) {
    int i = get_global_id(0);
    C[i] = (A[i]+B[i])/2;
}

__kernel void minmaxavg(
        __global const float *I,
        __global float *mins,
        __global float *maxs,
        __global float *avgs,
        int arraySize
        ) {
    int gid = get_global_id(0);
    int size = get_global_size(0);

    int minIndex = 0;
    float currentMin = INFINITY;

    int maxIndex = 0;
    float currentMax = -INFINITY;
        printf("id: %i", gid);

    // Min
    // Or i = 0; i < size; i++ ??? I think slower??
    for (int i = gid; i < arraySize; i+=size) {
        if (I[i] < currentMin) {
            currentMin = I[i];
        }
    }

    mins[gid] = currentMin;
    /* printf("min: %f on %i\n", mins[gid], gid); */

    arraySize = arraySize / 2;
    size = size / 2;

    /* printf("array size: %i\n", arraySize); */
    while (size > 0) {
        for (int i = gid; i < arraySize; i+=size) {
            if (mins[i] < currentMin) {
                currentMin = mins[i];
            }
        }
        size = size / 2;
        arraySize = arraySize / 2;
        /* printf("array size: %i\n", arraySize); */
    }

    mins[gid] = currentMin;

    /* printf("min: %f on %i\n", mins[gid], gid); */
}

