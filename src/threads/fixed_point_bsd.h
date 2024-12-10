#define F (1 << 14) 

int convert_to_fixed_point(int n);
int convert_x_to_integer_toward_zero(int x);
int convert_x_to_integer_nearest(int x);
int add_x_y(int x, int y);
int substract_y_from_x(int x, int y);
int add_x_n(int x, int n);
int substract_n_x(int x, int n);
int multiply_x_y(int x, int y);
int multiply_x_n(int x, int n);
int divide_x_y(int x, int y);
int divide_x_n(int x, int n);


int convert_to_fixed_point(int n) {
    return n * F;
}

int convert_x_to_integer_toward_zero(int x) {
    return x / F;
}

int convert_x_to_integer_nearest(int x) {
    if (x >= 0) {
        return (x + F / 2) / F;
    } else {
        return (x - F / 2) / F;
    }
}

int add_x_y(int x, int y) {
    return x + y;
}

int substract_y_from_x(int x, int y) {
    return x - y;
}

int add_x_n(int x, int n) {
    return x + n * F;
}

int substract_n_x(int x, int n) {
    return x - n * F;
}

int multiply_x_y(int x, int y) {
    return ((int64_t) x) * y / F;
}

int multiply_x_n(int x, int n) {
    return x * n;
}

int divide_x_y(int x, int y) {
    return ((int64_t) x) * F / y;
}

int divide_x_n(int x, int n) {
    return x / n;
}

