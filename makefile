CC = gcc
CFLAGS = -Wall -Wextra -O2 -I/opt/homebrew/opt/openssl@3/include
LDFLAGS = -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto

# Target principal
all-chaves: build gerar-chaves exec-gerar-chaves	
all-cripto: build criptografar exec-cripto

build: 
	@mkdir -p build

# Compila o gerador de chaves
gerar-chaves: gerar_chaves.c
	@$(CC) $(CFLAGS) $(LDFLAGS) gerar_chaves.c -o build/gerar_chaves

criptografar: ./criptografia/main.c
	@$(CC) $(CFLAGS) $(LDFLAGS) ./criptografia/main.c -o build/criptografar

# Executa o gerador de chaves
exec-gerar-chaves: gerar-chaves
	@./build/gerar_chaves

exec-cripto: criptografar
	@./build/criptografar

# Limpa os arquivos compilados
clean:
	@rm -rf build

chaves: all-chaves

cripto: all-cripto

.PHONY: all-chaves all-cripto build clean gerar-chaves cripto exec-gerar-chaves exec-cripto