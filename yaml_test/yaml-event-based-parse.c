#include <stdio.h>
#include <yaml.h>

static puts_tab(int tab_cnt)
{
	int i = 0;
	
	for (i = 0; i < tab_cnt; i++)
		printf("  ");
}

int main(void)
{
	int tab_cnt = 0, i;
  FILE *fh = fopen("test.yaml", "r");
  yaml_parser_t parser;
  yaml_event_t  event;   /* New variable */

  /* Initialize parser */
  if(!yaml_parser_initialize(&parser))
    fputs("Failed to initialize parser!\n", stderr);
  if(fh == NULL)
    fputs("Failed to open file!\n", stderr);

  /* Set input file */
  yaml_parser_set_input_file(&parser, fh);

  /* START new code */
  do {
    if (!yaml_parser_parse(&parser, &event)) {
       printf("Parser error %d\n", parser.error);
       exit(EXIT_FAILURE);
    }

    switch(event.type)
    { 
    case YAML_NO_EVENT: puts("No event!"); break;
    /* Stream start/end */
    case YAML_STREAM_START_EVENT: 
    	puts("STREAM START"); 
    	tab_cnt++;
    	break;
    case YAML_STREAM_END_EVENT:   
    	puts("STREAM END");   
    	tab_cnt--;
    	break;
    /* Block delimeters */
    case YAML_DOCUMENT_START_EVENT: 
    	puts_tab(tab_cnt);
    	tab_cnt++;
    	puts("Start Document"); 
    	break;
    case YAML_DOCUMENT_END_EVENT:   
    	tab_cnt--;    	    	
    	puts_tab(tab_cnt);
    	puts("End Document");   
    	break;
    case YAML_SEQUENCE_START_EVENT: 
        puts_tab(tab_cnt);
    	tab_cnt++;
    	puts("Start Sequence"); 
    	break;
    case YAML_SEQUENCE_END_EVENT:   
    	tab_cnt--;
    	puts_tab(tab_cnt);
    	puts("End Sequence");   
    	break;
    case YAML_MAPPING_START_EVENT:  
    	puts_tab(tab_cnt);
    	tab_cnt++;
    	printf("Start Mapping");  //(%s)", event.data.mapping_start.anchor
    	break;
    case YAML_MAPPING_END_EVENT:    
    	tab_cnt--;
    	puts_tab(tab_cnt);
    	puts("End Mapping");    
    	break;
    /* Data */
    case YAML_ALIAS_EVENT: 
    	puts_tab(tab_cnt); 
    	printf("Got alias (anchor %s)\n", event.data.alias.anchor); 
    	break;
    case YAML_SCALAR_EVENT: 
    	puts_tab(tab_cnt);
    	printf("Got scalar (value %s) \n", event.data.scalar.value); 
    	break;
    }
    
    if(event.type != YAML_STREAM_END_EVENT)
      yaml_event_delete(&event);
  } while(event.type != YAML_STREAM_END_EVENT);
  yaml_event_delete(&event);
  /* END new code */

  /* Cleanup */
  yaml_parser_delete(&parser);
  fclose(fh);
  return 0;
}
