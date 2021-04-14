#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

/* use to pass the arguments to the thread function.*/
typedef struct 
{
    int **resultMat;
    int **Mat_One; 
    int **Mat_Two; 
    int beginHeight;
    int beginWidth;
    int workload; 
    int resultSize[2]; 
    int calSize;
}package;


int **readFiles(const char *path, int matrix_Size[2], int** matrix);
void makeArgs(int **resultMat, int **Mat_One, int **Mat_Two, int index[2], int workload, int resultSize[2], int calSize, package *args);
void updateBeginEle(int workerNum, int workload, int rowsize, int eleArr[2]);
void* calMatrix(void* args);
int calElement(int **Mat_One, int **Mat_Two, int h, int w, int size);
void resultToFile(const char *path, int **result, int size[2]);
void validation(int sizeOne[2], int sizeTwo[2], int threadNum);
int stoi(char const str[]);
void printError(char *str);

int main(int argc, char const *argv[])
{
    if (argc != 8)
    {
        printf("The parameter number %d is incorrect.\n", argc);
        printError("");
    }

    int matrix_One_Size[2] = {stoi(argv[1]), stoi(argv[2])};
    int matrix_Two_Size[2] = {stoi(argv[2]), stoi(argv[3])};
    int THREAD_NUM = stoi(argv[7]);
    validation(matrix_One_Size, matrix_Two_Size, THREAD_NUM);

    int *matrix_One[matrix_One_Size[0]]; 
    readFiles(argv[4], matrix_One_Size, matrix_One);
    int *matrix_Two[matrix_Two_Size[0]]; 
    readFiles(argv[5], matrix_Two_Size, matrix_Two);

    int result_Size[2] = {matrix_One_Size[0], matrix_Two_Size[1]};
    int *resultMat[result_Size[0]]; 
    for (size_t i = 0; i < result_Size[0]; i++)
    {
        resultMat[i] = (int *)malloc(result_Size[1] * sizeof(int));
        for (size_t j = 0; j < result_Size[1]; j++)
        {
            resultMat[i][j] = 0;
        }
    }
    
    pthread_t workers[THREAD_NUM]; 
    int workload = result_Size[0] * result_Size[1] / THREAD_NUM;
    int beginEle[2] = {0, 0};

    for (size_t i = 0; i < THREAD_NUM; i++)
    {   if(i == THREAD_NUM-1)
        {
            workload = result_Size[0] * result_Size[1] - workload*i;
        }
        package *args = malloc(sizeof *args);
        makeArgs(resultMat, matrix_One, matrix_Two, beginEle, workload, result_Size, matrix_One_Size[1], args);
        pthread_create(&workers[i], NULL, calMatrix, (void*) args);
        updateBeginEle(i+1, workload, result_Size[1], beginEle);

    }

    for (size_t i = 0; i < THREAD_NUM; i++)
    {
        pthread_join(workers[i], NULL);
    }
    
    resultToFile(argv[6], resultMat, result_Size);

    return 0;
}

void makeArgs(int **resultMat, int **Mat_One, int **Mat_Two, int index[2], int workload, int resultSize[2], int calSize, package *args)
{
    args->beginHeight = index[0];
    args->beginWidth = index[1];
    args->calSize = calSize;
    args->Mat_One = Mat_One;
    args->Mat_Two = Mat_Two;
    args->resultMat = resultMat;
    args->resultSize[0] = resultSize[0];
    args->resultSize[1] = resultSize[1];
    args->workload = workload;
}

void updateBeginEle(int workerNum, int workload, int rowsize, int eleArr[2])
{
    eleArr[0] = workerNum * workload / rowsize;
    eleArr[1] = workerNum * workload % rowsize;
   // printf("The h become %d, and the width become %d.\n", eleArr[0], eleArr[1]);
}

void *calMatrix(void* ptr)
{
    package *args = (package *)ptr;
    int height = args->beginHeight;
    int width = args->beginWidth;
    for (int i = 0; i < args -> workload; i++)
    {
        args -> resultMat[height][width] = calElement(args -> Mat_One, args -> Mat_Two, height, width, args -> calSize);
        width++;
        if(width == args->resultSize[1])
        {
            width =0;
            height++;
        }
    }
    return NULL;
}

int calElement(int **Mat_One, int **Mat_Two, int h, int w, int size)
{
    int res = 0;
    for (int i = 0; i < size; i++)
    {
        res += Mat_One[h][i] * Mat_Two[i][w];
    }
    return res;
}


void resultToFile(const char *path, int **result, int size[2])
{
    FILE *file = fopen(path, "w");
    if (file == NULL)
    {
        printf("The file at %s fails to open.\n", path);
        printError("");
    }

    for (int i = 0; i < size[0]; i++)
    {
        for (int j = 0; j < size[1]; j++)
        {
            fprintf(file, "%d ", result[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);

}

void validation(int sizeOne[2], int sizeTwo[2], int threadNum)
{
    if (sizeOne[0] <= 0 || sizeOne[1] <= 0 || sizeTwo[1] <= 0)
    {
        printError("Invalid matrix size.\n");
    }

    //too many worker will make them not do work as 1/2 = 1, and then last worker will do all the work.
    if (threadNum <= 0 || threadNum > sizeOne[0]*sizeTwo[1])
    {
        printError("Invalid thread worker number.\n");
    }
}

int **readFiles(const char *path, int matrix_Size[2] , int** matrix)
{
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        printf("The file at %s fails to open.\n", path);
        printError("");
    }
    int height = matrix_Size[0];
    int width = matrix_Size[1];

    for (size_t h = 0; h < height; h++)
    {
        matrix[h] = (int *)malloc(width * sizeof(int));
        for (size_t w = 0; w < width; w++)
        {
            fscanf(file, "%d", &matrix[h][w]);
            //printf("The number read in is %d.\n", matrix[h][w]);
        }
    }
    fclose(file);

    return matrix;
}

int stoi(char const str[])
{
    if (str == NULL || str[0] == 0)
    {
        printError("The string is empty.\n");
    }
    int negative = (str[0] == '-');
    int i = 0;
    if (str[0] == '+' || str[0] == '-')
    {
        i++;
    }

    int result = 0;
    while (str[i] != 0)
    {
        if (str[i] < '0' || str[i] > '9')
        {
            printError("The string contains non-digit.\n");
        }
        result *= 10;
        result += str[i] - '0';
        i++;
        //printf("%d\n", result);
    }
    return negative ? -result : result;
}

void printError(char *str)
{
    printf("%s", str);
    exit(1);
}