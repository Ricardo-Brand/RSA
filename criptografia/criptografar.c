#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef unsigned long long ull;
typedef __uint128_t u128;

// Converte uma string decimal para u128. Retorna 1 se for válido, 0 caso contrário.
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

// Lê uma linha do usuário com prompt e converte para u128.
int read_u128(const char *prompt, u128 *out) {
    char buffer[256];

    printf("%s", prompt);

    if (!fgets(buffer, sizeof(buffer), stdin))
        return 0;

    return parse_u128(buffer, out);
}

// Imprime um valor u128 com zero à esquerda para manter todos os blocos com a mesma largura.
void print_padded_u128(u128 value, int width) {
    char buffer[64];
    int i = 0;

    if (value == 0) {
        buffer[i++] = '0';
    } else {
        while (value > 0) {
            buffer[i++] = '0' + (int)(value % 10);
            value /= 10;
        }
    }

    int padding = width - i;

    while (padding-- > 0)
        putchar('0');

    while (i--)
        putchar(buffer[i]);
}

// ============= FUNÇÕES MATEMÁTICAS =============

// Realiza a multiplicação modular (a * b) % mod sem overflow direto de u128.
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
// Este método é usado para cifrar e decifrar no RSA.
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

// Calcula o máximo divisor comum estendido entre a e b.
// Retorna g = gcd(a, b) e encontra os coeficientes x, y tais que ax + by = g.
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

// Criptografa um único caractere usando a chave pública (n, e).
// Converte o caractere para seu valor ASCII (0-255) e aplica a exponenciação modular.
// Exemplo: 'A' (ASCII 65) → 65^e mod n
u128 encrypt_char(char c, u128 n, u128 e) {
    u128 m = (u128)(unsigned char)c;
    return mod_exp(m, e, n);
}

// Retorna a quantidade de dígitos decimais de um valor u128.
// Usado para padronizar o comprimento de cada bloco criptografado.
int count_digits_u128(u128 value) {
    int digits = 1;

    while (value >= 10) {
        value /= 10;
        digits++;
    }

    return digits;
}

// ============= PROGRAMA PRINCIPAL =============

// Programa principal de criptografia RSA.
// Lê a chave pública (n, e) e um texto do usuário, criptografa cada caractere
// e imprime o resultado como blocos numéricos com largura fixa.
int main() {
    u128 n, e;
    char *texto = NULL;
    size_t texto_size = 0, tamanho_texto = 0;

    printf("\n========== CRIPTOGRAFIA RSA ==========\n\n");

    if (!read_u128("Informe a chave pública N: ", &n)) {
        fprintf(stderr, "Erro: entrada inválida para N. Digite apenas números.\n");
        return 1;
    }

    if (!read_u128("Informe a chave pública E: ", &e)) {
        fprintf(stderr, "Erro: entrada inválida para E. Digite apenas números.\n");
        return 1;
    }

    // Validações básicas das chaves
    if (n < 2) {
        fprintf(stderr, "Erro: N deve ser maior que 1\n");
        return 1;
    }

    if (e < 2 || e >= n) {
        fprintf(stderr, "Erro: E deve estar entre 2 e N-1\n");
        return 1;
    }

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
    tamanho_texto = status;
    if (tamanho_texto > 0 && texto[tamanho_texto - 1] == '\n') {
        texto[tamanho_texto - 1] = '\0';
        tamanho_texto--;
    }

    // Criptografa cada caractere (letra, número ou símbolo) individualmente.
    // Cada caractere é convertido para seu código ASCII e criptografado.
    // Exemplo: 'h' (ASCII 104) → bloco criptografado de números
    printf("\nTEXTO CRIPTOGRAFADO:\n\n");
    u128 max_value = (n > 0 ? n - 1 : 0);
    int width = count_digits_u128(max_value);

    for (int i = 0; i < (int)tamanho_texto; i++) {
        u128 criptografado = encrypt_char(texto[i], n, e);
        print_padded_u128(criptografado, width);
    }
    printf("\n\n");

    free(texto);
    return 0;
}
