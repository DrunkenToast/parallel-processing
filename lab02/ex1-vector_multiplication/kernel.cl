__kernel void vector_mult(__global const int *A, __global const int *B, __global int *O) {
	int i = get_global_id(0);
    O[i] = A[i] * B[i];
}

