/* 
 * File: utils.h
 * Pavlos Spanoudakis (sdi1800184)
 * 
 * Contains definitions for useful functions, as well as widely used #define commands.
 */

#define EXIT_MESSAGE "TERM"                 // termination message
#define RESEND_MESSAGE "resend"             // message/signal for retransmission
#define PERMS 0660                          // permissions for semaphores and shared memory segments
#define SHMSIZE 100                         // shared memory segment size

union semun
{
    int val;
    struct semid_ds *buff;
    unsigned short *array;
};

void malloc_error_exit(void);
void get_line(char **buffer);
char* get_line_buffer(void);
int check_md5(const char *data, const char *hash);
char* md5_hash(const char *data);
void add_noise(char *data);
int sem_up(int semid, struct sembuf *semops, int semnum);
int sem_down(int semid, struct sembuf *semops, int semnum);
int sem_init(int semid, int semnum, int value);