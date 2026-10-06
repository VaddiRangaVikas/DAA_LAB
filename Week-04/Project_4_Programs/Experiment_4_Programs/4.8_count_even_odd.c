#include <stdio.h>
int main() {
    int n, i, even = 0, odd = 0;
    int a[50];
    printf("Enter number of elements: ");
    scanf("%d", &n);
    printf("Enter %d elements: ", n);
    for (i = 0; i < n; i++) {
        scanf("%d", &a[i]);
        if (a[i] % 2 == 0) {
            even++;
        } else {
            odd++;
        }
    }
    printf("Even count = %d\n", even);
    printf("Odd count = %d", odd);
    return 0;
}
