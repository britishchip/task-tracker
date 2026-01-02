#include "stdbool.h"
#include <stdio.h>
#include "assert.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

time_t currentTime;

char *filename = "data.bin";
#define MAX_TASKS 20

struct Task
{
    int id;
    char desc[256];
    char status[20];
    char createdAt[40];
    char updatedAt[40];
};

// factory function for struct
struct Task *Task_create(char *desc, int id, char *currentTime)
{
    struct Task *create = malloc(sizeof(struct Task));
    assert(create != NULL);

    strcpy(create->desc,desc);
    create->id = id;
    strcpy(create->status,"todo");
    strcpy(create->createdAt,currentTime);
    strcpy(create->updatedAt,currentTime);

    return create;
}

struct Task *read_data(char *filename)
{
    FILE *file;
    file = fopen(filename, "rb");

    if(file == NULL)
        return NULL;

    //allocate size for data being read from the file
    struct Task *data = malloc(sizeof(struct Task) * MAX_TASKS);
    if (!data) {
        fclose(file);
        return NULL;
    }
    //get the actual size of data being read
    size_t n = fread(data, sizeof(struct Task), MAX_TASKS, file);  
    // zero unused entries 
    memset(&data[n], 0, (MAX_TASKS - n)*sizeof *data);
    fclose(file);

    return data;
}

_Bool write_data(char *filename, struct Task *task)
{
    struct Task *taskLists = read_data(filename);
    if (!taskLists) {
        taskLists = calloc(MAX_TASKS, sizeof(struct Task));
    }

    FILE *file;
    file = fopen(filename, "wb");

    assert(file != NULL);

    //find first empty slot
    int slot = -1;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (taskLists[i].id == 0) {
            slot = i;
            break;
        }
    }

    if (slot != -1) {
        taskLists[slot] = *task;
    }

    if(fwrite(taskLists, sizeof(struct Task), MAX_TASKS, file) != MAX_TASKS)
    {
        return false;
    }
    
    if (fclose(file)==EOF) return false;
    free(taskLists);
    return true;
}


_Bool addTask(char *task, int id, char *currentTime, char *filename){
    struct Task *new_task = Task_create(task, id, currentTime);
    write_data(filename, new_task);
    free(new_task);
 
}

int getNextId()
{
    struct Task *taskLists = read_data(filename);

    if (taskLists == NULL) {
        return 1;
    }

    int nextID = 0;

    for (int i=0; i < MAX_TASKS; i++) {
        if (taskLists[i].id == 0) break;
        if(taskLists[i].id >=  nextID){
            nextID = taskLists[i].id;
        }
    }
    free(taskLists);
    return nextID + 1;
}


_Bool deleteTask(int id){
    struct Task *taskLists = read_data(filename);

    assert(taskLists != NULL);
    
    int lastID = getNextId()-1;
    
    for(int i = id-1; i < getNextId()-1; i++){
        taskLists[i] = taskLists[i + 1];
        if (taskLists[i].id > id){
            taskLists[i].id -= 1;
        } 
    }

    //for (int i = lastID-1; i < MAX_TASKS-1; i++) {
    memset(&taskLists[lastID-1], 0, sizeof(struct Task));
    
    
    FILE *file = fopen(filename, "wb");
    assert(file!=NULL);

    fwrite(taskLists, sizeof(struct Task), MAX_TASKS, file);

    free(taskLists);
    fclose(file);

}

_Bool updateTask(int id, char *desc, char *currentTimeInString)
{
    struct Task *taskLists = read_data(filename);

    assert(taskLists != NULL);

    strcpy(taskLists[id - 1].desc, desc);
    strcpy(taskLists[id-1].updatedAt, currentTimeInString);

    FILE *file = fopen(filename, "rb+");
    assert(file != NULL);

    //move the pointer to the task in focus
    if (fseek(file, sizeof(struct Task) * (id - 1), SEEK_SET)==0)
    {
        fwrite(&taskLists[id-1], sizeof(struct Task), 1, file);
    }
    
    free(taskLists);
    fclose(file);
   
}


void markTask(int id, char *status, char *currentTimeInString)
{
    struct Task *taskLists = read_data(filename);

    assert(taskLists != NULL);

    strcpy(taskLists[id - 1].status, status);
    strcpy(taskLists[id-1].updatedAt, currentTimeInString);

    FILE *file = fopen(filename, "rb+");
    assert(file != NULL);

    //move the pointer to the task in focus
    if (fseek(file, sizeof(struct Task) * (id - 1), SEEK_SET)==0)
    {
        fwrite(&taskLists[id-1], sizeof(struct Task), 1, file);
    }
    
    free(taskLists);
    fclose(file);
}

void listAll(){
    struct Task *taskLists = read_data(filename);
    
    assert(taskLists!=NULL);

    for (int i=0; i < MAX_TASKS; i++) {
        if (taskLists[i].id != 0) {
            printf("Description: %s\n", taskLists[i].desc);
            printf("         ID: %d\n", taskLists[i].id);
            printf("     Status: %s\n", taskLists[i].status);
            printf(" Created At: %s", taskLists[i].createdAt);
            printf(" Updated At: %s\n", taskLists[i].updatedAt);
        }
    }
    free(taskLists);
}

void listTask(char *status){
    struct Task *taskLists = read_data(filename);
    
    assert(taskLists!=NULL);

    for (int i=0; i < MAX_TASKS; i++) {
        if (strcmp(taskLists[i].status, status) == 0) {
            printf("Description: %s\n", taskLists[i].desc);
            printf("         ID: %d\n", taskLists[i].id);
            printf("     Status: %s\n", taskLists[i].status);
            printf(" Created At: %s", taskLists[i].createdAt);
            printf(" Updated At: %s\n", taskLists[i].updatedAt);
        }
    }
    free(taskLists);
}

int main(int argc, char *argv[])
{
    time_t currentTime;
    char *currentTimeInString;

    currentTime = time(NULL);
    currentTimeInString = ctime(&currentTime);
    
    char *arg2 =NULL;
    char *arg3 = NULL; 


    if(argc < 2){
         printf("USAGE: task-cli <command> <arg>\n");
         printf("%s", currentTimeInString);
         return 1;
    }   

    // parsing command line arguments
    char *command = argv[1];
    if (argc >= 3){
        arg2 = argv[2];
    }
    if (argc >= 4) {
        arg3 = argv[3];
    }
    
    if (strcmp(command, "add") == 0){ 
        if (argc < 3) {
            printf("nothing to add\n");
            return 1;
        }
        int n = getNextId();
        if (addTask(arg2, n, currentTimeInString, filename)) {
            printf("Task added successfully (ID: %d)\n", n-1);
        }
    } else if (strcmp(command, "update") == 0) {
        if (argc != 4 ){
            printf("Update failed: Incomplete arguments\n");
            return 1;
        }
        if (updateTask(atoi(arg2), arg3, currentTimeInString)) {
            printf("Task updated\n");
        }
    } else if (strcmp(command, "delete") == 0) {
        if (argc < 3) {
            printf("nothing to delete\n");
            return 1;
        }
        if (deleteTask(atoi(arg2))) {
            printf("Task deleted\n");
        }
    } else if (strcmp(command, "list") == 0) {
        if (argc == 2)
            listAll();
        else if (argc > 2)
            listTask(arg2);    
    } else if (strcmp(command, "mark-in-progress") == 0) {
        markTask(atoi(arg2), "in-progress", currentTimeInString);
    } else if (strcmp(command, "mark-done") == 0) {
        markTask(atoi(arg2), "done", currentTimeInString);
    }
    return 0;
}