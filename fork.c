#include<stdio.h>
#include<string.h>
#include<math.h>
#include<time.h>
#include <stdlib.h>
#include <unistd.h>
#include<omp.h>
#include<sys/stat.h>

int lenM = 0;
int lenP = 0;
long int hashM = 0;
int a = 0;
int pfd[2],retval,pid;

char pattern[] = "the";
void calculate_length(char *p, char *s);
int hash_function(char *p, char *s, int l);
int roll_hash_funct(int index,char* string);
void hash_function_parallel();
clock_t begin, end;
double time_spent = 0.0;
char *bigString;
struct stat st;

int main(int argc, char** argv) 
{
	FILE *fp;
	char c;

	fp = fopen("Arts.txt", "r");
	stat("Arts.txt", &st);
	bigString = (char*) malloc(st.st_size);
	char *trav = bigString;
	if (fp == NULL) 
	{
		printf("error\n");
	} 
	else 
	{
		while ((c = fgetc(fp)) != EOF) 
		{
			*trav = c;
			trav++;
		}
	}

	calculate_length(pattern, bigString);
	begin = clock();
	hash_function_parallel();

free(bigString);
return 0;

}

/**Rolling hashing function which returns hashing value from given index to length of pattern**/
int roll_hash_funct(int index,char* string)
{
	int digit ;
	int sub = index -1;
	digit = index+lenP-1;
	int remove_char_hash = 0,add_char_hash = 0;
	remove_char_hash =( string[sub]* (long int) (pow(7,lenP)) ) % 147665127123456;
	add_char_hash = string[digit]* (long int) (pow(7,1));
	hashM = ((hashM - remove_char_hash) * 7 + add_char_hash ) % 147665127123456;
	return hashM;
}

void calculate_length(char *p, char *s) 
{
	while (*p != '\0') 
	{
		p++;
		lenP++;
	}
	printf("\n");
	while (*s != '\0') 
	{
		s++;
		lenM++;
	}
}

int hash_function(char *p, char *s, int l) 
{
	char * traverse1 = s;
	char * traverse2 = s;
	int new_len = 0;
	new_len = l;
	int count = 0;
	int hashP = 0;
	int b = 0;
	/*Calculates hash value of pattern  */
	for (a = 0; a < lenP; a++) 
	{
		hashP = (hashP + pattern[a] * (long int) pow(7, lenP - a)) % 147665127123456;
	}

	printf("value of hash of pattern is:%d\n", hashP);
	/*Calculates hash value of main string at value index 0  */
	for (a = 0; a < lenP; a++) 
	{
		hashM = (hashM + (*traverse1) * (long int) (pow(7, lenP - a))) % 147665127123456;
		traverse1++;
	}
	while (b < new_len - lenP + 1) 
	{
		if (hashP == hashM) 
		{
			count++;
			b++;
			hashM = roll_hash_funct(b, traverse2);
		} 
		else 
		{
			b++;
			hashM = roll_hash_funct(b, traverse2);
		}
	}
	return count;
}

/****FORKING DIvide main string into 2 parts.Let 2 process search in each of them individually*************/

void hash_function_parallel() 
{
	char *trav1 = bigString;
	int i, j;
	int pid;
	int count1 = 0;
	int count2 = 0;
	int main_count = 0;
	int mid = ceil(lenM / 2);
	char *string1 = (char*) malloc(sizeof(char) * st.st_size);
	char *string2 = (char*) malloc(sizeof(char) * st.st_size);
	double time_spent_child = 0;
	char *temp1 = string1;
	char *temp2 = string2;

	while (trav1[mid] != ' ') 
	{
		mid = mid + 1;
	}
	for (i = 0; i < mid; i++) 
	{
		*temp1 = *trav1;
		trav1++;
		temp1++;
	}
	printf("\n");
	for (j = mid; j < lenM; j++) 
	{
		*temp2 = *trav1;
		trav1++;
		temp2++;
	}
	retval = pipe(pfd);
	pid = fork();
	if (pid < 0)
	{
		printf("Cannot fork process\n");

	} 
	else if (pid == 0)
	{
		//work on first half
		printf("Child Process. My id is %d\n",getpid());
		count1 = hash_function(pattern, string1, mid);
		write(pfd[1],&count1,sizeof(int));
		printf("Count by child process is: %d\n", (count1));
		end = clock();		
		time_spent = (double) (end - begin) / CLOCKS_PER_SEC;
		printf("time spent by child process is: %f\n", time_spent);
		write(pfd[1],&time_spent,sizeof(double));
	} 
	else 
	{
		//work on second half
		printf("Parent Process. My id is %d\n",getpid());
		count2 = hash_function(pattern, string2, lenM - mid);
		printf("Count by parent process is: %d\n", count2);
		read(pfd[0],&main_count,sizeof(main_count));
		printf("\n****Total count found in string is: %d****\n", (count2 + main_count));
		end = clock();
		time_spent = (double) (end - begin) / CLOCKS_PER_SEC;
		printf("time spent is parent process is: %f\n", time_spent);
		read(pfd[0],&time_spent_child,sizeof(time_spent_child));
		printf("\n****Total time spent is: %f****\n",(time_spent_child + time_spent));		
	}

	free(string1);
	free(string2);
}
