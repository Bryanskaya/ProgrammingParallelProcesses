#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int ARR_SIZE = 100000;


void printArr(int *arr) {
    for (int i = 0; i < ARR_SIZE; i++) {
        if (i > 100) {
            printf("...");
            break;
        }
        
        printf("%d ", arr[i]);
    }

    printf("\n");
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


int main(int argc, char *argv[]) {
    int myrank, nprocs, len;
	char name[MPI_MAX_PROCESSOR_NAME];
    int* data_arr;

    if (argc > 1) {
        ARR_SIZE = atoi(argv[1]);
    }

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Get_processor_name(name, &len);

	// printf("Hello from host %s[%d] %d of %d\n", name, len, myrank, nprocs);

    if (myrank == 0) {
        srand(396);
        data_arr = (int *)malloc(ARR_SIZE * sizeof(int));

        for (int i = 0; i < ARR_SIZE; i++) {
            data_arr[i] = (int)rand() % 10000000;
        }

        printf("[%d] Random array before sort: \n", myrank);
        printArr(data_arr);

        double startTime = MPI_Wtime();
        quickSort(data_arr, 0, ARR_SIZE - 1);
        double endTime = MPI_Wtime();

        // printf("[%d] Sorted array: \n", myrank);
        printArr(data_arr);
        printf("[%d] It took: %f s\n", myrank, endTime - startTime);
    }

	MPI_Finalize();

	return 0;
}