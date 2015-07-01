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
void hash_function_serial();
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
	hash_function_serial();

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

void hash_function_serial() 
{
	int r = 0;
	r = hash_function(pattern, bigString, lenM);

	if (r == 0)
		printf("pattern not found\n");
	else {
		printf("Pattern found %d number of times\n", r);
	}
}


