__kernel void hello_world(__global const char *I, __global char *O) {
	int i = get_global_id(0);
    O[i] = I[i];
}

