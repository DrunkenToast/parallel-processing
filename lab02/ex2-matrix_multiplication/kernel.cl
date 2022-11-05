__kernel void matrix(
        __global const int *A,
        __global const int *B,
        __global int *C,
        int Awidth,
        int Bwidth) {
    int i = get_global_id(1);
    int j = get_global_id(0);

    int sum = 0;

    for (int k = 0; k < Awidth; k++) {
        sum += A[i*Awidth+k] * B[k*Bwidth+j];
    }

    C[i * Bwidth + j] = sum;
}

