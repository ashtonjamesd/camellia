void write(char c) {
    asm(
        "mov rax, 2"
    );

    asm(
        "mov rax, 5"
    );
}

int main() {
    return 0;
}