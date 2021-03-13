#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

long file_size(const char *path, int v)
{
	struct stat stat_buf;
	int stat_ret = stat(path, &stat_buf);

	if(stat_ret != 0) {
		if(errno == ENOENT) {
			if(v) { fprintf(stderr, "%s does not exist!\n", path); }
			return -1;
		} else {
			if(v) { fprintf(stderr, "stat(%s) failed! %s\n", path, strerror(errno)); }
			return -2;
		}
	}

	if(!S_ISREG(stat_buf.st_mode)) {
		if(v) { fprintf(stderr, "%s is not a regular file!\n", path); }
		return -3;
	}

	return (long)stat_buf.st_size;
}

char* get_file(const char *path)
{
	FILE *f;
	long size;
	size_t z;
	char *data;

	size = file_size(path, 1);
	if(size <= 0) { return NULL; }

	f = fopen(path, "r");
	if(!f) {
		fprintf(stderr, "fopen(%s, r) failed! %s\n", path, strerror(errno));
		return NULL;
	}

	data = malloc(size+1);
	z = fread(data, size, 1, f);
	if (z != 1) {
		fprintf(stderr, "fread(data, %ld, 1, %s) failed!\n", size, path);
		free(data);
		return NULL;
	}

	fclose(f);
	data[size] = 0;
	return data;
}
