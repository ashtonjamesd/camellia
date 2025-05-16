int main() {
    int x = 2;
    int c = 0;

    while (c < 4) {
        x = x * 2;

        if (x == 32) {
            return x;
        }

        c = c + 1;
    }

    return x;
}