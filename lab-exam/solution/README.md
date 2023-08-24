Peter Leconte 3AD r0830684

# Practical exam Parallel processing

## Optimizations done

- Coalesced access for writing to results
- Using NULL for local work size (not 5,1 or similar)
- Unrolling the loop that sums up the pixels
- Using constant memory for offsets in the kernel

Turn off the optimizations flags for better performance, as 
these may cause performance losses.

Platform and device info:
```
Name: NVIDIA CUDA
Vendor: NVIDIA Corporation
Version: OpenCL 3.0 CUDA 12.0.89
Profile: FULL_PROFILE

Name: NVIDIA GeForce RTX 2060
Vendor: NVIDIA Corporation
```

## Findings

### Reference code (no OpenCL)

Elapsed time 1 = 249.643000 ms
Elapsed time 2 = 750.429000 ms
Elapsed time 3 = 673.428000 ms
Elapsed time 4 = 369.929000 ms

These values were very inconsistent but never went under 249 ms.

### First/naïve implementation (coalesced)

No unrolling, no local memory but coalesced since I did it from the start.

Average time = 1.61 ms

```c
    //Coalesced
    res[col+sa_width*row] = sum;
```

### Uncoalesced (for fun)

Changing the writing access so it isn't coalesced, just to try it out and see 
the difference:
```c
    // NOT coalesced
    res[row+sa_width*col] = sum;
```

Average time = 4.13 ms

Significantly slower

### Unrolling

Unrolling manual/hardcoded unroll:
Average time = 1.51 ms

```c
    sum += pixels[pos + offsets[0]];
    sum += pixels[pos + offsets[1]];
    sum += pixels[pos + offsets[2]];
    sum += pixels[pos + offsets[3]];
    sum += pixels[pos + offsets[4]];
```

Unrolling pragma unroll:
Average time = 1.51 ms

```c
    #pragma unroll
    for (uint i = 0; i < count; i++) {
         //int abpos = pos + offsets[i];
        /* sum += pixels[pos + localOffsets[i]]; */
        sum += pixels[pos + offsets[i]];
    }
```

Manual unroll vs pragma unroll had no noticeble difference,
thus pragma is preferable sinnce manual had count hardcoded.

### Use of local memory

Local memory wasn't very useful or fast, likely due to low 
item count in the local memory not being work the overhead.

#### Local work size null (like before), copy by async

Like this now:
```c
    async_work_group_copy(localOffsets, offsets, count, 0);
```

Average time = 1.71 ms

Slower than plain global memory access

#### Local work size null, copy by unrolled for loop

Average time = 2.08 ms

Slower than async

#### Local work size 5, copy by local id

Like this:
```c
int tid = get_local_id(0); */
// Copy to local mem */
localOffsets[tid] = offsets[tid]; */
```

Average time = 10.8 ms - 12.5 ms

Significantly lower due to low item size.

### Use of constant memory

> Constant memory is a memory space to hold data that is accessed simultaneously by all work-items
> Usually maps to specialized caching hardware that has a fixed size
> - It should NOT be used for general input data (e.g. an input buffer) that is read-only
> Examples of useful data to place in constant memory
> - Convolution filters, Kmeans cluster centriods, etc.
> Advantages for AMD hardware
> - If all work-items access the same address, then only one access request will be generated per wavefront 
> - Constant memory can reduce pressure from L1 cache
> - Constant memory has lower latency than L1 cache

#### Setting offset to constant

Offset does not change so we can try using constant memory instead.

```c
    //__global const int *offsets,
    __constant const int *offsets,
```

Average time = 1.32 ms

Does does increase performance by quite a margin.

#### Setting pixels to constant?

> It should **NOT** be used for general input data (e.g. an input buffer) 
> that is read-only.

If we do this we do get an invalid kernel arg error (-52).

