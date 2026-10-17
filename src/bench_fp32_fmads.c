//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)

const char *kernel_fp32_fma_src = 
"__kernel void test_fp32_fma(__global float* data, int iterations) {\n"
"    int gid = get_global_id(0);\n"
"    \n"
"    int base_idx = gid * 8;\n"
"    float v0 = data[base_idx];     float v1 = data[base_idx + 1];\n"
"    float v2 = data[base_idx + 2]; float v3 = data[base_idx + 3];\n"
"    float v4 = data[base_idx + 4]; float v5 = data[base_idx + 5];\n"
"    float v6 = data[base_idx + 6]; float v7 = data[base_idx + 7];\n"
"    \n"
"    float m1 = 1.00001f; float m2 = 0.99999f;\n"
"    float a1 = 0.00001f; float a2 = -0.00001f;\n"
"    \n"
"    for(int i = 0; i < iterations; i++) {\n"
"        // Для современных GPU\n"
"        v0 = fma(v0, m1, a1); v0 = fma(v0, m2, a2);\n"
"        v1 = fma(v1, m1, a1); v1 = fma(v1, m2, a2);\n"
"        v2 = fma(v2, m1, a1); v2 = fma(v2, m2, a2);\n"
"        v3 = fma(v3, m1, a1); v3 = fma(v3, m2, a2);\n"
"        v4 = fma(v4, m1, a1); v4 = fma(v4, m2, a2);\n"
"        v5 = fma(v5, m1, a1); v5 = fma(v5, m2, a2);\n"
"        v6 = fma(v6, m1, a1); v6 = fma(v6, m2, a2);\n"
"        v7 = fma(v7, m1, a1); v7 = fma(v7, m2, a2);\n"
"    }\n"
"    \n"
"    data[base_idx]     = v0; data[base_idx + 1] = v1;\n"
"    data[base_idx + 2] = v2; data[base_idx + 3] = v3;\n"
"    data[base_idx + 4] = v4; data[base_idx + 5] = v5;\n"
"    data[base_idx + 6] = v6; data[base_idx + 7] = v7;\n"
"}\n";

const char *kernel_fp32_mad_src = 
"__kernel void test_fp32_mad(__global float* data, int iterations) {\n"
"    int gid = get_global_id(0);\n"
"    \n"
"    int base_idx = gid * 8;\n"
"    float v0 = data[base_idx];     float v1 = data[base_idx + 1];\n"
"    float v2 = data[base_idx + 2]; float v3 = data[base_idx + 3];\n"
"    float v4 = data[base_idx + 4]; float v5 = data[base_idx + 5];\n"
"    float v6 = data[base_idx + 6]; float v7 = data[base_idx + 7];\n"
"    \n"
"    float m1 = 1.00001f; float m2 = 0.99999f;\n"
"    float a1 = 0.00001f; float a2 = -0.00001f;\n"
"    \n"
"    for(int i = 0; i < iterations; i++) {\n"
"        v0 = mad(v0, m1, a1); v0 = mad(v0, m2, a2);\n"
"        v1 = mad(v1, m1, a1); v1 = mad(v1, m2, a2);\n"
"        v2 = mad(v2, m1, a1); v2 = mad(v2, m2, a2);\n"
"        v3 = mad(v3, m1, a1); v3 = mad(v3, m2, a2);\n"
"        v4 = mad(v4, m1, a1); v4 = mad(v4, m2, a2);\n"
"        v5 = mad(v5, m1, a1); v5 = mad(v5, m2, a2);\n"
"        v6 = mad(v6, m1, a1); v6 = mad(v6, m2, a2);\n"
"        v7 = mad(v7, m1, a1); v7 = mad(v7, m2, a2);\n"
"    }\n"
"    \n"
"    data[base_idx]     = v0; data[base_idx + 1] = v1;\n"
"    data[base_idx + 2] = v2; data[base_idx + 3] = v3;\n"
"    data[base_idx + 4] = v4; data[base_idx + 5] = v5;\n"
"    data[base_idx + 6] = v6; data[base_idx + 7] = v7;\n"
"}\n";

// ------------------------------------------
// FP32 float8 FMA
// ------------------------------------------
{
    const double TARGET_BENCHMARK_TIME = 1.5;
    size_t bytes = total_elements * sizeof(float) * 8; 
    float *h_data = (float*)malloc(bytes);
    
    if(h_data) {
        for(int i = 0; i < total_elements * 8; i++) h_data[i] = 1.0f;
        
        cl_mem d_data = clCreateBuffer(context, CL_MEM_READ_WRITE, bytes, NULL, &ret);
        clEnqueueWriteBuffer(command_queue, d_data, CL_TRUE, 0, bytes, h_data, 0, NULL, NULL);
        
        cl_program program = clCreateProgramWithSource(context, 1, &kernel_fp32_fma_src, NULL, &ret);
        clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
        cl_kernel kernel = clCreateKernel(program, "test_fp32_fma", &ret);
        
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
                    double total_ops = (double)global_size * (double)final_iterations * 32.0;
                    double gflops = (total_ops / actual_time) / 1e9;
                    
                    print_report(" FP32-FMA  : %10.2f GFLOPS (%7.2f TFLOPS) [%.2fs, Iters.: %7d]\n", 
                                 gflops, gflops / 1000.0, actual_time, final_iterations);
                    
                    clReleaseEvent(main_event);
                } else { print_report(" FP32-FMA  : Error main kernel (%d).\n", ret); }
            } else { print_report(" FP32-FMA  : Calibration failed or error setting arguments.\n"); }
            
            clReleaseKernel(kernel);
        } else { print_report(" FP32-FMA  : Compilation/Buffer error (%d).\n", ret); }
        clReleaseProgram(program); 
        clReleaseMemObject(d_data); 
        free(h_data);
    }
}

// ------------------------------------------
// FP32 float8 MAD
// ------------------------------------------
{
    const double TARGET_BENCHMARK_TIME = 1.5;
    size_t bytes = total_elements * sizeof(float) * 8; 
    float *h_data = (float*)malloc(bytes);
    
    if(h_data) {
        for(int i = 0; i < total_elements * 8; i++) h_data[i] = 1.0f;
        
        cl_mem d_data = clCreateBuffer(context, CL_MEM_READ_WRITE, bytes, NULL, &ret);
        clEnqueueWriteBuffer(command_queue, d_data, CL_TRUE, 0, bytes, h_data, 0, NULL, NULL);
        
        cl_program program = clCreateProgramWithSource(context, 1, &kernel_fp32_mad_src, NULL, &ret);
        clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
        cl_kernel kernel = clCreateKernel(program, "test_fp32_mad", &ret);
        
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
                    double total_ops = (double)global_size * (double)final_iterations * 32.0;
                    double gflops = (total_ops / actual_time) / 1e9;
                    
                    print_report(" FP32-MAD  : %10.2f GFLOPS (%7.2f TFLOPS) [%.2fs, Iters.: %7d]\n", 
                                 gflops, gflops / 1000.0, actual_time, final_iterations);
                    
                    clReleaseEvent(main_event);
                } else { print_report(" FP32-MAD  : Error main kernel (%d).\n", ret); }
            } else { print_report(" FP32-MAD  : Calibration failed or error setting arguments.\n"); }
            
            clReleaseKernel(kernel);
        } else { print_report(" FP32-MAD  : Compilation/Buffer error (%d).\n", ret); }
        clReleaseProgram(program); 
        clReleaseMemObject(d_data); 
        free(h_data);
    }
}
