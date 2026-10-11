//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)

const char *kernel_int32_src = 
"__kernel void test_int32(__global int4* data, int iterations) {\n"
"    int gid = get_global_id(0);\n"
"    int4 val = data[gid];\n"
"    int4 multiplier = (int4)(1103515245);\n"
"    int4 increment  = (int4)(12345);\n"
"    for(int i = 0; i < iterations; i++) {\n"
"        val = (val * multiplier) + increment;\n"
"        val = val ^ (int4)(0x55555555);\n"
"    }\n"
"    data[gid] = val;\n"
"}\n";

// ------------------------------------------
// INT32 int4
// ------------------------------------------
{
    const double TARGET_BENCHMARK_TIME = 1.5;
    size_t bytes = total_elements * sizeof(int) * 4; 
    
    int *h_data = NULL;
#if defined(_WIN32)
    h_data = (int*)_aligned_malloc(bytes, 64);
    int mem_alloc_ok = (h_data != NULL);
#else
    int mem_alloc_ok = (posix_memalign((void**)&h_data, 64, bytes) == 0);
#endif
    
    if(mem_alloc_ok) {
        for(int i = 0; i < total_elements * 4; i++) h_data[i] = 1;
        
        cl_mem d_data = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, bytes, h_data, &ret);
        clEnqueueWriteBuffer(command_queue, d_data, CL_TRUE, 0, bytes, h_data, 0, NULL, NULL);
        
        cl_program program = clCreateProgramWithSource(context, 1, &kernel_int32_src, NULL, &ret);
        clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
        cl_kernel kernel = clCreateKernel(program, "test_int32", &ret);
        
        if (ret == CL_SUCCESS) {
            size_t global_size = total_elements;
            
            clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&d_data);
            
            int final_iterations = calibrate_kernel_iterations(command_queue, kernel, global_size, local_size, 1, TARGET_BENCHMARK_TIME);
            
            if (final_iterations > 0) {
                clSetKernelArg(kernel, 1, sizeof(int), (void *)&final_iterations);
                
                cl_event main_event;
                ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, &main_event);
                
                if (ret == CL_SUCCESS) {
                    clWaitForEvents(1, &main_event);
                    double actual_time = get_execution_time(main_event);
                    double total_ops = (double)global_size * (double)final_iterations * 12.0;
                    double giops = (total_ops / actual_time) / 1e9;
                    
                    print_report(" INT32-I4  : %10.2f GIOPS  (%7.2f TIOPS ) [%.2fs, Iters.: %7d]\n", 
                                 giops, giops / 1000.0, actual_time, final_iterations);
                    
                    clReleaseEvent(main_event);
                } else { 
                    print_report(" INT32-I4  : Error executing kernel (%d).\n", ret);
                }
            } else { print_report(" INT32-I4  : Calibration failed.\n"); }
            clReleaseKernel(kernel);
        } else { print_report(" INT32-VI4  : Compilation error (%d).\n", ret); }
        clReleaseProgram(program); clReleaseMemObject(d_data);
#if defined(_WIN32)
        _aligned_free(h_data);
#else
        free(h_data);
#endif
    }
}
