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

#define IQ_DEBUG		0

static const char defpath[] = "iq-data.bin";
static const char deffile_t[] = "iq_t.svg";
static const char deffile_f[] = "iq_f.svg";
static const char deffile_ph[] = "iq_ph.svg";

struct frame {
	uint8_t chan_id;
	uint8_t ignore;
	uint16_t zero;
	uint32_t timestamp;
	uint8_t *payload;
} __attribute__((packed));

struct chan_desc {
	uint8_t chan_id;
	uint8_t ext_header;
	uint8_t payload_size;
};

struct iq_file {
	uint8_t num_channels;
	struct chan_desc *desc;
	struct frame *frame;
};

struct iq_result {
	int16_t i;
	int16_t q;
	uint32_t ts;

	struct iq_result *next;
};

static void iq_cleanup(struct iq_file *iq_file)
{

	if (iq_file->num_channels == 0)
		return;

	if (iq_file->desc)
		free(iq_file->desc);
}

static void iq_cleanup_list(struct iq_result **result_head)
{
	struct iq_result *cursor;

	while (*result_head) {
		cursor = *result_head;
		*result_head = cursor->next;
		free(cursor);
	}
}

static int iq_read(char *path, uint8_t **buf, uint32_t *len)
{
	FILE *fp;
	struct stat st; 
	int ret;
	int i;

	fp = fopen(path, "rb");
	if (!fp) {
		printf("Usage: iq [path_to_iq_file] [channel]\n");
		perror("fopen");
		return errno;
	}

	if (stat(path, &st) != 0) {
		fprintf(stderr, "Cannot determine size of %s: %s\n",
		path, strerror(errno));
		return errno; 
	}

	if (st.st_size < 562) {
		printf("file is too small with %ld bytes\n", st.st_size);
		return -EINVAL;
	}

	*len = st.st_size;

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
	char path[LINE_MAX];
	int ret;
	struct iq_file iq_file;
	uint8_t *buf = NULL;
	uint32_t len;
	uint8_t channel = 0;
	struct iq_result *result_head;
#if IQ_DEBUG
	struct iq_result *cursor;
#endif
	int opt;
	char filename_t[LINE_MAX];
	char filename_f[LINE_MAX];
	char filename_ph[LINE_MAX];

	strcpy(path, defpath);
	strcpy(filename_t, deffile_t);
	strcpy(filename_f, deffile_f);
	strcpy(filename_ph, deffile_ph);

	while ((opt = getopt(argc, argv, "hi:c:t:f:")) != -1) {
		switch(opt) {
		case 'i':
			if (strlen(optarg) >= LINE_MAX) {
				printf("Path to file too long\n");
				return -EINVAL;
			}
			strcpy(path, optarg);
			break;
		case 'c':
			if (strlen(optarg) > 2)
				return -EINVAL;

			channel = atoi(optarg);
			break;
		case 't':
			if (strlen(optarg) >= LINE_MAX)
				return -EINVAL;

			strcpy(filename_t, optarg);
			break;
		case 'f':
			if (strlen(optarg) >= LINE_MAX)
				return -EINVAL;

			strcpy(filename_f, optarg);
			break;
		case 'p':
			if (strlen(optarg) >= LINE_MAX)
				return -EINVAL;

			strcpy(filename_ph, optarg);
			break;
		case 'h':
		default:
			fprintf(stderr, "Usage: %s [-i iq-input-file] "
				"[-c channel] [-t time-output-file] "
				"[-f freq-output-file] [-p phase-output-file]\n",
				argv[0]);
			return -EINVAL;
		}
	}

	ret = iq_read(path, &buf, &len);
	if (ret)
		return ret;

	if (!buf)
		return -ENOMEM;

	iq_file.num_channels = 0;

	result_head = malloc(sizeof(struct iq_result));
	if (!result_head)
		return errno;

	result_head->i = 0;
	result_head->q = 0;
	result_head->ts = 0;
	result_head->next = NULL;

	free(buf);

	iq_cleanup_list(&result_head);
	iq_cleanup(&iq_file);

	return 0;
}
