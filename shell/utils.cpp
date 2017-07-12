
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#define MAX_ARGUMENTS 256
#ifndef NULL
#define NULL 0
#endif

#define FREE(X) if(X) free((void*)X)

//static int variable for exit status of proccess
static int status = 0;

//a struct for commands
typedef struct cmdLine
{
	char * const arguments[MAX_ARGUMENTS]; /* command line arguments (arg 0 is the command)*/
	int argCount;		/* number of arguments */
	char blocking;	/* boolean indicating blocking/non-blocking */
	int idx;				/* index of current command in the chain of cmdLines (0 for the first) */
	struct cmdLine *next;	/* next cmdLine in chain */
} cmdLine;

//a function that copy the given string with memory allocation
static char *strClone(const char *source)
{
	char* clone = (char*)malloc(strlen(source) + 1);
	strcpy(clone, source);
	return clone;
}

//passing on spaces
static int isEmpty(const char *str)
{
	if (!str)
		return 1;

	while (*str)
		if (!isspace(*(str++)))
			return 0;

	return 1;
}

//a function that manipulates the enviromental variables
static cmdLine *envVariables(cmdLine *pCmdLine){
	//passing on each argument
	for(int i = 0; i < pCmdLine->argCount; i++){
		char *str = pCmdLine->arguments[i];
		char *_str = (char*)malloc(1);
		int place = 0;
		_str[place] = '\0';
		//passing on the string
		for(unsigned j = 0; j < strlen(str); j++){
			//switching '~' as the first letter to home
            		if (j == 0 && str[j] == '~'){
            			char *tmp = getenv("HOME");
            			char* res = NULL;
        			if (tmp != NULL) {
            				res = strClone(tmp);
        			}else{
            				res = strClone("");
        			}
        			if(res != NULL){
            				_str = (char*)realloc(_str,place + strlen(res) +1 );
            				strcat(_str, res);
        			}
        			place+=strlen(res);
        			FREE(res);
            		}
			//checking for '$' and then a correct continuity
			else if (str[j] == '$' && j < strlen(str) - 1 && (str[j+1] != '$')){
				//'$?' case - switching to the status
				if(str[j+1] == '?'){
					char tmp[10];
					sprintf(tmp, "%d", status);
					_str = (char*)realloc(_str,place + strlen(tmp) + 2 );
                    			strcat(_str, tmp);
                    			place+=strlen(tmp);
                    			j++;
				}
				//normal '$VAR'
				else if(isalpha(str[j+1])){
					unsigned k;
					for(k=j+2;k<strlen(str) && (isalnum(str[k]) || str[k] == '_' );k++);
					//from j+1 to k = var
					char *replacement = (char*)malloc(k-j);
					memcpy(replacement, &str[j+1], k-j-1);
					replacement[k-j] = '\0';
					char *tmp = getenv(replacement);
					char* res = NULL;
					//checking if there is VAR, if not setting as ""
					if (tmp != NULL) {
						res = strClone(tmp);
					}else{
						res = strClone("");
					}
					FREE(replacement);
					if(res != NULL){
                        			_str = (char*)realloc(_str,place + strlen(res) +1 );
                        			strcat(_str, res);
                    			}
                    			j = k - 1;
                    			place+=strlen(res);
                    			FREE(res);
                		}
            		}
			//copying the current char
            		else{
                		_str = (char*)realloc(_str, strlen(_str) + 1);
                		_str[place] = str[j];
                		_str[place + 1] = '\0';
                		place++;
            		}
        	}
		//setting the new string after changes to the argument
		FREE((pCmdLine->arguments)[i]);
		((char**)pCmdLine->arguments)[i] = _str;
	}
	return pCmdLine;
}

//parsing the line by white spaces
static cmdLine *parseSingleCmdLine(const char *strLine)
{
	char const *delimiter = " ";
	char *line, *result;

	if (isEmpty(strLine))
		return NULL;

	cmdLine* pCmdLine = (cmdLine*)malloc( sizeof(cmdLine) ) ;
	memset(pCmdLine, 0, sizeof(cmdLine));

	line = strClone(strLine);

	result = strtok( line, delimiter);
	while( result && pCmdLine->argCount < MAX_ARGUMENTS-1) {
		((char**)pCmdLine->arguments)[pCmdLine->argCount++] = strClone(result);
		result = strtok ( NULL, delimiter);
	}

	FREE(line);
	return pCmdLine;
}

//checking the line and parsing it
static cmdLine* _parseCmdLines(char *line)
{
	cmdLine *pCmdLine;

	if (isEmpty(line))
		return NULL;

	pCmdLine = parseSingleCmdLine(line);
	if (!pCmdLine)
		return NULL;

	return pCmdLine;
}

//Parses a given string to arguments, returns NULL if there is nothing to parse
inline static cmdLine *parseCmdLines(const char *strLine)
{
	char* line, *ampersand;
	cmdLine *head, *last;
	int idx = 0;

	if (isEmpty(strLine))
		return NULL;

	line = strClone(strLine);
	if (line[strlen(line)-1] == '\n')
		line[strlen(line)-1] = 0;
	//checking for &(background proccess)
	ampersand = strchr( line,  '&');
	if (ampersand)
		*(ampersand) = 0;

	if ( (last = head = _parseCmdLines(line)) )
	{
		while (last->next)
			last = last->next;
		last->blocking = ampersand? 0:1;
	}

	for (last = head; last; last = last->next)
		last->idx = idx++;

	FREE(line);
	//checking for enviromntal vars
	head = envVariables(head);
	return head;
}

//Free parsed line
static void freeCmdLines(cmdLine *pCmdLine)
{
	int i;
	if (!pCmdLine)
		return;

	for (i=0; i<pCmdLine->argCount; ++i)
		FREE(pCmdLine->arguments[i]);

	if (pCmdLine->next)
		freeCmdLines(pCmdLine->next);

	FREE(pCmdLine);
}
