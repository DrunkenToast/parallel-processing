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


    // Min
    for (int i = gid; i < arraySize; i+=size) {
        if (I[i] < currentMin) {
            currentMin = I[i];
        }
    }

    mins[gid] = currentMin;

    arraySize = arraySize / 2;
    size = size / 2;

    while (size > 0) {
            printf("while");
        for (int i = gid; i < arraySize; i+=size) {
            printf("for");
            if (mins[i] < currentMin) {
                currentMin = mins[i];
            }
        }
        size = size / 2;
        arraySize = arraySize / 2;
        mins[gid] = currentMin;
    }
}

