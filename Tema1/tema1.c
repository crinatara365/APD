#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_INPUT 30000

typedef struct {
    int exp_max;
    int mappers;
    int size;
    FILE *files[MAX_INPUT];
    FILE *output; 
    long id_thread;
    int **list;
    int *sizes;
} Parametres;

pthread_barrier_t barrier;

long long power2 (int a, int b) {
    long long result = 1;
    while(b > 0) {
        result *= a;
        b--;
    }
    return result;
}

int size_file (FILE *file) {
    fseek(file, 0L, SEEK_END);
    int res = ftell(file);
    fseek(file, 0L, SEEK_SET);
    return res;
}

int smallest_dim (int mappers, int files, int list[mappers][files], FILE *inputFiles[]) {
    int min = 2147483647;
    int mapper=-1;
    for(int i = 0; i < mappers; i++) {
        int total = 0;
        for(int j = 0; j < files; j++) {
            if(list[i][j] != -1) {
                FILE *aux = inputFiles[list[i][j]];
                total += size_file(aux);
            }
            else break;
        }
        if(total < min) {
            min = total;
            mapper = i;
        }
    }
    return mapper;
}

int countDistinct(int a[], int n) {
   int i, j, count = 1;
   for (i = 1; i < n; i++) {
      for (j = 0; j < i; j++) {
         if (a[i] == a[j]) {
            break;             
        }
    }
        if (i == j) {
            count++;     
        }
    }
   return count;      
}

void *f_thread(void *param) {
    Parametres *parametres = (Parametres*) param;    

    /* Mapperii isi incep executia */
    if(parametres->id_thread < parametres->mappers) {
        for(int h = 0; h < parametres->size; h++) {
            int nr_values;
            fscanf(parametres->files[h], "%d", &nr_values);
      
            for(int i = 0; i < nr_values; i++) {
                int value;  
                fscanf(parametres->files[h], "%d", &value);
                if(value == 0) continue;
                else if(value == 1) 
                    for(int k = 0; k < parametres->exp_max-1; k++) {
                        parametres->list[k][parametres->sizes[k]++] = 1;
                    }
                else {     
                    for(int n = 2; n <= parametres->exp_max; n++) {
                        int low = 2, high = value;  
                        while(low < high) {
                            unsigned long long p = power2(low, n);
                            unsigned long long h = power2(high,n);
                            if(p == value || h == value) {
                                parametres->list[n-2][parametres->sizes[n-2]++] = value;
                                break;
                            }
                            int mid = (low + high) / 2;
                            unsigned long long res = power2(mid, n);
                            if(res == value) {
                                parametres->list[n-2][parametres->sizes[n-2]++] = value;
                                break;
                            }
                            else if(res > value) { //stanga 
                                high = mid - 1;
                            }
                            else{ //dreapta 
                                low = mid + 1;   
                            } 
                        }
                    }
                }
            }
        }
    }
    pthread_barrier_wait(&barrier); 

    if(parametres->id_thread >= parametres->mappers) {
        fprintf(parametres->output, "%d", countDistinct(parametres->list[parametres->id_thread-parametres->mappers], parametres->sizes[parametres->id_thread-parametres->mappers]));
    }

  	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int nr_mappers = atoi(argv[1]);
    int nr_reducers = atoi(argv[2]);
    FILE *input = fopen(argv[3], "r"); 
    if(input == NULL) return 0;
    int nr_files;
    fscanf(input, "%d", &nr_files);
    FILE *inputFiles[nr_files];

    pthread_t threads[nr_mappers + nr_reducers];
    long ids[nr_mappers + nr_reducers];
    long id;
  	int r;
  	void *status;

    /* citim in inputFiles fisierele de intrare */
    for(int i = 0; i < nr_files; i++) {
        char s[30];
        fscanf(input, "%s", s);
        inputFiles[i] = fopen(s, "r");

        if(inputFiles[i] == NULL) {
            return 0;
        }
    }

    /* Sortam descrescator lista de fisiere dupa dimensiune */
    for (int i = 0 ; i < nr_files - 1; i++) {
        for (int j = 0 ; j < nr_files - i - 1; j++) {
            if(size_file(inputFiles[j]) < size_file(inputFiles[j+1])) {
                FILE *temp = inputFiles[j];
                inputFiles[j] = inputFiles[j+1];
                inputFiles[j+1] = temp;
            }
        }
    }

    int split_files[nr_mappers][nr_files];
    for(int i = 0; i < nr_mappers; i++)
        for(int j = 0; j < nr_files; j++)
            split_files[i][j] = -1;  
    for(int i = 0; i < nr_files; i++) {
        /* se adauga fisierul in sirul mapper-ului cu cea mai mica dimensiune totala a fisierelor */
        int aux = smallest_dim(nr_mappers, nr_files, split_files, inputFiles);
        for(int j = 0; j < nr_files; j++) {
            if(split_files[aux][j] == -1) {
                split_files[aux][j] = i;
                break;
            }
        }
    }

    pthread_barrier_init(&barrier, NULL, nr_mappers+nr_reducers);
    int **list;
    list = (int**)malloc((nr_reducers) * sizeof(int*));
    for(int i = 0; i < MAX_INPUT; i++)
       list[i] = (int*)malloc(MAX_INPUT * sizeof(int));
    if(list == NULL) printf("Nu s-a reusit alocarea\n");
    int* s = (int*)malloc((nr_reducers+1)*sizeof(int*));
    for(int i = 0; i < nr_reducers; i++)
        s[i] = 0;

  	for (id = 0; id < nr_mappers + nr_reducers; id++) {
            Parametres * param = malloc(sizeof(Parametres));
            param->sizes = s;
            param->size = 0;
            param->list = list;
            param->exp_max = nr_reducers + 1;
            param->mappers = nr_mappers;
            if(id < nr_mappers) {
                FILE* f[nr_files];
                for(int i = 0; i < nr_files; i++)
                    if(split_files[id][i] != -1) {
                        f[i] = inputFiles[split_files[id][i]];
                        param->size++;
                    }
                    else break;
                memcpy(param->files, f, sizeof f);
            }
            else {
                char c = '0' + (id-nr_mappers+2);
                char s1[] = "out";
                strncat(s1, &c, 1);
                strcat(s1, ".txt");
                FILE *f;
                f = fopen(s1, "w");
                if(f == NULL) printf("Eroare la creare\n");
                param->output = f;
            }
            ids[id] = id;
            param->id_thread = ids[id];
		    r = pthread_create(&threads[id], NULL, f_thread, param);
		    if (r) {
	  		    printf("Eroare la crearea thread-ului %ld\n", id);
	  		    exit(-1);
		    }
        }

    for (id = 0; id < nr_mappers + nr_reducers; id++) {
        r = pthread_join(threads[id], &status);
		if (r) {
	  	    printf("Eroare la asteptarea thread-ului %ld\n", id);
	  	    exit(-1);
		}    
  	}

    free(list);
    free(s);
    fclose(input);
    pthread_barrier_destroy(&barrier);
    pthread_exit(NULL);
}
