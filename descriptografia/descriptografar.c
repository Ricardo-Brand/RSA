#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

typedef unsigned long long ull;
typedef __uint128_t u128;

// Converte uma string decimal para um valor u128.
// Retorna 1 se a string for um número válido, 0 caso contrário.
int parse_u128(const char *str, u128 *out) {
    u128 result = 0;

    while (*str == ' ' || *str == '\t')
        str++;

    if (*str == '\0' || *str == '\n')
        return 0;

    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    while (*str == ' ' || *str == '\t')
        str++;

    if (*str != '\0' && *str != '\n')
        return 0;

    *out = result;
    return 1;
}

// Lê uma linha de entrada do usuário, exibe um prompt, e converte para u128.
int read_u128(const char *prompt, u128 *out) {
    char buffer[256];

    printf("%s", prompt);

    if (!fgets(buffer, sizeof(buffer), stdin))
        return 0;

    return parse_u128(buffer, out);
}

// ============= FUNÇÕES MATEMÁTICAS =============

// Multiplica dois inteiros grandes módulo mod de forma segura.
// Usa soma e deslocamento para evitar overflow direto em u128.
u128 mul_mod(u128 a, u128 b, u128 mod) {
    u128 result = 0;

    a %= mod;

    while (b > 0) {
        if (b & 1)
            result = (result + a) % mod;

        a = (a << 1) % mod;
        b >>= 1;
    }

    return result;
}

// Calcula base^exp mod mod usando exponenciação modular rápida.
// Essencial para RSA, onde potências grandes são reduzidas pelo módulo.
u128 mod_exp(u128 base, u128 exp, u128 mod) {
    u128 result = 1;
    base %= mod;

    while (exp > 0) {
        if (exp & 1)
            result = mul_mod(result, base, mod);

        base = mul_mod(base, base, mod);
        exp >>= 1;
    }

    return result;
}

// Descriptografa um bloco numérico usando a chave privada (n, d).
// Aplica a exponenciação modular ao bloco e retorna o caractere original.
// O resultado é convertido de volta para seu valor ASCII (0-255).
// Exemplo: bloco criptografado → 104 (ASCII) → caractere 'h'
unsigned char decrypt_char(u128 c, u128 n, u128 d) {
    u128 m = mod_exp(c, d, n);
    return (unsigned char)(m & 0xFF);
}

// Conta quantos dígitos decimais são necessários para representar o valor.
// Usado para determinar a largura fixa de cada bloco criptografado.
int count_digits_u128(u128 value) {
    int digits = 1;

    while (value >= 10) {
        value /= 10;
        digits++;
    }

    return digits;
}

// Programa principal de descriptografia RSA.
// Lê a chave privada (n, d) e o texto criptografado em blocos numéricos,
// decifra cada bloco e imprime o texto original.
int main() {
    u128 n, d;
    char *ciphertext = NULL;
    size_t bufsize = 0;
    ssize_t status;

    printf("\n========== DESCRIPTOGRAFIA RSA ==========\n\n");

    if (!read_u128("Informe a chave privada N: ", &n)) {
        fprintf(stderr, "Erro: entrada inválida para N. Digite apenas números.\n");
        return 1;
    }

    if (!read_u128("Informe a chave privada D: ", &d)) {
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

    u128 max_value = (n > 0 ? n - 1 : 0);
    int width = count_digits_u128(max_value);
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
        char block[128];
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

        u128 value;

        if (!parse_u128(block, &value)) {
            fprintf(stderr, "Erro: bloco criptografado inválido\n");
            free(ciphertext);
            return 1;
        }

        if (value >= n) {
            fprintf(stderr, "Erro: bloco criptografado inválido (maior que N-1)\n");
            free(ciphertext);
            return 1;
        }

        // Descriptografa o bloco numérico e converte de volta para o caractere original.
        // Cada bloco (números) é convertido para seu código ASCII e exibido como caractere.
        // Exemplo: bloco 104 (criptografado) → descriptografa → 104 → exibe 'h'
        unsigned char plain = decrypt_char(value, n, d);
        putchar(plain);
    }

    printf("\n\n");
    free(ciphertext);
    return 0;
}
