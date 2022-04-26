#include "jumbo_file_system.h"
#include <stdio.h> // printf
#include <string.h> // strncmp(?)
#include <stdlib.h> // strncpy

// C does not have a bool type, so I created one that you can use
typedef char bool_t;
#define TRUE 1
#define FALSE 0


static block_num_t current_dir;


// optional helper function you can implement to tell you if a block is a dir node or an inode
static bool_t is_dir(block_num_t block_num) {
  // Get block
  int status = read_block(block_num, (void *) (&buf));
  if(status == -1){ printf("is_dir,,, is angry\n"); }

  // Check if dir
  if(buf.is_dir == 0){
    return TRUE;
  } else {
    return FALSE;
  }
}


/* jfs_mount
 *   prepares the DISK file on the _real_ file system to have file system
 *   blocks read and written to it.  The application _must_ call this function
 *   exactly once before calling any other jfs_* functions.  If your code
 *   requires any additional one-time initialization before any other jfs_*
 *   functions are called, you can add it here.
 * filename - the name of the DISK file on the _real_ file system
 * returns 0 on success or -1 on error; errors should only occur due to
 *   errors in the underlying disk syscalls.
 */
int jfs_mount(const char* filename) {
  int ret = bfs_mount(filename);
  current_dir = 1;

 
  return ret;
}


/* jfs_mkdir
 *   creates a new subdirectory in the current directory
 * directory_name - name of the new subdirectory
 * returns 0 on success or one of the following error codes on failure:
 *   E_EXISTS, E_MAX_NAME_LENGTH, E_MAX_DIR_ENTRIES, E_DISK_FULL
 */
int jfs_mkdir(const char* directory_name) {

  // *** Get current_dir ***
  struct block buf2;
  int status3 = read_block(current_dir, (void *) (&buf2));
  if(status3 == -1){ printf("ERROR: jfs_mkdir,,, is angry (3)\n"); }

  printf("\tCurr dir num: %d\n", current_dir);
  if(buf2.contents.dirnode.num_entries > MAX_DIR_ENTRIES){
    return E_MAX_DIR_ENTRIES;
  }

  // Check if dir exists
  for(int i = 0; i < buf2.contents.dirnode.num_entries; i++){
    // printf("Checking: \n\t%s\n\tand %s", buf2.contents.dirnode.entries[i].name, directory_name);
    if(strncmp(buf2.contents.dirnode.entries[i].name, directory_name, strlen(directory_name)) == 0){
      return E_EXISTS;
    }
  }
  printf("\tDir doesn't exist! (good)\n");

  
  // *** If dir doesn't exit ***

  // Make new block
  block_num_t block_num = allocate_block();
  struct block buf;
  int status = read_block(block_num, (void *) (&buf));
  if(status == -1){printf("ERROR: jfs_mkdir,,, is angry (1)\n"); }

  printf("\tNew Block Num: %d\n", block_num);

  // Set directory fields
  buf.is_dir = 0;
  buf.contents.dirnode.num_entries = 0;

  // Push changes to new dir block
  int status2 = write_block(block_num, (void *) (&buf));
  if(status2 == -1){ printf("ERROR: jfs_mkdir,,, is angry (2)\n");  }

  // Update parent node
  int num_entries = buf2.contents.dirnode.num_entries;
  buf2.contents.dirnode.entries[num_entries].block_num = block_num;
  strncpy(buf2.contents.dirnode.entries[num_entries].name, directory_name, strlen(directory_name));
  buf2.contents.dirnode.num_entries++;

  // Push changes to parent block
  int status4 = write_block(current_dir, (void *) (&buf2));
  if(status4 == -1){ printf("ERROR: jfs_mkdir,,, is angry (4)\n"); }

  printf("\tDir '%s' was made in current block: %d (block num)\n", directory_name, current_dir);
  return E_SUCCESS;


  // TODO: E_MAX_NAME_LENGTH, E_DISK_FULL *
}


