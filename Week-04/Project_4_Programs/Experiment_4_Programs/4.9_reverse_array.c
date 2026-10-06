#include <stdio.h>
int main() {
    int n, i;
    int a[50];
    printf("Enter number of elements: ");
    scanf("%d", &n);
    printf("Enter %d elements: ", n);
    for (i = 0; i < n; i++) {
        scanf("%d", &a[i]);
    }
    printf("Array in reverse order: ");
    for (i = n - 1; i >= 0; i--) {
        printf("%d ", a[i]);
    }
    return 0;
}
