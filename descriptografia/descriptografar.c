#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef unsigned long long ull;

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
ull decrypt_char(ull c, ull n, ull d) {
    ull m = mod_exp(c, d, n);
    return (char)(m & 0xFF);
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

    printf("Informe a chave privada N: ");
    if (scanf("%llu", &n) != 1) {
        fprintf(stderr, "Erro: entrada inválida para N. Digite apenas números.\n");
        return 1;
    }

    printf("Informe a chave privada D: ");
    if (scanf("%llu", &d) != 1) {
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

    getchar(); // Limpar o buffer

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

        ull value = strtoull(block, NULL, 10);
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
