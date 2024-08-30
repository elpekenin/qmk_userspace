#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

// defined by zig
extern void zig_main(void);
extern void qp_set_device_by_name(const char *name, void *ptr);
extern void *qp_get_device_by_name(const char *name);

// stubs
uint16_t qp_get_width() { return 0; }
uint16_t qp_get_height() { return 0; }
bool qp_rect() { return true; }
bool qp_power() { return true; }


// stub for the logging function, zig will call it
int logging(int _1, int _2, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
    printf("\n");
    return 0;
}

// do some tests
#define test(x) do { \
    if (!(x)) { \
        return 1; \
    } \
} while(0)

int main() {
    zig_main();

    char NAME[] = "testing";
    void *in = (void *)1234;
    qp_set_device_by_name(NAME, in);
    test(qp_get_device_by_name(NAME) == in);

    test(qp_get_device_by_name("404") == NULL);

    return 0;
}
