# Tiny-File-System
Virtual file management system

Connor Aleksandrowicz
Ryan Berardi

## Tfs_init:

Tfs_init begins by calling dev_open() on diskfile_path.If the return value is -1, we call tfs_mkfs.
Otherwise we malloc space for the inode bitmap, datablock bitmap, and the superblock, and then
read the superblock from disk.

## Tfs_destroy:

Tfs_destroy consists of four simple lines. We freethe inode bitmap, data block bitmap, and
superblock, and then call dev_close().

## Tfs_getattr:

Tfs_getattr begins by locking our global mutex lock.It then attempts to get the target inode using
the path provided and the function get_node_by_path().If the return value of this function is -1,
then we know that the target does not exist, so weunlock the mutex and return -ENOENT to
signify that no target was found from the given path.Otherwise, we continue with the given
target retrieved, and begin extracting data from theinode to fill into struct stat* stbuf. Once all
required values are set/transferred we unlock themutex and return 0 to signify completion.

## Tfs_opendir:

Tfs_opendir begins by locking our global mutex lock.It then attempts to get the target inode
using the path provided and the function get_node_by_path().If the inode retrieved is not valid,
we unlock the mutex and return -1, otherwise 0 isreturned.

## Tfs_readdir:

Tfs_readdir begins by locking our global mutex lock.It then attempts to get the target inode
using the path provided and the function get_node_by_path().If the inode retrieved is not valid,
we unlock the mutex and exit. Otherwise, we browsethe directory entries of this target, and add
them to the buffer using the function filler(). Uponcompletion we unlock the mutex and return 0.

## Tfs_mkdir:

Tfs_mkdir begins by locking our global mutex lock.It then creates two copies of path **_(This is a
workaround to an issue that presented itself. Whencalling basename and dirname on the
same string, the returned strings would be warbled)_** and uses them to get basename and
dirname. Get_node_by_path() is then called to getthe parent directory, if the returned value is -
we unlock the mutex and exit. Otherwise, we get thenext available inode, and call dir_add() to
add a new directory into the parent directory. Wethen free malloc’d variables, unlock the mutex,
and return 0.

## Tfs_rmdir:

Tfs_rmdir begins by locking our global mutex lock.It then creates two copies of path **_(Same
issue mentioned above)_** and gets basename and dirnamefrom them. We then attempt to get the
target directory, if it does not exist we free, unlock,and exit. If it does exist, we run through its
direct_ptr’s to find out if it is empty or not, ifnot we return with an error stating that you are
attempting to remove a non-empty directory. If thedirectory is empty, we read the bitmaps from
disk, make the necessary modifications, and writethem back to disk. We then get the inode of
the parent directory and call dir_remove(). Finally,we free, unlock, and return 0


## Tfs_create:

Tfs_create begins by locking our global mutex lock.It then creates two copies of path **_(Same
issue mentioned above)_** and gets basename and dirnamefrom them. We then attempt to get the
parent directory, if it does not exist we unlock andexit. Otherwise, we get the next available
inode and call dir_add(). We then write the properinode information to disk and set the
direct_ptr array to 0. We then free, unlock, and return0.

## Tfs_open:

Tfs_open begins by locking our global mutex lock.It then attempts to get the inode via
get_node_by_path(). If the inode exists, we unlockand return 0. Otherwise, we unlock and return
-1.

## Tfs_read:

Tfs_read begins by locking our global mutex lock.It then attempts to get the inode via
get_node_by_path(). If the inode does not exist, weunlock and return -1. Otherwise, we create a
temp block of memory large enough to hold all of thedata in this file's direct_ptr’s. We copy the
data from disk to this temp variable and then callstrncpy() to transfer size number of bytes from
the offset within temp to the buffer variable. Oncethis is done we write the data back to disk,
free temp, unlock the mutex, and return size.

## Tfs_write:

Tfs_write begins by locking our global mutex lock.It then attempts to get the inode via
get_node_by_path(). If the inode does not exist, weunlock and return -1. Otherwise, we create a
temp block of memory large enough to hold all of thedata in this file's direct_ptr’s. We copy the
data from disk to this temp variable and then callstrncpy() to transfer size number of bytes from
the buffer variable to the offset within temp. Oncethis is done we write the data back to disk,
free temp, unlock the mutex, and return size.

## Tfs_unlink:

Tfs_unlink begins by locking our global mutex lock.It then reads in both the data block bitmap
and the inode bitmap from disk. We get the basenameand the dirname from path and then
attempt to get the inode of the target file, if itdoes not exist we unlock, free, and return
ENOENT. If it is found, we clear the data block bitmapof the target file, and then clear the inode
bitmap. We then get the node of the parent directory,if it does not exist we free, unlock, and
return ENOENT. If it does exist, we call dir_remove()to remove the target from the parent, if
this does not work we return an error, free, unlock,and exit. Otherwise, we free variables, write
bitmaps to disk, unlock the mutex, and return 0.


# Benchmark Results

## Total Blocks Used:

When running the simple_test benchmark with a BLOCK_SIZEof 4,096 we make:

```
● 102 calls to get_avail_ino()
● 107 calls to get_avail_blkno()
```
When running the simple_test benchmark with a BLOCK_SIZEof 8,192 we make:

```
● 102 calls to get_avail_ino()
● 104 calls to get_avail_blkno()
```
When running the simple_test benchmark with a BLOCK_SIZEof 16,384 we make:

```
● 102 calls to get_avail_ino()
● 103 calls to get_avail_blkno()
```
## Time to Run:

When running the simple_test benchmark with a BLOCK_SIZEof 4,096 the timer recorded

**2,000 ms**.

When running the simple_test benchmark with a BLOCK_SIZEof 8,192 the timer recorded
**4,000 ms**.

When running the simple_test benchmark with a BLOCK_SIZEof 16,384 the timer recorded

**4,000 ms**.


