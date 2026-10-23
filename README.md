# ChimbaBenchCL [![Github Releases](https://img.shields.io/github/downloads/Shedou/ChimbaBenchCL/total.svg)](https://github.com/Shedou/ChimbaBenchCL/releases)

ChimbaBenchCL — это легковесный кроссплатформенный бенчмарк с открытым исходным кодом, предназначенный для измерения производительности центральных (CPU) и графических (GPU) процессоров с использованием стандарта OpenCL. Проект был создан при участии искусственного интеллекта (не является полностью сгенерированным ИИ), о чем имеется соответствующая пометка в исходном коде. Использование ИИ позволило значительно ускорить процесс разработки проекта.

<details>
  <summary>English translation</summary>
  
ChimbaBenchCL is a lightweight, cross-platform, open-source benchmark designed to measure the performance of central processing units (CPUs) and graphics processing units (GPUs) using the OpenCL standard. The project was created with the assistance of artificial intelligence (is not entirely AI-generated), a fact noted within the source code. The use of AI significantly accelerated the project's development process.

</details>

<img width="1920" height="1080" alt="Screenshot" src="https://github.com/Shedou/ChimbaBenchCL/blob/main/Screenshot.jpg" />

## System requirements:
- OS: [64-bit (x86_64) - Windows 7+, Linux (GLIBC 2.7+), FreeBSD](https://github.com/Shedou/ChimbaBenchCL/releases/tag/CBCL_12)
- OS: [32-bit (x86) - Windows XP+, Linux (GLIBC 2.7+)](https://github.com/Shedou/ChimbaBenchCL/releases/tag/CBCL_12)
- GPU: 512 MB+ (GeForce 8000+ / Radeon HD 4000+ / Intel Gen7+)
- API: OpenCL 1.0+

## Набор тестов (v1.2):
- PCIe Bandwidth - пропускная способность шины PCI Express.
- VRAM Bandwidth - пропускная способность памяти.
- VRAM Latency - задержки случайного доступа памяти.
- FP16-H8 - половинная точность (half8).
- FP32-F8 - одинарная точность (float8).
- FP32-FMA - одинарная точность с использованием инструкций FMA.
- FP32-MAD - одинарная точность с использованием инструкций MAD.
- FP64-D4 - двойная точность (double4).
- INT8-U16 - целочисленный тест низкой точности (uchar16).
- INT32-I4 - целочисленный тест обычной точности (int4).
- INT64-L2 - целочисленный тест высокой точности (long2).
- Image Blur - симуляция обработки графики в реальных задачах (1920x1080, Box Blur).

<details>
  <summary>English translation</summary>
  
Test suite (v1.2):
- PCIe Bandwidth – the bandwidth of the PCI Express bus.
- VRAM Bandwidth – memory bandwidth.
- VRAM Latency – memory random-access latency.
- FP16-H8 – half-precision (half8).
- FP32-F8 – single-precision (float8).
- FP32-FMA – single-precision using FMA (fused multiply-add) instructions.
- FP32-MAD – single-precision using MAD (multiply-add) instructions.
- FP64-D4 – double-precision (double4).
- INT8-U16 – low-precision integer test (uchar16).
- INT32-I4 – standard-precision integer test (int4).
- INT64-L2 – high-precision integer test (long2).
- Image Blur – simulation of graphics processing in real-world tasks (1920x1080, Box Blur).

</details>

## Примечание:
Нормальное время прохождения бенчмарка для одного устройства — около 20-30 секунд. Если тест длится слишком долго, возможно, у вас Intel UHD 600 под управлением Linux. В таком случае вы можете отключить проблемное устройство, используя команды `-disable-gpu-intel` и подобные. Узнать список команд можно, запустив программу с аргументом `-help` или `--help`.

<details>
  <summary>English translation</summary>
  
Note:\
A typical benchmark run for a single device takes about 30 seconds. If the test takes significantly longer, you might be using an Intel UHD 600 GPU on Linux. In such cases, you can disable the problematic device using commands like `-disable-gpu-intel` (or similar). You can view a list of available commands by running the program with the `-help` or `--help` argument.

</details>

## Development/Build tools
OpenCL-SDK and w64devkit: [https://github.com/Shedou/ChimbaBenchCL/releases/tag/ChimbaBenchCL-Build-Tools-11](https://github.com/Shedou/ChimbaBenchCL/releases/tag/ChimbaBenchCL-Build-Tools-11)
