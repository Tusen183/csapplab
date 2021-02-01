#include "cachelab.h"
#include <stdio.h>	/* fopen freopen perror */
#include <stdint.h>	/* uintN_t */
#include <unistd.h> /* getopt */
#include <getopt.h> /* getopt -std=c99 POSIX macros defined in <features.h> prevents <unistd.h> from including <getopt.h>*/
#include <stdlib.h> /* atol exit*/
#include <errno.h>  /* errno */
#include <stdbool.h>
#define fasle 0
#define true 1



//cache data struct 
typedef struct{
	bool vaild;
	int tag;
	int LRUcounter;
}Line;

typedef struct{
	Line* lines;
}Set;

typedef struct{
	int s;
	int E;
	int b;
	Set* sets;
}Cache;

typedef struct{
	int hit;
	int miss;
	int eviction;
}Result;

Cache InitCache(int s,int E,int b);
void Release(Cache cache);
Result Test(FILE* fp,Cache cache,bool verbose);
Result Hme(Set Targetset,int tag,int E,Result result,bool verbose);
int main (int argc,char ** argv)
{
	const char *help_message = "Usage: \"Your complied program\" [-hv] -s <s> -E <E> -b <b>-\t<tracefile>\n""<s> <E> <b> should all above zero and \below64.\n""Complied with std=c99\n";
	const char *command_options = "hvs:E:b:t:";
	int s,E,b;
	s = E = b = 0;
	FILE *fp = NULL;
	bool verbose;
	Cache New_cache;
	Result result = {0,0,0};
	
	//commandline;
	char ch;
	while((ch = getopt(argc,argv,command_options))!= -1)
	{
		
		switch(ch)
		{
			case 'h':printf("%s",help_message);
						exit(EXIT_SUCCESS);
			case 'v':	verbose = true;break;
			case 's': s = atol(optarg);	break;
			case 'E': E = atol(optarg);	break;
			case 'b': b = atol(optarg);	break;
			case 't': if((fp = fopen(optarg,"r"))== NULL)
						{
							printf("Failed to open tacefile!\n");
							exit(EXIT_FAILURE);
						}
						break;
			default:
				printf("%s",help_message);
				exit(EXIT_FAILURE);
		}
	}
	if(s == 0 || E == 0 || b==0||fp == NULL)
	{
		printf("%s",help_message);
		exit(EXIT_FAILURE);
	}
	New_cache = InitCache(s,E,b);
	result = Test(fp,New_cache,verbose);
	Release(New_cache);
	printSummary(result.hit,result.miss,result.eviction);
	return 0;
}

Cache InitCache(int s,int E,int b)
{
	Cache cache;
	int S = 1 << s;
	cache.s = s;
	cache.E = E;
	cache.b = b;
	
	cache.sets = (Set*)malloc(sizeof(Set)*S);
	if(cache.sets == NULL)
		perror("Failed to malloc sets int cache!");

	for(int i = 0;i <S;i++)
	{
		if((cache.sets[i].lines = (Line*)malloc(sizeof(Line)*E))== NULL)
			perror("Failed to malloc sets int cache!");
	}
	return cache;
}

void Release(Cache cache)
{
	int S = 1<< cache.s;
	for(int i = 0;i <S;i++)
	{
		free(cache.sets[i].lines);
	}
	free(cache.sets);
}

Result Test(FILE* fp,Cache cache,bool verbose)
{
	Result result = {0,0,0};
	char ch;
	long unsigned int address;
	int s = cache.s,E = cache.E,b = cache.b,size;
	int S = 1<<s;
	int set_index,tag;
	while((fscanf(fp, " %c %lx%*[^\n]", &ch, &address)) == 2)
	{
		
        if(ch == 'I')   continue;
		else 
		{
			int set_index = (address>>b)&(S-1);
			int tag = (address>>b)>>s;
			Set Targetset = cache.sets[set_index];
			if(ch == 'L' || ch == 'S')
			{
				if(verbose) printf("%c %lx ",ch,address);
				result = Hme(Targetset,tag,E,result,verbose);
			}else{
				if(verbose) printf("%c %lx ",ch,address);
				result = Hme(Targetset,tag,E,result,verbose);
				result = Hme(Targetset,tag,E,result,verbose);
			}
				
		}
            
	}
	return result;
}

Result Hme(Set Targetset,int tag,int E,Result result,bool verbose)
{
	int i,index;
	bool hit_flag = false;
	
	for(i = 0;i < E;i++)
	{
		if(Targetset.lines[i].vaild){
			if(Targetset.lines[i].tag == tag){
				if(verbose) printf("hit\n");
				result.hit++;
				Targetset.lines[i].LRUcounter++;
				hit_flag = true;
				break;
				}
			}
	}
	
	if(!hit_flag)
	{
		if(verbose) printf("miss");
		result.miss++;
		int Max_LRU = Targetset.lines[0].LRUcounter;
		int Min_LRU = Targetset.lines[0].LRUcounter;
		index = 0;
		for(i = 0;i < E;i++)
		{
			if(Min_LRU > Targetset.lines[i].LRUcounter)
				{
					Min_LRU = Targetset.lines[i].LRUcounter;
					index = i;
				}
			if(Max_LRU < Targetset.lines[i].LRUcounter)
			{
				Max_LRU = Targetset.lines[i].LRUcounter;
			}	
		}
		Targetset.lines[index].LRUcounter = Max_LRU+1;
		Targetset.lines[index].tag = tag;
		
		if(Targetset.lines[index].vaild)
		{
			if(verbose) printf("and eviction\n");
			result.eviction++;
		}else{
			if(verbose)printf("\n");
			Targetset.lines[index].vaild = true;
		}
	}
	return result;
}

