//MIT License
//
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

// Внимание! Часть кода написана мной, а часть - с помощью ИИ.
// Благодаря помощи ИИ разработка проекта значительно ускорилась, даже несмотря на необходимость тщательной проверки, исправления и доработки кода.
// Тестирование проекта, как правило, проводится вручную на реальном оборудовании и в виртуальных машинах.
// Так как проект новый и проверялся в ограниченном количестве операционных систем (Windows/Linux/FreeBSD), некоторые зависимости для сборки могли быть пропущены.

// ==============================================================================
// Dependencies for build: gcc, ocl-icd-opencl-dev (linux) / ocl-icd (freebsd) / cmake (for compile OpenCL-SDK Sources)
// Compile scripts for FreeBSD / Linux / Windows:
// build-for-freebsd.sh  - Native script for FreeBSD
// build-for-linux.sh    - Native script for Linux
// build-for-windows.sh  - Script for WINE (Linux/FreeBSD)
// build-for-windows.bat - Native script for Windows
//
// OpenCL-SDK:
// https://github.com/KhronosGroup/OpenCL-SDK
// https://github.com/KhronosGroup/OpenCL-SDK/releases/tag/v2026.05.29
// w64devkit (build tools for Windows):
// https://github.com/skeeto/w64devkit
// ==============================================================================

// Костыль для Linux, понижающий требования к версии злополучного GLIBC //
#ifdef __linux__
extern int __real___libc_start_main(int (*main)(int, char **, char **), int argc, char **ubp_av, void (*init)(void), void (*fini)(void), void (*rtld_fini)(void), void *stack_end);
__asm__(".symver __real___libc_start_main,__libc_start_main@GLIBC_2.2.5");
int __wrap___libc_start_main(int (*main)(int, char **, char **), int argc, char **ubp_av, void (*init)(void), void (*fini)(void), void (*rtld_fini)(void), void *stack_end) {
    return __real___libc_start_main(main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}
#endif
// ==================================================================== //

// -------------------------- //
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
#include <time.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #include <direct.h>
    #define mkdir(dir) _mkdir(dir)
#else
    #include <sys/types.h>
    #include <unistd.h>
#endif

#ifdef __APPLE__
    #include <OpenCL/opencl.h>
#else
    #include <CL/cl.h>
#endif
// -------------------------- //

//        общие флаги        //
    int disable_gpu_intel = 0;
    int disable_gpu_amd = 0;
    int disable_gpu_nvidia = 0;
    int disable_cpu = 0;
    int disable_gpu = 0;

// ------------------------- //

// Основной код находится в следующих файлах //
#include "file.c"
#include "md5.c"
#include "benchmarks.c"
// ----------------------------------------  //

const char *ProjectName = "ChimbaBenchCL v1.2 (OpenCL Benchmark, https://github.com/Shedou/ChimbaBenchCL)\n";
const char *ProjectLicense = "MIT License - (c) 2026 Андрей Цымбалов (Chimbal)\n";

void help_function() {
	printf(ProjectName);
	printf(ProjectLicense);
	printf("\nTo disable the CPU/GPU, use the following arguments:\n");
	printf("-disable-cpu: Skip CPU testing.\n");
	printf("-disable-gpu: Skip testing on all GPUs.\n");
	printf("-disable-gpu-amd: Skip testing on AMD GPUs only.\n");
	printf("-disable-gpu-intel: Skip testing on Intel GPUs only.\n");
	printf("-disable-gpu-nvidia: Skip testing on NVIDIA GPUs only.\n");
	printf("\nPress Enter to exit.\n");
	char c; scanf("%c", &c); // Pause...
	exit(0); // Закрывает программу без возврата к функции main()...
}

int main(int argc, char* argv[]) {
    #ifdef _WIN32 // UTF-8 кодировка для консоли Windows
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
    #endif
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0) { help_function(); } // Вызвав функцию справки назад дороги нет...
        if (strcmp(argv[i], "-disable-cpu") == 0) { disable_cpu = 1; }
        if (strcmp(argv[i], "-disable-gpu") == 0) { disable_gpu = 1; }
        if (strcmp(argv[i], "-disable-gpu-intel") == 0) { disable_gpu_intel = 1; }
        if (strcmp(argv[i], "-disable-gpu-amd") == 0) { disable_gpu_amd = 1; }
        if (strcmp(argv[i], "-disable-gpu-nvidia") == 0) { disable_gpu_nvidia = 1; }
    }
    
    char exe_md5[33] = {0};
    get_binary_md5(argv[0], exe_md5);
    
    create_directory_if_not_exists("Results"); // Проверка каталога и его создание при отсутствии
    
    time_t t = time(NULL); // Получаение текущего системного времени
    struct tm tm = *localtime(&t);
    
    char filename[64]; // Формирование уникального имени файла
    snprintf(filename, sizeof(filename), "Results/benchmark_%04d-%02d-%02d_%02d-%02d-%02d.txt",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
    report_file = fopen(filename, "w"); // Открытие файла для записи
    
    if (report_file == NULL) {
        printf("Warning: Failed to create the report file %s\n", filename);
    }/* else {
        printf("The results will be saved to a file: %s\n", filename);
    }*/
    
    print_report(ProjectName);
    print_report(ProjectLicense);
    print_report("Executable MD5: %s - %04d-%02d-%02d %02d:%02d:%02d\n", exe_md5, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec); // Выводим хэш в терминал и файл результатов
    
    cl_uint num_platforms;
    cl_int ret = clGetPlatformIDs(0, NULL, &num_platforms);
    if (ret != CL_SUCCESS || num_platforms == 0) {
        print_report("Error: No OpenCL platforms found.\n");
        return 1;
    }
    
    cl_platform_id *platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
    clGetPlatformIDs(num_platforms, platforms, NULL);
    print_report("OpenCL platforms detected: %u\n", num_platforms);
    
    // Поиск всех устройств
    for (cl_uint i = 0; i < num_platforms; i++) {
        char platform_name[256] = {0};
        clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL);
        
        cl_uint num_devices;
        ret = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
        if (ret != CL_SUCCESS || num_devices == 0) continue;
        
        cl_device_id *devices = (cl_device_id*)malloc(sizeof(cl_device_id) * num_devices);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
        
        for (cl_uint j = 0; j < num_devices; j++) {
            benchmark_device(devices[j], platform_name);
        }
        free(devices);
    }
    free(platforms);
    printf("\nPress Enter to exit.\n");
    char c; scanf("%c", &c); // Pause...
    return 0;
}
