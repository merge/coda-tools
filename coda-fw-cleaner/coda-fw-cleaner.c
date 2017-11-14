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

#define CODA_FW_SIZE		0x3e000
#define CODA_FW_START_DEC_VC1	0x0f000

static const char defpath[] = "vpu_fw_imx6q.bin";
static const char defoutpath[] = "vpu_fw_imx6q_clean.bin";

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
		fprintf(stderr, "opening %s failed.\n", path);
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
	FILE *write_ptr;

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

	if (len != CODA_FW_SIZE) {
		fprintf(stderr, "ERROR: unsupported firmware file\n");
		free(buf);
		free(path);
		free(outpath);
		free(cleaner);
		return -EINVAL;
	}

	if (cleaner->verbose)
		printf("remove everything except h264 decoding\n");
	/* remove everything after decode_vc1 starts */
	memset(buf + CODA_FW_START_DEC_VC1, 0, CODA_FW_SIZE - CODA_FW_START_DEC_VC1);

	write_ptr = fopen(outpath,"wb");
	if (!write_ptr) {
		perror("fopen");
		return errno;
	}

	ret = fwrite(buf, len, 1, write_ptr);
	if (ret != 1)
		fprintf(stderr, "ERROR: %d bytes written\n", ret);

	if (cleaner->verbose)
		printf("%s written\n", outpath);

	fclose(write_ptr);
	free(buf);
	free(path);
	free(outpath);
	free(cleaner);

	return 0;
}
