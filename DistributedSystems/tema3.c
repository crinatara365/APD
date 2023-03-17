#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int notify_workers(FILE* file, int source, int topology[]) {
    int nr_workers, rank;
    fscanf(file, "%d", &nr_workers);
    for(int i = 0; i < nr_workers; i++) {
        fscanf(file, "%d", &rank);
        printf("M(%d,%d)\n", source, rank);
        /* Procesul parinte isi notifica workerii */
        MPI_Send(&source, 1, MPI_INT, rank, 0, MPI_COMM_WORLD);
        topology[i] = rank;
    }
    return nr_workers;
}

int main (int argc, char *argv[])
{
    int numtasks, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int workers_0, workers_1, workers_2, workers_3;
    int topology[numtasks-4];
    int source;
    int dim_vector = atoi(argv[1]);
    int vector[dim_vector];

    if(rank == 0) {
        FILE* file = fopen("cluster0.txt", "r");
        if (file == NULL) printf("File can't be opened\n");
        else workers_0 = notify_workers(file, 0, topology);
    } else if(rank == 1) {
        FILE* file = fopen("cluster1.txt", "r");
        if (file == NULL) printf("File can't be opened\n");
        else workers_1 = notify_workers(file, 1, topology);
    } else if(rank == 2) {
        FILE* file = fopen("cluster2.txt", "r");
        if (file == NULL) printf("File can't be opened\n");
        else workers_2 = notify_workers(file, 2, topology);
    } else if(rank == 3){
        FILE* file = fopen("cluster3.txt", "r");
        if (file == NULL) printf("File can't be opened\n");
        else workers_3 = notify_workers(file, 3, topology);
    } else { /* Procesele care nu sunt parinte fac recv */
        MPI_Status status;
        MPI_Recv(&source, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
    }

    /* Pana aici, toti copii si-au aflat parintii */
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) {
        printf("M(0,3)\n");
        MPI_Send(&topology, workers_0, MPI_INT, 3, 0, MPI_COMM_WORLD);
    }
    if(rank == 3) {
        /* Procesul 3 primeste topologia de pana acum de la procesul 0 */
        int workers0;
        FILE* file = fopen("cluster0.txt", "r");
        if (file == NULL) printf("File can't be opened\n");
        else fscanf(file, "%d", &workers0);
        MPI_Status status;
        int topology_aux[workers0];
        MPI_Recv(&topology_aux, workers0, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        int topology_now[workers0+workers_3];
        int i = 0;
        for(; i < workers0; i++)
            topology_now[i] = topology_aux[i];
        for(int j = 0; j < workers_3; j++) {
            topology_now[i] = topology[j];
            i++;
        }    
        printf("M(3,2)\n");
        MPI_Send(&topology_now, workers0+workers_3, MPI_INT, 2, 0, MPI_COMM_WORLD);
    }
    if(rank == 2) {
        /* Procesul 2 primeste topologia de la 3 */
        int workers0;
        FILE* file1 = fopen("cluster0.txt", "r");
        if (file1 == NULL) printf("File can't be opened\n");
        else fscanf(file1, "%d", &workers0);
        int workers3;
        FILE* file2 = fopen("cluster3.txt", "r");
        if (file2 == NULL) printf("File can't be opened\n");
        else fscanf(file2, "%d", &workers3);
        MPI_Status status;
        int topology_aux[workers0+workers3];
        MPI_Recv(&topology_aux, workers0+workers3, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);
        int topology_now[workers0+workers3+workers_2], i = 0;
        for( ; i < workers0+workers3; i ++)
            topology_now[i] = topology_aux[i];
        for(int j = 0; j < workers_2; j ++) {
            topology_now[i] = topology[j];
            i++;
        }
        printf("M(2,1)\n");
        MPI_Send(&topology_now, workers0+workers3+workers_2, MPI_INT, 1, 0, MPI_COMM_WORLD);
    }
    if(rank == 1) {
        int workers0;
        FILE* file1 = fopen("cluster0.txt", "r");
        if (file1 == NULL) printf("File can't be opened\n");
        else fscanf(file1, "%d", &workers0);
        int workers3;
        FILE* file2 = fopen("cluster3.txt", "r");
        if (file2 == NULL) printf("File can't be opened\n");
        else fscanf(file2, "%d", &workers3);
        int workers2;
        FILE* file3 = fopen("cluster2.txt", "r");
        if (file3 == NULL) printf("File can't be opened\n");
        else fscanf(file3, "%d", &workers2);
        int topology_aux[workers0+workers3+workers2];
        MPI_Recv(&topology_aux, workers0+workers3+workers2, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        /* Procesul 1 a aflat toata topologia */
        char final_topology[100000] = "";
        strcat(final_topology, " -> 0:");
        
        for(int i = 0; i < workers0; i++){
            char s[2];
            sprintf(s, "%d", topology_aux[i]);
            if(i == workers0 - 1) {
                strcat(final_topology, s);
                strcat(final_topology, " ");
            } else {
                strcat(final_topology, s);
                strcat(final_topology, ",");
            }
        }
        strcat(final_topology, "1:");    
        FILE* file4 = fopen("cluster1.txt", "r");
        if (file4 == NULL) printf("File can't be opened\n");
        else {
            int rank;
            fscanf(file4, "%d", &rank);
            for(int i = 0; i < workers_1; i++) {
                fscanf(file4, "%d", &rank);
                char s[2];
                sprintf(s, "%d", rank);
                if(i == workers_1 - 1) {
                    strcat(final_topology, s);
                    strcat(final_topology, " ");
                } else {
                    strcat(final_topology, s);
                    strcat(final_topology, ",");
                }
            }

        }
        strcat(final_topology, "2:");   
        for(int i = workers0 + workers3; i < workers0 + workers3 + workers2; i++) {
            char s[2];
            sprintf(s, "%d", topology_aux[i]);
            if(i == workers0 + workers3 + workers2 - 1) {
                strcat(final_topology, s);
                strcat(final_topology, " ");
            } else {
                strcat(final_topology, s);
                strcat(final_topology, ",");
            }
        }
        strcat(final_topology, "3:");  
        char aux[200] = "";
        for(int i = workers0; i < workers0 + workers3; i++) {
            char s[2];
            sprintf(s, "%d", topology_aux[i]);
            if(i == workers0 + workers3 - 1) {
                strcat(final_topology, s);
                strcat(final_topology, " ");
            }
            else {
                strcat(final_topology, s);
                strcat(final_topology, ",");
            }
        }

        printf("M(1,2)\n");
        MPI_Send(&final_topology, 100000, MPI_CHAR, 2, 0, MPI_COMM_WORLD);
        /* 1 trimite copiilor lui topologia finala */
        for(int i = 0; i < workers_1; i++) {
            printf("M(1,%d)\n", topology[i]);
            MPI_Send(&final_topology, 100000, MPI_CHAR, topology[i], 0, MPI_COMM_WORLD);
        }

        char str[20];
        sprintf(str, "%d", rank);
        strcat(str, final_topology);
        printf("%s\n", str);
    }
    if(rank == 2) {
        char final_topology[100000];
        MPI_Recv(&final_topology, 100000, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("M(2,3)\n");
        MPI_Send(&final_topology, 100000, MPI_CHAR, 3, 0, MPI_COMM_WORLD);
        for(int i = 0; i < workers_2; i++) {
            printf("M(2,%d)\n", topology[i]);
            MPI_Send(&final_topology, 100000, MPI_CHAR, topology[i], 0, MPI_COMM_WORLD);
        }
        char str[20];
        sprintf(str, "%d",rank);
        strcat(str, final_topology);
        printf("%s\n", str);
    }
    if(rank == 3) {
        char final_topology[100000];
        MPI_Recv(&final_topology, 100000, MPI_CHAR, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        printf("M(3,0)\n");
        MPI_Send(&final_topology, 100000, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        for(int i = 0; i < workers_3; i++) {
            printf("M(3,%d)\n", topology[i]);
            MPI_Send(&final_topology, 100000, MPI_CHAR, topology[i], 0, MPI_COMM_WORLD);
        }
        char str[20];
        sprintf(str, "%d",rank);
        strcat(str, final_topology);
        printf("%s\n", str);
    }
    if(rank == 0) {
        char final_topology[100000];
        MPI_Recv(&final_topology, 100000, MPI_CHAR, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
        for(int i = 0; i < workers_0; i++) {
            printf("M(0,%d)\n", topology[i]);
            MPI_Send(&final_topology, 100000, MPI_CHAR, topology[i], 0, MPI_COMM_WORLD);
        }
        char str[20];
        sprintf(str, "%d",rank);
        strcat(str, final_topology);
        printf("%s\n", str);
    }
    
    /* Pana aici toti parintii au aflat topologia si au afisat-o */
    if(rank > 3) {
        char final_topology[100000];
        MPI_Recv(&final_topology, 100000, MPI_CHAR, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        char str[20];
        sprintf(str, "%d",rank);
        strcat(str, final_topology);
        printf("%s\n", str);
    }

    /* toate procesele stiu topologia, incepe partea de calcule */
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) {
        /* ansamblam vectorul de dimensiune dim_vector */
        for(int i = 0; i < dim_vector; i++)
            vector[i] = dim_vector - i - 1;
        MPI_Send(&vector, dim_vector, MPI_INT, 3, 0, MPI_COMM_WORLD);

        FILE* file = fopen("cluster0.txt", "r");
        if (file == NULL) printf("File can't be opened\n");
        else {
            int rank;
            fscanf(file, "%d", &rank);
            for(int i = 0; i < workers_0; i++) {
                fscanf(file, "%d", &rank);
                MPI_Send(&vector, dim_vector, MPI_INT, rank, 0, MPI_COMM_WORLD);
            }
        }
    }
    if(rank == 3) {
        MPI_Recv(&vector, dim_vector, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&vector, dim_vector, MPI_INT, 2, 0, MPI_COMM_WORLD);
        
        FILE* file = fopen("cluster3.txt", "r");
        if (file == NULL) printf("File can't be opened\n");
        else {
            int rank;
            fscanf(file, "%d", &rank);
            for(int i = 0; i < workers_3; i++) {
                fscanf(file, "%d", &rank);
                MPI_Send(&vector, dim_vector, MPI_INT, rank, 0, MPI_COMM_WORLD);
            }
        }
    }
    if(rank == 2) {
        MPI_Recv(&vector, dim_vector, MPI_INT, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&vector, dim_vector, MPI_INT, 1, 0, MPI_COMM_WORLD);
        
        FILE* file = fopen("cluster2.txt", "r");
        if (file == NULL) printf("File can't be opened\n");
        else {
            int rank;
            fscanf(file, "%d", &rank);
            for(int i = 0; i < workers_2; i++) {
                fscanf(file, "%d", &rank);
                MPI_Send(&vector, dim_vector, MPI_INT, rank, 0, MPI_COMM_WORLD);
            }
        }
    }
    if(rank == 1) {
        MPI_Recv(&vector, dim_vector, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        FILE* file = fopen("cluster1.txt", "r");
        if (file == NULL) printf("File can't be opened\n");
        else {
            int rank;
            fscanf(file, "%d", &rank);
            for(int i = 0; i < workers_1; i++) {
                fscanf(file, "%d", &rank);
                MPI_Send(&vector, dim_vector, MPI_INT, rank, 0, MPI_COMM_WORLD);
            }
        }    
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank > 3) {
        /* Fiecare worker modifica partea lui */
        MPI_Recv(&vector, dim_vector, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int process_index = rank - 4;
        int start = (process_index * dim_vector) / (numtasks - 4);
        int end = ((process_index + 1) * dim_vector) / (numtasks - 4);
        for(int i = start; i < end; i++) 
            vector[i] = 5 * vector[i]; 
        /* Fiecare worker retrimite vectorul la coordonatul lui */   
        MPI_Send(&vector, dim_vector, MPI_INT, source, 0, MPI_COMM_WORLD);
    }
    if(rank == 1) {
        FILE* file = fopen("cluster1.txt", "r");
        if (file == NULL) printf("File can't be opened\n");
        else {
            int rank;
            fscanf(file, "%d", &rank);
            for(int i = 0; i < workers_1; i++) {
                int vector_aux[dim_vector];
                fscanf(file, "%d", &rank);
                MPI_Recv(&vector_aux, dim_vector, MPI_INT, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int process_index = rank - 4;
                int start = (process_index * dim_vector) / (numtasks - 4);
                int end = ((process_index + 1) * dim_vector) / (numtasks - 4);
                for(int i = start; i < end; i++)
                    vector[i] = vector_aux[i];
            }
        }
        MPI_Send(&vector, dim_vector, MPI_INT, 2, 0, MPI_COMM_WORLD);   
    }
    if(rank == 2) {
        int vector_aux[dim_vector];
        MPI_Recv(&vector_aux, dim_vector, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for(int i = 0; i < dim_vector; i++) 
            vector[i] = vector_aux[i];
        FILE* file = fopen("cluster2.txt", "r");
        if (file == NULL) printf("File can't be opened\n");
        else {
            int rank;
            fscanf(file, "%d", &rank);
            for(int i = 0; i < workers_2; i++) {
                fscanf(file, "%d", &rank);
                MPI_Recv(&vector_aux, dim_vector, MPI_INT, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int process_index = rank - 4;
                int start = (process_index * dim_vector) / (numtasks - 4);
                int end = ((process_index + 1) * dim_vector) / (numtasks - 4);
                for(int i = start; i < end; i++)
                    vector[i] = vector_aux[i];
            }
        }
        MPI_Send(&vector, dim_vector, MPI_INT, 3, 0, MPI_COMM_WORLD);   
    }
    if(rank == 3) {
        int vector_aux[dim_vector];
        MPI_Recv(&vector_aux, dim_vector, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for(int i = 0; i < dim_vector; i++) 
            vector[i] = vector_aux[i];
        FILE* file = fopen("cluster3.txt", "r");
        if (file == NULL) printf("File can't be opened\n");
        else {
            int rank;
            fscanf(file, "%d", &rank);
            for(int i = 0; i < workers_3; i++) {
                fscanf(file, "%d", &rank);
                MPI_Recv(&vector_aux, dim_vector, MPI_INT, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int process_index = rank - 4;
                int start = (process_index * dim_vector) / (numtasks - 4);
                int end = ((process_index + 1) * dim_vector) / (numtasks - 4);
                for(int i = start; i < end; i++)
                    vector[i] = vector_aux[i];
            }
        }
        MPI_Send(&vector, dim_vector, MPI_INT, 0, 0, MPI_COMM_WORLD);   
    }
    if(rank == 0) {
       int vector_aux[dim_vector];
        MPI_Recv(&vector_aux, dim_vector, MPI_INT, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for(int i = 0; i < dim_vector; i++) 
            vector[i] = vector_aux[i];
        FILE* file = fopen("cluster0.txt", "r");
        if (file == NULL) printf("File can't be opened\n");
        else {
            int rank;
            fscanf(file, "%d", &rank);
            for(int i = 0; i < workers_0; i++) {
                fscanf(file, "%d", &rank);
                MPI_Recv(&vector_aux, dim_vector, MPI_INT, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int process_index = rank - 4;
                int start = (process_index * dim_vector) / (numtasks - 4);
                int end = ((process_index + 1) * dim_vector) / (numtasks - 4);
                for(int i = start; i < end; i++)
                    vector[i] = vector_aux[i];
            }
        }
        printf("Rezultat: ");
        for(int i = 0; i < dim_vector; i++)
            printf("%d ", vector[i]);  
        printf("\n");    
    }

    MPI_Finalize();
}