#include <stdio.h>
int main() {
    int n, i;
    int a[50];
    printf("Enter number of elements: ");
    scanf("%d", &n);
    printf("Enter %d elements: ", n);
    for(i = 0; i < n; i++) {
        scanf("%d", &a[i]);
    }
    printf("Array elements are: ");
    for(i = 0; i < n; i++) {
        printf("%d ", a[i]);
    }
    return 0;
}
