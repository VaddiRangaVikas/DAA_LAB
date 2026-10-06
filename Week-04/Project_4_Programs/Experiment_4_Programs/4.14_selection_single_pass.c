#include <stdio.h>
int main() {
    int n, i, minIndex, temp;
    int a[50];
    printf("Enter number of elements: ");
    scanf("%d", &n);
    printf("Enter %d elements: ", n);
    for (i = 0; i < n; i++) {
        scanf("%d", &a[i]);
    }
    minIndex = 0;
    for (i = 1; i < n; i++) {
        if (a[i] < a[minIndex]) {
            minIndex = i;
        }
    }
    temp = a[0];
    a[0] = a[minIndex];
    a[minIndex] = temp;
    printf("Array after selecting first minimum: ");
    for (i = 0; i < n; i++) {
        printf("%d ", a[i]);
    }
    return 0;
}
