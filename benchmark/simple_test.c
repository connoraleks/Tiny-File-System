#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>

/* You need to change this macro to your TFS mount point*/
#define TESTDIR "/tmp/cja142/mountdir"

#define N_FILES 100
#define BLOCKSIZE 4096
#define FSPATHLEN 256
#define ITERS 16
#define FILEPERM 0666
#define DIRPERM 0755

char buf[BLOCKSIZE];

int main(int argc, char **argv) {
	static struct timeval tm1;
	gettimeofday(&tm1, NULL);
	int i, fd = 0, ret = 0;
	struct stat st;
	printf("This is TESTDIR: %s\n", TESTDIR);
	if ((fd = creat(TESTDIR "/file", FILEPERM)) < 0) {
		printf("This is errno: %d\n", errno);
		perror("creat");
		printf("TEST 1: File create failure %d \n", fd);
		exit(1);
	}
	printf("TEST 1: File create Success \n");


	/* Perform sequential writes*/ 
	for (i = 0; i < ITERS; i++) {
		//memset with some random data
		memset(buf, 0x61 + i, BLOCKSIZE);
		int j;
		if ((j=write(fd, buf, BLOCKSIZE)) != BLOCKSIZE) {
			printf("TEST 2: File write failure %d bytes\n", j);
			exit(1);
		}
	}
	
	fstat(fd, &st);
	if (st.st_size != ITERS*BLOCKSIZE) {
		printf("TEST 2: File write failure %d stsize != %d\n", st.st_size, ITERS*BLOCKSIZE);
		exit(1);
	}
	printf("TEST 2: File write Success \n");


	/*Close operation*/	
	if (close(fd) < 0) {
		printf("TEST 3: File close failure \n");
	}
	printf("TEST 3: File close Success \n");


	/* Open for reading */
	if ((fd = open(TESTDIR "/file", FILEPERM)) < 0) {
		perror("open");
		exit(1);
	}


	/* Perform sequential reading */
	for (i = 0; i < ITERS; i++) {
		//clear buffer
		memset(buf, 0, BLOCKSIZE);
		int j;
		if ((j = read(fd, buf, BLOCKSIZE)) != BLOCKSIZE) {
			printf("TEST 4: File read failure %d vs %d\n",j, BLOCKSIZE );
			exit(1);
		}
		//printf("buf %s \n", buf);
	}
	int j; 
	if ((j = pread(fd, buf, BLOCKSIZE, 2*BLOCKSIZE)) != BLOCKSIZE) {
		perror("pread");
		printf("TEST 4: File read failure %d vs %d\n", j, BLOCKSIZE);
		exit(1);
	}
    
	printf("TEST 4: File read Success \n");
	close(fd);


	/* Unlink the file */
	if ((ret = unlink(TESTDIR "/file")) < 0) {
		perror("unlink");
		printf("TEST 5: File unlink failure %d\n", ret);
		exit(1);
	}
	printf("TEST 5: File unlink success \n");


	/* Directory creation test */
	if ((ret = mkdir(TESTDIR "/files", DIRPERM)) < 0) {
		perror("mkdir");
		printf("TEST 6: failure. Check if dir %s already exists, and "
			"if it exists, manually remove and re-run \n", TESTDIR "/files");
		exit(1);
	}
	printf("TEST 6: Directory create success \n");

	
	/* Sub-directory creation test */
	for (i = 0; i < N_FILES; ++i) {
		char subdir_path[FSPATHLEN];
		memset(subdir_path, 0, FSPATHLEN);

		sprintf(subdir_path, "%s%d", TESTDIR "/files/dir", i);
		if ((ret = mkdir(subdir_path, DIRPERM)) < 0) {
			perror("mkdir");
			printf("TEST 7: Sub-directory create failure \n");
			exit(1);
		}
	}
	
	for (i = 0; i < N_FILES; ++i) {
		DIR *dir;
		char subdir_path[FSPATHLEN];
		memset(subdir_path, 0, FSPATHLEN);

		sprintf(subdir_path, "%s%d", TESTDIR "/files/dir", i);
		if ((dir = opendir(subdir_path)) == NULL) {
			perror("opendir");
			printf("TEST 7: Sub-directory create failure \n");
			exit(1);
		}
	}
	printf("TEST 7: Sub-directory create success \n");
	struct timeval tm2;
	gettimeofday(&tm2, NULL);
	unsigned long long t = (1000 * (tm2.tv_sec - tm1.tv_sec)) + ((tm2.tv_usec - tm2.tv_usec)/1000);
	printf("Benchmark completed in %llu ms\n", t);
	return 0;
}
