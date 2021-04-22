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
	int tab_cnt = 0;
  FILE *fh = fopen("suricata.yaml", "r");
  yaml_parser_t parser;
  yaml_token_t  token;   /* new variable */

  /* Initialize parser */
  if(!yaml_parser_initialize(&parser))
    fputs("Failed to initialize parser!\n", stderr);
  if(fh == NULL)
    fputs("Failed to open file!\n", stderr);

  /* Set input file */
  yaml_parser_set_input_file(&parser, fh);

  /* BEGIN new code */
  do {
    yaml_parser_scan(&parser, &token);
    switch(token.type)
    {
    /* Stream start/end */
    case YAML_STREAM_START_TOKEN: 
    	tab_cnt++;
    	puts("STREAM START"); 
    	break;
    case YAML_STREAM_END_TOKEN:   
    	tab_cnt--;
    	puts("STREAM END");   
    	break;
    /* Token types (read before actual token) */
    case YAML_KEY_TOKEN:   
    	puts_tab(tab_cnt);
    	printf("(Key token)   "); 
    	break;
    case YAML_VALUE_TOKEN: 
    	printf("(Value token) "); 
    	break;
    /* Block delimeters */
    case YAML_BLOCK_SEQUENCE_START_TOKEN: 
    	puts_tab(tab_cnt);
    	tab_cnt++;
    	puts("Start Block (Sequence)"); 
    	break;
    case YAML_BLOCK_ENTRY_TOKEN:          
    	puts_tab(tab_cnt);
    	tab_cnt++;
    	puts("Start Block (Entry)");    
    	break;
    case YAML_BLOCK_END_TOKEN:            
    	puts_tab(tab_cnt);
    	tab_cnt--;
    	puts("End block");              
    	break;
    /* Data */
    case YAML_BLOCK_MAPPING_START_TOKEN:  
    	puts_tab(tab_cnt);
    	tab_cnt++;
    	puts("[Block mapping]");            
    	break;
    case YAML_SCALAR_TOKEN:  
    	puts_tab(tab_cnt);
    	printf("scalar %s \n", token.data.scalar.value); break;
    /* Others */
    default:
      puts_tab(tab_cnt);
      printf("Got token of type %d\n", token.type);
    }
    if(token.type != YAML_STREAM_END_TOKEN)
      yaml_token_delete(&token);
  } while(token.type != YAML_STREAM_END_TOKEN);
  yaml_token_delete(&token);
  /* END new code */

  /* Cleanup */
  yaml_parser_delete(&parser);
  fclose(fh);
  return 0;
}
