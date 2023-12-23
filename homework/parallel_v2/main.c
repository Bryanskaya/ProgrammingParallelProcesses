#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#define RECEIVE_LABEL 111

int ARR_SIZE  = 10;
int nprocs, rank;


void printArr(int *arr, int n) {
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

int* run_nproc(int *arr, int arr_size, int subarr_size, int rank) {
	MPI_Status status;
	int cur_size;
	int* result = (int *)malloc(arr_size * sizeof(int));

	for (int i = 0; i < nprocs - 1; i++)
	{
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
	
	quickSort(subarr, 0, subarr_size - 1);

	// Receive data
	int result_size = subarr_size;
	for (int i = 0; i < nprocs; i++)
	{
		if (i > 0)
		{
			int cur_subarray_size;
			MPI_Recv(&cur_subarray_size, 1, MPI_INT, i, RECEIVE_LABEL, MPI_COMM_WORLD, &status);

			int* temp_subarray = (int *)malloc(cur_subarray_size * sizeof(int));
			MPI_Recv(temp_subarray, cur_subarray_size, MPI_INT, i, RECEIVE_LABEL, MPI_COMM_WORLD, &status);

			int* temp_result = (int *)malloc(result_size * sizeof(int));
			for (int j = 0; j < result_size; j++)
				temp_result[j] = result[j];

			merge(temp_subarray, temp_result, result, cur_subarray_size, result_size);

			result_size += cur_subarray_size;
		}
		else
		{
			for (int j = 0; j < subarr_size; j++)
				result[j] = subarr[j];

			free(subarr);
		}
	}

	return result;
}

int* run_proc(int *arr, int arr_size) {
	int* result = (int *)malloc(arr_size * sizeof(int));

	quickSort(arr, 0, arr_size - 1);
	for (int i = 0; i < arr_size; i++)
		result[i] = arr[i];
	
	return result;
}

int main(int argc, char *argv[])
{
	int subarr_size, len;
	char name[MPI_MAX_PROCESSOR_NAME];
	MPI_Status status;

	if (argc > 1) {
        ARR_SIZE = atoi(argv[1]);
    }

	int arr_size = ARR_SIZE;
	int *arr = (int *)malloc(arr_size * sizeof(int));
	int *result;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Get_processor_name(name, &len);

	if (rank == 0) {
		srand(396);
		for (int i = 0; i < arr_size; i++)
			arr[i] = (int)rand() % 10000000;
		
		printf("[%d] Random array before sort: \n", rank);
        printArr(arr, arr_size);
	}

	subarr_size = (int)arr_size / nprocs;

	double start_timer = MPI_Wtime();
	
	if (rank == 0) {
		if (nprocs > 1)
			result = run_nproc(arr, arr_size, subarr_size, rank);
		else
			// if it runs only in a single node
			result = run_proc(arr, arr_size);
	}
	else {
		int cur_subarr_size;
		MPI_Recv(&cur_subarr_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

		int *sub_arr = (int *)malloc(cur_subarr_size * sizeof(int));

		MPI_Recv(sub_arr, cur_subarr_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

		quickSort(sub_arr, 0, cur_subarr_size - 1);

		MPI_Send(&cur_subarr_size, 1, MPI_INT, 0, RECEIVE_LABEL, MPI_COMM_WORLD);
		MPI_Send(sub_arr, cur_subarr_size, MPI_INT, 0, RECEIVE_LABEL, MPI_COMM_WORLD);

		free(sub_arr);
	}

	double finish_timer = MPI_Wtime();

	if (rank == 0) {
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