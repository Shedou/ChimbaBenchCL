//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)

const char *kernel_blur_fixed_src = 
"__kernel void test_image_blur(__global const float* src, __global float* dst, int width, int height, int iterations) {\n"
"    int gid = get_global_id(0);\n"
"    int x = gid % width;\n"
"    int y = gid / width;\n"
"    if (x >= width || y >= height) return;\n"
"\n"
"    float current_val = src[gid];\n"
"    \n"
"    for (int iter = 0; iter < iterations; iter++) {\n"
"        float sum = current_val;\n"
"        int count = 1;\n"
"        \n"
"        if (x > 0)          { sum += src[gid - 1]; count++; }\n"
"        if (x < width - 1)  { sum += src[gid + 1]; count++; }\n"
"        if (y > 0)          { sum += src[gid - width]; count++; }\n"
"        if (y < height - 1) { sum += src[gid + width]; count++; }\n"
"        \n"
"        current_val = sum / (float)count;\n"
"    }\n"
"    dst[gid] = current_val;\n"
"}\n";

{
    const double TARGET_BENCHMARK_TIME = 1.5;
    
    const int width = 1920;
    const int height = 1080;
    size_t global_size = (size_t)width * (size_t)height;
    size_t bytes = global_size * sizeof(float);
    
    float *h_src = (float*)malloc(bytes);
    float *h_dst = (float*)malloc(bytes);
    
    if (h_src && h_dst) { 
        for (size_t i = 0; i < global_size; i++) h_src[i] = 1.0f;
        
        cl_mem d_src = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, &ret);
        cl_mem d_dst = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bytes, NULL, &ret);
        clEnqueueWriteBuffer(command_queue, d_src, CL_TRUE, 0, bytes, h_src, 0, NULL, NULL);
        
        cl_program program = clCreateProgramWithSource(context, 1, &kernel_blur_fixed_src, NULL, &ret);
        clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
        cl_kernel kernel = clCreateKernel(program, "test_image_blur", &ret);
        
        if (ret == CL_SUCCESS) { 
            clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&d_src);
            clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&d_dst);
            clSetKernelArg(kernel, 2, sizeof(int), (void *)&width);
            clSetKernelArg(kernel, 3, sizeof(int), (void *)&height);
            
            int final_iterations = calibrate_kernel_iterations(command_queue, kernel, global_size, local_size, 4, TARGET_BENCHMARK_TIME);
            
            if (final_iterations > 0) { 
                clSetKernelArg(kernel, 4, sizeof(int), (void *)&final_iterations);
                
                cl_event main_event;
                ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, &main_event);
                
                if (ret == CL_SUCCESS) {
                    clWaitForEvents(1, &main_event);
                    double actual_time = get_execution_time(main_event);
                    double total_pixels_processed = (double)global_size * (double)final_iterations;
                    double mp_s = (total_pixels_processed / actual_time) / 1e6;
                    
                    print_report(" Img Blur  : %10.2f MP/s   (%7.2f GP/s  ) [%.2fs, Iters.: %7d]\n", 
                                 mp_s, mp_s / 1000.0, actual_time, final_iterations); 
                    
                    clReleaseEvent(main_event);
                } else { print_report(" Image Blur  : Error executing kernel (%d).\n", ret); }
            } else { print_report(" Img Blur  : Calibration failed.\n"); }
            clReleaseKernel(kernel);
        } else { print_report(" Img Blur  : Compilation error (%d).\n", ret); }
        
        clReleaseProgram(program); clReleaseMemObject(d_src); clReleaseMemObject(d_dst);
        free(h_src); free(h_dst);
    }
}
