/* --------------------------------------------------
   z80.h  –  cabeçalho mínimo para o core z80.c
   -------------------------------------------------- */
   #ifndef Z80_H
   #define Z80_H
   
   #include <stdint.h>
   
   /* Opaque forward declaration para permitir ponteiros */
   struct z80;
   typedef struct z80 z80;
   
   /* Tipos de callbacks que o utilizador fornece */
   typedef uint8_t (*z80_read_f)(void *ud, uint16_t addr);
   typedef void    (*z80_write_f)(void *ud, uint16_t addr, uint8_t v);
   typedef uint8_t (*z80_port_in_f)(z80 *cpu, uint8_t port);
   typedef void    (*z80_port_out_f)(z80 *cpu, uint8_t port, uint8_t v);
   
   /* Definição completa da struct – deve corresponder ao z80.c */
   struct z80 {
       /* registos de 8 bits */
       uint8_t a, f;
       uint8_t b, c, d, e, h, l;
       uint8_t a_, f_, b_, c_, d_, e_, h_, l_;   /* sombras */
   
       /* registos de 16 bits e utilitários */
       uint16_t pc, sp, ix, iy, mem_ptr;
   
       /* flags isoladas para acesso rápido */
       uint8_t sf, zf, yf, hf, xf, pf, nf, cf;
   
       /* estado de interrupções */
       uint8_t  i, r, int_data, interrupt_mode;
       uint8_t  iff1, iff2, iff_delay;
       uint8_t  halted, int_pending, nmi_pending;
   
       /* contador total de ciclos */
       uint64_t cyc;
   
       /* callbacks configuráveis */
       z80_read_f     read_byte;
       z80_write_f    write_byte;
       z80_port_in_f  port_in;
       z80_port_out_f port_out;
       void          *userdata;
   };
   
   #ifdef __cplusplus
   extern "C" {
   #endif
   
   /* API pública do core */
   void z80_init(z80 *cpu);   /* inicializa registos/flags */
   void z80_step(z80 *cpu);   /* executa uma instrução */
   
   #ifdef __cplusplus
   }
   #endif
   
   #endif /* Z80_H */
   