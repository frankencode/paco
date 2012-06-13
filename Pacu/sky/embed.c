#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	FILE* file = 0;
	int k = 0, nc = 16, i = 0, ch = 0;
	char* name = 0;
	
	if (argc != 2) {
		printf("usage: embed <file>\n");
		return 1;
	}
	
	name = argv[1];
	while (name[k] && (name[k] != '.'))
		++k;
	name = malloc(k + 1);
	memcpy(name, argv[1], k);
	name[k] = 0;
	
	file = fopen(argv[1], "r");
	if (!file) {
		int ret = errno;
		perror("embed");
		return ret;
	}
	
	printf("static unsigned char %s[] = {\n", name);
	
	while ((ch = fgetc(file)) != EOF) {
		if (i != 0) {
			putchar(',');
			if (i % nc == 0)
				putchar('\n');
		}
		printf("0x%02X", ch);
		++i;
	}
	
	printf(
		"\n"
		"};\n"
		"static int %sSize = %d;\n", name, i
	);
	
	fclose(file);
	
	return 0;
}
