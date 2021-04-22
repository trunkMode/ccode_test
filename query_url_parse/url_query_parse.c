#include <unistd.h>

int get_query_value(const char *query)
{
    int left_len, len;
    const char *p = query;
    char buf[2][64];
//    json_t *obj = json_object();
//    json_t *json_val = NULL;

    if (query == NULL) return NULL;

    left_len = strlen(query);
    memset(buf, 0x00, sizeof(buf));

    while (sscanf(p, "%[^=]=%[^&]", buf[0], buf[1]) == 2 && left_len >= 3) {
	if (*p == '&') {
		left_len--;	
		p++;
		continue;
	}
	if (sscanf(p, "%[^=]=%[^&]", buf[0], buf[1]) == 2) {
        	len = strlen(buf[0]) + strlen(buf[1]) + 2;
        	left_len -= len;
        	p += len;
		printf("buf[0] = %s\n", buf[0]);
        //	json_val = json_string(buf[1]);
	} else {
		p++;
		left_len--;
	}
		
//        json_object_set_new(obj, buf[0], json_val);
//		memset(buf, 0x00, sizeof(buf));
    }
    return 0;
}

int main()
{
	get_query_value("staMac=11:22:33:44:55:66&&timeRange=11&test=1&&test_str=hello");
	return 0;
}
