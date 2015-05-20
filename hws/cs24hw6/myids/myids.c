#include <stdio.h>

void get_ids(int *uid, int *gid);

int main() {
    printf("Running myids\n");

    int uid, gid;
    // Call the assembly function I wrote
    get_ids(&uid, &gid);
    printf("User ID is %d. Group ID is %d.\n", uid, gid);

    return 0;
}
