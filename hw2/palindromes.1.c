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

char lines[MAX_LINES][WORD_LENGTH];
char r_lines[MAX_LINES][WORD_LENGTH];
int palindromes[MAX_PALINDROMES];

int nr_of_lines;
int nr_threads;

/**
 * Returns 0 for equal, < 0 for w1 < w2 and > 0 for w1 > w2
 * 
 */ 
int compare(char *w1, char *w2) {
    while (*w1 == *w2)
        if (*w1 == '\0')
            return 0;
        else{
            w1++;
            w2++;        
        }

    if (*w1 == '\0')
        return -1;
    else if (*w2 == '\0')
        return 1;

    return (*w1 - *w2);
}

char* clean_line(char *string) {
    int i_insert = 0, i_read = 0;
    char temp;
    while ((temp = string[i_read]) != '\0'){
        if (temp >= 'A' && temp <= 'Z')
            string[i_insert++] = string[i_read] - ('A' - 'a');
        else if (temp == '\'')
            ;
        else
            string[i_insert++] = string[i_read];    

        i_read++;
    }

    string[i_insert] = '\0';

    return string;
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

int main(int argc, char* argv[]) {
    
    FILE *fp;
    double start_time, end_time;

    // Read command line arguments
    nr_threads = (argc > 1) ? atoi(argv[1]) : MAX_THREADS;

    // Initiate threading.
    omp_set_num_threads(nr_threads);

    // Open and read file...
    if (!(fp = fopen("words", "r"))) {
        fprintf(stderr, "Failed to open file");
    }

    // Read
    int lineNr = 0;
    while (fgets(lines[lineNr], WORD_LENGTH, fp))
        lineNr++;

    // Recording nr of lines in the file.
    nr_of_lines = lineNr;
    int chunk_size = nr_of_lines / omp_get_max_threads();
    int chunk_start, chunk_end;
    int line_index;
    int search_index, temp_p_index;
    volatile int palindrome_index = 0;    

    start_time = omp_get_wtime();
    
    // Create reverse and cleaned list
    #pragma omp parallel private(line_index, chunk_start, chunk_end, search_index, temp_p_index)
    {
        chunk_start = omp_get_thread_num() * chunk_size;
        chunk_end = chunk_start + chunk_size;

        printf("[%d] %d -> %d\n", omp_get_thread_num(), chunk_start, chunk_end);
        
        for (line_index = chunk_start; line_index < chunk_end; line_index++){
            int line_start = 0;
            int line_end = 0;

            while(lines[line_index][line_end] != '\n')
                line_end++;

            // Make sure the string ends here.. (remove '\n')
            lines[line_index][line_end] = '\0';
            line_end--;

            // Reverse
            while (line_start < line_end) {
                r_lines[line_index][line_start] = lines[line_index][line_end];
                r_lines[line_index][line_end] = lines[line_index][line_start];
                line_start++;
                line_end--;
            }

            // Middle letter
            if (line_start == line_end)
                r_lines[line_index][line_start] = lines[line_index][line_start];

            // Clean unwanted symbols (not sorted numerically)
            clean_line(r_lines[line_index]);
            clean_line(lines[line_index]);
        }

        // Wait here for things to finish. Then find palindromes.
        #pragma omp barrier

        // Finding palindromes    
        for (line_index = chunk_start; line_index < chunk_end; line_index++) {
            // This is a palindrome
            if ((search_index = binary_search(r_lines[line_index])) != -1){
                // We should probably add local result storage... 
                temp_p_index = __sync_fetch_and_add(&palindrome_index, 1);
                palindromes[temp_p_index] = search_index;
            }
        }
    }

    end_time = omp_get_wtime();

    for (int i = 0; i < palindrome_index; i++)
        printf("%s : %s \n", lines[palindromes[i]], r_lines[palindromes[i]]);

    printf("Execution time: %g\n", end_time - start_time);
    return 0;
}