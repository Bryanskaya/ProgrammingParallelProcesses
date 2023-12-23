#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define RECEIVE_LABEL 111

int nprocs, rank;
int ARR_SIZE = 1024;

void printArr(int *arr, int n)
{
	for (int i = 0; i < n; i++) {
		if (i > 100) {
			printf("...");
			break;
		}

		printf("%d ", arr[i]);
	}

	printf("\n");
}

bool isCorrect(int *arr) {
	for (int i = 0; i < ARR_SIZE - 1; i++)
		if (arr[i] > arr[i + 1])
			return false;
	return true;
}

int partition(int *arr, int left, int right) {
    int x = arr[left];
    int temp, t = left;

    for (int i = left + 1; i <= right; i++) {
        if (arr[i] < x) {
            t++;
            temp = arr[t];
            arr[t] = arr[i];
            arr[i] = temp;
        }
    }

    temp = arr[left];
    arr[left] = arr[t];
    arr[t] = temp;

    return t;
}

void quickSort(int *arr, int left, int right) {
    int temp;
    if (left < right) {
        temp = partition(arr, left, right);
        quickSort(arr, left, temp);
        quickSort(arr, temp + 1, right);
    }
}

void quickSortOpenMP_4(int *arr, int left, int right) {
    int temp, tempLeft, tempRight;
    if (left >= right)
        return;

    temp = partition(arr, left, right);
    #pragma omp parallel sections
    {
        #pragma omp section 
        {
            tempLeft = partition(arr, left, temp);
        }
        #pragma omp section
        {
            tempRight = partition(arr, temp + 1, right);
        }
    }

    #pragma omp parallel sections
    {
        #pragma omp section 
        {
            quickSort(arr, left, tempLeft);
        }
        #pragma omp section
        {
            quickSort(arr, tempLeft + 1, temp);
        }
        #pragma omp section 
        {
            quickSort(arr, temp + 1, tempRight);
        }
        #pragma omp section
        {
            quickSort(arr, tempRight + 1, right);
        }
    }
}


void merge(int *fstArr, int *sndArr, int *result, int fstSize, int sndSize)
{
	int i = 0, j = 0, k = 0;

	while (i < fstSize && j < sndSize) {

		if (fstArr[i] < sndArr[j])
			result[k++] = fstArr[i++];
		else
			result[k++] = sndArr[j++];

		if (i == fstSize)
			while (j < sndSize)
				result[k++] = sndArr[j++];
		else if (j == sndSize)
			while (i < fstSize)
				result[k++] = fstArr[i++];
	}
}

int* run_nproc(int *arr, int arr_size)
{
	int subarr_size = (int)arr_size / nprocs;
	int *result = (int *)malloc(arr_size * sizeof(int));
	MPI_Status status;

	if (rank == 0) {
		int cur_size;
		for (int i = 0; i < nprocs - 1; i++) {
			if (i == nprocs - 1 - 1)
				cur_size = arr_size - (i + 1) * subarr_size;
			else
				cur_size = subarr_size;

			MPI_Send(&cur_size, 1, MPI_INT, i + 1, 0, MPI_COMM_WORLD);
			MPI_Send(&arr[(i + 1) * subarr_size], cur_size, MPI_INT, i + 1, 0, MPI_COMM_WORLD);
		}

		int *subarr = (int *)malloc(subarr_size * sizeof(int));
		for (int i = 0; i < subarr_size; i++)
			subarr[i] = arr[i];

		quickSortOpenMP_4(subarr, 0, subarr_size - 1);

		for (int j = 0; j < subarr_size; j++)
			result[j] = subarr[j];

		free(subarr);
	} else {
		MPI_Recv(&subarr_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(result, subarr_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

		quickSortOpenMP_4(result, 0, subarr_size - 1);
	}

	int rank_tmp = rank, step = 1, sender;
	int result_size = subarr_size;
	while (rank_tmp % 2 == 0 && step < nprocs) {
		sender = rank + step;

		if (sender < nprocs) {
			int to_merge_size;
			MPI_Recv(&to_merge_size, 1, MPI_INT, sender, RECEIVE_LABEL, MPI_COMM_WORLD, &status);

			int* to_merge_subarr = (int *)malloc(to_merge_size * sizeof(int));
			MPI_Recv(to_merge_subarr, to_merge_size, MPI_INT, sender, RECEIVE_LABEL, MPI_COMM_WORLD, &status);

			int* temp_result = (int *)malloc(result_size * sizeof(int));
			for (int j = 0; j < result_size; j++)
				temp_result[j] = result[j];

			merge(to_merge_subarr, temp_result, result, to_merge_size, result_size);

			result_size += to_merge_size;
		}

		rank_tmp /= 2;
		step *= 2;
	}

	if (rank != 0) {
		MPI_Send(&result_size, 1, MPI_INT, rank - step, RECEIVE_LABEL, MPI_COMM_WORLD);
		MPI_Send(result, result_size, MPI_INT, rank - step, RECEIVE_LABEL, MPI_COMM_WORLD);
	}

	return result;
}

int *run_proc(int *arr, int arr_size) {
	int *result = (int *)malloc(arr_size * sizeof(int));

	quickSortOpenMP_4(arr, 0, arr_size - 1);
	for (int i = 0; i < arr_size; i++)
		result[i] = arr[i];

	return result;
}

int main(int argc, char *argv[]) {
	if (argc > 1) {
        ARR_SIZE = atoi(argv[1]);
    }

	int arr_size = ARR_SIZE;
	int n = arr_size * sizeof(int), subarr_size;
	int *arr = (int *)malloc(n), *result = (int *)malloc(n);

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

	if (rank == 0)
	{
		for (int i = 0; i < arr_size; i++)
			arr[i] = (int)rand() % 10000000;

		printf("[%d] Random array before sort: \n", rank);
		printArr(arr, arr_size);
	}

	double start_timer = MPI_Wtime();
	if (nprocs > 1)
		result = run_nproc(arr, arr_size);
	else
		result = run_proc(arr, arr_size);
	double finish_timer = MPI_Wtime();

	if (rank == 0)
	{
		printf("[%d] Sorted array: \n", rank);
        printArr(arr, arr_size);
        printf("[%d] It took: %f s\n", rank, finish_timer - start_timer);

		if (isCorrect(result))
			printf("Correct!\n");
		else
			printf("Error..Not sorted correctly\n");
		
		free(arr);
	}

	MPI_Finalize();
}