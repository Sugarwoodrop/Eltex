#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>

#define SUCCESS 0
#define ERROR  -1
#define MAX_DRIVERS 64

typedef struct {
    pid_t  pid;
    int    active;
    volatile int    pending_timer;
    volatile time_t busy_until;
} slot_t;

typedef struct {
    slot_t slots[MAX_DRIVERS];
    int    count;
} shared_t;

static shared_t *shm; 
static int my_slot = -1;


static volatile sig_atomic_t f_task = 0, f_busy = 0, f_done = 0;
static volatile sig_atomic_t busy_report = 0;

static void on_task(int sig) {
    (void)sig;
    slot_t *s = &shm->slots[my_slot];
    time_t now = time(NULL);
    if (s->busy_until > now) {
        busy_report = (int)(s->busy_until - now);
        f_busy = 1;
    } else {
        int t = s->pending_timer;
        s->busy_until = now + t;
        alarm(t);
        f_task = t;
    }
}

static void on_alarm(int sig) {
    (void)sig;
    shm->slots[my_slot].busy_until = 0;
    f_done = 1;
}

static void on_term(int sig) {
    (void)sig;
    _exit(0);
}

static void driver_main(int slot) {
    my_slot = slot;
    pid_t me = getpid();
    shm->slots[slot].pid = me;
    shm->slots[slot].busy_until = 0;

    struct sigaction sa = {0};
    sa.sa_handler = on_task;  sigaction(SIGUSR1, &sa, NULL);
    sa.sa_handler = on_alarm; sigaction(SIGALRM, &sa, NULL);
    sa.sa_handler = on_term;  sigaction(SIGTERM, &sa, NULL);

    printf("[driver %d] started, Available\n", me);
    fflush(stdout);

    while (1) {
        pause();
        if (f_task)  { printf("[driver %d] task accepted for %ds\n", me, (int)f_task); fflush(stdout); f_task = 0; }
        if (f_busy)  { printf("[driver %d] Busy %d\n", me, (int)busy_report);          fflush(stdout); f_busy = 0; }
        if (f_done)  { printf("[driver %d] now Available\n", me);                       fflush(stdout); f_done = 0; }
    }
}


static slot_t *find_by_pid(pid_t pid) {
    for (int i = 0; i < shm->count; i++)
        if (shm->slots[i].active && shm->slots[i].pid == pid)
            return &shm->slots[i];
    return NULL;
}

static void print_status(slot_t *s) {
    time_t now = time(NULL);
    long remaining = (long)s->busy_until - (long)now;
    if (remaining > 0)
        printf("Busy %ld\n", remaining);
    else
        printf("Available\n");
}

static int cmd_create_driver(int pid, int timer) {
    (void)pid; (void)timer;
    if (shm->count >= MAX_DRIVERS) { printf("Лимит драйверов\n"); return 0; }
    int i = shm->count;
    shm->slots[i].active = 1;
    shm->slots[i].busy_until = 0;
    shm->slots[i].pending_timer = 0;

    pid_t cpid = fork();
    if (cpid == ERROR) { perror("fork"); shm->slots[i].active = 0; return 0; }
    if (cpid == 0) {
        driver_main(i);
        _exit(0);
    }
    shm->slots[i].pid = cpid;
    shm->count++;
    printf("Created driver with pid %d\n", cpid);
    return 0;
}

static int cmd_send_task(int pid, int timer) {
    slot_t *s = find_by_pid(pid);
    if (!s) { printf("Нет драйвера с pid %d\n", pid); return 0; }
    s->pending_timer = timer;
    kill(pid, SIGUSR1);
    return 0;
}

static int cmd_get_status(int pid, int timer) {
    (void)timer;
    slot_t *s = find_by_pid(pid);
    if (!s) { printf("Нет драйвера с pid %d\n", pid); return 0; }
    printf("Driver %d: ", pid);
    print_status(s);
    return 0;
}

static int cmd_get_drivers(int pid, int timer) {
    (void)pid; (void)timer;
    if (shm->count == 0) { printf("Драйверов нет\n"); return 0; }
    for (int i = 0; i < shm->count; i++) {
        if (!shm->slots[i].active) continue;
        printf("Driver %d: ", shm->slots[i].pid);
        print_status(&shm->slots[i]);
    }
    return 0;
}

static int cmd_help(int pid, int timer) {
    (void)pid; (void)timer;
    printf("create_driver | send_task <pid> <timer> | "
           "get_status <pid> | get_drivers | help | exit\n");
    return 0;
}

static int cmd_exit(int pid, int timer) {
    (void)pid; (void)timer;
    return 1;
}


typedef struct {
    const char *name;
    int         min_args;
    int       (*fn)(int pid, int timer);
} command_t;

static const command_t commands[] = {
    { "create_driver", 1, cmd_create_driver },
    { "send_task",     3, cmd_send_task     },
    { "get_status",    2, cmd_get_status    },
    { "get_drivers",   1, cmd_get_drivers   },
    { "help",          1, cmd_help          },
    { "exit",          1, cmd_exit          },
};
#define N_COMMANDS (sizeof(commands) / sizeof(commands[0]))

static void cleanup(void) {
    for (int i = 0; i < shm->count; i++)
        if (shm->slots[i].active) {
            kill(shm->slots[i].pid, SIGTERM);
            waitpid(shm->slots[i].pid, NULL, 0);
        }
}

int main(void) {
    shm = mmap(NULL, sizeof(shared_t), PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shm == MAP_FAILED) { perror("mmap"); exit(ERROR); }
    memset(shm, 0, sizeof(shared_t));

    printf("Taxi dispatch. Команды: create_driver | send_task <pid> <timer> | "
           "get_status <pid> | get_drivers | exit\n");

    char line[256];
    while (1) {
        printf("dispatch> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = '\0';

        char cmd[64];
        int pid = 0, timer = 0;
        int nargs = sscanf(line, "%63s %d %d", cmd, &pid, &timer);
        if (nargs < 1) continue;

        const command_t *c = NULL;
        for (size_t i = 0; i < N_COMMANDS; i++)
            if (strcmp(cmd, commands[i].name) == 0) { c = &commands[i]; break; }

        if (!c) {
            printf("Неизвестная команда: %s\n", cmd);
            continue;
        }
        if (nargs < c->min_args) {
            printf("Недостаточно аргументов для '%s'\n", c->name);
            continue;
        }
        if (c->fn(pid, timer))
            break;

        usleep(50000);
    }

    cleanup();
    munmap(shm, sizeof(shared_t));
    return SUCCESS;
}
