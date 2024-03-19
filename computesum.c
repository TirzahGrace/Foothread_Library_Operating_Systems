#include <stdio.h>
#include <stdlib.h>
#include "foothread.h"
#define MAX_NODES 100 // Maximum number of nodes in the tree

// Structure to represent a node in the tree
typedef struct TreeNode {
    int value;              // Holds user input if it's a leaf node, otherwise stores partial sum
    int parent;             // Parent node index
    int num_children;       // Number of children
    int *children;          // Array of children node indices
    int status;     // 0 -> not done, 1-> done.
} TreeNode;

typedef struct node_arg{
    TreeNode *tree;
    int index;
}node_arg;

// Function prototypes
void readTreeFromFile(char *filename, int *n, int parent[]);
int initializeTree(TreeNode tree[], int parent[], int n );
int nodeThread(void *arg) ;
int global_sum = 0, leaves = 0;

foothread_mutex_t node_mutex;

int main() {
    int n;                      // Number of nodes
    int parent[MAX_NODES];      // Array to store parent representation of the tree
    TreeNode tree[MAX_NODES];   // Array of tree nodes

    // Read tree from file
    readTreeFromFile("tree.txt", &n, parent);
    foothread_mutex_init(&node_mutex);

    // Initialize tree structure
    int root = initializeTree(tree, parent, n);
    node_arg argument[n] ;
    
    foothread_t threads[n];
    // Create threads for each node
    for (int i = 0; i < n; i++) {
        foothread_attr_t attr = FOOTHREAD_ATTR_INITIALIZER;
        argument[i].index = i; argument[i].tree = tree ;
        foothread_create(&threads[i], &attr, nodeThread, (void *)&argument[i]);
    }

    while(tree[root].status == 0 ){ }

    if((tree[root].value) != global_sum) printf("NOT CORRECT\n");
    else printf("CORRECT\n");

    foothread_mutex_destroy(&node_mutex);
    return 0;
}

// Function to read tree representation from file
void readTreeFromFile(char *filename, int *n, int parent[]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "%d", n);
    for (int i = 0; i < *n; i++) {
        fscanf(file, "%d %d", &i, &parent[i]);
    }

    fclose(file);
}

// Function to initialize tree structure
int  initializeTree(TreeNode tree[], int parent[], int n) {
    
    int root = -1;
    for (int i = 0; i < n; i++) {
        tree[i].value = 0;
        tree[i].parent = parent[i];
        tree[i].num_children = 0;
        tree[i].children = NULL;
        tree[i].status = 0;
    }

    // Count number of children for each node
    for (int i = 0; i < n; i++) {
        if (parent[i] != i) {
            tree[parent[i]].num_children++;
        }
        else{
            root = i;
        }
    }

    // Allocate memory for children arrays
    for (int i = 0; i < n; i++) {
        if (tree[i].num_children > 0) {
            tree[i].children = (int*)malloc(tree[i].num_children * sizeof(int));
        }
        else leaves++;
    }

    // Populate children arrays
    for (int i = 0; i < n; i++) {
        if (parent[i] != i) {
            int parentIndex = parent[i];
            tree[parentIndex].children[tree[parentIndex].num_children - 1] = i;
            tree[parentIndex].num_children--;
        }
    }

    // Count number of children for each node again, cause they were used up previously.
    for (int i = 0; i < n; i++) {
        if (parent[i] != i) {
            tree[parent[i]].num_children++;
        }
    }

    return root;

}

// Function to handle execution of each node thread
int nodeThread(void *arg) {
    node_arg *argument = (node_arg *)arg;
    TreeNode * node = argument->tree; 
    int index = argument->index;
    if (node[index].num_children == 0) {
        // Leaf node
        foothread_mutex_lock(&node_mutex);
        printf("Leaf node %d :: Enter a positive integer: ", index);
        scanf("%d", &(node[index].value));
        global_sum = global_sum + node[index].value;
        leaves--;
        node[index].status = 1;
        // printf("node index: %d -> value: %d\n", index, node[index].value);
        foothread_mutex_unlock(&node_mutex);
    } else {
        while(1){
            if(leaves > 0) continue;
        // Parent node
        while (1) {
            int allChildrenDone = 1;
            for (int i = 0; i < node[index].num_children; i++) {
                foothread_mutex_lock(&node_mutex);
                if (node[node[index].children[i]].status == 0) {
                    allChildrenDone = 0;
                }
                foothread_mutex_unlock(&node_mutex);
            }
            if (allChildrenDone) {
                break;
            }
        }

        // Calculate sum of children's values
        int sum = 0;
        for (int i = 0; i < node[index].num_children; i++) {
            sum += node[node[index].children[i]].value;
        }

        foothread_mutex_lock(&node_mutex);
        // Assign sum as value of the parent node
        node[index].value = sum;
        printf("Internal node %d gets the partial sum %d from its children.\n", index, node[index].value);
        if(index == node[index].parent)
        {
            // root node.
            // Print the total sum at the root node
            printf("Sum at root(node %d) = %d\n", index, node[index].value);
        }
        node[index].status = 1; // Set status to done
        foothread_mutex_unlock(&node_mutex);
        break;
        } 
    }
    return 0;
}
