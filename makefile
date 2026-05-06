CC = gcc
CFLAGS = -Wall -Wextra -O2 -I/opt/homebrew/opt/openssl@3/include
LDFLAGS = -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto

# Target principal
all-chaves: build gerar-chaves exec-gerar-chaves	
all-cripto: build criptografar exec-cripto
all-descripto: build descriptografar exec-descriptografar

build: 
	@mkdir -p build

# Compila o gerador de chaves
gerar-chaves: build gerar_chaves.c
	@$(CC) $(CFLAGS) $(LDFLAGS) gerar_chaves.c -o build/gerar_chaves

criptografar: build ./criptografia/criptografar.c
	@$(CC) $(CFLAGS) $(LDFLAGS) ./criptografia/criptografar.c -o build/criptografar

descriptografar: build descriptografia/descriptografar.c
	@$(CC) $(CFLAGS) $(LDFLAGS) descriptografia/descriptografar.c -o build/descriptografar

chaves: all-chaves

cripto: all-cripto

descripto: all-descripto

# Executa o gerador de chaves
exec-gerar-chaves: gerar-chaves
	@./build/gerar_chaves

exec-cripto: criptografar
	@./build/criptografar

exec-descriptografar: descriptografar
	@./build/descriptografar

# Limpa os arquivos compilados
clean:
	@rm -rf build

.PHONY: all-chaves all-cripto build clean gerar-chaves criptografar descriptografar cripto exec-gerar-chaves exec-cripto exec-descriptografar