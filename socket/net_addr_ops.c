#include <unistd.h>
#include <stdio.h>


int get_netmask_prefix_len(unsigned int mask)
{
	int prefix = 0, max_loop = sizeof(mask)*8 + 1;
	while (mask ) {
		if (mask & 1)
			prefix++;
		mask >>= 1;
		printf("mask = 0x%x\n", mask);
	}
	return prefix;
}

int main()
{

	int prefix = get_netmask_prefix_len(0xffffff00);
	printf("prefix = %d\n", prefix);
	return 0;
}
