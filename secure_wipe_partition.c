/*
 * Copyright (c) 2020 Intel Corporation.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>

#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <inttypes.h>


int secure_wipe_partition(char* partition, uint64_t start, uint64_t end)
{
	if(partition == NULL)
		return false;
	int fd = open(partition, O_RDWR);
	if( fd == -1) {
		fprintf(stderr, "Cannot open the block device\n");
		return false;
	}
	uint64_t partition_range[2] = {0, 0};
	partition_range[0] = start;
	partition_range[1] = end-start;
	fprintf(stderr, "ranage is from %" PRIu64 " to " "%" PRIu64 "\n", partition_range[0], partition_range[1]);
	if(ioctl(fd, BLKSECDISCARD, &partition_range) == -1) { 
		unsigned int zeroes;
		if (ioctl(fd, BLKDISCARDZEROES, &zeroes) == 0 && zeroes != 0) {
			if (ioctl(fd, BLKDISCARD, &partition_range) == -1) {
				fprintf(stderr, "BLKDISCARD failed\n");
				close(fd);
				return false;
			}
		}
		else {
			if (ioctl(fd, BLKZEROOUT, &partition_range) == -1) {
				fprintf(stderr, "BLKDZEROOUT failed\n");
				close(fd);
				return false;
			}
		}
	}
	close(fd);
	return true;
}

