# 🕹️ Emulador Sinclair ZX81 (Windows • Compilação Manual)

Este repositório contém o código-fonte completo de um **emulador funcional do ZX81**, escrito em C, com motor de CPU Z80 incluído e interface gráfica em **SDL 1.2**.

---

## 🧰 Pré-requisitos para Windows

Para compilar e correr o emulador no Windows, vais precisar de:

1. **Compilador C** compatível com `gcc`  
   👉 Recomendado: [MinGW-w64](https://www.mingw-w64.org/)
2. **Biblioteca SDL 1.2**:  
   👉 Usa a versão `SDL-devel-1.2.15-...-mingw32.zip` disponível no [site oficial](https://www.libsdl.org/download-1.2.php)
3. **Make** (opcional, se quiseres usar o `Makefile`)
4. **Editor de código** (Visual Studio Code, Code::Blocks, etc.)

---

## 📁 Ficheiros incluídos

| Ficheiro       | Descrição                                 |
|----------------|-------------------------------------------|
| `zx81.c`       | Código principal do emulador              |
| `z80.c`/`.h`    | Implementação da CPU Z80                 |
| `zx81rom.h`    | ROM do ZX81 embutida (array C)            |
| `Makefile`     | Script de compilação opcional             |

---

## 🔧 Compilação (com MinGW)

Assumindo que tens o `gcc` e a SDL 1.2 corretamente instalados:

```bash
gcc zx81.c z80.c -IC:\SDL\include -LC:\SDL\lib -lmingw32 -lSDLmain -lSDL -o zx81.exe
```

> ⚠️ **Nota:** Substitui `C:\SDL` pelo caminho onde extraíste os ficheiros de desenvolvimento da SDL.

### Alternativa com `Makefile` (usando MSYS2)

Se tiveres o `make` instalado (ex: com [MSYS2](https://www.msys2.org/)):

```bash
make
```

---

## ▶️ Como correr o emulador

Após compilares com sucesso:

```bash
zx81.exe
```

Deverás ver a clássica tela branca do ZX81 com o cursor `K`.

---

## 🖱️ Controlos

- Teclado mapeado para o layout original do ZX81 (via SDL)
- Usa as teclas: letras, `SHIFT`, `RETURN`, `SPACE`, etc.
- Sem suporte a som ou ficheiros `.P` (por enquanto).

---

## 🧠 Sobre este projeto

Este emulador foi criado com foco em:

- **Fidelidade ao comportamento do ZX81 original**
- Código simples, direto e fácil de expandir
- Sem dependências externas além da SDL

---

## 🛠️ Próximos passos

- Suporte a ficheiros `.P` e `SAVE/LOAD`
- Melhoria na emulação de vídeo
- Otimizações de desempenho e timing

---

## ❓ Dúvidas ou sugestões?

Sente-te à vontade para contactar ou contribuir. Boas emulações! 🚀
