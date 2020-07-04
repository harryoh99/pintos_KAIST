#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "filesys/fat.h"
#include "devices/disk.h"

/* The disk that contains the file system. */
struct disk *filesys_disk;

static void do_format (void);
struct dir *parse_path(char * path_name, char *file_name);

/* Initializes the file system module.
 * If FORMAT is true, reformats the file system. */
void
filesys_init (bool format) {
	filesys_disk = disk_get (0, 1);
	if (filesys_disk == NULL)
		PANIC ("hd0:1 (hdb) not present, file system initialization failed");

	inode_init ();

#ifdef EFILESYS
	fat_init ();

	if (format)
		do_format ();

	fat_open ();
#else
	/* Original FS */
	free_map_init ();

	if (format)
		do_format ();

	free_map_open ();
#endif
}

/* Shuts down the file system module, writing any unwritten data
 * to disk. */
void
filesys_done (void) {
	/* Original FS */
#ifdef EFILESYS
	fat_close ();
#else
	free_map_close ();
#endif
}

/* Creates a file named NAME with the given INITIAL_SIZE.
 * Returns true if successful, false otherwise.
 * Fails if a file named NAME already exists,
 * or if internal memory allocation fails. */

bool
filesys_create (const char *name, off_t initial_size) {
	cluster_t clst = fat_create_chain (0);
	disk_sector_t inode_sector = cluster_to_sector(clst);
	char tmp_name[PATH_MAX +1];
	struct dir *dir = parse_path(name, tmp_name);
	bool success = (dir != NULL
			&& clst
			&& inode_create (inode_sector, initial_size, false)
			&& dir_add (dir, tmp_name, inode_sector));
	if (!success && inode_sector != 0)
		fat_put(clst, 0);
	dir_close (dir);

	return success;
}

/* Opens the file with the given NAME.
 * Returns the new file if successful or a null pointer
 * otherwise.
 * Fails if no file named NAME exists,
 * or if an internal memory allocation fails. */
struct file *
filesys_open (const char *name) {
	char tmp_name[NAME_MAX+1];
	struct dir *dir = parse_path(name,tmp_name);
	struct inode *inode = NULL;

	if (dir != NULL)
		dir_lookup (dir, tmp_name, &inode);
	else
		return NULL;
	dir_close (dir);
}

/* Deletes the file named NAME.
 * Returns true if successful, false on failure.
 * Fails if no file named NAME exists,
 * or if an internal memory allocation fails. */
bool
filesys_remove (const char *name) {
	char tmp_name[NAME_MAX+1];
	struct dir *dir =parse_path(name,tmp_name);
	struct inode * i =NULL;
	dir_lookup(dir,tmp_name,&i);

	char tmp_dir_name[PATH_MAX_LEN+1];

	struct dir *tmp_dir= NULL;

	if(inode_is_dir(i) ==NULL)


	struct dir *cur_dir = NULL;
  	char temp[PATH_MAX_LEN + 1];
	bool success = false;

	if (!inode_is_dir (inode) || ((cur_dir = dir_open (inode)) && !dir_readdir (cur_dir, temp))){
		if(!dir && dir_remove(dir,name))
			success = true;
	}

	dir_close (dir);
	return success;
}

/* Formats the file system. */
static void
do_format (void) {
	printf ("Formatting file system...");

#ifdef EFILESYS
	/* Create FAT and save it to the disk. */
	fat_create ();
	fat_close ();
#else
	free_map_create ();
	if (!dir_create (ROOT_DIR_SECTOR, 16))
		PANIC ("root directory creation failed");
	free_map_close ();
#endif

	printf ("done.\n");
}
struct dir *parse_path(char * path_name, char *file_name){
	struct dir *dir = NULL;
	if(path_name ==NULL)
		return NULL;
	if(file_name ==NULL)
		return NULL;
	if(strlen(path_name)==0)
		return NULL;
	//절대경로
	if(path_name[0] =='/')
		dir = dir_open_root();
	//상대경로
	else
		dir = dir_reopen(thread_current()->cur_dir);
	struct inode * tmp = dir_get_inode(dir);
	if(inode_is_dir(tmp)==false)
		return NULL;
  	char *token, *next_token, *save_ptr;
  	token = strtok_r (path, "/", &save_ptr);
	next_token = strtok_r (NULL, "/", &save_ptr);

	if(token==NULL){
		strlcpy(file_name,".",NAME_MAX);
		return dir;
	}
	while(token!=NULL && next_token != NULL){
		struct inode * tempo =NULL;
		//NO DIR
		if(dir_lookup(dir,token,&tempo)==false){
			dir_close(dir);
			return NULL;
		}
		if(inode_is_dir(tempo)==false){
			dir_close(dir);
			return NULL;
		}
		//DONE
		dir_close(dir);

		//NEXT PART
		dir = dir_open (tempo);
		token = next_token;
		next_token = strtok_r (NULL, "/", &save_ptr);
	}
	strlcpy (file_name, token, NAME_MAX);
	return dir;
}

bool filesys_create_dir(const char *name){
	cluster_t clst = fat_create_chain (0);
	disk_sector_t inode_sector = cluster_to_sector(clst);
	char tmp_name[PATH_MAX +1];
	struct dir *dir = parse_path(name, tmp_name);
	bool success = (dir != NULL
			&& clst
			&& dir_create (inode_sector, 16)
			&& dir_add (dir, tmp_name, inode_sector));
	if (!success && inode_sector != 0)
		fat_put(clst, 0);

	if(success)
	{
		struct dir *created_dir = dir_open(inode_open(inode_sector));
		dir_add(created_dir, ".", inode_sector);
		struct inode * tmp = dir_get_inode(dir);
		dir_add(created_dir, "..", inode_get_inumber(tmp));
		dir_close(created_dir);
	}
	dir_close(dir);
	return success;
}