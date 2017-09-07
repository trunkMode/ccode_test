#include <unistd.h>
#include <stdio.h>

int main()
{
	struct {
		int a;	    //4
		char b;	    //1 + 3
		double c;   //8
		short d;    //2 + 2
		float e;    //4 
		int f;	    //4
		char g;     //1
	}A;

	struct {
		char a;     //1 + 1
		short b;    // 2
		char c;     //1 + 1
	}B;
	char array[] = {1,2,3,4,5};
	char *p = array;
	printf("sizeof(A.a) = %d\n", sizeof(A.a));
	printf("sizeof(A.b) = %d\n", sizeof(A.b));
	printf("sizeof(A.c) = %d\n", sizeof(A.c));
	printf("sizeof(A.d) = %d\n", sizeof(A.d));
	printf("sizeof(A.e) = %d\n", sizeof(A.e));
	printf("sizeof(A.f) = %d\n", sizeof(A.f));
	printf("sizeof(A) = %d\n", sizeof(A));
	
	printf("sizeof(B) = %d\n", sizeof(B));
	printf("*p++ = %d\n", *p++);  //1, p=p+1
	printf("*++p = %d\n", *++p); //3
	printf("*(p++) = %d\n", *(p++));//3, p=p+1	
	printf("(*p)++ = %d\n", (*p)++);//4
	printf ("*p = %d\n", *p); //5

	int range = atoi("123");
	printf("range = %d\n", range);	
	range = atoi(NULL);
	printf("range = %d\n", range);
	/*hgr update */	
}
