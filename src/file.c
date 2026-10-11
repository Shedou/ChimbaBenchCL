//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)
// ...

FILE *report_file = NULL;

void print_report(const char *format, ...) {
    va_list args;
    
    // Вывод в консоль
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    // Вывод в файл отчета
    if (report_file != NULL) {
        va_start(args, format);
        vfprintf(report_file, format, args);
        va_end(args);
        fflush(report_file);
    }
}

void write_report(const char *format, ...) {
    va_list args;
    
    if (report_file != NULL) {
        va_start(args, format);
        vfprintf(report_file, format, args);
        va_end(args);
        fflush(report_file);
    }
}

void create_directory_if_not_exists(const char *path) {
    #ifdef _WIN32
        struct stat st;
        memset(&st, 0, sizeof(st));
        if (stat(path, &st) == -1) {
            mkdir(path);
        }
    #else
        if (access(path, F_OK) == -1) {
            mkdir(path, 0777);
        }
    #endif
}
