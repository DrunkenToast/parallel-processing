// Kernel written by Peter Leconte r0830684
// README.md contains more details about my findings and reasoning

__kernel void findPatternOffsets(
        const int sa_width,
        const int W,
        const int count,
        __global const uchar *pixels,
        __constant const int *offsets,
        __global int *res,
        __local int *localOffsets
        ) {
    int col = get_global_id(0);
    int row = get_global_id(1);

    int pos = col + row * W;

    // This is printing 998 x 0 for me, which is very strange...
    // If you know why this is, please let me know :)
    /* if (col == 0 && row == 0) */
    /*     printf("Size: %i x %i\n", get_local_size(0), get_local_size(1)); */

    // README: Local work size 5, copy by local id
    /* int tid = get_local_id(0); */
    /* // Copy to local mem */
    /* localOffsets[tid] = offsets[tid]; */

    // README: Local work size NULL, copy by sync
    /* async_work_group_copy(localOffsets, offsets, count, 0); */
    /* #pragma unroll */
    /* for (uint i = 0; i < count; i++) { */
    /*     localOffsets[i] = offsets[i]; */
    /* } */


    int sum = 0;

    // README: Unrolling 
    // Significantly faster
    #pragma unroll
    for (uint i = 0; i < count; i++) {
         //int abpos = pos + offsets[i];
        /* sum += pixels[pos + localOffsets[i]]; */
        sum += pixels[pos + offsets[i]];
    }

    // Manual/hardcoded unroll
    /* sum += pixels[pos + localOffsets[0]]; */
    /* sum += pixels[pos + localOffsets[1]]; */
    /* sum += pixels[pos + localOffsets[2]]; */
    /* sum += pixels[pos + localOffsets[3]]; */
    /* sum += pixels[pos + localOffsets[4]]; */

    /* sum += pixels[pos + offsets[0]]; */
    /* sum += pixels[pos + offsets[1]]; */
    /* sum += pixels[pos + offsets[2]]; */
    /* sum += pixels[pos + offsets[3]]; */
    /* sum += pixels[pos + offsets[4]]; */


    //Coalesced
    res[col+sa_width*row] = sum;
}
