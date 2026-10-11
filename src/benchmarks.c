//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)

// Функция измерения времени
double get_execution_time(cl_event event) {
    cl_ulong time_start, time_end;
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
    return (double)(time_end - time_start) / 1000000000.0;
}

// Функция калибровки итераций.
int calibrate_kernel_iterations(cl_command_queue command_queue, cl_kernel kernel, size_t global_size, size_t local_size, int iterations_arg_index, double target_time) {
    cl_int ret;
    int calibration_iterations = 8;
    double calib_time = 0.0;
    const double MIN_STABLE_TIME = 0.01; 
    const double MAX_SAFE_CALIB_TIME = 1.4;
    
    while (calibration_iterations <= 6250) {
        ret = clSetKernelArg(kernel, iterations_arg_index, sizeof(int), (void *)&calibration_iterations);
        if (ret != CL_SUCCESS) return -1;
        
        cl_event calib_event;
        ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, &calib_event);
        
        if (ret == CL_SUCCESS) {
            clWaitForEvents(1, &calib_event);
            calib_time = get_execution_time(calib_event);
            // Защита от компиляторов, которые «схлопывают» быстрые циклы в 0 секунд
            if (calib_time <= 0.000000001) {
                calib_time = 0.0001; // Принудительно задаем микроскопическое ненулевое время
            }
            
            clReleaseEvent(calib_event);
            
            // Если калибровка идет слишком долго, немедленно выходим! GeForce 9500 GT не любит работать дольше двух секунд...
            if (calib_time >= MIN_STABLE_TIME || calib_time >= MAX_SAFE_CALIB_TIME || calibration_iterations >= 6250) {
                break;
            }
            
            calibration_iterations *= 2; // Во сколько раз увеличивается нагрузка с каждой калибровкой
        } else {
            return -1;
        }
    }
    
    // Защита от переполнения таймера при сбое
    if (calib_time <= 0.0 || calib_time > 100.0) return 1;
    
    double operations_per_second = (double)calibration_iterations / calib_time;
    int final_iterations = (int)(operations_per_second * target_time);
    
    if (final_iterations < 1) final_iterations = 1;
    return final_iterations;
}

