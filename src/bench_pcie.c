//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)

{
    const int pcie_sizes_mb[] = {16, 64};
    const int num_pcie_sizes = sizeof(pcie_sizes_mb) / sizeof(pcie_sizes_mb[0]);
    const int num_runs = 50; 
    cl_int ret;
    
    print_report("------------------------------- PCIe Bandwidth ---------------------------------\n");
    print_report(" Block size:          avg -          min -          max - stability | %d runs\n", num_runs);
    
    // ==========================================
    // PCIe Write (Host -> Device)
    // ==========================================
    print_report(" PCIe Write:\n");
    
    for (int s = 0; s < num_pcie_sizes; s++) {
        int target_mb = pcie_sizes_mb[s];
        size_t bytes = (size_t)target_mb * 1024 * 1024;
        
        float *h_buf = NULL;
#if defined(_WIN32)
        h_buf = (float*)VirtualAlloc(NULL, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
        if (posix_memalign((void**)&h_buf, 4096, bytes) != 0) h_buf = NULL;
#endif
        if (!h_buf) continue;
        for (size_t i = 0; i < bytes / sizeof(float); i++) h_buf[i] = 1.0f;
        
        cl_mem d_buf = clCreateBuffer(context, CL_MEM_READ_WRITE, bytes, NULL, &ret);
        if (ret == CL_SUCCESS) {
            double min_time_taken = -1.0; double max_time_taken = -1.0; double total_time_accum = 0.0;
            int successful_runs = 0;
            
            cl_event warmup_event;
            if (clEnqueueWriteBuffer(command_queue, d_buf, CL_TRUE, 0, bytes, h_buf, 0, NULL, &warmup_event) == CL_SUCCESS) {
                clWaitForEvents(1, &warmup_event); clReleaseEvent(warmup_event);
            }
            
            for (int run = 0; run < num_runs; run++) {
                cl_event event;
                ret = clEnqueueWriteBuffer(command_queue, d_buf, CL_FALSE, 0, bytes, h_buf, 0, NULL, &event);
                if (ret == CL_SUCCESS) {
                    clWaitForEvents(1, &event);
                    double current_time = get_execution_time(event);
                    clReleaseEvent(event);
                    total_time_accum += current_time; successful_runs++;
                    if (min_time_taken < 0.0 || current_time < min_time_taken) min_time_taken = current_time;
                    if (max_time_taken < 0.0 || current_time > max_time_taken) max_time_taken = current_time;
                } else { break; }
            }
            
            if (min_time_taken > 0.0 && successful_runs > 0) {
                double max_bandwidth = ((double)bytes / min_time_taken) / (1e9);
                double min_bandwidth = ((double)bytes / max_time_taken) / (1e9);
                double avg_bandwidth = ((double)bytes / (total_time_accum / successful_runs)) / (1e9);
                double stability = (avg_bandwidth / max_bandwidth) * 100.0;
                print_report("   %4d MB : %7.2f GB/s - %7.2f GB/s - %7.2f GB/s - %4.1f%%\n", 
                    target_mb, avg_bandwidth, min_bandwidth, max_bandwidth, stability);
            }
            clReleaseMemObject(d_buf);
        }
#if defined(_WIN32)
        VirtualFree(h_buf, 0, MEM_RELEASE);
#else
        free(h_buf);
#endif
    }

    // ==========================================
    // PCIe Read (Device -> Host)
    // ==========================================
    print_report(" PCIe Read:\n");
    
    for (int s = 0; s < num_pcie_sizes; s++) {
        int target_mb = pcie_sizes_mb[s];
        size_t bytes = (size_t)target_mb * 1024 * 1024;
        
        float *h_buf = NULL;
#if defined(_WIN32)
        h_buf = (float*)VirtualAlloc(NULL, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
        if (posix_memalign((void**)&h_buf, 4096, bytes) != 0) h_buf = NULL;
#endif
        if (!h_buf) continue;
        
        cl_mem d_buf = clCreateBuffer(context, CL_MEM_READ_WRITE, bytes, NULL, &ret);
        if (ret == CL_SUCCESS) {
            double min_time_taken = -1.0; double max_time_taken = -1.0; double total_time_accum = 0.0;
            int successful_runs = 0;
            
            // Разогрев перед чтением
            cl_event warmup_event;
            if (clEnqueueReadBuffer(command_queue, d_buf, CL_TRUE, 0, bytes, h_buf, 0, NULL, &warmup_event) == CL_SUCCESS) {
                clWaitForEvents(1, &warmup_event); clReleaseEvent(warmup_event);
            }
            
            for (int run = 0; run < num_runs; run++) {
                cl_event event;
                // Используем clEnqueueReadBuffer для скачивания данных на Хост
                ret = clEnqueueReadBuffer(command_queue, d_buf, CL_FALSE, 0, bytes, h_buf, 0, NULL, &event);
                if (ret == CL_SUCCESS) {
                    clWaitForEvents(1, &event);
                    double current_time = get_execution_time(event);
                    clReleaseEvent(event);
                    total_time_accum += current_time; successful_runs++;
                    if (min_time_taken < 0.0 || current_time < min_time_taken) min_time_taken = current_time;
                    if (max_time_taken < 0.0 || current_time > max_time_taken) max_time_taken = current_time;
                } else { break; }
            }
            
            if (min_time_taken > 0.0 && successful_runs > 0) {
                double max_bandwidth = ((double)bytes / min_time_taken) / (1e9);
                double min_bandwidth = ((double)bytes / max_time_taken) / (1e9);
                double avg_bandwidth = ((double)bytes / (total_time_accum / successful_runs)) / (1e9);
                double stability = (avg_bandwidth / max_bandwidth) * 100.0;
                print_report("   %4d MB : %7.2f GB/s - %7.2f GB/s - %7.2f GB/s - %4.1f%%\n", 
                    target_mb, avg_bandwidth, min_bandwidth, max_bandwidth, stability);
            }
            clReleaseMemObject(d_buf);
        }
#if defined(_WIN32)
        VirtualFree(h_buf, 0, MEM_RELEASE);
#else
        free(h_buf);
#endif
    }
}
