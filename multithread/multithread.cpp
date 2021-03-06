#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <process.h>
#include <omp.h>

int MAT_SIZE = 1024;
int THREAD_NUM = 4;

float **mat_a;
float **mat_b;
float **mat_c;
float **mat_d;
//HANDLE **hMutex;

struct MAT_TEMP {
	int n, stride, k;
};

static void matmat(int n, float **mat_c, float **mat_a, float **mat_b)
{
	for (int i = 0; i < n; i++) {
		for (int k = 0; k < n; k++) {
			for (int j = 0; j < n; j++) {
				mat_c[i][j] += mat_a[i][k] * mat_b[k][j];
			}
		}
	}
}

static void randMat(float **mat)
{
	for (int i = 0; i < MAT_SIZE; i++) {
		for (int j = 0; j < MAT_SIZE; j++) {
			mat[i][j] = rand() % 10;
		}
	}
}


float **array(int row, int col)
{
	float **arr;
	arr = (float **)malloc(row * sizeof(float *));
	for (int i = 0; i < row; i++) {
		arr[i] = (float *)malloc(col * sizeof(float));
	}
	return arr;
}

void initArray(int n, float **mat)
{
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			mat[i][j] = 0;
		}
	}
}

static void transMat(float **mat)
{
	for (int i = 0; i < MAT_SIZE - 1; i++) {
		for (int j = i; j < MAT_SIZE; j++) {
			float temp = mat[i][j];
			mat[i][j] = mat[j][i];
			mat[j][i] = temp;
		}
	}
}

void printMat(int n, float **mat)
{
	return;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			printf("%.0f,", mat[i][j]);
		}
		printf("\n");
	}
}

int equalMat(float **mat_a, float **mat_b)
{
	for (int i = 0; i < MAT_SIZE; i++) {
		for (int j = 0; j < MAT_SIZE; j++) {
			if (mat_a[i][j] != mat_b[i][j]) {
				return 0;
			}
		}
	}
	return 1;
}

unsigned __stdcall ThreadFun1(void *param)
{
	struct MAT_TEMP *mat_temp = (struct MAT_TEMP*)param;
	int n = mat_temp->n;
	int stride = mat_temp->stride;
	int k = mat_temp->k;
	int sum = 0;
	for (int i = k; i < n; i +=stride) {
		for (int j = 0; j < n; j ++) {
			for (int k = 0; k < n; k++) {
				mat_d[i][j] += mat_a[i][k] * mat_b[j][k];
			}
		}
	}
	_endthreadex(0);
	return 0;
}

static void multhreadMat(int n, float **mat_c, float **mat_a, float **mat_b)
{
	transMat(mat_b);
	printf("Mat_b:\n");
	printMat(MAT_SIZE, mat_b);

	HANDLE* hEven = new HANDLE[THREAD_NUM];
	unsigned threadID;
	for (int k = 0; k < THREAD_NUM; k++) {
		MAT_TEMP *temp;
		temp = (struct MAT_TEMP *)malloc(sizeof(struct MAT_TEMP));
		temp->n = n;
		temp->k = k;
		temp->stride = THREAD_NUM;
		hEven[k] = (HANDLE)_beginthreadex(NULL, 0, &ThreadFun1, temp, NULL, &threadID);
	}

	for(int k = 0;k<THREAD_NUM;k++)
	{
		WaitForSingleObject(hEven[k], INFINITE);
		CloseHandle(hEven[k]);
	}
}

int main()
{
	int t1, t2;
	mat_a = array(MAT_SIZE, MAT_SIZE);
	mat_b = array(MAT_SIZE, MAT_SIZE);
	mat_c = array(MAT_SIZE, MAT_SIZE);
	mat_d = array(MAT_SIZE, MAT_SIZE);

	randMat(mat_a);
	randMat(mat_b);
	initArray(MAT_SIZE, mat_c);
	initArray(MAT_SIZE, mat_d);

	printf("Mat_a:\n");
	printMat(MAT_SIZE, mat_a);

	LARGE_INTEGER num;
	LONGLONG start, end, freq;
	QueryPerformanceFrequency(&num);
	freq = num.QuadPart;
	QueryPerformanceCounter(&num);
	start = num.QuadPart;
	matmat(MAT_SIZE, mat_c, mat_a, mat_b);
	QueryPerformanceCounter(&num);
	end = num.QuadPart;
	t1 = (end - start) * 1000 / freq;
	printf("Matmat running time: %d\n", t1);
	printMat(MAT_SIZE, mat_c);

	QueryPerformanceCounter(&num);
	start = num.QuadPart;
	multhreadMat(MAT_SIZE, mat_d, mat_a, mat_b);
	QueryPerformanceCounter(&num);
	end = num.QuadPart;
	t2 = (end - start) * 1000 / freq;
	printf("Multi_mat running time: %d\n", t2);
	printMat(MAT_SIZE, mat_d);

	if (t1 != 0 && t2 != 0) {
		printf("T1/T2 = %.2f\n", (float)t1 / (float)t2);
	}

	if (equalMat(mat_c, mat_d) == 1) {
		printf("EQUAL!\n");
	}
	else {
		printf("NOT EQUAL!\n");
	}

	system("pause");
	return 0;
}


