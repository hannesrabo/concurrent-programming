#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#define MAX_LINES 26000
#define MAX_PALINDROMES 600
#define WORD_LENGTH 25
#define MAX_THREADS 10
#define WORD_N_EQUAL 123456789
#define MEAN_NR_OF_TIMES 20

char lines[MAX_LINES][WORD_LENGTH];
char line_length[MAX_LINES];
int palindromes[MAX_PALINDROMES];

int nr_of_lines;
int nr_of_palindromes;
int nr_threads;

double time_result[MEAN_NR_OF_TIMES];

/**
 * Returns 0 for equal, < 0 for w1 < w2 and > 0 for w1 > w2
 * 
 */
int compare(char *w1, char *w2) {
    int a, b;

    while (*w1 != '\0') {
        // w2 is shorter.       
        if (*w2 == '\0')
            return 1;

        a = *w1;
        b = *w2;

        if (a >= 'A' && a <= 'Z')
            a -= ('A' - 'a');

        if (b >= 'A' && b <= 'Z')
            b -= ('A' - 'a');

        if (a == '\'' && b != '\'') {
            int temp = compare(w1 + 1, w2);
            if (temp == 0)
                return 1;
            else 
                return temp;
        }

        if (b == '\'' && a != '\'') {
            int temp = compare(w1, w2 + 1);
            if (temp == 0)
                return 1;
            else 
                return temp;
        }

        // Strings not equal
        if (a != b)
            return (a - b);

        // Increment and continue.
        w1++;
        w2++;
    }

    // Equal        
    if (*w2 == '\0')
        return 0;
    
    // First is shorter
    return -1;
}

int binary_search(char *word) {
    int left, right, middle;
    int temp;
    left = 0;
    right = nr_of_lines - 1;

    while (left < right) {
        middle = left + (right - left) / 2;

        temp = compare(word, lines[middle]);
        if (temp < 0) {
            if (right == middle)
                break;
            right = middle;
        } else if (temp > 0) {
            if (left == middle)
                break;
            left = middle;
        } else {
            // We found it!
            return middle;
        }
    }

    return -1;
}

void clean_line(int line_nr) {
    int line_end = 0;
    while(lines[line_nr][line_end] != '\n')
        line_end++;

    // Make sure the string ends here.. (remove '\n')
    lines[line_nr][line_end] = '\0';
    line_length[line_nr] = line_end;
}

void find_palindromes() {
    // Recording nr of lines in the file.
    int chunk_size = nr_of_lines / omp_get_max_threads();
    int chunk_start, chunk_end;
    int line_index;
    int search_index, temp_p_index, line_start, line_end;
    char line_buffer[WORD_LENGTH];
    volatile int palindrome_index = 0;  

    #pragma omp parallel private(line_index, chunk_start, chunk_end, search_index, temp_p_index, line_buffer, line_start, line_end)
    {
        chunk_start = omp_get_thread_num() * chunk_size;
        chunk_end = chunk_start + chunk_size;

        for (line_index = chunk_start; line_index < chunk_end; line_index++){
            line_start = 0;
            line_end = line_length[line_index] - 1;

            // Reverse
            while (line_start < line_end) {
                line_buffer[line_start] = lines[line_index][line_end];
                line_buffer[line_end] = lines[line_index][line_start];
                line_start++;
                line_end--;
            }

            line_buffer[line_length[line_index]] = '\0';

            // Middle letter
            if (line_start == line_end)
                line_buffer[line_start] = lines[line_index][line_start];

            // If this is a palindrome  
            if ((search_index = binary_search(line_buffer)) != -1){
                // We should probably add local result storage... 
                temp_p_index = __sync_fetch_and_add(&palindrome_index, 1);
                palindromes[temp_p_index] = search_index;
            }
        }
    }

    nr_of_palindromes = palindrome_index;
}

int cmpfunc (const void * a, const void * b) {
   return ( *(double*)a - *(double*)b );
}

void benchmark_function(void* function, int start, int end, int step) {
    int val, nr_workers;
    double start_time, mean;
    char filename[120];
    FILE *fp;

    for (val = start; val <= end; val += step) {
        nr_of_lines = val;

		sprintf(filename, "result-s-%d", val);

		fp = fopen(filename, "w");

		for (nr_workers = 1; nr_workers <= 10; nr_workers++) {
			omp_set_num_threads(nr_workers);

			for (int i = 0; i < MEAN_NR_OF_TIMES; i++)
			{   
                start_time = omp_get_wtime();
                find_palindromes();
				time_result[i] = omp_get_wtime() - start_time;
			}

			// Calculate mean value
			qsort((void *)time_result, MEAN_NR_OF_TIMES, sizeof(double), cmpfunc);

			if ((MEAN_NR_OF_TIMES % 2) == 0)
			{  // Even number of elements
				mean = (time_result[MEAN_NR_OF_TIMES / 2 - 1] + time_result[MEAN_NR_OF_TIMES / 2]) / 2;
			}
			else
			{  // Odd number of elements
				mean = time_result[MEAN_NR_OF_TIMES / 2];
			}

			fprintf(fp, "%d %g\n", omp_get_max_threads(), mean);

			printf(".");
			fflush(stdout);

		}

		fclose(fp);

		printf("\rWrote file for matrix size: %s\n", filename);
	}
}

int main(int argc, char* argv[]) {
    
    FILE *fp;
    double start_time_parallel, end_time_parallel;
    double start_time, end_time;

    start_time = omp_get_wtime();

    // Open and read file...
    if (!(fp = fopen("words", "r"))) {
        fprintf(stderr, "Failed to open file");
    }

    // Read
    int lineNr = 0;
    while (fgets(lines[lineNr], WORD_LENGTH, fp)) {
        clean_line(lineNr);
        lineNr++;
    }  
    nr_of_lines = lineNr;    

    // benchmark_function(&find_palindromes, 5000, nr_of_lines, 5000);
    benchmark_function(&find_palindromes, nr_of_lines, nr_of_lines, 5000);

    FILE *fout;
    if (!(fout = fopen("result.txt", "w"))) {
        fprintf(stderr, "Could not open file for writing\n");
    }

    for (int i = 0; i < nr_of_palindromes; i++)
        fprintf(fout, "%s\n", lines[palindromes[i]]);


    end_time = omp_get_wtime();

    fclose(fout);

    printf("Execution time: %g\n", end_time - start_time);
    return 0;
}