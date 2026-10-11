//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)

const char *kernel_fp16_h8_src =
"#pragma OPENCL EXTENSION cl_khr_fp16 : enable\n"
"\n"
"__kernel void test_fp16_h8_src(__global half8* data, int iterations) {\n"
"    int gid = get_global_id(0);\n"
"    half8 val = data[gid];\n"
"    \n"
"    half8 mask1 = (half8)(1.00001h);\n"
"    half8 mask2 = (half8)(0.99999h);\n"
"    \n"
"    for(int i = 0; i < iterations; i++) {\n"
"        val = val * mask1 + (half8)(0.00001h);\n"
"        val = val * mask2 - (half8)(0.00001h);\n"
"    }\n"
"    data[gid] = val;\n"
"}\n";

// ------------------------------------------
// FP16 half8
// ------------------------------------------
{
    const double TARGET_BENCHMARK_TIME = 1.5;
    if (ext_fp16_present != 1) {
        print_report(" FP16-H8   : Skipped (Extension not supported).\n");
    } else {
        size_t bytes = total_elements * sizeof(unsigned short) * 8; 
        unsigned short *h_data = (unsigned short*)malloc(bytes);
        
        if(h_data) {
            for(int i = 0; i < total_elements * 8; i++) h_data[i] = 1;
            
            cl_mem d_data = clCreateBuffer(context, CL_MEM_READ_WRITE, bytes, NULL, &ret);
            clEnqueueWriteBuffer(command_queue, d_data, CL_TRUE, 0, bytes, h_data, 0, NULL, NULL);
            
            cl_program program = clCreateProgramWithSource(context, 1, &kernel_fp16_h8_src, NULL, &ret);
            clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
            cl_kernel kernel = clCreateKernel(program, "test_fp16_h8_src", &ret);
            
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
                        
                        print_report(" FP16-H8   : %10.2f GFLOPS (%7.2f TFLOPS) [%.2fs, Iters.: %7d]\n", 
                                     gflops, gflops / 1000.0, actual_time, final_iterations);
                        
                        clReleaseEvent(main_event);
                    } else { print_report(" FP16-H8   : Error main kernel (%d).\n", ret); }
                } else { print_report(" FP16-H8   : Calibration failed or error setting arguments.\n"); }
                
                clReleaseKernel(kernel);
            } else { print_report(" FP16-H8   : Compilation/Buffer error (%d).\n", ret); }
            clReleaseProgram(program);
            clReleaseMemObject(d_data);
            free(h_data);
        }
    }
}
