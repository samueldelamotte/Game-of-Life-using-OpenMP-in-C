#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>

/*
	MOD FUNCTION FOR WRAPPING
*/
int mod(int i, int max)
{
	while (i < 0) {
		i += max;
	}
	return i % max;
}

int main(int argc, char *argv[])
{
	struct timeval start, end; 	// used for timing,
	gettimeofday(&start, NULL);  	// the program
//====================================================================================================
/*
	DECLARING VARIABLES AND ALLOCATING MEMORY
*/
	const int WIDTH = strtol(argv[1], NULL, 10);	// board width
	const int HEIGHT = WIDTH;	// board height = width
	const int STEPS = 100;	// amount of cycles that the board will update
	int **board;	// main board
	int **storage;	// storage board
	int a;	// chance of cell being alive initially
	int n;	// neighbour count
	//FILE *fp;	(used to print array to file, not used during timing purposes)
	board = malloc(sizeof(int*)*WIDTH);	// dynamically allocate memory,
	storage = malloc(sizeof(int*)*WIDTH);	// to both the main and storage board,
	omp_set_num_threads(4);	// set to 1 for sequential, 4 for parallelism

	#pragma omp parallel for schedule(static)
	for (int i = 0; i < WIDTH; i++) {	// in two dimensions
		board[i] = malloc(sizeof(int)*HEIGHT);
		storage[i] = malloc(sizeof(int)*HEIGHT);
	}
/*
	INITIALISE AND POPULATE MAIN AND STORAGE BOARD
*/
	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			a = rand() %10 +1;
			if (a < 6) { 			// 40%(10-6=4) chance of being a live cell
				board[x][y] = 0;	// dead
			} else {
				board[x][y] = 1; 	// alive
			}
			storage[x][y] = 0;
		}
	}
/*
	CHECK EACH CELL ON THE BOARD FOR ADJACENT NEIGHBOURS
*/
	for (int i = 0; i < STEPS; i++) {
		#pragma omp parallel for private(n) schedule(static)
		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				n = 0;
				if (board[x][mod(y-1,HEIGHT)] == 1) {
					n++;
				}
				if (board[x][mod(y+1,HEIGHT)] == 1) {
					n++;
				}
				if (board[mod(x-1,WIDTH)][y] == 1) {
					n++;
				}
				if (board[mod(x+1, WIDTH)][y] == 1) {
					n++;
				}
/*
	APPLY THE RULES OF LIFE TO EACH CELL
	1) A live cell dies if it has less than two live neighbours due to loneliness
	2) A live cell with four live neighbours dies due to overpopulation
	3) A live cell with two or three live neigbours continues to live
	4) Any dead cell with exactly three live neighbours becomes live due to reproduction
*/
				if (board[x][y] == 1) { 
					if (n < 2 || n == 4) { 		// rule #1 and #2
						storage[x][y] = 0;
					} else { 					// rule #3
						storage[x][y] = 1;
					}
				} else if (board[x][y] == 0) {
					if (n == 3) {				// rule #4
						storage[x][y] = 1;
					} else {
						storage[x][y] = 0;		// no change
					}
				}
			}
		}
/*
	WRITE BOARD TO .TXT FILE FOR REPORT
	(used to print array to file, not used during timing purposes)
		if (i == 0 || i == 4 || i == 99) {
			if (i == 0) {
				fp = fopen("0.txt", "w");
			} else if (i == 4) {
				fp = fopen("4.txt", "w");
			} else {
				fp = fopen("99.txt", "w");
			}
			for (int x = 0; x < WIDTH; x++) { 
				for (int y = 0; y < HEIGHT; y++) {
					fprintf(fp, "%d ", board[x][y]);
				}
				fprintf(fp, "%s", "\n");
			}
			fclose(fp);
		}
*/
/*
	UPDATE MAIN BOARD AND RESET STORAGE BOARD
*/
		#pragma omp parallel for schedule(static)
		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				board[x][y] = storage[x][y];
				storage[x][y] = 0;
			}
		}
	}
	free(board);	// free dynamically allocated memory
	free(storage);	// of both boards
//====================================================================================================
	gettimeofday(&end, NULL);
	float delta = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
	printf("time=%12.10f\n",delta);
}
