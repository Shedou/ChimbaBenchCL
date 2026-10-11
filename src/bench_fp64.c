//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)

const char *kernel_fp64_safe_src = 
"#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
"__kernel void test_fp64(__global double* data, int iterations) {\n"
"    int gid = get_global_id(0);\n"
"    \n"
"    int base_idx = gid * 4;\n"
"    double v0 = data[base_idx];     double v1 = data[base_idx + 1];\n"
"    double v2 = data[base_idx + 2]; double v3 = data[base_idx + 3];\n"
"    \n"
"    double mask1 = 1.00000000001;\n"
"    double mask2 = 0.99999999999;\n"
"    double anim  = 0.00000000001;\n"
"    \n"
"    for(int i = 0; i < iterations; i++) {\n"
"        v0 = v0 * mask1 + anim; v0 = v0 * mask2 - anim;\n"
"        v1 = v1 * mask1 + anim; v1 = v1 * mask2 - anim;\n"
"        v2 = v2 * mask1 + anim; v2 = v2 * mask2 - anim;\n"
"        v3 = v3 * mask1 + anim; v3 = v3 * mask2 - anim;\n"
"    }\n"
"    \n"
"    data[base_idx]     = v0; data[base_idx + 1] = v1;\n"
"    data[base_idx + 2] = v2; data[base_idx + 3] = v3;\n"
"}\n";

// ------------------------------------------
// FP64 double4
// ------------------------------------------
{
    const double TARGET_BENCHMARK_TIME = 1.5;
    if (ext_fp64_present != 1) {
        print_report(" FP64-D4   : Skipped (Extension not supported).\n");
    } else {
        // Узнаем объем памяти устройства для защиты от TDR на старых картах
        cl_ulong global_mem_size = 0;
        clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem_size), &global_mem_size, NULL);

        size_t fp64_global_size = total_elements / 4; 
        
        // ПРЕДОХРАНИТЕЛЬ: если памяти на карте меньше 1.5 ГБ,
        // искусственно урезаем сетку в 8 раз, чтобы проход не вешал систему.
        int is_old_gpu = (global_mem_size < (cl_ulong)1500 * 1024 * 1024);
        if (is_old_gpu) {
            fp64_global_size /= 8;
        }

        // Выравнивание под локальную группу
        if (fp64_global_size % local_size != 0) {
            fp64_global_size = ((fp64_global_size / local_size) + 1) * local_size;
        }
        
        size_t bytes = fp64_global_size * sizeof(double) * 4; 
        double *h_data = (double*)malloc(bytes);
        
        if(h_data) {
            for(size_t i = 0; i < fp64_global_size * 4; i++) h_data[i] = 1.0;
            
            cl_mem d_data = clCreateBuffer(context, CL_MEM_READ_WRITE, bytes, NULL, &ret);
            clEnqueueWriteBuffer(command_queue, d_data, CL_TRUE, 0, bytes, h_data, 0, NULL, NULL);
            
            cl_program program = clCreateProgramWithSource(context, 1, &kernel_fp64_safe_src, NULL, &ret);
            clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
            cl_kernel kernel = clCreateKernel(program, "test_fp64", &ret);
            
            if (ret == CL_SUCCESS) {
                clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&d_data);
                
                int final_iterations = calibrate_kernel_iterations(command_queue, kernel, fp64_global_size, local_size, 1, TARGET_BENCHMARK_TIME);
                
                if (final_iterations > 0) {
                    clSetKernelArg(kernel, 1, sizeof(int), (void *)&final_iterations);
                    
                    cl_event main_event;
                    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &fp64_global_size, &local_size, 0, NULL, &main_event);
                    
                    if (ret == CL_SUCCESS) {
                        clWaitForEvents(1, &main_event);
                        double actual_time = get_execution_time(main_event);
                        double total_flops = (double)fp64_global_size * (double)final_iterations * 16.0;
                        double gflops = (total_flops / actual_time) / 1e9;
                        
                        print_report(" FP64-D4   : %10.2f GFLOPS (%7.2f TFLOPS) [%.2fs, Iters.: %7d]\n", 
                                     gflops, gflops / 1000.0, actual_time, final_iterations);
                        
                        clReleaseEvent(main_event);
                    } else { print_report(" FP64-D4   : Error main kernel (%d).\n", ret); }
                } else { print_report(" FP64-D4   : Calibration failed.\n"); }
                clReleaseKernel(kernel);
            } else { print_report(" FP64-D4   : Compilation/Buffer error (%d).\n", ret); }
            clReleaseProgram(program); 
            clReleaseMemObject(d_data); 
            free(h_data);
        }
    }
}
