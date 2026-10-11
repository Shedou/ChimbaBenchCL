//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)

const char *kernel_latency_src = 
"__kernel void test_latency(__global const int* data, __global int* output, int iterations) {\n"
"    int gid = get_global_id(0);\n"
"    if (gid > 0) return; // Тест задержки всегда выполняется строго в ОДНОМ потоке\n"
"\n"
"    int idx = 0;\n"
"    for (int i = 0; i < iterations; i++) {\n"
"        idx = data[idx]; // Переход по случайному указателю (Pointer Chasing)\n"
"    }\n"
"    output[0] = idx; // Предотвращаем мертвый код\n"
"}\n";

// Адаптивная калибровка шагов для теста задержки памяти
int calibrate_latency_hops(cl_command_queue command_queue, cl_kernel kernel, double target_time) {
    cl_int ret;
    int calib_hops = 10000;
    double calib_time = 0.0;
    const double MIN_STABLE_TIME = 0.01;
    size_t global_size = 1;
    size_t local_size = 1;

    // Плавный подъем нагрузки (10k -> 50k -> 250k -> 1.25M -> 6.25M)
    while (calib_hops <= 6250000) {
        ret = clSetKernelArg(kernel, 2, sizeof(int), (void *)&calib_hops);
        if (ret != CL_SUCCESS) return -1;

        cl_event calib_event;
        ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, &calib_event);
        
        if (ret == CL_SUCCESS) {
            clWaitForEvents(1, &calib_event);
            calib_time = get_execution_time(calib_event);
            clReleaseEvent(calib_event);

            if (calib_time >= MIN_STABLE_TIME || calib_hops >= 1250000) {
                break;
            }
            calib_hops *= 5;
        } else {
            return -1;
        }
    }

    if (calib_time <= 0.0) return 10000;

    double hops_per_second = (double)calib_hops / calib_time;
    int final_hops = (int)(hops_per_second * target_time);
    
    if (final_hops < 10000) final_hops = 10000;
    if (final_hops > 5000000) final_hops = 5000000;
    
    return final_hops;
}

// ------------------------------------------
// Memory Latency Test
// ------------------------------------------
{
    const size_t latency_elements = 64 * 1024 * 1024;  // 64 * 1024 * 1024 = 64 MB
    size_t bytes_in = latency_elements * sizeof(int);
    size_t bytes_out = sizeof(int);
    
    int *h_lat_data = (int*)malloc(bytes_in);
    int *h_lat_out = (int*)malloc(bytes_out);
    
    if (h_lat_data && h_lat_out) {
        for (size_t i = 0; i < latency_elements; i++) h_lat_data[i] = (int)i;
        
        srand(1337); 
        for (size_t i = latency_elements - 1; i > 0; i--) {
            size_t j = rand() % (i + 1);
            int temp = h_lat_data[i];
            h_lat_data[i] = h_lat_data[j];
            h_lat_data[j] = temp;
        }
        
        // Используем чистый изолированный буфер без флага HOST_PTR
        cl_mem d_lat_data = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes_in, NULL, &ret);
        cl_mem d_lat_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bytes_out, NULL, &ret);
        clEnqueueWriteBuffer(command_queue, d_lat_data, CL_TRUE, 0, bytes_in, h_lat_data, 0, NULL, NULL);
        
        cl_program program = clCreateProgramWithSource(context, 1, &kernel_latency_src, NULL, &ret);
        clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
        cl_kernel kernel = clCreateKernel(program, "test_latency", &ret);
        
        if (ret == CL_SUCCESS) {
            size_t global_size = 1; 
            size_t local_size_lat = 1;
            
            clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&d_lat_data);
            clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&d_lat_out);
            
            int lat_iterations = calibrate_latency_hops(command_queue, kernel, 1.5);
            
            if (lat_iterations > 0) {
                clSetKernelArg(kernel, 2, sizeof(int), (void *)&lat_iterations);
                
                cl_event main_event;
                ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size_lat, 0, NULL, &main_event);
                
                if (ret == CL_SUCCESS) {
                    clWaitForEvents(1, &main_event);
                    double actual_time = get_execution_time(main_event);
                    
                    double latency_ns = (actual_time * 1e9) / (double)lat_iterations;
                    
                    print_report(" VRAM Lat. : %10.2f ns     [%7.2fs, Hops: %d, 64 MB]\n", 
                                 latency_ns, actual_time, lat_iterations);
                    
                    clReleaseEvent(main_event);
                } else { print_report(" VRAM Lat. : Error main kernel (%d).\n", ret); }
            } else { print_report(" VRAM Lat. : Calibration failed.\n"); }
            clReleaseKernel(kernel);
        } else { print_report(" VRAM Lat. : Compilation error (%d).\n", ret); }
        
        clReleaseProgram(program); clReleaseMemObject(d_lat_data); clReleaseMemObject(d_lat_out);
        free(h_lat_data); free(h_lat_out);
    }
}
