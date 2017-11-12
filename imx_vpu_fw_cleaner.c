/*
 * Copyright (c) 2017 Martin Kepplinger <martink@posteo.de>
 */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <asm/types.h>
#include <limits.h>

static const char defpath[] = "upstream/vpu_fw_imx6q.bin";
static const char defoutpath[] = "upstream/vpu_fw_imx6q_clean.bin";

struct cleaner {
	int	verbose;
};

static int imx6_read(struct cleaner *cleaner, char *path,
		     uint8_t **buf, uint32_t *len)
{
	FILE *fp;
	struct stat st; 
	int ret;
	int i;

	fp = fopen(path, "rb");
	if (!fp) {
		perror("fopen");
		return errno;
	}

	if (stat(path, &st) != 0) {
		fprintf(stderr, "Cannot determine size of %s: %s\n",
		path, strerror(errno));
		return errno; 
	}

	*len = st.st_size;

	if (cleaner->verbose)
		printf("file: %s size: %d\n", path, *len);

	*buf = (uint8_t *)malloc(st.st_size);
	if (*buf == NULL)
		return errno;

	for (i = 0; i < st.st_size; i++) {
		ret = fread(((uint8_t *)(*buf + i)), 1, 1, fp);
		if (!ret) {
			if (feof(fp)) {
				break;
			} else {
				perror("fread");
				fclose(fp);
				return errno;
			}
		}
	}

	fclose(fp);

	return 0;
}

int main (int argc, char **argv)
{
	struct cleaner *cleaner;
	char *path;
	char *outpath;
	int ret;
	uint8_t *buf = NULL;
	uint32_t len;
	int opt;

	cleaner = calloc(1, sizeof(struct cleaner));
	if (!cleaner)
		return -ENOMEM;

	path = strdup(defpath);
	if (!path) {
		perror("strdup");
		return errno;
	}

	outpath = strdup(defoutpath);
	if (!path) {
		perror("strdup");
		return errno;
	}

	while ((opt = getopt(argc, argv, "hi:o:v")) != -1) {
		switch(opt) {
		case 'i':
			free(path);
			path = strdup(optarg);
			if (!path) {
				perror("strdup");
				return errno;
			}
			break;
		case 'o':
			free(outpath);
			outpath = strdup(optarg);
			if (!outpath) {
				perror("strdup");
				return errno;
			}
			break;
		case 'v':
			cleaner->verbose = 1;
			break;
		case 'h':
		default:
			fprintf(stderr, "Usage: %s [-i input-file]"
				"[-o output-file] [-v]\n",
				argv[0]);
			return -EINVAL;
		}
	}

	ret = imx6_read(cleaner, path, &buf, &len);
	if (ret)
		return ret;

	if (!buf)
		return -ENOMEM;

	/* TODO clean buf of size len */

	free(buf);
	free(path);
	free(outpath);
	free(cleaner);

	return 0;
}
