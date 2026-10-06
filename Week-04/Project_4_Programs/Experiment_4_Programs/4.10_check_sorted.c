#include <stdio.h>
int main() {
    int n, i, sorted = 1;
    int a[50];
    printf("Enter number of elements: ");
    scanf("%d", &n);
    printf("Enter %d elements: ", n);
    for (i = 0; i < n; i++) {
        scanf("%d", &a[i]);
    }
    for (i = 0; i < n - 1; i++) {
        if (a[i] > a[i + 1]) {
            sorted = 0;
            break;
        }
    }
    if (sorted == 1) {
        printf("Array is already sorted in ascending order");
    } else {
        printf("Array is not sorted");
    }
    return 0;
}
