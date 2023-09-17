#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "concurrent_list.h"

struct node {
	int value;

	// add more fields

	pthread_mutex_t lock;
	struct node *next;


};

struct list {
	// add fields

	struct node *head;

};

void print_node(node* node)
{
	// DO NOT DELETE
	if (node)
	{
		printf("%d ", node->value);
	}
}

list* create_list()
{
	// add code here

	struct list *myList = (list *)malloc(sizeof(list));
	if (myList == NULL)
	{
		printf("Error! memory not allocated.");
		exit(0);
	}
	myList->head = NULL;

	return myList;

	// REPLACE WITH YOUR OWN CODE
}

void delete_list(list* list)													 
{

	// add code here

	if (list == NULL)  
		return;

	if (list->head == NULL)
	{
		free(list);
		return;
	}

	if (list != NULL)
	{
		struct node *tptr;
		struct node *next;

		if (list->head != NULL)
		{
			tptr = list->head;
			next = list->head;
			pthread_mutex_lock(&(tptr->lock));
		}

		while (tptr != NULL)
		{
			next = tptr->next;
			if (next != NULL)
			{
				pthread_mutex_lock(&(next->lock));
			}

			pthread_mutex_unlock(&(tptr->lock));
			pthread_mutex_destroy(&(tptr->lock));
			free(tptr);
			tptr = next;
		}

		list->head = NULL;
		free(list);
	}

}


void print_list(list* list)
{
	// add code here
	if (list != NULL)
	{
		struct node *ptrtemp;
		struct node *pr;
		ptrtemp = list->head;
		pr = list->head;
		if (ptrtemp != NULL)
		{
			pthread_mutex_lock(&(ptrtemp->lock));
		}
		while (ptrtemp != NULL)
		{
			print_node(ptrtemp);
			pr = ptrtemp;
			ptrtemp = ptrtemp->next;
			if (ptrtemp != NULL)
			{
				pthread_mutex_lock(&(ptrtemp->lock));
			}
			pthread_mutex_unlock(&(pr->lock));
		}
	}

	printf("\n"); // DO NOT DELETE
}

void insert_value(list* list, int value)
{
	// add code here

	if (list == NULL)
		return;

	if (list != NULL)
	{
		struct node *newNode = (node *)malloc(sizeof(node));
		if (newNode == NULL)
		{
			printf("Error! memory not allocated.");
			exit(0);
		}
		newNode->value = value;
		pthread_mutex_init(&(newNode->lock), NULL);
		pthread_mutex_lock(&(newNode->lock));


		struct node *ptr2 = list->head;
		struct node *last = list->head;;
		if (ptr2 != NULL)
		{
			pthread_mutex_lock(&(ptr2->lock));
		}
		int coun = 0;
		if (ptr2 == NULL)
		{
			list->head = newNode;
			newNode->next = NULL;
			pthread_mutex_unlock(&(newNode->lock));

		}
		else
		{
			while (ptr2 != NULL)
			{

				if (ptr2->value >= value)
				{
					if (coun == 0)							  
					{
						newNode->next = ptr2;
						list->head = newNode;
						pthread_mutex_unlock(&(newNode->lock));
					}

					else                                     
					{
						last->next = newNode;
						newNode->next = ptr2;
						pthread_mutex_unlock(&(last->lock));
						pthread_mutex_unlock(&(newNode->lock));

					}
					pthread_mutex_unlock(&(ptr2->lock));
					break;
				}
				else
				{
					if (coun != 0)
					{
						pthread_mutex_unlock(&(last->lock));
					}
					if (ptr2->next == NULL)					 
					{
						ptr2->next = newNode;
						newNode->next = NULL;
						pthread_mutex_unlock(&(newNode->lock));
						pthread_mutex_unlock(&(ptr2->lock));
						break;
					}
					last = ptr2;

					ptr2 = ptr2->next;
					if (ptr2 != NULL)
					{
						pthread_mutex_lock(&(ptr2->lock));
					}
				}
				coun++;
			}
		}
	}
}


void remove_value(list* list, int value)
{
	// add code here

	if (list == NULL)
		return;

	if (list != NULL)
	{
        int ind = 0;
        struct node *prev = list->head;
		struct node *ptrtmp = list->head;
		struct node *next = list->head;
		if (ptrtmp != NULL)
		{
			pthread_mutex_lock(&(ptrtmp->lock));
			if (ptrtmp->next != NULL)
			{
				pthread_mutex_lock(&(ptrtmp->next->lock));								
				next = ptrtmp->next;
			}
		}

		while (ptrtmp != NULL)
		{
			if (ptrtmp->value == value)
			{
				if (ind== 0)								
				{
					list->head = ptrtmp->next;
					if (ptrtmp->next != NULL)
					{
						pthread_mutex_unlock(&(ptrtmp->next->lock));
					}
					pthread_mutex_unlock(&(ptrtmp->lock));
					pthread_mutex_destroy(&(ptrtmp->lock));

				}

				else if (ptrtmp->next == NULL)				
				{
					prev->next = NULL;
					pthread_mutex_unlock(&(prev->lock));
					pthread_mutex_unlock(&(ptrtmp->lock));
					pthread_mutex_destroy(&(ptrtmp->lock));
				}
				else                                         
				{
					prev->next = ptrtmp->next;
					pthread_mutex_unlock(&(next->lock));
					pthread_mutex_unlock(&(prev->lock));
					pthread_mutex_unlock(&(ptrtmp->lock));
					pthread_mutex_destroy(&(ptrtmp->lock));
				}

				free(ptrtmp);
				break;
			}

			if (ind != 0)
			{
				pthread_mutex_unlock(&(prev->lock));
			}

			if (next != NULL)
			{
				if (next->next != NULL)
				{
					pthread_mutex_lock(&(next->next->lock));
				}
			}
			
			prev = ptrtmp;
			ptrtmp = ptrtmp->next;
			if (next != NULL)
			{
				next = next->next;
			}
			if (ptrtmp != NULL)
			{
				//DO NOTHING
			}
			else
			{
				pthread_mutex_unlock(&(prev->lock));
			}
			ind++;
		}

	}

}

void count_list(list* list, int(*predicate)(int))
{
	int count = 0; // DO NOT DELETE

	// add code here
	if (list != NULL)
	{
		struct node *ptrtohead = list->head;
		struct node *last = list->head;
		if (ptrtohead != NULL)
		{
			pthread_mutex_lock(&(ptrtohead->lock));
		}
		while (ptrtohead != NULL) {
			if (predicate(ptrtohead->value))
			{
				count++;
			}
			last = ptrtohead;
			ptrtohead = ptrtohead->next;
			if (ptrtohead != NULL)
			{
				pthread_mutex_lock(&(ptrtohead->lock));
			}
			pthread_mutex_unlock(&(last->lock));
		}
	}
  printf("%d items were counted\n", count); // DO NOT DELETE
}




