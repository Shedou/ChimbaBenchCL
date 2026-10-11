//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)

const char *kernel_vram_src = 
"__kernel void test_vram(__global const float16* src, __global float16* dst) {\n"
"    int gid = get_global_id(0);\n"
"    dst[gid] = src[gid];\n" 
"}\n";

{
    const int mem_sizes_mb[] = {16, 64, 256, 512}; // Массив тестируемых объемов памяти в Мегабайтах
    const int num_sizes = sizeof(mem_sizes_mb) / sizeof(mem_sizes_mb[0]);
    const int num_runs = 50; // Количество проходов
    
    print_report("------------------------------- VRAM Bandwidth ---------------------------------\n");
    print_report(" VRAM size :          avg -          min -          max - stability | %d runs\n", num_runs);
    
    for (int s = 0; s < num_sizes; s++) {
        int total_target_mb = mem_sizes_mb[s];
        int single_buffer_mb = total_target_mb / 2; // Делить на 2, так как объём распределяется между ДВУМЯ буферами
        
        // Переводим МБ одного буфера в количество элементов float (1 float = 4 байта)
        const int size = single_buffer_mb * 262144; 
        size_t bytes = size * sizeof(float); 
        
        float *h_src = (float*)malloc(bytes);
        if (!h_src) {
            print_report(" Error: Failed to allocate host memory for %d MB\n", total_target_mb);
            continue;
        }
        
        cl_mem d_src = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, &ret);
        cl_mem d_dst = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bytes, NULL, &ret);
        cl_program program = clCreateProgramWithSource(context, 1, &kernel_vram_src, NULL, &ret);
        clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
        cl_kernel kernel = clCreateKernel(program, "test_vram", &ret);
        
        if (ret == CL_SUCCESS) {
            clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&d_src); 
            clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&d_dst);
            
            size_t global_size = size / 16; // Для float16 ядла делить надо на 16
            double min_time_taken = -1.0;   // Для лучшего результата (Max Speed)
            double max_time_taken = -1.0;   // Для худшего результата (Min Speed)
            double total_time_accum = 0.0;  // Для среднего значения (Avg Speed)
            int successful_runs = 0;
            
            // 1 разогревочный (warmup) запуск вне зачета
            cl_event warmup_event;
            if (clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, &warmup_event) == CL_SUCCESS) {
                clWaitForEvents(1, &warmup_event);
                clReleaseEvent(warmup_event);
            }
            
            // Основной цикл замера скорости
            for (int run = 0; run < num_runs; run++) {
                cl_event event;
                ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, &event);
                
                if (ret == CL_SUCCESS) {
                    clWaitForEvents(1, &event);
                    double current_time = get_execution_time(event);
                    clReleaseEvent(event);
                    
                    total_time_accum += current_time;
                    successful_runs++;
                    
                    // Ищем минимальное время (максимальная скорость)
                    if (min_time_taken < 0.0 || current_time < min_time_taken) { min_time_taken = current_time; }
                    // Ищем максимальное время (минимальная скорость)
                    if (max_time_taken < 0.0 || current_time > max_time_taken) { max_time_taken = current_time; }
                } else {
                    print_report("  [Size %d MB] Error enqueuing kernel on run %d (Code: %d)\n", total_target_mb, run, ret);
                    break;
                }
            }
            
            // Расчет и вывод расширенных результатов
            if (min_time_taken > 0.0 && successful_runs > 0) {
                double total_bytes_transferred = (double)bytes * 2.0; 
                double max_bandwidth = (total_bytes_transferred / min_time_taken) / (1e9); // 1. Максимальная скорость (лучший прогон)
                double min_bandwidth = (total_bytes_transferred / max_time_taken) / (1e9); // 2. Минимальная скорость (худший прогон)
                double avg_time_taken = total_time_accum / successful_runs;
                double avg_bandwidth = (total_bytes_transferred / avg_time_taken) / (1e9); // 3. Средняя скорость
                double stability = (avg_bandwidth / max_bandwidth) * 100.0; // 4. Расчет стабильности в процентах
                print_report("   %4d MB : %7.2f GB/s - %7.2f GB/s - %7.2f GB/s - %4.1f%%\n", 
                    total_target_mb, avg_bandwidth, min_bandwidth, max_bandwidth , stability);
            }
            
            clReleaseKernel(kernel);
        } else {
            print_report(" Error creating kernel or buffers for %d MB (Code: %d)\n", total_target_mb, ret);
        }
        clReleaseProgram(program); 
        clReleaseMemObject(d_src); 
        clReleaseMemObject(d_dst); 
        free(h_src);
    }
    print_report("------------------------------ Compute Benchmarks ------------------------------\n");
}
