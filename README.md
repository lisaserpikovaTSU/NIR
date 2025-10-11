Пример кода в формате JSON находится в файле primer.json
КОД НА СИ:

#include <stdio.h> 
#include <stdlib.h>

// Структура для узла списка 
struct Node {
  int data;
  struct Node* next; 
};

int main() {
// Создаем голову списка и инициализируем её как NULL (пустой список) 
  struct Node* head = NULL;
  
  // 1. Добавляем узел со значением 3
  struct Node* newNode = (struct Node*)malloc(sizeof(struct Node)); 
  newNode->data = 3;
  newNode->next = NULL;
  head = newNode;
  
  // 2. Добавляем узел со значением 7 в начало списка 
  newNode = (struct Node*)malloc(sizeof(struct Node)); 
  newNode->data = 7;
  newNode->next = head;
  head = newNode;
  
  // Теперь список: 7 -> 3 -> NULL
  
  // 3. Добавляем узел со значением 1 между 7 и 3 
  newNode = (struct Node*)malloc(sizeof(struct Node)); 
  newNode->data = 1;
  newNode->next = head->next; // 1 указывает на 3 
  head->next = newNode; // 7 теперь указывает на 1
  
  // Теперь список: 7 -> 1 -> 3 -> NULL
  
  // 4. Удаляем узел со значением 7 (опционально, если нужно) 
  struct Node* temp = head;
  head = head->next; // Теперь голова — 1
  free(temp);
  
  // Теперь список: 1 -> 3 -> NULL
  
  // 5. Добавляем узел со значением 5 в конец 
  newNode = (struct Node*)malloc(sizeof(struct Node)); 
  newNode->data = 5;
  newNode->next = NULL;
  
  struct Node* current = head; 
  while (current->next != NULL) {
    current = current->next; 
  }
  current->next = newNode;
  
  // Теперь список: 1 -> 3 -> 5 -> NULL
  
  // 6. Меняем местами первый и второй узлы
  if (head != NULL && head->next != NULL) {
    struct Node* first = head;
    struct Node* second = head->next;
    first->next = second->next; 
    second->next = first;
    head = second;
  }
  
  // Теперь список: 3 -> 1 -> 5 -> NULL
  
  // 7. Выводим список
  printf("Список после всех операций:\n"); 
  current = head;
  while (current != NULL) {
    printf("%d ", current->data);
    current = current->next; 
  }
  printf("\n");
  
  // 8. Освобождаем память current = head;
  while (current != NULL) {
    struct Node* temp = current; 
    current = current->next; 
    free(temp);
  }
  
  return 0; 
}
