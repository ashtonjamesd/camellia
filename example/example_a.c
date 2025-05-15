int get_five() {
    return 5;
}

int func() {
    int x = 10;
    int y = 10;

    return x + get_five() + 10 - y * 2 + 20;
}

int main() {
    return func(); // should be 25
}