// Функция тестирования
void benchmark_device(cl_device_id device_id, const char* platform_name, int force_intel) {
    cl_int ret;
    
    // Сбор базовой информации о тестируемом устройстве
    char device_name[256] = {0}; char driver_version[256] = {0}; char device_info_cl_ver[256] = {0};
    int device_info_max_cu = 0; int device_info_max_clock = 0;
    cl_ulong device_info_global_mem = 0; cl_ulong cache_size = 0; cl_uint cacheline_size = 0;
    clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(device_name), &device_name, NULL);
    clGetDeviceInfo(device_id, CL_DRIVER_VERSION, sizeof(driver_version), &driver_version, NULL);
    clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(device_info_max_cu), &device_info_max_cu, NULL);
    clGetDeviceInfo(device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(device_info_max_clock), &device_info_max_clock, NULL);
    clGetDeviceInfo(device_id, CL_DEVICE_VERSION, sizeof(device_info_cl_ver), device_info_cl_ver, NULL);
    clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(device_info_global_mem), &device_info_global_mem, NULL);
    clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cache_size), &cache_size, NULL);
    clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(cacheline_size), &cacheline_size, NULL);
    
    double global_mem_gib = (double)device_info_global_mem / (1024.0 * 1024.0 * 1024.0);
    double global_mem_mib = (double)device_info_global_mem / (1024.0 * 1024.0);
    
    // Определяется тип устройства (CPU или GPU)
    cl_device_type dev_type;
    clGetDeviceInfo(device_id, CL_DEVICE_TYPE, sizeof(dev_type), &dev_type, NULL);
    const char* type_str = (dev_type & CL_DEVICE_TYPE_CPU) ? "CPU" : "GPU";
    const int is_cpu = (dev_type & CL_DEVICE_TYPE_CPU);
    
    // Получение расширений OpenCL
    size_t ext_size = 0;
    clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, 0, NULL, &ext_size);
    char* extensions = (char*)malloc(ext_size + 1);
    if (extensions == NULL) { print_report("Memory Allocation Error for Extensions.\n"); return; }
    clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, ext_size, extensions, NULL);
    extensions[ext_size] = '\0';
    
    // Вывод базовой информации о системе
    print_report("\n================================================================================\n");
    print_report(" Device    : %s (%s)\n", device_name, type_str);
    print_report(" CL Info   : %s\n", device_info_cl_ver);
    print_report(" HW Info   : CUs: %d, Mem: %.2f GB (%.0f MB), Freq (base): %d MHz\n", 
                                    device_info_max_cu, global_mem_gib, global_mem_mib, device_info_max_clock);
    print_report(" Cache Info: Cache Size: %lld KB, Line Size: %d Bytes\n", cache_size / 1024, cacheline_size);
    print_report(" Platform  : %s (%s)\n", platform_name, driver_version);
    
    // Проверка, предотвращающая баги или зависание при тестировании Intel GPU.
    if (strstr(platform_name, "Intel") != NULL || strstr(device_name, "Intel(R)") != NULL) {
        if (!is_cpu) {
            // В ходе проверки на встроенной Intel(R) UHD 600 выяснилось, что тесты не работают в среде Linux.
            // В среде Windows нет возможности протестировать данную графику, поэтому Intel GPU пока отключаем по умолчанию.
            
            // Если флаг форсирования НЕ передан, то пропускаем устройство
            if (!force_intel) {
                print_report(" Intel iGPU/GPU detected (bugs): skipping.\n");
                print_report(" Use the \"-enable-intel\" parameter to run the test on the GPU.\n");
                free(extensions); 
                return;
            } else {
                print_report(" [Warning] Intel iGPU forced via parameter. Results may be unstable.\n");
            }
        }
    }
    
    // Проверка наличия расширений fp64 и fp16
    int ext_fp64_present = (strstr(extensions, "cl_khr_fp64") != NULL) ? 1 : 0;
    int ext_fp16_present = 0;
    if (strstr(extensions, "cl_khr_fp16") != NULL) {
        ext_fp16_present = 1; // Обычный путь (AMD, Intel, Mesa, POCL)
    } else if (strstr(platform_name, "NVIDIA") != NULL || strstr(device_name, "GeForce") != NULL || strstr(device_name, "RTX") != NULL) {
        ext_fp16_present = 1; // Путь для NVIDIA (скрывают поддержку FP16): аппаратно FP16 есть на всех картах начиная с архитектуры Volta/Turing...
    }
    
    // Инициализация контекса и очереди команд
    cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    if (ret != CL_SUCCESS) { print_report("Create Context Error.\n"); free(extensions); return; }
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);
    if (ret != CL_SUCCESS) { print_report("Create Command Queue Error.\n"); clReleaseContext(context); free(extensions); return; }
    
    // Динамический размер LOCAL WORK SIZE для старых или слабых устройств
    size_t max_local_size = 1;
    clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_local_size), &max_local_size, NULL);
    
    size_t local_size = 256;
    if (is_cpu) {
        local_size = 64;
    }
    
    // Коррекция local_size, если устройство аппаратно не поддерживает столько
    if (local_size > max_local_size) {
        local_size = max_local_size;
    }
    
    //int total_elements = 512 * 512 * 32;
    // Задаем базовое количество элементов на один вычислительный блок (CU)
    // Для GPU типа 9500GT (4 CUs) это даст ~524k элементов. Для RTX 4070 (~46 CUs) — ~6M.
    size_t elements_per_cu = 128 * 1024; 
    if (is_cpu) {
        elements_per_cu = 64 * 1024;
    }
    
     // Итоговый объем данных прямо пропорционален мощности карты
    long long total_elements = (long long)device_info_max_cu * elements_per_cu;
    
    // Выравниваем total_elements, чтобы он был строго кратен local_size
    total_elements = ((total_elements + local_size - 1) / local_size) * local_size;
    
    // Защитные лимиты (Опционально)
    const long long MIN_ELEMENTS = 256 * 1024;       // Меньше этого делать тест нецелесообразно
    const long long MAX_ELEMENTS = 512 * 512 * 64;   // Максимальный лимит
    
    if (total_elements < MIN_ELEMENTS) total_elements = MIN_ELEMENTS;
    if (total_elements > MAX_ELEMENTS) total_elements = MAX_ELEMENTS;
    
    print_report(" [System]  : Workload dynamically set to %lld elements based on %d CUs.\n", 
                 total_elements, device_info_max_cu);
    
    // Модули тестов
    #include "bench_vram.c"
    #include "bench_latency.c"
    #include "bench_fp16_h8.c"
    #include "bench_fp32_f8.c"
    #include "bench_fp32_fma.c"
    #include "bench_fp64.c"
    #include "bench_int8_u16.c"
    #include "bench_int32.c"
    #include "bench_int64.c"
    #include "bench_image_blur.c"
    
    // Вывод расширений OpenCL в файл (без вывода в консоль)
    write_report("\n List of OpenCL Device Extensions:\n");
    char* ext_copy = strdup(extensions);
    if (ext_copy != NULL) {
        char *token = strtok(ext_copy, " ");
        while (token != NULL) {
            write_report("%s\n", token);
            token = strtok(NULL, " ");
        }
        free(ext_copy);
    }
    
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
    free(extensions);
}

