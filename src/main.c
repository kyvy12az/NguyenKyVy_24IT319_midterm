#include "listing.h"
#include "options.h"

#include <locale.h>

int main(int argc, char **argv) {
    Options options;
    int first_operand;

    setlocale(LC_ALL, "");
    options_init(&options);
    first_operand = options_parse(&options, argc, argv);
    if (first_operand < 0) {
        return 2;
    }

    return list_operands(argc - first_operand, argv + first_operand, &options);
}
