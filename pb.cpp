#include <stdio.h>
#include <iostream>
#include <windows.h>

void DoProgress(int t, int n) {
    putchar('[');
    for (int i = 0; i < n; i++) {
        putchar(i < t ? '>' : ' '); // 输出> 或者 ' '
    }
    putchar(']');
    printf("%3d%%",(int)((double(t)/n) *100));

    // 光标回退，实现刷新
    for (int i = 0; i != n + 6; i++) {
        putchar('\b');
    }

}

int main() {
    printf("here is pb\n");
    for (int i = 0; i < 100; i++) {
        DoProgress(i, 100); // 显示进度条
        fflush(stdout);
        Sleep(1000); // 每次显示延迟1s
    }
    return 0;
}