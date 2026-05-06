#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

typedef unsigned long long ull;

int parse_ull(const char *str, ull *out) {
    char *endptr;
    errno = 0;
    unsigned long long value = strtoull(str, &endptr, 10);
    if (errno == ERANGE) return 0;
    if (endptr == str) return 0;
    while (*endptr == ' ' || *endptr == '\t') endptr++;
    if (*endptr != '\n' && *endptr != '\0') return 0;
    *out = (ull)value;
    return 1;
}

int read_ull(const char *prompt, ull *out) {
    char buffer[128];
    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
    return parse_ull(buffer, out);
}

// ============= FUNÇÕES MATEMÁTICAS =============

ull mul_mod(ull a, ull b, ull mod) {
    ull result = 0;
    a %= mod;

    while (b > 0) {
        if (b & 1)
            result = (result + a) % mod;

        a = (a << 1) % mod;
        b >>= 1;
    }

    return result;
}

ull mod_exp(ull base, ull exp, ull mod) {
    ull result = 1;
    base %= mod;

    while (exp > 0) {
        if (exp & 1)
            result = mul_mod(result, base, mod);

        base = mul_mod(base, base, mod);
        exp >>= 1;
    }

    return result;
}

// Descriptografa um bloco usando a chave privada (n, d)
unsigned char decrypt_char(ull c, ull n, ull d) {
    ull m = mod_exp(c, d, n);
    return (unsigned char)(m & 0xFF);
}

int count_digits(ull value) {
    int digits = 1;
    while (value >= 10) {
        value /= 10;
        digits++;
    }
    return digits;
}

int main() {
    ull n, d;
    char *ciphertext = NULL;
    size_t bufsize = 0;
    ssize_t status;

    printf("\n========== DESCRIPTOGRAFIA RSA ==========\n\n");

    if (!read_ull("Informe a chave privada N: ", &n)) {
        fprintf(stderr, "Erro: entrada inválida para N. Digite apenas números.\n");
        return 1;
    }

    if (!read_ull("Informe a chave privada D: ", &d)) {
        fprintf(stderr, "Erro: entrada inválida para D. Digite apenas números.\n");
        return 1;
    }

    if (n < 2) {
        fprintf(stderr, "Erro: N deve ser maior que 1\n");
        return 1;
    }

    if (d < 2 || d >= n) {
        fprintf(stderr, "Erro: D deve estar entre 2 e N-1\n");
        return 1;
    }

    printf("Informe o texto criptografado: ");
    status = getline(&ciphertext, &bufsize, stdin);
    if (status == -1 || ciphertext == NULL) {
        fprintf(stderr, "Erro ao ler o texto criptografado\n");
        free(ciphertext);
        return 1;
    }

    if (status == 1 && ciphertext[0] == '\n') {
        fprintf(stderr, "Entrada vazia\n");
        free(ciphertext);
        return 1;
    }

    size_t length = strlen(ciphertext);
    if (length > 0 && ciphertext[length - 1] == '\n') {
        ciphertext[length - 1] = '\0';
        length--;
    }

    ull max_value = (n > 0 ? n - 1 : 0);
    int width = count_digits(max_value);
    if (width <= 0) {
        fprintf(stderr, "Erro interno: largura de bloco inválida\n");
        free(ciphertext);
        return 1;
    }

    if (length % width != 0) {
        fprintf(stderr, "Erro: texto criptografado inválido. Deve ser múltiplo de %d dígitos.\n", width);
        free(ciphertext);
        return 1;
    }

    printf("\nTEXTO DESCRIPTOGRAFADO:\n\n");
    for (size_t pos = 0; pos < length; pos += width) {
        char block[32];
        if (width >= (int)sizeof(block)) {
            fprintf(stderr, "Erro interno: bloco muito grande\n");
            free(ciphertext);
            return 1;
        }

        memcpy(block, ciphertext + pos, width);
        block[width] = '\0';

        for (int i = 0; i < width; i++) {
            if (!isdigit((unsigned char)block[i])) {
                fprintf(stderr, "Erro: texto criptografado contém caractere inválido\n");
                free(ciphertext);
                return 1;
            }
        }

        char *endptr;
        errno = 0;
        unsigned long long parsed = strtoull(block, &endptr, 10);
        if (errno == ERANGE || endptr != block + width) {
            fprintf(stderr, "Erro: bloco criptografado inválido\n");
            free(ciphertext);
            return 1;
        }

        ull value = (ull)parsed;
        if (value >= n) {
            fprintf(stderr, "Erro: bloco criptografado inválido (maior que N-1)\n");
            free(ciphertext);
            return 1;
        }

        char plain = decrypt_char(value, n, d);
        putchar(plain);
    }

    printf("\n\n");
    free(ciphertext);
    return 0;
}
