//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)

const char *kernel_uchar16_safe_src = 
"__kernel void test_uchar16(__global uchar* data, int iterations) {\n"
"    int gid = get_global_id(0);\n"
"    \n"
"    int base_idx = gid * 16;\n"
"    uchar v0  = data[base_idx];      uchar v1  = data[base_idx + 1];\n"
"    uchar v2  = data[base_idx + 2];  uchar v3  = data[base_idx + 3];\n"
"    uchar v4  = data[base_idx + 4];  uchar v5  = data[base_idx + 5];\n"
"    uchar v6  = data[base_idx + 6];  uchar v7  = data[base_idx + 7];\n"
"    uchar v8  = data[base_idx + 8];  uchar v9  = data[base_idx + 9];\n"
"    uchar v10 = data[base_idx + 10]; uchar v11 = data[base_idx + 11];\n"
"    uchar v12 = data[base_idx + 12]; uchar v13 = data[base_idx + 13];\n"
"    uchar v14 = data[base_idx + 14]; uchar v15 = data[base_idx + 15];\n"
"    \n"
"    for(int i = 0; i < iterations; i++) {\n"
"        v0  = v0  * 2 + 1;  v0  = v0  * 3 - 1;\n"
"        v1  = v1  * 2 + 1;  v1  = v1  * 3 - 1;\n"
"        v2  = v2  * 2 + 1;  v2  = v2  * 3 - 1;\n"
"        v3  = v3  * 2 + 1;  v3  = v3  * 3 - 1;\n"
"        v4  = v4  * 2 + 1;  v4  = v4  * 3 - 1;\n"
"        v5  = v5  * 2 + 1;  v5  = v5  * 3 - 1;\n"
"        v6  = v6  * 2 + 1;  v6  = v6  * 3 - 1;\n"
"        v7  = v7  * 2 + 1;  v7  = v7  * 3 - 1;\n"
"        v8  = v8  * 2 + 1;  v8  = v8  * 3 - 1;\n"
"        v9  = v9  * 2 + 1;  v9  = v9  * 3 - 1;\n"
"        v10 = v10 * 2 + 1;  v10 = v10 * 3 - 1;\n"
"        v11 = v11 * 2 + 1;  v11 = v11 * 3 - 1;\n"
"        v12 = v12 * 2 + 1;  v12 = v12 * 3 - 1;\n"
"        v13 = v13 * 2 + 1;  v13 = v13 * 3 - 1;\n"
"        v14 = v14 * 2 + 1;  v14 = v14 * 3 - 1;\n"
"        v15 = v15 * 2 + 1;  v15 = v15 * 3 - 1;\n"
"    }\n"
"    \n"
"    data[base_idx]      = v0;  data[base_idx + 1]  = v1;\n"
"    data[base_idx + 2]  = v2;  data[base_idx + 3]  = v3;\n"
"    data[base_idx + 4]  = v4;  data[base_idx + 5]  = v5;\n"
"    data[base_idx + 6]  = v6;  data[base_idx + 7]  = v7;\n"
"    data[base_idx + 8]  = v8;  data[base_idx + 9]  = v9;\n"
"    data[base_idx + 10] = v10; data[base_idx + 11] = v11;\n"
"    data[base_idx + 12] = v12; data[base_idx + 13] = v13;\n"
"    data[base_idx + 14] = v14; data[base_idx + 15] = v15;\n"
"}\n";


// ------------------------------------------
// INT8 uchar16
// ------------------------------------------
{
    const double TARGET_BENCHMARK_TIME = 1.5;
    size_t bytes = total_elements * sizeof(unsigned char) * 16; 
    
    unsigned char *h_data = NULL;
#if defined(_WIN32)
    h_data = (unsigned char*)_aligned_malloc(bytes, 64);
    int mem_alloc_ok = (h_data != NULL);
#else 
    int mem_alloc_ok = (posix_memalign((void**)&h_data, 64, bytes) == 0);
#endif
    
    if(mem_alloc_ok) {
        for(int i = 0; i < total_elements * 16; i++) h_data[i] = 1;
        
        cl_mem d_data = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, bytes, h_data, &ret);
        clEnqueueWriteBuffer(command_queue, d_data, CL_TRUE, 0, bytes, h_data, 0, NULL, NULL);
        
        cl_program program = clCreateProgramWithSource(context, 1, &kernel_uchar16_safe_src, NULL, &ret);
        clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
        cl_kernel kernel = clCreateKernel(program, "test_uchar16", &ret);
        
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
                    
                    double total_ops = (double)global_size * (double)final_iterations * 64.0;
                    double giops = (total_ops / actual_time) / 1e9;
                    
                    print_report(" INT8-U16  : %10.2f GIOPS  (%7.2f TIOPS ) [%.2fs, Iters.: %7d]\n", 
                                 giops, giops / 1000.0, actual_time, final_iterations);
                    
                    clReleaseEvent(main_event);
                } else { print_report(" INT8-U16  : Error main kernel (%d).\n", ret); }
            } else { print_report(" INT8-U16  : Calibration failed or error setting arguments.\n"); }
            clReleaseKernel(kernel);
        } else { print_report(" INT8-U16  : Compilation/Buffer error (%d).\n", ret); }
        clReleaseProgram(program);
        clReleaseMemObject(d_data);
        
#if defined(_WIN32)
        _aligned_free(h_data);
#else
        free(h_data);
#endif
    }
}
