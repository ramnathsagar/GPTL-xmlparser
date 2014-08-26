/*
** Author: Ramnath Sai (ramnath.sagar@gmail.com)
**
** XML parser to parse the xml config file and set the profiling parameters
*/



#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>

#ifdef HAVE_LIBXML2
#include "setxmlConfig.h"



static void setDefault()
{
	int ret;
	ret = GPTLsetoption(GPTLstacksize, 128);
	ret = GPTLsetoption(GPTLoverhead, 1);
	ret = GPTLsetutr(1); /*Use gettimeofday */
}

static inline void setMPIOptions(xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *value;
	//If MPI is enabled
	if (!strcmp(xmlGetProp(cur, "enabled"),"yes"))
		printf("MPI is enabled\n");//value = xmlGetProp(cur, "enabled"); 

}

static void setOpenMPOptions(xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *value;
	//Check if OpenMP is enabled
	if (!strcmp(xmlGetProp(cur, "enabled"), "yes"))
		printf("OpenMP is enabled\n");

}


static void setPthreadOptions(xmlDocPtr doc , xmlNodePtr cur )
{
	xmlChar *value;
	//Check if Pthreads is enabled
	if (!strcmp(xmlGetProp(cur, "enabled"), "yes"))
		printf("Pthreads is enabled\n");

}

/*
Function to set the call stack depth using GPTLsetoption
Input:
	xmlNodePtr cur which points to the current Node in the xml tree
*/

static void setCallStackDepth(xmlNodePtr cur)
{
	xmlChar *strvalue;
	int value;
	
	if (!strcmp(xmlGetProp(cur, "enabled"),"yes"))
	{
		strvalue = xmlGetProp(cur, "depth");
		value = atoi(strvalue+strlen(strvalue)-1);
		if (GPTLsetoption(GPTLstacksize,value))
			printf("Warning: GPTLstacksize could not be set\n");
	}
}

/*
Function to find which timer to use and set the GPTL timer
Input :
	xmlNodePtr cur which points to the current Node in the xml tree 
*/

static void setTimer(xmlNodePtr cur)
{
	xmlChar *value;
	int ret, timer;
	value = xmlGetProp(cur, "option");
	
	timer = str2time (value);
	//If Timer exits use it , else use the default - gettimeofday
	ret = (timer > 0 ) ? timer : 1;
	//printf("ret valu=%d\n",ret);
	
	GPTLsetutr(ret);

}

/*
Function which is used to convert the "String counter" to an "Int counter". 
Passes the String name to str2counter and gets the "number" as defined in papi.h

Input:
	A String of type xmlChar *
	Two indices which is used to keep track of the position in the String - cur_pos, prev_pos
*/

static int findCounterfromString(xmlChar *Str, int cur_pos, int prev_pos)
{
	char *counter;
	int counter_number;
	counter = (char*)malloc((cur_pos-prev_pos+1)*sizeof(char));

	strncpy(counter, Str+prev_pos,(cur_pos-prev_pos));
	counter[cur_pos-prev_pos]='\0';
	
	counter_number = str2counter(counter);
	
	if(GPTLsetoption(counter_number,1))
		printf("Warning: PAPI counter %s could not be set\n",counter);

	free(counter);

	return (cur_pos + 1);
}

/*
Function to set the PAPI counters in GPTL
Input:
	xmlDocPtr
	xmlNodePtr 

*/

static void setPAPIcounters(xmlDocPtr doc, xmlNodePtr cur )
{
	xmlChar *value;
	char *counter_str;
	int i;
	int cur_ptr = 0, prev_ptr = 0;
	if(! strcmp( xmlGetProp(cur, "enabled"),"yes"))
	{
		value = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		for(i = 0;value[i] != '\0'; i++)
		{
			cur_ptr = i;
			if(value[i] == ',')
				//Pass the curent position, previous position and the string
				prev_ptr = findCounterfromString(value, cur_ptr, prev_ptr);
		}
		//Last substring..
		prev_ptr = findCounterfromString(value, cur_ptr+1, prev_ptr);
	}
}


/*
	xmlparser is a function which will parse the input xml file
	and set the GPTLoptions and GPTLsetur. One can specify the 
	events in the xml file.

	xml file has to be exported to GPTL_CONFIG_FILE environment variable
	
*/

int GPTLxmlparser(void)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	
	FILE *in;
	char *buffer;
	long numBytes;
	
	char const *gptl_config_filename = getenv("GPTL_CONFIG_FILE");

	if (gptl_config_filename == NULL)
	{
		printf("Warning.. GPTL config file not specified. Use GPTL_CONFIG_FILE env variable\n");
		printf("Using default options\n");
		setDefault();
		return 1;
	}

	if ( (in = fopen(gptl_config_filename, "r")) == NULL)
	{
		printf("Warning:.. Config file %s not found.\n",gptl_config_filename);
		printf("Using default options\n");
		setDefault();
		return 1;
	}

	//Read the xml file into memory
	fseek(in, 0L, SEEK_END);
	numBytes = ftell(in);

	fseek(in, 0L, SEEK_SET);

	buffer = (char *)calloc(numBytes, sizeof(char));

	if(buffer == NULL)
		return 1;

	fread(buffer, sizeof(char), numBytes, in);

	fclose(in);

	//Now parse the Memory which has the xml tags
	if( (doc = xmlParseMemory(buffer, numBytes)) == NULL)
	{
		fprintf(stderr, "Cannot read the XML config parameters..\n Will abort now\n");
		exit(0);
	}
	
	if ( (cur = xmlDocGetRootElement(doc)) == NULL)
	{
		fprintf(stderr, "XML Config File is empty\n.. Will abort now");
		xmlFreeDoc(doc);
		exit(0);
	}

	cur = cur->xmlChildrenNode;
	while(cur != NULL)
	{
		//printf("The 1st value is %s\n",cur->name);
		if((!xmlStrcmp(cur->name, (const xmlChar *)"mpi")))
			setMPIOptions(doc, cur);

		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"openmp")))
			setOpenMPOptions(doc, cur);
		
		else if((!xmlStrcmp(cur->name, (const xmlChar *)"pthreads")))
			setPthreadOptions(doc, cur);

		else if((!xmlStrcmp(cur->name, (const xmlChar *)"callers")))
			setCallStackDepth(cur);

		else if((!xmlStrcmp(cur->name, (const xmlChar *)"PAPI")))
			setPAPIcounters(doc, cur);
	
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timer")))
			setTimer(cur);
		else if((!xmlStrcmp(cur->name, (const xmlChar *)"overhead")))
			GPTLsetoption(GPTLoverhead, 1);//parseEstats(doc, cur);
				
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"text")) || (!xmlStrcmp(cur->name, (const xmlChar *)"comment")))
		{	/*Do nothing;*/}
		else
			printf("Unrecognized option in XML file %s\n",cur->name);
		
		cur = cur->next;
	}

	xmlFreeDoc(doc);
	free(buffer);
	
	return 0;
		
}
#endif
