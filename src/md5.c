//MIT License
//Copyright (c) 2026 Андрей Цымбалов (Chimbal)

#define MD5_F(x,y,z) (((x) & (y)) | (~(x) & (z)))
#define MD5_G(x,y,z) (((x) & (z)) | ((y) & ~(z)))
#define MD5_H(x,y,z) ((x) ^ (y) ^ (z))
#define MD5_I(x,y,z) ((y) ^ ((x) | ~(z)))
#define MD5_ROT(x,n) (((x) << (n)) | ((x) >> (32-(n))))

#define MD5_STEP(f,a,b,c,d,x,s,ac) { \
    (a) += f((b),(c),(d)) + (x) + (unsigned int)(ac); \
    (a) = MD5_ROT((a),(s)) + (b); \
}

void get_binary_md5(const char* exec_path, char* out_md5) {
    strcpy(out_md5, "UNKNOWN/ERROR");
    
    FILE* f = fopen(exec_path, "rb");
    if (!f) return;
    
    // Определяем размер файла
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (file_size <= 0) { fclose(f); return; }
    
    // Выделяем память под весь файл + запас на выравнивание (padding) MD5
    long padded_size = ((file_size + 8) / 64 + 1) * 64;
    unsigned char* buf = (unsigned char*)calloc(padded_size, 1);
    if (!buf) { fclose(f); return; }
    
    // Считываем файл целиком в память
    fread(buf, 1, file_size, f);
    fclose(f);
    
    // Набивка (Padding) по стандарту MD5
    buf[file_size] = 0x80;
    unsigned long long bits_size = (unsigned long long)file_size * 8;
    memcpy(&buf[padded_size - 8], &bits_size, 8);
    
    // Инициализация констант MD5
    unsigned int h0 = 0x67452301, h1 = 0xefcdab89, h2 = 0x98badcfe, h3 = 0x10325476;
    
    // Основной цикл алгоритма по 64-байтным блокам
    for (long offset = 0; offset < padded_size; offset += 64) {
        unsigned int* x = (unsigned int*)(buf + offset);
        unsigned int a = h0, b = h1, c = h2, d = h3;
        
        /* Раунд 1 */
        MD5_STEP(MD5_F, a, b, c, d, x[0],  7, 0xd76aa478); MD5_STEP(MD5_F, d, a, b, c, x[1], 12, 0xe8c7b756);
        MD5_STEP(MD5_F, c, d, a, b, x[2], 17, 0x242070db); MD5_STEP(MD5_F, b, c, d, a, x[3], 22, 0xc1bdceee);
        MD5_STEP(MD5_F, a, b, c, d, x[4],  7, 0xf57c0faf); MD5_STEP(MD5_F, d, a, b, c, x[5], 12, 0x4787c62a);
        MD5_STEP(MD5_F, c, d, a, b, x[6], 17, 0xa8304613); MD5_STEP(MD5_F, b, c, d, a, x[7], 22, 0xfd469501);
        MD5_STEP(MD5_F, a, b, c, d, x[8],  7, 0x698098d8); MD5_STEP(MD5_F, d, a, b, c, x[9], 12, 0x8b44f7af);
        MD5_STEP(MD5_F, c, d, a, b, x[10],17, 0xffff5bb1); MD5_STEP(MD5_F, b, c, d, a, x[11],22, 0x895cd7be);
        MD5_STEP(MD5_F, a, b, c, d, x[12], 7, 0x6b901122); MD5_STEP(MD5_F, d, a, b, c, x[13],12, 0xfd987193);
        MD5_STEP(MD5_F, c, d, a, b, x[14],17, 0xa679438e); MD5_STEP(MD5_F, b, c, d, a, x[15],22, 0x49b40821);
        /* Раунд 2 */
        MD5_STEP(MD5_G, a, b, c, d, x[1],  5, 0xf61e2562); MD5_STEP(MD5_G, d, a, b, c, x[6],  9, 0xc040b340);
        MD5_STEP(MD5_G, c, d, a, b, x[11],14, 0x265e5a51); MD5_STEP(MD5_G, b, c, d, a, x[0], 20, 0xe9b6c7aa);
        MD5_STEP(MD5_G, a, b, c, d, x[5],  5, 0xd62f105d); MD5_STEP(MD5_G, d, a, b, c, x[10], 9, 0x02441453);
        MD5_STEP(MD5_G, c, d, a, b, x[15],14, 0xd8a1e681); MD5_STEP(MD5_G, b, c, d, a, x[4], 20, 0xe7d3fbc8);
        MD5_STEP(MD5_G, a, b, c, d, x[9],  5, 0x21e1cde6); MD5_STEP(MD5_G, d, a, b, c, x[14], 9, 0xc33707d6);
        MD5_STEP(MD5_G, c, d, a, b, x[3], 14, 0xf4d50d87); MD5_STEP(MD5_G, b, c, d, a, x[8], 20, 0x455a14ed);
        MD5_STEP(MD5_G, a, b, c, d, x[13], 5, 0xa9e3e905); MD5_STEP(MD5_G, d, a, b, c, x[2],  9, 0xfcefa3f8);
        MD5_STEP(MD5_G, c, d, a, b, x[7], 14, 0x676f02d9); MD5_STEP(MD5_G, b, c, d, a, x[12],20, 0x8d2a4c8a);
        /* Раунд 3 */
        MD5_STEP(MD5_H, a, b, c, d, x[5],  4, 0xfffa3942); MD5_STEP(MD5_H, d, a, b, c, x[8], 11, 0x8771f681);
        MD5_STEP(MD5_H, c, d, a, b, x[11],16, 0x6d9d6122); MD5_STEP(MD5_H, b, c, d, a, x[14],23, 0xfde5380c);
        MD5_STEP(MD5_H, a, b, c, d, x[1],  4, 0xa4beea44); MD5_STEP(MD5_H, d, a, b, c, x[4], 11, 0x4bdecfa9);
        MD5_STEP(MD5_H, c, d, a, b, x[7], 16, 0xf6bb4b60); MD5_STEP(MD5_H, b, c, d, a, x[10],23, 0xbebfbc70);
        MD5_STEP(MD5_H, a, b, c, d, x[13], 4, 0x289b7ec6); MD5_STEP(MD5_H, d, a, b, c, x[0], 11, 0xeaa127fa);
        MD5_STEP(MD5_H, c, d, a, b, x[3], 16, 0xd4ef3085); MD5_STEP(MD5_H, b, c, d, a, x[6], 23, 0x04881d05);
        MD5_STEP(MD5_H, a, b, c, d, x[9],  4, 0xd9d4d039); MD5_STEP(MD5_H, d, a, b, c, x[12],11, 0xe6db99e5);
        MD5_STEP(MD5_H, c, d, a, b, x[15],16, 0x1fa27cf8); MD5_STEP(MD5_H, b, c, d, a, x[2], 23, 0xc4ac5665);
        /* Раунд 4 */
        MD5_STEP(MD5_I, a, b, c, d, x[0],  6, 0xf4292244); MD5_STEP(MD5_I, d, a, b, c, x[7], 10, 0x432aff97);
        MD5_STEP(MD5_I, c, d, a, b, x[14],15, 0xab9423a7); MD5_STEP(MD5_I, b, c, d, a, x[5], 21, 0xfc93a039);
        MD5_STEP(MD5_I, a, b, c, d, x[12], 6, 0x655b59c3); MD5_STEP(MD5_I, d, a, b, c, x[3], 10, 0x8f0ccc92);
        MD5_STEP(MD5_I, c, d, a, b, x[10],15, 0xffeff47d); MD5_STEP(MD5_I, b, c, d, a, x[1], 21, 0x85845dd1);
        MD5_STEP(MD5_I, a, b, c, d, x[8],  6, 0x6fa87e4f); MD5_STEP(MD5_I, d, a, b, c, x[15],10, 0xfe2ce6e0);
        MD5_STEP(MD5_I, c, d, a, b, x[6], 15, 0xa3014314); MD5_STEP(MD5_I, b, c, d, a, x[13],21, 0x4e0811a1);
        MD5_STEP(MD5_I, a, b, c, d, x[4],  6, 0xf7537e82); MD5_STEP(MD5_I, d, a, b, c, x[11],10, 0xbd3af235);
        MD5_STEP(MD5_I, c, d, a, b, x[2], 15, 0x2ad7d2bb); MD5_STEP(MD5_I, b, c, d, a, x[9], 21, 0xeb86d391);
        
        h0 += a; h1 += b; h2 += c; h3 += d;
    }
    
    // Переводим результат в шестнадцатеричную строку
    unsigned int stages[4] = {h0, h1, h2, h3};
    unsigned char digest[16];
    for (int i = 0; i < 4; i++) {
        digest[i*4]   = stages[i] & 0xFF;        digest[i*4+1] = (stages[i] >> 8) & 0xFF;
        digest[i*4+2] = (stages[i] >> 16) & 0xFF; digest[i*4+3] = (stages[i] >> 24) & 0xFF;
    }
    
    for (int i = 0; i < 16; i++) sprintf(&out_md5[i*2], "%02x", digest[i]);
    out_md5[32] = '\0';
    free(buf);
}
