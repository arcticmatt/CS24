int gcd(int a, int b) {
    int remainder;

    if (b == 0) {
        return a;
    } else {
        remainder = a - (a / b);
        return gcd(b, remainder);
    }
}
