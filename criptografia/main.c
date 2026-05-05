#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

long long extended_gcd(long long a, long long b, long long *x, long long *y) {
    if (b == 0) {
        *x = 1;
        *y = 0;
        return a;
    }

    long long x1, y1;
    long long g = extended_gcd(b, a % b, &x1, &y1);

    *x = y1;
    *y = x1 - (a / b) * y1;

    return g;
}

// ============= CRIPTOGRAFIA RSA =============

// Criptografa um caractere usando a chave pública (n, e)
ull encrypt_char(char c, ull n, ull e) {
    ull m = (ull)c;
    return mod_exp(m, e, n);
}

// ============= PROGRAMA PRINCIPAL =============

int main() {
    ull n, e;
    char *texto = NULL;
    size_t texto_size = 0, tamanho_texto = 0;

    printf("\n========== CRIPTOGRAFIA RSA ==========\n\n");

    printf("Informe a chave pública N: ");
    scanf("%llu", &n);
    printf("Informe a chave pública E: ");
    scanf("%llu", &e);
    getchar(); // Limpar o buffer

    printf("Informe o texto a criptografar: ");
    ssize_t status = getline(&texto, &texto_size, stdin);

    if (status == -1 || texto == NULL) {
        fprintf(stderr, "Erro ao ler o texto\n");
        return 1;
    }

    if (status == 1 && texto[0] == '\n') {
        fprintf(stderr, "Entrada vazia\n");
        return 1;
    }

    // Remove a quebra de linha do final
    tamanho_texto = texto_size;
    if (tamanho_texto > 0 && texto[tamanho_texto - 1] == '\n') {
        texto[tamanho_texto - 1] = '\0';
        tamanho_texto--;
    }

    // Criptografa cada caractere
    printf("\nTEXTO CRIPTOGRAFADO:\n");
    for (int i = 0; i < tamanho_texto; i++) {
        ull criptografado = encrypt_char(texto[i], n, e);
        printf("%llu ", criptografado);
    }
    printf("\n\n");

    free(texto);
    return 0;
}