/* jfs_chdir
 *   changes the current directory to the specified subdirectory, or changes
 *   the current directory to the root directory if the directory_name is NULL
 * directory_name - name of the subdirectory to make the current
 *   directory; if directory_name is NULL then the current directory
 *   should be made the root directory instead
 * returns 0 on success or one of the following error codes on failure:
 *   E_NOT_EXISTS, E_NOT_DIR
 */
int jfs_chdir(const char* directory_name) {

  // *** Get current_dir ***
  struct block buf;
  int status = read_block(current_dir, (void *) (&buf));
  if(status == -1){ printf("ERROR: jfs_chdir,,, is angry (1)\n"); }

  printf("\tCurr dir num: %d\n", current_dir);
  // printf("\tNum dir entries in current: %d\n", buf2.contents.dirnode.num_entries);


  // Check if dir exists
  int found_num = 0;
  for(int i = 0; i < buf.contents.dirnode.num_entries; i++){
    // printf("\tChecking: %s and %s\n", buf.contents.dirnode.entries[i].name, directory_name);
    // printf("\t%d\n", strncmp(buf.contents.dirnode.entries[i].name, directory_name, strlen(directory_name)));
    if(strncmp(buf.contents.dirnode.entries[i].name, directory_name, strlen(directory_name)) == 0){
      found_num = buf.contents.dirnode.entries[i].block_num;
      // printf("FOUND %d\n", found_num);
    }
  }

  // If directory not found, error
  if(found_num == 0){
    printf("\tDir doesn't exist! (bad)\n");
    return E_NOT_EXISTS;
  }

    
  // If found is a directory, change directory
  if(is_dir(found_num)){
    current_dir = found_num;
    printf("\tCurr dir updated: %d\n", current_dir);
    return E_SUCCESS;
  } else {
    return E_NOT_DIR;
  }
}


/* jfs_ls
 *   finds the names of all the files and directories in the current directory
 *   and writes the directory names to the directories argument and the file
 *   names to the files argument
 * directories - array of strings; the function will set the strings in the
 *   array, followed by a NULL pointer after the last valid string; the strings
 *   should be malloced and the caller will free them
 * file - array of strings; the function will set the strings in the
 *   array, followed by a NULL pointer after the last valid string; the strings
 *   should be malloced and the caller will free them
 * returns 0 on success or one of the following error codes on failure:
 *   (this function should always succeed)
 */
int jfs_ls(char* directories[MAX_DIR_ENTRIES+1], char* files[MAX_DIR_ENTRIES+1]) {
  // *** Get current_dir ***
  struct block buf;
  int status = read_block(current_dir, (void *) (&buf));
  if(status == -1){ printf("ERROR: jfs_ls,,, is angry (1)\n"); }

  printf("\tCurr dir num: %d\n", current_dir);
  printf("\tCur dir entries: %d\n", buf.contents.dirnode.num_entries);

  int dir_entries= 0;
  int file_entries = 0;

  // Check all blocks
  for(int i = 0; i < buf.contents.dirnode.num_entries; i++){
    printf("\t\tChecking current dir: %s\n", buf.contents.dirnode.entries[i].name);
    printf("\t\twith block num: %d\n", buf.contents.dirnode.entries[i].block_num);
    
    // If directory
    if(is_dir(buf.contents.dirnode.entries[i].block_num)){
      directories[dir_entries] = malloc((strlen(buf.contents.dirnode.entries[i].name)) + 1);

      // Check for good memory allocation 
      if (!directories[dir_entries]){ printf("ERROR: jfs_ls,,, is angry (2)\n"); }

      // Copy memory contents
      strncpy(directories[dir_entries], buf.contents.dirnode.entries[i].name, strlen(buf.contents.dirnode.entries[i].name) + 1);

      dir_entries++;

    // If File
    } else {
      files[file_entries] = malloc((strlen(buf.contents.dirnode.entries[i].name)) + 1);
      
      // Check for good memory allocation 
      if (!files[file_entries]){ printf("ERROR: jfs_ls,,, is angry (2)\n"); }

      // Copy memory contents
      strncpy(files[file_entries], buf.contents.dirnode.entries[i].name, strlen(buf.contents.dirnode.entries[i].name) + 1);

      file_entries++;
    } 
  }

  // Readd null terminators
  directories[dir_entries] = NULL;
  files[file_entries] = NULL;


  return E_SUCCESS;;
}


