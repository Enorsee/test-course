int sum_to_n(int n) {
    int sum = 0;

    for (int i = 0; i < n; i = i + 1) {
        if (i % 2 == 0) {
            sum = sum + i;
        } else {
            continue;
        }
    }

    return sum;
}
