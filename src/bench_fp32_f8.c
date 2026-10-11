//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)

const char *kernel_fp32_float8_safe_src = 
"__kernel void test_fp32_float8(__global float* data, int iterations) {\n"
"    int gid = get_global_id(0);\n"
"    \n"
"    int base_idx = gid * 8;\n"
"    float v0 = data[base_idx];     float v1 = data[base_idx + 1];\n"
"    float v2 = data[base_idx + 2]; float v3 = data[base_idx + 3];\n"
"    float v4 = data[base_idx + 4]; float v5 = data[base_idx + 5];\n"
"    float v6 = data[base_idx + 6]; float v7 = data[base_idx + 7];\n"
"    \n"
"    float mask1 = 1.00001f; float mask2 = 0.99999f;\n"
"    float anim  = 0.00001f;\n"
"    \n"
"    for(int i = 0; i < iterations; i++) {\n"
"        v0 = v0 * mask1 + anim; v0 = v0 * mask2 - anim;\n"
"        v1 = v1 * mask1 + anim; v1 = v1 * mask2 - anim;\n"
"        v2 = v2 * mask1 + anim; v2 = v2 * mask2 - anim;\n"
"        v3 = v3 * mask1 + anim; v3 = v3 * mask2 - anim;\n"
"        v4 = v4 * mask1 + anim; v4 = v4 * mask2 - anim;\n"
"        v5 = v5 * mask1 + anim; v5 = v5 * mask2 - anim;\n"
"        v6 = v6 * mask1 + anim; v6 = v6 * mask2 - anim;\n"
"        v7 = v7 * mask1 + anim; v7 = v7 * mask2 - anim;\n"
"    }\n"
"    \n"
"    data[base_idx]     = v0; data[base_idx + 1] = v1;\n"
"    data[base_idx + 2] = v2; data[base_idx + 3] = v3;\n"
"    data[base_idx + 4] = v4; data[base_idx + 5] = v5;\n"
"    data[base_idx + 6] = v6; data[base_idx + 7] = v7;\n"
"}\n";

{
    const double TARGET_BENCHMARK_TIME = 1.5;
    size_t bytes = total_elements * sizeof(float) * 8; 
    float *h_data = (float*)malloc(bytes);
    
    if(h_data) {
        for(int i = 0; i < total_elements * 8; i++) h_data[i] = 1.0f;
        
        cl_mem d_data = clCreateBuffer(context, CL_MEM_READ_WRITE, bytes, NULL, &ret);
        clEnqueueWriteBuffer(command_queue, d_data, CL_TRUE, 0, bytes, h_data, 0, NULL, NULL);
        
        cl_program program = clCreateProgramWithSource(context, 1, &kernel_fp32_float8_safe_src, NULL, &ret);
        clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
        cl_kernel kernel = clCreateKernel(program, "test_fp32_float8", &ret);
        
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
                    
                    print_report(" FP32-F8   : %10.2f GFLOPS (%7.2f TFLOPS) [%.2fs, Iters.: %7d]\n", 
                                 gflops, gflops / 1000.0, actual_time, final_iterations);
                    
                    clReleaseEvent(main_event);
                } else { print_report(" FP32-F8   : Error main kernel (%d).\n", ret); }
            } else { print_report(" FP32-F8   : Calibration failed.\n"); }
            
            clReleaseKernel(kernel);
        } else { print_report(" FP32-F8   : Compilation/Buffer error (%d).\n", ret); }
        clReleaseProgram(program); 
        clReleaseMemObject(d_data); 
        free(h_data);
    }
}
