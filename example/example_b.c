int main() {
    int x = 10;
    int y = 5;

    if (1) {
        x = x - 2;

        if (0) {
            x = x - 2;
        } else {
            x = x + 2;
        }

        if (1) {
            x = x + y;
        }

        x = x + 1;

    } else {
        x = x + 2;
    }

    return x + y; // 21
}