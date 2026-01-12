
int main() {
    int i = 1;

    for (i; i <= 100; i = i + 1) {
        if (i % 15 == 0) {
            print_str("FizzBuzz\n");
        } else if (i % 3 == 0) {
            print_str("Fizz\n");
        } else if (i % 5 == 0) {
            print_str("Buzz\n");
        } else {
            print_int(i);
            print_str("\n");
        }
    }
    return 0;
}
