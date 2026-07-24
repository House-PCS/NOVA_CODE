#include <stdio.h>
#include <stdlib.h>

void splitBinFile(const char *inputFilePath) 
{
    FILE *inputFile = fopen(inputFilePath, "rb");
    if (inputFile == NULL) 
    {
        perror("Error opening input file");
        return;
    }

    FILE *outputFiles[4];
    
    char outputFilePaths[4][20] = {"CortexM00.bin", "CortexM01.bin", "CortexM02.bin", "CortexM03.bin"};

    // 打开四个输出文件
    for (int i = 0; i < 4; i++) 
    {
        outputFiles[i] = fopen(outputFilePaths[i], "wb");
        if (outputFiles[i] == NULL) 
        {
            perror("Error creating output files");
            fclose(inputFile);

            for (int j = 0; j < i; j++) 
            {
                fclose(outputFiles[j]);
            }
            return;
        }
    }

    unsigned char byte;
    int byteCount = 0;

    // 读取输入文件并写入到相应的输出文件
    while (fread(&byte, sizeof(unsigned char), 1, inputFile) == 1) 
    {
        int fileIndex = byteCount % 4;
        fwrite(&byte, sizeof(unsigned char), 1, outputFiles[fileIndex]);
        byteCount++;
    }

    // 关闭所有文件
    fclose(inputFile);

    for (int i = 0; i < 4; i++) 
    {
        fclose(outputFiles[i]);
    }
}


void BinToCoe(const char *binFilePath, const char *coeFilePath) 
{
    FILE *binFile = fopen(binFilePath, "rb");
    if (binFile == NULL) 
    {
        perror("Error opening bin file");
        return;
    }

    // 获取文件大小
    fseek(binFile, 0, SEEK_END);
    long fileSize = ftell(binFile);
    fseek(binFile, 0, SEEK_SET);

    // 读取文件内容
    unsigned char *buffer = (unsigned char *)malloc(fileSize);
    fread(buffer, fileSize, 1, binFile);
    fclose(binFile);

    // 创建coe文件并写入数据
    FILE *coeFile = fopen(coeFilePath, "w");
    if (coeFile == NULL) 
    {
        perror("Error creating coe file");
        free(buffer);
        return;
    }

    // 写入coe文件头
    fprintf(coeFile, "memory_initialization_radix=16;\n");
    fprintf(coeFile, "memory_initialization_vector=\n");

    // 写入十六进制数据
    for (int i = 0; i < fileSize; i++) 
    {
        fprintf(coeFile, "%02X", buffer[i]);
        if (i < fileSize - 1) 
        {
            fprintf(coeFile, ",\n");
        }
    }
    fprintf(coeFile, ";\n");

    fclose(coeFile);
    free(buffer);
}



int main() 
{
    
    const char *inputBinFilePath = "CortexM0.bin";
    // const char *inputBinFilePath = "E:/WorkSpace/Vivado2019/RockDcCtrl/Software/Out/CortexM0.bin";

    // 拆分bin文件
    splitBinFile(inputBinFilePath);

    // 将拆分后的bin文件转换为coe文件
    char binFilePaths[4][50] = {"CortexM00.bin", "CortexM01.bin", "CortexM02.bin", "CortexM03.bin"};
    char coeFilePaths[4][50] = {"CortexM00.coe", "CortexM01.coe", "CortexM02.coe", "CortexM03.coe"};

    for (int i = 0; i < 4; i++) 
    {
        BinToCoe(binFilePaths[i], coeFilePaths[i]);
        remove(binFilePaths[i]);
    }

    printf("Convert done!\n");


    return 0;
}