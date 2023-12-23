#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

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

bool isCorrect(int *arr)
{
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

void quickSortParallel(int *arr, int left, int right, int subgroup, int idParentProc, int myrank) {
    int curLength;
    MPI_Status status;
    MPI_Request re;
    int *buf_arr;
    
    // Only one processor to choose
    if (subgroup == 1) {
        assert(myrank == idParentProc);
        quickSort(arr, left, right);
        return;
    }

    int rightSubgroup = subgroup / 2;
    int leftSubgroup = subgroup - rightSubgroup;
    int border, destNode = idParentProc + leftSubgroup;
    if (myrank == idParentProc) {
        border = partition(arr, left, right);

        curLength = right - border;

        MPI_Send(&curLength, 1, MPI_INT, destNode, myrank, MPI_COMM_WORLD);
        if (curLength != 0)
            MPI_Send(arr + border + 1, curLength, MPI_INT, destNode, myrank, MPI_COMM_WORLD);
    }

    if (myrank == destNode) {
        MPI_Recv(&curLength, 1, MPI_INT, idParentProc, idParentProc, MPI_COMM_WORLD, &status);
        if (curLength != 0) {
            buf_arr = (int *)malloc(curLength * sizeof(int));
            if (buf_arr == 0)
                printf("Malloc memory error");
            else
                MPI_Recv(buf_arr, curLength, MPI_INT, idParentProc, idParentProc, MPI_COMM_WORLD, &status);
        }
    }
    
    if (myrank < destNode) {
        // left subgroup
        quickSortParallel(arr, left, border - 1, leftSubgroup, idParentProc, myrank);
    } else {
        // right subgroup
        quickSortParallel(buf_arr, 0, curLength - 1, rightSubgroup, destNode, myrank);
    }

    if ((myrank == destNode) && (curLength != 0))
        MPI_Send(buf_arr, curLength, MPI_INT, idParentProc, destNode, MPI_COMM_WORLD);
    if ((myrank == idParentProc) && (curLength != 0))
        MPI_Recv(arr + border + 1, curLength, MPI_INT, destNode, destNode, MPI_COMM_WORLD, &status);
}


int main(int argc, char *argv[]) {
    int rank, nprocs, len;
	char name[MPI_MAX_PROCESSOR_NAME];
    int* arr;
    int arr_size;

    if (argc > 1) {
        ARR_SIZE = atoi(argv[1]);
    }

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Get_processor_name(name, &len);

    if (rank == 0) {
        srand(396);
        arr_size = ARR_SIZE;
        arr = (int *)malloc(arr_size * sizeof(int));

        for (int i = 0; i < arr_size; i++) {
            arr[i] = (int)rand() % 10000000;
        }

        printf("[%d] Random array before sort: \n", rank);
        printArr(arr);
    }
    double startTime = MPI_Wtime();

    if (nprocs == 1)
        quickSort(arr, 0, arr_size - 1);
    else {
        MPI_Bcast(&arr_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        quickSortParallel(arr, 0, arr_size - 1, nprocs, 0, rank);
    }
    
    double endTime = MPI_Wtime();

    if (rank == 0) {
        printf("[%d] Sorted array: \n", rank);
        printArr(arr);
        printf("[%d] It took: %f s\n", rank, endTime - startTime);

        if (isCorrect(arr))
			printf("Correct!\n");
		else
			printf("Error..Not sorted correctly\n");
        
        free(arr);
    }

	MPI_Finalize();
}
