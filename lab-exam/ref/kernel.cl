#define SOFTENING 1e-9f

__kernel void nbody(
        __global float4 *pos,
        __global float4 *vel,
        __local float4 *localPos,
        float dt,
        int N
        ) {
    int i = get_global_id(0);
    int tid = get_local_id(0);

    int localSize = get_local_size(0);
    int numTiles = N / localSize;

    float4 F = {0,0,0,0};

    for (int t = 0; t < numTiles; t++) {
        int idx = t * localSize + tid;
        /* float4* currentPos = &pos[idx]; */
        /* localPos[tid] = *currentPos; */
        localPos[tid] = pos[idx];

        barrier(CLK_LOCAL_MEM_FENCE);
        
        #pragma unroll
        for (int j = 0; j < localSize; j++) {
            /* float4 d = {0,0,0,0}; */
            int globalIdx = t * localSize + j;
            float4 d = localPos[j] - pos[i]; // idk

            float distSqr = d.x*d.x + d.y*d.y + d.z*d.z + SOFTENING;

            float invDist = 1.0f / sqrt(distSqr);
            float invDist3 = invDist * invDist * invDist;

            F.x += d.x * invDist3;
            F.y += d.y * invDist3;
            F.z += d.z * invDist3;
        }

        // Sync for next tile load
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    vel[i].x += dt*F.x;
    vel[i].y += dt*F.y;
    vel[i].z += dt*F.z;

    pos[i].x += vel[i].x*dt;
    pos[i].y += vel[i].y*dt;
    pos[i].z += vel[i].z*dt;

}