/* jfs_rmdir
 *   removes the specified subdirectory of the current directory
 * directory_name - name of the subdirectory to remove
 * returns 0 on success or one of the following error codes on failure:
 *   E_NOT_EXISTS, E_NOT_DIR, E_NOT_EMPTY
 */
int jfs_rmdir(const char* directory_name) {
  // *** Get current_dir ***
  struct block buf;
  int status1 = read_block(current_dir, (void *) (&buf));
  if(status1 == -1){ printf("ERROR: jfs_rmdir,,, is angry (1)\n"); }

  printf("\tCurr dir num: %d\n", current_dir);

  // Check if dir exists
  int found = 0;
  int idx = 0;
  for(int i = 0; i < buf.contents.dirnode.num_entries; i++){
    // printf("Checking: \n\t%s\n\tand %s", buf2.contents.dirnode.entries[i].name, directory_name);
    if(strncmp(buf.contents.dirnode.entries[i].name, directory_name, strlen(directory_name)) == 0){
      found = buf.contents.dirnode.entries[i].block_num;
      idx = i;

      printf("\tFound %s, at block %d\n", buf.contents.dirnode.entries[i].name, found);
    }
  }

  if (found == 0 && idx == 0){ // file doesn't exist
    printf("\tDir doesn't exist! (bad)\n");
    return E_NOT_EXISTS;
  } else if (is_dir(found)){ // file is actually a directory
    printf("\tDirectory is actually a file; use rm.\n");
    return E_NOT_DIR;
  }
  

  // TODO: E_NOT_EMPTY
  
  // *** If dir exists ***
  // Remove block
  int status2 = release_block(found);
  if(status2 == -1){printf("ERROR: jfs_rmdir,,, is angry (1)\n"); }


  // Update parent node
  int num_entries = buf.contents.dirnode.num_entries;

  printf("Shifting: %d, to %d\n", idx, num_entries-1); 
  for(int i = idx; i < num_entries - 1; i++){
    printf("\t [Re]moving block %d, with name %s\n", buf.contents.dirnode.entries[i].block_num, buf.contents.dirnode.entries[i].name);
    buf.contents.dirnode.entries[i].block_num = buf.contents.dirnode.entries[i+1].block_num;
    strncpy(buf.contents.dirnode.entries[i].name, buf.contents.dirnode.entries[i+1].name, strlen(buf.contents.dirnode.entries[i+1].name));
  }
  buf.contents.dirnode.num_entries--;

  // Push changes to parent block
  int status4 = write_block(current_dir, (void *) (&buf));
  if(status4 == -1){ printf("ERROR: jfs_rmdir,,, is angry (4)\n"); }

  return E_SUCCESS;
}


/* jfs_creat
 *   creates a new, empty file with the specified name
 * file_name - name to give the new file
 * returns 0 on success or one of the following error codes on failure:
 *   E_EXISTS, E_MAX_NAME_LENGTH, E_MAX_DIR_ENTRIES, E_DISK_FULL
 */
