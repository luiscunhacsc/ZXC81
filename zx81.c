/* ------------------------------------------------------------------
   zx81.c – Emulador Sinclair ZX-81 (SDL-1.2) com core z80.c
   ------------------------------------------------------------------ */

   #include <stdio.h>
   #include <stdint.h>
   #include <string.h>
   #include <SDL/SDL.h>
   
   #include "z80.h"
   #include "zx81rom.h"
   
   /* aliases para manter nomenclatura original */
   typedef uint8_t  BYTE;
   typedef uint16_t WORD;
   
   /* ponteiro do Display File na RAM do ZX-81 */
   #define D_FILE 0x400c
   
   /* ------------------------------------------------------------------
      estado global
      ------------------------------------------------------------------ */
   static z80  cpu;
   static BYTE memory  [ 65536 ];
   static BYTE keyboard[ 9 ];          /* 8 linhas + 1 dummy            */
   static BYTE sdlk2scan[ SDLK_LAST ]; /* mapa SDL → matriz ZX-81       */
   
   static SDL_Surface *screen;         /* janela principal              */
   static SDL_Surface *charset;        /* superfície com o charset      */
   
   /* ------------------------------------------------------------------
      Callbacks de barramento para o core z80.c
      ------------------------------------------------------------------ */
   static uint8_t zx_read(void *ud, uint16_t addr)
   {
       (void)ud;
       return memory[ addr ];
   }
   
   static void zx_write(void *ud, uint16_t addr, uint8_t val)
   {
       (void)ud;
       if (addr >= 0x4000)             /* protege ROM */
           memory[ addr ] = val;
   }
   
   /* Leitura de portas – apenas teclado é usado */
   static uint8_t zx_in(z80 *z, uint8_t port)
   {
       if ((port & 1) == 0) {          /* bit 0 a zero ⇒ teclado */
           uint8_t row_bits = z->b;    /* ROM usa sempre IN A,(C) */
   
           for (int row = 0; row < 8; ++row) {
               if ((row_bits & 1) == 0)
                   return keyboard[ row ];
               row_bits >>= 1;
           }
           return 0xFF;
       }
       return 0xFF;                    /* bus flutuante para outras portas */
   }
   
   static void zx_out(z80 *z, uint8_t port, uint8_t val)
   {
       (void)z; (void)port; (void)val; /* ZX-81 não usa OUT */
   }
   
   /* ------------------------------------------------------------------
      Gera superfície com 128 caracteres (dobro de escala)
      ------------------------------------------------------------------ */
   static int create_charset(void)
   {
       SDL_Surface *tmp;
       Uint32 rm, gm, bm, black, white;
       Uint32 *pix;
       int addr = 0x1e00;              /* bitmap na ROM */
   
   #if SDL_BYTEORDER == SDL_BIG_ENDIAN
       rm = 0xFF000000; gm = 0x00FF0000; bm = 0x0000FF00;
   #else
       rm = 0x000000FF; gm = 0x0000FF00; bm = 0x00FF0000;
   #endif
       tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, 4096, 16, 32, rm, gm, bm, 0);
       if (!tmp) return 0;
   
       black = SDL_MapRGB(tmp->format, 0, 0, 0);
       white = SDL_MapRGB(tmp->format, 255, 255, 255);
   
       pix = (Uint32*)tmp->pixels;
       int pitch = tmp->pitch / 4;
   
       for (int ch = 0; ch < 64; ++ch) {
           for (int row = 0; row < 8; ++row) {
               int b = rom[ addr++ ];
               for (int col = 0; col < 8; ++col) {
                   Uint32 ink = (b & 0x80) ? black : white;
                   Uint32 pap = (b & 0x80) ? white : black;
   
                   pix[           0] = ink;
                   pix[           1] = ink;
                   pix[ pitch +   0] = ink;
                   pix[ pitch +   1] = ink;
   
                   pix[        2048] = pap;
                   pix[        2049] = pap;
                   pix[ pitch+2048] = pap;
                   pix[ pitch+2049] = pap;
   
                   pix += 2;
                   b <<= 1;
               }
               pix += pitch * 2 - 16;
           }
           pix -= pitch * 16 - 16;
       }
       charset = SDL_DisplayFormat(tmp);
       SDL_FreeSurface(tmp);
       return 1;
   }
   
   /* ------------------------------------------------------------------
      Inicialização da máquina
      ------------------------------------------------------------------ */
   static void setup_emulation(void)
   {
       /* carrega ROM (ghost em 0x2000) */
       memcpy(memory + 0x0000, rom, 0x2000);
       memcpy(memory + 0x2000, rom, 0x2000);
   
       /* patch DISPLAY-5 → RET para acelerar vídeo */
       memory[0x02B5 + 0x0000] = 0xC9;
       memory[0x02B5 + 0x2000] = 0xC9;
   
       /* teclado todo “solto” (bits = 1) */
       memset(keyboard, 0xFF, sizeof keyboard);
   
       /* tabela SDL-key → linha/coluna ZX-81 */
       memset(sdlk2scan, 8 << 5, sizeof sdlk2scan);
       sdlk2scan[SDLK_LSHIFT] = 0 << 5 | 1;
       sdlk2scan[SDLK_RSHIFT] = 0 << 5 | 1;
       sdlk2scan[SDLK_z]      = 0 << 5 | 2;
       sdlk2scan[SDLK_x]      = 0 << 5 | 4;
       sdlk2scan[SDLK_c]      = 0 << 5 | 8;
       sdlk2scan[SDLK_v]      = 0 << 5 | 16;
       sdlk2scan[SDLK_a]      = 1 << 5 | 1;
       sdlk2scan[SDLK_s]      = 1 << 5 | 2;
       sdlk2scan[SDLK_d]      = 1 << 5 | 4;
       sdlk2scan[SDLK_f]      = 1 << 5 | 8;
       sdlk2scan[SDLK_g]      = 1 << 5 | 16;
       sdlk2scan[SDLK_q]      = 2 << 5 | 1;
       sdlk2scan[SDLK_w]      = 2 << 5 | 2;
       sdlk2scan[SDLK_e]      = 2 << 5 | 4;
       sdlk2scan[SDLK_r]      = 2 << 5 | 8;
       sdlk2scan[SDLK_t]      = 2 << 5 | 16;
       sdlk2scan[SDLK_1]      = 3 << 5 | 1;
       sdlk2scan[SDLK_2]      = 3 << 5 | 2;
       sdlk2scan[SDLK_3]      = 3 << 5 | 4;
       sdlk2scan[SDLK_4]      = 3 << 5 | 8;
       sdlk2scan[SDLK_5]      = 3 << 5 | 16;
       sdlk2scan[SDLK_0]      = 4 << 5 | 1;
       sdlk2scan[SDLK_9]      = 4 << 5 | 2;
       sdlk2scan[SDLK_8]      = 4 << 5 | 4;
       sdlk2scan[SDLK_7]      = 4 << 5 | 8;
       sdlk2scan[SDLK_6]      = 4 << 5 | 16;
       sdlk2scan[SDLK_p]      = 5 << 5 | 1;
       sdlk2scan[SDLK_o]      = 5 << 5 | 2;
       sdlk2scan[SDLK_i]      = 5 << 5 | 4;
       sdlk2scan[SDLK_u]      = 5 << 5 | 8;
       sdlk2scan[SDLK_y]      = 5 << 5 | 16;
       sdlk2scan[SDLK_RETURN] = 6 << 5 | 1;
       sdlk2scan[SDLK_l]      = 6 << 5 | 2;
       sdlk2scan[SDLK_k]      = 6 << 5 | 4;
       sdlk2scan[SDLK_j]      = 6 << 5 | 8;
       sdlk2scan[SDLK_h]      = 6 << 5 | 16;
       sdlk2scan[SDLK_SPACE]  = 7 << 5 | 1;
       sdlk2scan[SDLK_PERIOD] = 7 << 5 | 2;
       sdlk2scan[SDLK_m]      = 7 << 5 | 4;
       sdlk2scan[SDLK_n]      = 7 << 5 | 8;
       sdlk2scan[SDLK_b]      = 7 << 5 | 16;
   
       /* inicializa CPU */
       z80_init(&cpu);
       cpu.read_byte  = zx_read;
       cpu.write_byte = zx_write;
       cpu.port_in    = zx_in;
       cpu.port_out   = zx_out;
   }
   
   /* ------------------------------------------------------------------
      Executa ~65 000 T-states (≈ 20 ms de ZX-81)
      ------------------------------------------------------------------ */
   static void run_some(void)
   {
       uint64_t target = cpu.cyc + 65000;
       while (cpu.cyc < target)
           z80_step(&cpu);
   }
   
   /* ------------------------------------------------------------------
      Eventos SDL → teclado ZX-81
      ------------------------------------------------------------------ */
   static int consume_events(void)
   {
       SDL_Event ev; BYTE scan;
       while (SDL_PollEvent(&ev)) {
           switch (ev.type) {
           case SDL_KEYDOWN:
               if (ev.key.keysym.sym == SDLK_BACKSPACE) {
                   keyboard[0] &= ~1; keyboard[4] &= ~1;   /* SHIFT */
               } else {
                   scan = sdlk2scan[ ev.key.keysym.sym ];
                   keyboard[ scan >> 5 ] &= ~(scan & 0x1F);
               }
               break;
   
           case SDL_KEYUP:
               if (ev.key.keysym.sym == SDLK_BACKSPACE) {
                   keyboard[0] |= 1; keyboard[4] |= 1;
               } else {
                   scan = sdlk2scan[ ev.key.keysym.sym ];
                   keyboard[ scan >> 5 ] |= (scan & 0x1F);
               }
               break;
   
           case SDL_QUIT:
               return 0;
           }
       }
       return 1;
   }
   
   /* ------------------------------------------------------------------
      Redesenha todo o ecrã (24×32 caracteres, 2× escalado)
      ------------------------------------------------------------------ */
   static void update_screen(void)
   {
       WORD dfile = memory[D_FILE] | (memory[D_FILE + 1] << 8);
       SDL_Rect src = {0, 0, 16, 16};
       SDL_Rect dst = {0, 0, 0, 0};
   
       for (int row = 0; row < 24; ++row) {
           dst.x = 0;
           for (int col = 0; col < 32; ++col) {
               src.x = memory[++dfile] << 4;  /* ×16 */
               SDL_BlitSurface(charset, &src, screen, &dst);
               dst.x += 16;
           }
           ++dfile;                           /* 0x76 fim-de-linha */
           dst.y += 16;
       }
       SDL_UpdateRect(screen, 0, 0, 0, 0);
   }
   
   /* ------------------------------------------------------------------
      main()
      ------------------------------------------------------------------ */
   int main(int argc, char *argv[])
   {
       (void)argc; (void)argv;
   
       if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
           fprintf(stderr, "SDL init: %s\n", SDL_GetError());
           return 1;
       }
       screen = SDL_SetVideoMode(512, 384, 0, SDL_SWSURFACE);
       if (!screen) {
           fprintf(stderr, "SDL video: %s\n", SDL_GetError());
           return 1;
       }
       if (!create_charset()) {
           fprintf(stderr, "Falha a criar charset\n");
           return 1;
       }
       setup_emulation();
   
       /* desliga auto-repeat */
   #if SDL_MAJOR_VERSION == 1
       SDL_EnableKeyRepeat(0, 0);
   #endif
   
       const uint32_t FRAME_MS = 20;   /* ~50 Hz */
       int running;
       do {
           uint32_t t0 = SDL_GetTicks();
   
           run_some();
           running = consume_events();
           update_screen();
   
           uint32_t elapsed = SDL_GetTicks() - t0;
           if (elapsed < FRAME_MS)
               SDL_Delay(FRAME_MS - elapsed);
       } while (running);
   
       SDL_Quit();
       return 0;
   }
   