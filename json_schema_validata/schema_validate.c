#include <unistd.h>
#include <jansson.h>
#include <time.h>
#include <wjelement.h>

struct RSValidateMsg {
  char *msg;
   int len;
};
#define MAX_ONE_MSG_SIZE 100
static int stringEscape(char *string, char *escape, int size)
{   
    char *str = string;
    int i = 0;
    int j = 0;
    char c1 = 0;
    char c2 = 0;

    if (!string || !string[0] || !escape)
    {
        return FALSE;
    }

    memset(escape, 0, size);

    c1 = str[i];
    if (c1 == '"')
    {
        escape[j++] = '\\';
    }

    i++; 
    c2 = str[i];

    while (c2 != '\0' && j < size - 1)
    {
        escape[j++] = c1;

        if (c2 == '"' && c1 != '\\')
        {
            escape[j++] = '\\';
        }
        else if (c1 == '\\' && 
                (c2 != 'n' || c2 != 't' || c2 != 'b' || c2 != 'r' || c2 != 'f' 
                 || c2 != '\\' || c2 != '\'' || c2 != '"' || c2 != 'a'))
        {
            escape[j++] = '\\';
        }

        c1 = c2;
        i++; 
        c2 = str[i];
    }

    if (c1 != '\0' && c2 == '\0' && j < size - 1)
    {
        escape[j] = c1;
    }

    return TRUE;
}
static void schemaError(void *client, const char *format, ...)
{
    struct RSValidateMsg *msg = client;
    char *errmsg = msg ? msg->msg : NULL;
    int size = msg ? msg->len - 1 : 0;
    va_list ap; 
    char tmpStr[MAX_ONE_MSG_SIZE] = {0};
    char escape[MAX_ONE_MSG_SIZE * 2] = {0};

    if (!errmsg || strlen(errmsg) >= size)
    {   
        return;
    }   

    va_start(ap, format);
    vsnprintf(tmpStr, MAX_ONE_MSG_SIZE, format, ap);
    va_end(ap);
    tmpStr[strlen(tmpStr)] = ' ';

    if (stringEscape(tmpStr, escape, MAX_ONE_MSG_SIZE * 2)) 
    {   
        snprintf(errmsg + strlen(errmsg), size - strlen(errmsg), 
                "%s", escape);
    }   
}

int jsonCfgSchemaValidateWithDefaultOnErr(const char *schema, const char *content,
        char *errmsg, int msglen, int defaultOnErr, char **newContent)
{
    XplBool isValid = FALSE;
    WJElement schemaE = WJEParse(schema);
    WJElement contentE = WJEParse(content);
    struct RSValidateMsg msg = {msg: errmsg, len: msglen};

    if (errmsg && msglen > 0) {
        msg.msg[0] = '\0';
    }   

//printf("%s %d: time = %d\n", __func__, __LINE__, (int)time(NULL));
    if (!defaultOnErr) {
//printf("%s %d: time = %d\n", __func__, __LINE__, (int)time(NULL));
        isValid = WJESchemaValidate(schemaE, contentE, schemaError, NULL, NULL, &msg);
//printf("%s %d: time = %d\n", __func__, __LINE__, (int)time(NULL));
    } else {
        isValid = WJESchemaValidateWithDefaultOnErr(schemaE, contentE, schemaError, NULL, NULL, &msg);
    }   

//printf("%s %d: time = %d\n", __func__, __LINE__, (int)time(NULL));
    if (isValid && newContent) {
        *newContent = WJEWriteMEM(contentE, 0, 0); 
    }   

//printf("%s %d: time = %d\n", __func__, __LINE__, (int)time(NULL));
#if FCGI_APP_DEBUG
    WJEDump(schemaE);
    WJEDump(contentE);
#endif /* FCGI_APP_DEBUG */

    WJECloseDocument(schemaE);
    WJECloseDocument(contentE);

//printf("Validation: %s\n", isValid ? "PASS" : "FAIL");

    return isValid ? 1 : 0;
}

static int jsonCfgValidateSetDefaultOnErr(const json_t *schema, const json_t *cfg,
        char *errmsg, int msglen, int defaultOnErr, json_t **newCfg)
{
    int isValid = 0;
    char *nCfg = NULL;
    char *s = json_dumps(schema, 0);
    char *c = json_dumps(cfg, 0);
//printf("%s %d: time = %d\n", __func__, __LINE__, (int)time(NULL));
    isValid = jsonCfgSchemaValidateWithDefaultOnErr(s, c, errmsg, msglen, defaultOnErr, &nCfg);
//printf(">>%s %d: time = %d\n", __func__, __LINE__, (int)time(NULL));
    if (isValid && nCfg && newCfg)
    {
        *newCfg = json_loads(nCfg, 0, 0);
        free(nCfg);
    }
//printf("%s %d: time = %d\n", __func__, __LINE__, (int)time(NULL));
    free(s);
    free(c);
    return isValid;
}

int main()
{
    json_t *newCfg = NULL;
    json_t *schema = json_load_file("schema.txt", 0, NULL);
    json_t *cfg = json_load_file("cfg.txt",0, NULL);
    while (1){
    jsonCfgValidateSetDefaultOnErr(schema, cfg, NULL, 0, 0, &newCfg);
    }
    return 0;
}

