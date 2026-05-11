int analyze(int x, int y) {
    int total = 0;

    if (x > y) {
        total = x - y;
    } else {
        total = y - x;
    }

    while (total > 0) {
        if (total % 2 == 0) {
            total = total - 2;
        } else {
            total = total - 1;
        }
    }

    return total;
}
