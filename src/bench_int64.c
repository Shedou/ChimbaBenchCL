//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)

const char *kernel_int64_src = 
"__kernel void test_int64(__global long2* data, int iterations) {\n"
"    int gid = get_global_id(0);\n"
"    long2 val = data[gid];\n"
"    long2 multiplier = (long2)(6364136223846793005L);\n"
"    long2 increment  = (long2)(1442695040888963407L);\n"
"    for(int i = 0; i < iterations; i++) {\n"
"        val = (val * multiplier) + increment;\n" 
"        val = val ^ (long2)(0x5555555555555555L);\n" 
"    }\n"
"    data[gid] = val;\n"
"}\n";


// ------------------------------------------
// INT64 long2
// ------------------------------------------
{
    const double TARGET_BENCHMARK_TIME = 1.5;
    
    size_t global_size = total_elements / 2;
    size_t bytes = global_size * sizeof(cl_long) * 2; 
    
    cl_long *h_data = NULL;
#if defined(_WIN32)
    h_data = (cl_long*)_aligned_malloc(bytes, 64);
    int mem_alloc_ok = (h_data != NULL);
#else
    int mem_alloc_ok = (posix_memalign((void**)&h_data, 64, bytes) == 0);
#endif
    
    if(mem_alloc_ok) {
        for(size_t i = 0; i < global_size * 2; i++) h_data[i] = 1;
        
        cl_mem d_data = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, bytes, h_data, &ret);
        clEnqueueWriteBuffer(command_queue, d_data, CL_TRUE, 0, bytes, h_data, 0, NULL, NULL);
        
        cl_program program = clCreateProgramWithSource(context, 1, &kernel_int64_src, NULL, &ret);
        clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
        cl_kernel kernel = clCreateKernel(program, "test_int64", &ret);
        
        if (ret == CL_SUCCESS) {
            clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&d_data);
            
            int final_iterations = calibrate_kernel_iterations(command_queue, kernel, global_size, local_size, 1, TARGET_BENCHMARK_TIME);
            
            if (final_iterations > 0) {
                clSetKernelArg(kernel, 1, sizeof(int), (void *)&final_iterations);
                
                cl_event main_event;
                ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, &main_event);
                
                if (ret == CL_SUCCESS) {
                    clWaitForEvents(1, &main_event);
                    double actual_time = get_execution_time(main_event);
                    
                    double total_ops = (double)global_size * (double)final_iterations * 6.0;
                    double giops = (total_ops / actual_time) / 1e9;
                    
                    print_report(" INT64-L2  : %10.2f GIOPS  (%7.2f TIOPS ) [%.2fs, Iters.: %7d]\n", 
                                 giops, giops / 1000.0, actual_time, final_iterations);
                    
                    clReleaseEvent(main_event);
                } else { print_report(" INT64-VL2  : Error executing kernel (%d).\n", ret); }
            } else { print_report(" INT64-L2  : Calibration failed.\n"); }
            clReleaseKernel(kernel);
        } else { print_report(" INT64-L2  : Compilation error (%d).\n", ret); }
        
        clReleaseProgram(program); clReleaseMemObject(d_data);
#if defined(_WIN32)
        _aligned_free(h_data);
#else
        free(h_data);
#endif
    }
}
