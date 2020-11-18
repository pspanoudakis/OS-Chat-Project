/* File: utils.h */

#define MAX_INPUT_SIZE 100
#define EXIT_MESSAGE "exit"

union semun
{
    int val;
    struct semid_ds *buff;
    unsigned short *array;
};

void get_line(char **buffer);
char* get_line_buffer(void);
int check_md5(const char *data, const char *hash);
char* md5_hash(const char *data);
void add_noise(char *data);
int sem_up(int semid, struct sembuf *semops, int semnum);
int sem_down(int semid, struct sembuf *semops, int semnum);
int sem_init(int semid, int semnum, int value);