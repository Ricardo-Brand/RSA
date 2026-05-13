#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/rand.h>
#include <errno.h>

typedef unsigned long long ull;
typedef __uint128_t u128;
typedef __int128_t i128;

// Converte uma string decimal para unsigned long long.
// Retorna 1 se a conversão for bem-sucedida e 0 em caso de erro.
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

// Lê um valor unsigned long long do usuário com prompt.
int read_ull(const char *prompt, ull *out) {
    char buffer[128];
    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
    return parse_ull(buffer, out);
}

// Converte uma string decimal para int.
// Retorna 1 para sucesso e 0 para falha de entrada.
int parse_int(const char *str, int *out) {
    char *endptr;
    errno = 0;
    long value = strtol(str, &endptr, 10);
    if (errno == ERANGE) return 0;
    if (endptr == str) return 0;
    while (*endptr == ' ' || *endptr == '\t') endptr++;
    if (*endptr != '\n' && *endptr != '\0') return 0;
    *out = (int)value;
    return 1;
}

// Lê um valor int do usuário com prompt.
int read_int(const char *prompt, int *out) {
    char buffer[128];
    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
    return parse_int(buffer, out);
}

// Retorna um número aleatório de 64 bits usando OpenSSL RAND_bytes.
// Usado para gerar candidatos aos primos p e q e o expoente d.
unsigned long long secure_rand_ull() {
    unsigned long long val;
    if (RAND_bytes((unsigned char*)&val, sizeof(val)) != 1) {
        fprintf(stderr, "Erro em RAND_bytes\n");
        exit(1);
    }
    return val;
}

// Calcula (a * b) % mod usando u128 para evitar overflow.
ull mul_mod(ull a, ull b, ull mod) {
    return ((u128)a * b) % mod;
}

// Calcula base^exp mod mod usando exponenciação modular rápida.
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

// Teste de primalidade baseado em Miller-Rabin com testemunhos determinísticos
// para números de até 64 bits. Retorna 1 para primo e 0 para composto.
int is_prime(ull n) {
    if (n < 2) return 0;

    ull small_primes[] = {
        2ULL, 3ULL, 5ULL, 7ULL, 11ULL,
        13ULL, 17ULL, 19ULL, 23ULL,
        29ULL, 31ULL, 37ULL
    };

    for (int i = 0; i < 12; i++) {
        if (n == small_primes[i])
            return 1;

        if (n % small_primes[i] == 0)
            return 0;
    }

    ull d = n - 1;
    int s = 0;

    while ((d & 1) == 0) {
        d >>= 1;
        s++;
    }

    ull test_bases[] = {
        2ULL,
        3ULL,
        5ULL,
        7ULL,
        11ULL,
        13ULL,
        23ULL
    };

    for (int i = 0; i < 7; i++) {
        ull a = test_bases[i] % n;

        if (a == 0)
            continue;

        ull x = mod_exp(a, d, n);

        if (x == 1 || x == n - 1)
            continue;

        int composite = 1;

        for (int r = 1; r < s; r++) {
            x = mul_mod(x, x, n);

            if (x == n - 1) {
                composite = 0;
                break;
            }
        }

        if (composite)
            return 0;
    }

    return 1;
}

// Calcula o máximo divisor comum estendido de a e b.
// Retorna gcd(a, b) e calcula x, y tais que a*x + b*y = gcd(a, b).
i128 extended_gcd(i128 a, i128 b, i128 *x, i128 *y) {
    if (b == 0) {
        *x = 1;
        *y = 0;
        return a;
    }

    i128 x1, y1;
    i128 g = extended_gcd(b, a % b, &x1, &y1);

    *x = y1;
    *y = x1 - (a / b) * y1;

    return g;
}

// Calcula o inverso modular de d modulo z.
// Retorna -1 se o inverso não existir.
i128 mod_inverse(i128 d, i128 z) {
    i128 x, y;
    i128 g = extended_gcd(d, z, &x, &y);

    if (g != 1) return -1;

    return (x % z + z) % z;
}

