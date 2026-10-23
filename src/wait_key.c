//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)

void wait_for_exit_key() {
    printf("\nPress Enter or Escape to exit.\n");
    
    while (1) {
        int key = 0;
        
#ifdef _WIN32
        key = _getch();
#else
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO); // Отключаем буферизацию строки и эхо ввода
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        
        key = getchar();
        
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Возвращаем настройки терминала назад
#endif
        // Проверяем коды клавиш:
        // 13  - Enter (в Windows \r)
        // 10  - Enter (в UNIX \n)
        // 27  - Escape
        if (key == 13 || key == 10 || key == 27) {
            break;
        }
    }
}
