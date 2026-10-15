# ChimbaBenchCL [![Github Releases](https://img.shields.io/github/downloads/Shedou/ChimbaBenchCL/total.svg)](https://github.com/Shedou/ChimbaBenchCL/releases)

ChimbaBenchCL — это легковесный кроссплатформенный бенчмарк с открытым исходным кодом, предназначенный для измерения производительности центральных (CPU) и графических (GPU) процессоров с использованием стандарта OpenCL. Проект был создан при участии искусственного интеллекта (не является сгенерированным ИИ), о чем имеется соответствующая пометка в исходном коде. Использование ИИ позволило значительно ускорить процесс разработки проекта.

<details>
  <summary>English translation</summary>
  
ChimbaBenchCL is a lightweight, cross-platform, open-source benchmark designed to measure the performance of central processing units (CPUs) and graphics processing units (GPUs) using the OpenCL standard. The project was created with the assistance of artificial intelligence (it is not AI-generated), a fact noted within the source code. The use of AI significantly accelerated the project's development process.

</details>

<img width="1920" height="1080" alt="Screenshot" src="https://github.com/Shedou/ChimbaBenchCL/blob/main/Screenshot.jpg" />

## System requirements:
- OS: 64-bit (x86_64) - Windows 7+, Linux (GLIBC 2.7+), FreeBSD
- GPU: 512 MB+ (GeForce 8000+ and Radeon HD 4000+)
- API: OpenCL 1.0+

## Ключевые особенности:
- Тестирование вычислительных блоков в задачах FP16, FP32, FP64, а также целочисленных операций INT8, INT32 и INT64.
- Тест с использованием инструкций FMA (Fused Multiply-Add) и симуляция обработки изображений (Image Blur).
- Измерение пропускной способности и задержек видеопамяти (VRAM Latency / Bandwidth).
- Динамическая калибровка в зависимости от мощности устройства для достижения высокой точности замеров.
- Автоматическое сохранение отчетов в текстовые файлы результатов.

<details>
  <summary>English translation</summary>
  
Key features:
- Testing of compute units using FP16, FP32, and FP64 workloads, as well as INT8, INT32, and INT64 integer operations.
- Testing using FMA (Fused Multiply-Add) instructions and image processing simulation (Image Blur).
- Measurement of VRAM bandwidth and latency.
- Dynamic calibration based on device power to ensure high measurement accuracy.
- Automatic saving of reports to text-based result files.

</details>

## Примечание:
Проект во время разработки проверялся на видеокартах AMD Radeon HD 4870, RX 570, NVIDIA GeForce 9500 GT, GTX 570, GTX 750 и RTX 3060 Mobile, поэтому в системных требованиях указаны только видеокарты NVIDIA и AMD. У меня нет полноценных видеокарт Intel для тестирования, поэтому GPU от Intel по умолчанию игнорируются при запуске теста, так как на встроенной Intel UHD Graphics 600 драйвер графики Intel оптимизировал код OpenCL слишком агрессивно, что приводило к зависаниям теста и бесконечным результатам.

Вполне возможно, с другими видеокартами и драйверами Intel тест заработает без проблем, поэтому я оставил возможность задействовать Intel GPU при тестировании. Для включения Intel GPU нужно запускать программу с аргументом `-enable-intel` (`ChimbaBenchCL.exe -enable-intel` или `./ChimbaBenchCL-FBSD -enable-intel` или `./ChimbaBenchCL-Linux -enable-intel`). Над этой проблемой буду думать уже в следующих версиях бенчмарка.

<details>
  <summary>English translation</summary>
  
Note:\
During development, the project was tested on AMD Radeon HD 4870, RX 570, and NVIDIA GeForce 9500 GT, GTX 570, GTX 750, and RTX 3060 Mobile graphics cards; consequently, only NVIDIA and AMD cards are listed in the system requirements. I do not have dedicated Intel graphics cards available for testing, so Intel GPUs are ignored by default when the test runs. This is because, on integrated Intel UHD Graphics 600, the Intel graphics driver optimized the OpenCL code too aggressively, causing the test to freeze and resulting in endless execution times.

It is quite possible that the test will run without issues on other Intel graphics cards and drivers, so I have included an option to enable Intel GPU testing. To enable Intel GPU support, launch the program with the `-enable-intel` argument (e.g., `ChimbaBenchCL.exe -enable-intel`, `./ChimbaBenchCL-FBSD -enable-intel`, or `./ChimbaBenchCL-Linux -enable-intel`). I plan to address this issue in future versions of the benchmark.

</details>

## Development/Build tools
OpenCL-SDK and w64devkit: [https://github.com/Shedou/ChimbaBenchCL/releases/tag/ChimbaBenchCL-Build-Tools](https://github.com/Shedou/ChimbaBenchCL/releases/tag/ChimbaBenchCL-Build-Tools)
