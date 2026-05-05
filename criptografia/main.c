#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned long long ull;

int is_prime(ull n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;

    ull d = n - 1;
    int s = 0;

    while ((d & 1) == 0) {
        d >>= 1;
        s++;
    }

    for (int i = 0; i < 5; i++) {
        ull a = 2 + rand() % (n - 3);
        ull x = mod_exp(a, d, n);

        if (x == 1 || x == n - 1)
            continue;

        int ok = 0;

        for (int r = 0; r < s - 1; r++) {
            x = (x * x) % n;

            if (x == n - 1) {
                ok = 1;
                break;
            }
        }

        if (!ok) return 0;
    }

    return 1;
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

long long mod_inverse(long long d, long long z) {
    long long x, y;
    long long g = extended_gcd(d, z, &x, &y);

    if (g != 1) return -1;

    return (x % z + z) % z;
}

ull generate_prime(ull min, ull max) {
    ull p;

    do {
        p = min + rand() % (max - min);
    } while (!is_prime(p));

    return p;
}

ull gcd(ull a, ull b) {
    while (b != 0) {
        ull t = b;
        b = a % b;
        a = t;
    }
    return a;
}

ull generate_d(ull z) {
    ull d;

    do {
        d = 2 + ((ull)rand() * rand()) % (z - 2);
    } while (gcd(d, z) != 1);

    return d;
}

int main() {
    unsigned long long int p, q, n, z, d, e;
    int min, max;
    int op;

    min = 2;
    max = 10;

    do{
        p = q = n = z = d = e = 0;

        printf("\n=== RSA MENU ===\n\n");
        printf("1 - Informar p e q (números primos)\n");
        printf("2 - Sistema gera p e q aleatórios\n");
        printf("0 - Sair\n");
        scanf("%d", &op);

        switch (op) {
            case 1:
                printf("Informe o valor de p: ");
                scanf("%llu", &p);
                printf("Informe o valor de q: ");
                scanf("%llu", &q);

                if (!is_prime(p) || !is_prime(q) || p == q) {
                    printf("Entrada inválida\n");
                    continue;
                }

                break;
            case 2:
                p = generate_prime(min, max);
                do {
                    q = generate_prime(min, max);
                } while (q == p); // Garante que p e q sejam diferentes
                
                break;
            case 0:
                printf("Saindo...\n");
                return 0;
            default:
                printf("Opção inválida. Tente novamente.\n");
                continue;
        }

        n = p * q;
        z = (p - 1) * (q - 1);
        d = generate_d(z);
        e = mod_inverse(d, z);

        if (e == -1) {
            printf("Erro: sem inverso modular\n");
            continue;
        }

        printf("\nRESULTADOS:\n");
        printf("N = %llu\n", n);
        printf("Z = %llu\n", z);
        printf("D (privada) = %llu\n", d);
        printf("E (publica) = %llu\n", e);

    } while(1);

    return 0;
}