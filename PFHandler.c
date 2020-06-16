/*
 ============================================================================
 Name        : c.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct page {
	int pid;
	int page;
	int frame;
} Page;

typedef struct pageNode {
	Page *data;
	struct pageNode *next;
} PageNode;

typedef struct pageQueue {
	PageNode *front, *rear;
} PageQueue;

typedef struct process {
	int pid;
	int pageFaults;
	int frameCount;
	PageQueue *queue;
} Process;

typedef struct processNode {
	Process *process;
	struct processNode *next;
} ProcessNode;

typedef struct processQueue {
	ProcessNode *front, *rear;
} ProcessQueue;

PageQueue *pageQueue;
ProcessQueue *processQueue;

void printLocal(ProcessQueue *);
void printGlobal(ProcessQueue *, PageQueue *);

PageNode *createPage(int, int, int);
PageQueue *createPageQueue();
void enQueue(PageQueue *, PageNode *);
PageNode *dequeue(PageQueue *);
PageNode *retrievePage(PageQueue *, int, int);
PageNode *removePage(PageQueue *, int, int);
void pageSwap(PageNode *, PageNode *);
void pageSort(PageNode *);

ProcessNode *createProcess(int);
ProcessQueue *createProcessQueue();
void enqueueProcess(ProcessQueue *, ProcessNode *);
ProcessNode *retrieveProcess(ProcessQueue *, int);
void processSwap(ProcessNode *, ProcessNode *);
void processSort(ProcessNode *);

int main(void) {
	setvbuf(stdout, NULL, _IONBF, 0);
	char input[10];
	char string[10];
	int isLocal;
	int pid, page;
	int numOfFrames; // M > 2
	int frameCounter = 0;
	//char inputbuffer[50];

	processQueue = createProcessQueue();
	pageQueue = createPageQueue();

	fgets(string, sizeof(string), stdin);
	sscanf(string, "%s", input); // Obtain LOCAL or GLOBAL

	isLocal = strcmp(input, "LOCAL"); // 0 is local else Global since all tests are assumed to have correct input

	fgets(string, sizeof(string), stdin);
	sscanf(string, "%d", &numOfFrames); // Obtain # of frames

	//fgets(inputbuffer, 50, stdin);

	fgets(string, sizeof(string), stdin);
	//scanf("%d", &page);

	while(strcmp(string, "\n\0") != 0){
		sscanf(string, "%d %d", &pid, &page);
		//printf("%d %d\n", pid, page);

		if(isLocal == 0){
			ProcessNode *processNode = retrieveProcess(processQueue, pid);

			if(processNode == NULL){
				processNode = createProcess(pid);
				processNode->process->pid = pid;
				enqueueProcess(processQueue, processNode);
			}

			PageNode *node = removePage(processNode->process->queue, pid, page);

			if(node == NULL){

				if(processNode->process->frameCount < numOfFrames){

					enQueue(processNode->process->queue, createPage(pid, page, frameCounter));
					processNode->process->frameCount++;
					frameCounter++;

				} else {

					node = dequeue(processNode->process->queue);
					enQueue(processNode->process->queue, createPage(pid, page, node->data->frame));

				}

				processNode->process->pageFaults++;

			} else {

				enQueue(processNode->process->queue, createPage(node->data->pid, node->data->page, node->data->frame));
			}

		} else {
			ProcessNode *processNode = retrieveProcess(processQueue, pid);

			if(processNode == NULL){

				processNode = createProcess(pid);
				processNode->process->pid = pid;
				enqueueProcess(processQueue, processNode);
			}

			PageNode *node = retrievePage(pageQueue, pid, page);

			if(node == NULL){

				if(frameCounter < numOfFrames){
					enQueue(pageQueue, createPage(pid, page, frameCounter));
					frameCounter++;

				} else {

					node = dequeue(pageQueue);
					enQueue(pageQueue, createPage(pid, page, node->data->frame));
				}

				processNode->process->pageFaults++;
			}
		}
		fgets(string, sizeof(string), stdin);
	}

	if(isLocal == 0)
		printLocal(processQueue);
	else
		printGlobal(processQueue, pageQueue);

	return 0;
}

void printLocal(ProcessQueue *processQueue){

	processSort(processQueue->front);
	printf("Expected Output\n");

	printf("PID Page Faults\n");

	ProcessNode *processNode = processQueue->front;

	while(processNode != NULL){
		printf("%d\t%d\n", processNode->process->pid, processNode->process->pageFaults);
		processNode = processNode->next;
	}
	printf("\n");

	ProcessNode *current = processQueue->front;
	while(current != NULL){
		pageSort(current->process->queue->front);

		printf("Process %d page table\n", current->process->pid);
		printf("Page\tFrame\n");

		PageNode *pageNode = current->process->queue->front;

		while(pageNode != NULL){
			printf("%d\t%d\n", pageNode->data->page, pageNode->data->frame);
			pageNode = pageNode->next;
		}
		current = current->next;
		printf("\n");
	}
}

void printGlobal(ProcessQueue *processQueue, PageQueue *pageQueue){
	PageNode *pageNode = pageQueue->front;

	while(pageNode != NULL){
		ProcessNode *processNode = retrieveProcess(processQueue, pageNode->data->pid);
		enQueue(processNode->process->queue, createPage(pageNode->data->pid, pageNode->data->page, pageNode->data->frame));
		pageNode = pageNode->next;
	}

	printLocal(processQueue);
}


////////////////////////////////////////
/////////// Page Queue /////////////////
////////////////////////////////////////

PageNode *createPage(int pid, int page, int frame){
	Page *pageData = (Page*)malloc(sizeof(Page));
	PageNode *pageNode = (PageNode*)malloc(sizeof(PageNode));

	pageData->pid = pid;
	pageData->page = page;
	pageData->frame = frame;

	pageNode->data = pageData;
	pageNode->next = NULL;

	return pageNode;
}

PageQueue *createPageQueue(){
	PageQueue *queue = (PageQueue*)malloc(sizeof(PageQueue));

	queue->front = queue->rear = NULL;

	return queue;

}

void enQueue(PageQueue *queue, PageNode *node){

	node->next = NULL;
	if(queue->front == NULL){
		queue->front = queue->rear = node;
		return;
	}

	queue->rear->next = node;
	queue->rear = node;

}

PageNode *dequeue(PageQueue *queue){
    if (queue->front == NULL)
       return NULL;

	PageNode *node;

	node = queue->front;
	queue->front = queue->front->next;
	return node;

}

PageNode *retrievePage(PageQueue *queue, int pid, int page){
    if (queue->front == NULL)
       return NULL;

	PageNode *current;
	current = queue->front;

	while(current != NULL){
		if(current->data->pid == pid && current->data->page == page){
			return current;
		} else {
			current = current->next;
		}
	}

	return NULL;
}


PageNode *removePage(PageQueue *queue, int pid, int page){
    if (queue->front == NULL)
       return NULL;

	PageNode *current, *previous;
	previous = NULL;
	current = queue->front;

	while(current != NULL){
		if(current->data->pid == pid && current->data->page == page) {
			if(current->next == NULL){
				queue->rear = previous;
			}
			if(previous != NULL){
				previous->next = current->next;
			} else {
				queue->front = current->next;
			}
			return current;
		}
		previous = current;
		current = current->next;
	}
	return NULL;
}

void pageSwap(PageNode *page1, PageNode *page2){
	Page *temp = page1->data;
	page1->data = page2->data;
	page2->data = temp;
}

void pageSort(PageNode *node){

	int sorted;
	PageNode *first;
	PageNode *last = NULL;

	if(node == NULL)
		return;

	do{
		sorted = 0;
		first = node;

		while(node->next != last){
			if(first->data->page > first->next->data->page){
				pageSwap(first, first->next);
				sorted = 1;
			}
			first = first->next;
		}
		last = first;
	}while(sorted);

}

////////////////////////////////////////
/////////// Process Queue //////////////
////////////////////////////////////////

ProcessNode *createProcess(int pid){
	Process *process = (Process*)malloc(sizeof(Process));
	ProcessNode *node = (ProcessNode*)malloc(sizeof(ProcessNode));

	process->pid = pid;
	process->pageFaults = 0;
	process->frameCount = 0;
	process->queue = createPageQueue();

	node->process = process;
	node->next = NULL;

	return node;
}

ProcessQueue *createProcessQueue() {
	ProcessQueue *queue = (ProcessQueue*)malloc(sizeof(ProcessQueue));

	queue->front = queue->rear = NULL;

	return queue;
}

void enqueueProcess(ProcessQueue *queue, ProcessNode *node){

	node->next = NULL;
	if(queue->front == NULL){
		queue->front = queue->rear = node;
		return;
	}
	queue->rear->next = node;
	queue->rear = node;

}

ProcessNode *retrieveProcess(ProcessQueue *queue, int pid){

	ProcessNode *current;
	current = queue->front;

	while(current != NULL){
		if(current->process->pid == pid){
			return current;
		}
		current = current->next;
	}
	return NULL;
}

void processSwap(ProcessNode *pid1, ProcessNode *pid2){
	Process *temp = pid1->process;
	pid1->process = pid2->process;
	pid2->process = temp;

}

void processSort(ProcessNode *node){
	int sorted;

	ProcessNode *first;
	ProcessNode *last = NULL;

	if(node == NULL)
		return;

	do{
		sorted = 0;
		first = node;

		while(first->next != last){
			if(first->process->pid > first->next->process->pid){
				processSwap(first, first->next);
				sorted = 1;
			}
			first = first->next;
		}
		last = first;
	} while (sorted);
}
