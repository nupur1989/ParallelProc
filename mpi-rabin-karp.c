#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>   
#include <sys/stat.h>

#define send_data_tag 2001
#define return_data_tag 2002
#define mod 147665127123456

int lenM = 0;
int lenP = 0;
long int hashM = 0;
int a = 0;
int pfd[2],retval,pid;

char pattern[] = "the";
void calculate_length(char *p, char *s);
int hash_function(char *p, char *s, int l);
void hash_function_serial();
int roll_hash_funct(int index,char* string);
char *bigString;
int start_index[20];
struct stat st;


long int sum, partial_sum;
MPI_Status status;
int my_id, root_process, ierr, i, num_rows, num_procs, 
    an_id, num_rows_to_receive, avg_rows_per_process, 
    sender, num_rows_received, start_row, end_row, num_rows_to_send,start_index_str;

/**Rolling hashing function which returns hashing value from given index to length of pattern**/
int roll_hash_funct(int index,char* string)
{
  int digit ;
  int sub = index -1;
  digit = index+lenP-1;
  int remove_char_hash = 0,add_char_hash = 0;
  remove_char_hash =( string[sub]* (long int) (pow(7,lenP)) ) % mod;
  add_char_hash = string[digit]* (long int) (pow(7,1));
  hashM = ((hashM - remove_char_hash) * 7 + add_char_hash ) % mod;
  return hashM;
}


void calculate_length(char *p, char *s) {
  while (*p != '\0') {
    p++;
    lenP++;
  }
  printf("\n");
  while (*s != '\0') {
    s++;
    lenM++;
  }
}

int hash_function(char *p, char *s, int l) {
  char * traverse1 = s;
  char * traverse2 = s;
  int new_len = 0;
  new_len = l;
  int count = 0;
  int hashP = 0;
  int b = 0;
  /*Calculates hash value of pattern  */
  for (a = 0; a < lenP; a++) {
    hashP = (hashP + pattern[a] * (long int) pow(7, lenP - a)) % mod;
  }

  printf("value of hash of pattern is:%d\n", hashP);
  /*Calculates hash value of main string at value index 0  */
  for (a = 0; a < lenP; a++) {
    hashM = (hashM + (*traverse1) * (long int) (pow(7, lenP - a))) % mod;
    traverse1++;
  }
  while (b < new_len - lenP + 1) {
    if (hashP == hashM) {
      count++;
      b++;
      hashM = roll_hash_funct(b, traverse2);
    } 
    else {
      b++;
      hashM = roll_hash_funct(b, traverse2);
    }
  }
  return count;
}

main(int argc, char **argv) {
  FILE *fp;
  char c;
  char *temp;
  fp = fopen("Arts.txt", "r");
  stat("Arts.txt", &st);
  bigString = (char*) malloc(st.st_size);
  char *trav = bigString;
  char *string1 = (char*) malloc(sizeof(char) * st.st_size);
  if (fp == NULL) {
    printf("error\n");
  } 
  else {
    while ((c = fgetc(fp)) != EOF) {
      *trav = c;
      trav++;
    }
  }
  int avg_length = 0;
  calculate_length(pattern, bigString);
  /* Now replicte this process to create parallel processes.
   * From this point on, every process executes a seperate copy
   * of this program */
  ierr = MPI_Init(&argc, &argv);
  root_process = 0;
  
  /* find out MY process ID, and how many processes were started. */
  
  ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
  ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  if(my_id == root_process) {
     
     /* I must be the root process, so I will query the user
      * to determine how many numbers to sum. */

    avg_rows_per_process = lenM / num_procs;
    char *trav1 = bigString;
    end_row = avg_rows_per_process;
    while (trav1[end_row] != ' ') {
      end_row = end_row + 1;
    }
    start_index[0] = end_row;
    /* distribute a portion of the main string to each child process */
    for(an_id = 1; an_id < num_procs; an_id++) {
      start_row = start_index[an_id-1]+1;
      end_row   =  start_row + avg_rows_per_process;
	    if((lenM - end_row) < 0)
        end_row = lenM;
      else {
        while (trav1[end_row] != ' ') {
          end_row = end_row + 1;
        }
      }
      start_index[an_id] = end_row;
      num_rows_to_send = end_row - start_row + 1;
      ierr = MPI_Send( &num_rows_to_send, 1 , MPI_INT,an_id, send_data_tag, MPI_COMM_WORLD);
      ierr = MPI_Send( &start_row, 1 , MPI_INT, an_id, send_data_tag, MPI_COMM_WORLD);
    }
    /* and calculate the number of matches of pattern in main string in the segment assigned
     * to the root process */
    char *temp_string = (char*) malloc(start_index[1]-1);        
    sum = 0;
    for(i = 0; i < (start_index[0]); i++) {
      temp_string[i] = trav1[i];   
    } 
    sum = hash_function(pattern, temp_string, start_index[1]-1);
    temp_string[i+1]='\0';
    printf("sum %lu calculated by root process\n", sum);

    /* and, finally, I collect the partial sums from the slave processes, 
     * print them, and add them to the grand sum, and print it */
    for(an_id = 1; an_id < num_procs; an_id++) {
      ierr = MPI_Recv( &partial_sum, 1, MPI_LONG, MPI_ANY_SOURCE,
      return_data_tag, MPI_COMM_WORLD, &status);
      sender = status.MPI_SOURCE;
      printf("Partial sum %lu returned from process %i\n", partial_sum, sender);
      sum += partial_sum;
     }

     printf("The grand total is: %lu\n", sum);
  }

  else {
    /* I must be a slave process, so I must receive my array segment,
     * storing it in a "local" array, array1. */

    ierr = MPI_Recv( &num_rows_to_receive, 1, MPI_INT, root_process, send_data_tag, MPI_COMM_WORLD, &status);
    ierr = MPI_Recv( &start_index_str, 1, MPI_INT, root_process, send_data_tag, MPI_COMM_WORLD, &status);
     
    char *temp = bigString;
    num_rows_received = num_rows_to_receive;

    char *temp_string = (char*) malloc(num_rows_received);
    /* Calculate the sum of my portion of the array */
    partial_sum = 0;
    for(i = 0; i < num_rows_to_receive; i++) {
      temp_string[i] = temp[start_index_str + i];   
    } 
    temp_string[num_rows_to_receive] = '\0';
    partial_sum = hash_function(pattern, temp_string,num_rows_received);
    ierr = MPI_Send( &partial_sum, 1, MPI_LONG, root_process, 
    return_data_tag, MPI_COMM_WORLD);
  }
  ierr = MPI_Finalize();
}