int jfs_creat(const char* file_name) {
  // *** Get current_dir ***
  struct block buf2;
  int status3 = read_block(current_dir, (void *) (&buf2));
  if(status3 == -1){ printf("ERROR: jfs_creat,,, is angry (3)\n"); }

  printf("\tCurr dir num: %d\n", current_dir);

  // TODO: E_MAX_NAME_LENGTH, E_DISK_FULL
  if(buf2.contents.dirnode.num_entries > MAX_DIR_ENTRIES){
    return E_MAX_DIR_ENTRIES;
  }

  // Check if dir exists
  for(int i = 0; i < buf2.contents.dirnode.num_entries; i++){
    if(strncmp(buf2.contents.dirnode.entries[i].name, file_name, strlen(file_name)) == 0){
      return E_EXISTS;
    }
  }
  printf("\tFile doesn't exist! (good)\n");

  
  // *** If file doesn't exit ***
  // Make new block
  block_num_t block_num = allocate_block();
  struct block buf;
  int status = read_block(block_num, (void *) (&buf));
  if(status == -1){printf("ERROR: jfs_creat,,, is angry (1)\n"); }

  printf("\tNew Block Num: %d\n", block_num);

  // Set file fields
  buf.is_dir = 1;
  buf.contents.inode.file_size = 0;

  // Push changes to new dir block
  int status2 = write_block(block_num, (void *) (&buf));
  if(status2 == -1){ printf("ERROR: jfs_creat,,, is angry (2)\n");  }

  

  // Update parent node
  int num_entries = buf2.contents.dirnode.num_entries;
  buf2.contents.dirnode.entries[num_entries].block_num = block_num;
  strncpy(buf2.contents.dirnode.entries[num_entries].name, file_name, strlen(file_name));
  printf("\tTest Write file name:%s, %s\n", buf2.contents.dirnode.entries[num_entries].name, file_name);
  buf2.contents.dirnode.num_entries++;

  // Push changes to parent block
  int status4 = write_block(current_dir, (void *) (&buf2));
  if(status4 == -1){ printf("ERROR: jfs_creat,,, is angry (4)\n"); }

  printf("\tFile '%s' was made in current block: %d (block num)\n", file_name, current_dir);
  return E_SUCCESS;
}


/* jfs_remove
 *   deletes the specified file and all its data (note that this cannot delete
 *   directories; use rmdir instead to remove directories)
 * file_name - name of the file to remove
 * returns 0 on success or one of the following error codes on failure:
 *   E_NOT_EXISTS, E_IS_DIR
 */
int jfs_remove(const char* file_name) {
  // *** Get current_dir ***
  struct block buf;
  int status1 = read_block(current_dir, (void *) (&buf));
  if(status1 == -1){ printf("ERROR: jfs_remove,,, is angry (1)\n"); }

  printf("\tCurr dir num: %d\n", current_dir);

  // Check if dir exists
  int found = 0;
  int idx = 0;
  for(int i = 0; i < buf.contents.dirnode.num_entries; i++){

    if(strncmp(buf.contents.dirnode.entries[i].name, file_name, strlen(file_name)) == 0){
      found = buf.contents.dirnode.entries[i].block_num;
      idx = i;

      printf("\tFound %s, at block %d\n", buf.contents.dirnode.entries[i].name, found);
    }
  }
  
  // Error checking
  if (found == 0 && idx == 0){ // file doesn't exist
    printf("\tFile doesn't exist! (bad)\n");
    return E_NOT_EXISTS;
  } else if (is_dir(found)){ // file is actually a directory
    printf("\tFile is actually a directory; use rmdir.\n");
    return E_IS_DIR;
  }
  
  
  // *** If dir exists ***

  // Remove block
  int status2 = release_block(found);
  if(status2 == -1){printf("ERROR: jfs_remove,,, is angry (1)\n"); }

  // Update parent node
  int num_entries = buf.contents.dirnode.num_entries;

  // Shift entries
  printf("Shifting: %d, to %d\n", idx, num_entries-1); 
  for(int i = idx; i < num_entries - 1; i++){
    printf("\t[Re]moving block %d, with name %s\n", buf.contents.dirnode.entries[i].block_num, buf.contents.dirnode.entries[i].name);
    buf.contents.dirnode.entries[i].block_num = buf.contents.dirnode.entries[i+1].block_num;
    strncpy(buf.contents.dirnode.entries[i].name, buf.contents.dirnode.entries[i+1].name, strlen(buf.contents.dirnode.entries[i+1].name));
  }
  buf.contents.dirnode.num_entries--;

  // Push changes to parent block
  int status3 = write_block(current_dir, (void *) (&buf));
  if(status3 == -1){ printf("ERROR: jfs_remove,,, is angry (4)\n"); }

  return E_SUCCESS;
}


