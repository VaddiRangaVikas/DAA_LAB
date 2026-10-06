#include <stdio.h>
int main() {
    int n, i, sum = 0;
    int a[50];
    printf("Enter number of elements: ");
    scanf("%d", &n);
    printf("Enter %d elements: ", n);
    for(i = 0; i < n; i++) {
        scanf("%d", &a[i]);
        sum = sum + a[i];
    }
    printf("Sum = %d", sum);
    return 0;
}
