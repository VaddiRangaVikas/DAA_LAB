#include <stdio.h>
int main() {
    int n, i, key, found = 0;
    int a[50];
    printf("Enter number of elements: ");
    scanf("%d", &n);
    printf("Enter %d elements: ", n);
    for (i = 0; i < n; i++) {
        scanf("%d", &a[i]);
    }
    printf("Enter element to search: ");
    scanf("%d", &key);
    for (i = 0; i < n; i++) {
        if (a[i] == key) {
            found = 1;
            printf("Element found at position %d", i + 1);
            break;
        }
    }
    if (found == 0) {
        printf("Element not found");
    }
    return 0;
}
