# ChimbaBenchCL [![Github Releases](https://img.shields.io/github/downloads/Shedou/ChimbaBenchCL/total.svg)](https://github.com/Shedou/ChimbaBenchCL/releases)

ChimbaBenchCL — это легковесный кроссплатформенный бенчмарк с открытым исходным кодом, предназначенный для измерения производительности центральных (CPU) и графических (GPU) процессоров с использованием стандарта OpenCL. Проект был создан при участии искусственного интеллекта (не является сгенерированным ИИ), о чем имеется соответствующая пометка в исходном коде. Использование ИИ позволило значительно ускорить процесс разработки проекта.

<details>
  <summary>English translation</summary>
  
ChimbaBenchCL is a lightweight, cross-platform, open-source benchmark designed to measure the performance of central processing units (CPUs) and graphics processing units (GPUs) using the OpenCL standard. The project was created with the assistance of artificial intelligence (it is not AI-generated), a fact noted within the source code. The use of AI significantly accelerated the project's development process.

</details>

<img width="1920" height="1080" alt="Screenshot" src="https://github.com/Shedou/ChimbaBenchCL/blob/main/Screenshot.jpg" />

## System requirements:
- OS: 64-bit (x86_64) - Windows 7+, Linux (GLIBC 2.7+), FreeBSD
- GPU: 512 MB+ (GeForce 8000+ / Radeon HD 4000+ / Intel Gen7+)
- API: OpenCL 1.0+

## Ключевые особенности:
- Тестирование вычислительных блоков в задачах FP16, FP32, FP64, а также целочисленных операций INT8, INT32 и INT64.
Тест с использованием инструкций FMA (Fused Multiply-Add), MAD (Multiply–Add) и симуляция обработки изображений (Image Blur).
- Измерение пропускной способности и задержек видеопамяти (VRAM Latency / Bandwidth).
- Динамическая калибровка в зависимости от мощности устройства для достижения высокой точности замеров.
- Автоматическое сохранение отчетов в текстовые файлы результатов.

<details>
  <summary>English translation</summary>
  
Key features:
- Testing of compute units using FP16, FP32, and FP64 workloads, as well as INT8, INT32, and INT64 integer operations.
- Testing using FMA (Fused Multiply-Add), MAD (Multiply–Add) instructions and image processing simulation (Image Blur).
- Measurement of VRAM bandwidth and latency.
- Dynamic calibration based on device power to ensure high measurement accuracy.
- Automatic saving of reports to text-based result files.

</details>

## Примечание:
Нормальное время прохождения бенчмарка для одного устройства — около 20-30 секунд. Если тест длится слишком долго, возможно, у вас Intel UHD 600 под управлением Linux. В таком случае вы можете отключить проблемное устройство, используя команды `-disable-gpu-intel` и подобные. Узнать список команд можно, запустив программу с аргументом `-help` или `--help`.

<details>
  <summary>English translation</summary>
  
Note:\
A typical benchmark run for a single device takes about 30 seconds. If the test takes significantly longer, you might be using an Intel UHD 600 GPU on Linux. In such cases, you can disable the problematic device using commands like `-disable-gpu-intel` (or similar). You can view a list of available commands by running the program with the `-help` or `--help` argument.

</details>

## Development/Build tools
OpenCL-SDK and w64devkit: [https://github.com/Shedou/ChimbaBenchCL/releases/tag/ChimbaBenchCL-Build-Tools](https://github.com/Shedou/ChimbaBenchCL/releases/tag/ChimbaBenchCL-Build-Tools)