/* jfs_stat
 *   returns the file or directory stats (see struct stat for details)
 * name - name of the file or directory to inspect
 * buf  - pointer to a struct stat (already allocated by the caller) where the
 *   stats will be written
 * returns 0 on success or one of the following error codes on failure:
 *   E_NOT_EXISTS
 */
int jfs_stat(const char* name, struct stats* buf) {
  // *** Get current_dir ***
  struct block buf1;
  int status = read_block(current_dir, (void *) (&buf1));
  if(status == -1){ printf("ERROR: jfs_stat,,, is angry (1)\n"); }

  printf("\tCurr dir num: %d\n", current_dir);

  // Check if dir exists
  int found = 0;
  int idx = 0;
  for(int i = 0; i < buf1.contents.dirnode.num_entries; i++){
    // printf("Checking: \n\t%s\n\tand %s", buf2.contents.dirnode.entries[i].name, directory_name);
    if(strncmp(buf1.contents.dirnode.entries[i].name, name, strlen(name)) == 0){
      found = buf1.contents.dirnode.entries[i].block_num;
      idx = i;
    }
  }
  if(found == 0 && idx == 0){
    return E_NOT_EXISTS;
  }
  printf("\tFile exists! (good)\n");



  struct block buf2;
  int status2 = read_block(current_dir, (void *) (&buf2));
  if(status2 == -1){ printf("ERROR: jfs_stat,,, is angry (2)\n"); }

  buf->is_dir = buf1.is_dir;

  return E_UNKNOWN;
}


/* jfs_write
 *   appends the data in the buffer to the end of the specified file
 * file_name - name of the file to append data to
 * buf - buffer containing the data to be written (note that the data could be
 *   binary, not text, and even if it is text should not be assumed to be null
 *   terminated)
 * count - number of bytes in buf (write exactly this many)
 * returns 0 on success or one of the following error codes on failure:
 *   E_NOT_EXISTS, E_IS_DIR, E_MAX_FILE_SIZE, E_DISK_FULL
 */
int jfs_write(const char* file_name, const void* buf, unsigned short count) {
  (void) file_name;
  (void) buf;
  (void) count;

  return E_UNKNOWN;
}


/* jfs_read
 *   reads the specified file and copies its contents into the buffer, up to a
 *   maximum of *ptr_count bytes copied (but obviously no more than the file
 *   size, either)
 * file_name - name of the file to read
 * buf - buffer where the file data should be written
 * ptr_count - pointer to a count variable (allocated by the caller) that
 *   contains the size of buf when it's passed in, and will be modified to
 *   contain the number of bytes actually written to buf (e.g., if the file is
 *   smaller than the buffer) if this function is successful
 * returns 0 on success or one of the following error codes on failure:
 *   E_NOT_EXISTS, E_IS_DIR
 */
int jfs_read(const char* file_name, void* buf, unsigned short* ptr_count) {
  (void) file_name;
  (void) buf;
  (void) ptr_count;

  return E_UNKNOWN;
}


/* jfs_unmount
 *   makes the file system no longer accessible (unless it is mounted again).
 *   This should be called exactly once after all other jfs_* operations are
 *   complete; it is invalid to call any other jfs_* function (except
 *   jfs_mount) after this function complete.  Basically, this closes the DISK
 *   file on the _real_ file system.  If your code requires any clean up after
 *   all other jfs_* functions are done, you may add it here.
 * returns 0 on success or -1 on error; errors should only occur due to
 *   errors in the underlying disk syscalls.
 */
int jfs_unmount() {
  int ret = bfs_unmount();
  return ret;
}