// Gera um número primo aleatório no intervalo [min, max].
// Testa apenas candidatos ímpares com Miller-Rabin.
ull generate_prime(ull min, ull max) {
    ull r, num;

    do {
        r = secure_rand_ull();
        num = min + (r % (max - min));
        num |= 1ULL; // Garante que seja ímpar
    } while (!is_prime(num));

    return num;
}

// Calcula o máximo divisor comum (gcd) de dois valores u128.
u128 gcd(u128 a, u128 b) {
    while (b != 0) {
        u128 t = b;
        b = a % b;
        a = t;
    }
    return a;
}

// Gera um expoente privado d tal que gcd(d, z) == 1.
// d é usado para calcular e como seu inverso modular.
u128 generate_d(u128 z) {
    u128 d;

    do {
        d = 2 + (u128)secure_rand_ull() % (z - 2);
    } while (gcd(d, z) != 1);

    return d;
}

// Imprime um valor u128 em decimal.
void print_u128(u128 value) {
    if (value == 0) {
        printf("0");
        return;
    }

    char buffer[50];
    int i = 0;

    while (value > 0) {
        buffer[i++] = '0' + (char)(value % 10);
        value /= 10;
    }

    while (i--)
        putchar(buffer[i]);
}

int main() {
    ull p, q;
    u128 n, z, d;
    ull min, max;
    i128 e;
    int op;

    min = 17;
    max = (1ULL << 62) - 1; // limite superior para p e q (62 bits) para garantir que n caiba em 128 bits

    do{
        p = q = n = z = d = e = 0;

        printf("\n=== RSA MENU ===\n\n");
        printf("1 - Informar p e q (números primos)\n");
        printf("2 - Sistema gera p e q aleatórios\n");
        printf("0 - Sair\n");

        if (!read_int("Escolha uma opção: ", &op)) {
            printf("Entrada inválida. Escolha uma das opções descritas.\n");
            continue;
        }

        switch (op) {
            case 1:
                if (!read_ull("Informe o valor de p: ", &p)) {
                    printf("Entrada inválida. Digite apenas números inteiros.\n");
                    continue;
                }

                if (!read_ull("Informe o valor de q: ", &q)) {
                    printf("Entrada inválida. Digite apenas números inteiros.\n");
                    continue;
                }

                if (!is_prime(p) || !is_prime(q)) {
                    printf("Entrada inválida: p e q devem ser números primos\n");
                    continue;
                } else if (p == q) {
                    printf("Entrada inválida: p e q devem ser números primos distintos\n");
                    continue;
                }

                if (p < (ull)min || p > (ull)max || q < (ull)min || q > (ull)max) {
                    printf("p e q devem estar entre %llu e %llu\n", min, max);
                    continue;
                }

                break;
            case 2:
                p = generate_prime(min, max);
                ull q_temp, diff;
                do {
                    q_temp = generate_prime(min, max);
                    diff = (p > q_temp) ? (p - q_temp) : (q_temp - p);
                } while (q_temp == p || diff < (1ULL << 30)); // Garante que p e q sejam diferentes e não próximos
                q = q_temp;
                
                break;
            case 0:
                printf("Saindo...\n");
                return 0;
            default:
                printf("Opção inválida. Tente novamente.\n");
                continue;
        }

        printf("p = %llu\n", p);
        printf("q = %llu\n", q);

        n = (u128)p * q;
        z = (u128)(p - 1) * (q - 1);
        d = generate_d(z);
        e = mod_inverse((i128)d, (i128)z);

        if (e == -1) {
            printf("Erro: sem inverso modular\n");
            continue;
        }

        printf("\nRESULTADOS:\n");
        printf("\nPUBLIC KEY (n, e): ");
        print_u128(n);
        printf(", ");
        print_u128((u128)e);
        printf("\n");
        printf("PRIVATE KEY (n, d): ");
        print_u128(n);
        printf(", ");
        print_u128(d);
        printf("\n");

    } while(1);

    return 0;
}