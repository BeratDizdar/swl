```c
#include "swl.h"

int main() {
    swl_CreateWindow("X", 400, 300);

    for (;!swl_ShouldClose();) {
        if (swl_IsKeyPressed(27)) swl_SendQuitEvent();

        swl_PollEvents();
        swl_PassScheduler();
    }

    swl_CloseWindow();

    return 0;
}
```
