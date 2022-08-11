#include <xtensa/config/specreg.h>
#include <xtensa/config/core-isa.h>
#include <xtensa/corebits.h>

.data
    .global vone

vone:
    .int 0xAA

.set voneaddr, vone

.text
.align 
.global test1, test2, f, taller3, invertirCadena, uart

test1:
    entry sp, 48
    movi a14, 0x3
    movi a3, 0x5
    movi a2, 0x27
    ssl a14
    ssr a3
    srl a2,a2
    retw

.align 4
test2:
    entry sp, 32
    slli a2, a2, 0x4
    nop
    retw

.align 4
f:
    entry sp, 48
    add a2, a2, a3
    nop
    retw

/*config uart*/
.align 4
uart:
    entry sp, 48
    /* baudaje = a2*/
    /* direccion uart = a3*/
    l32i a5, a3, 0
    movi a6, 0x4c4b4000
    quos a6, a6, a2
    movi a10, 0xf
    and a7, a7, a10
    slli a7, a7, 20
    srai a6, a6, 4
    or a6, a6, a7
    s32i a6, a3, 0
    nop
    retw

.align 4
taller3:
/*a2 = direccion, a3 = no bit, a4 = bit*/
    entry sp, 32
    mov a10, a2 //a10 = direccion
    l32i a2, a2, 0 //a2 = valor de la direccion
    beqz a4, es_cero
    movi a5, 1
    sub a3, a3, a5
    ssl a3
    sll a5, a3
    beqz a4, es_cero
    or a2, a2, a5
    j fin
es_cero: 
    movi a7, 0xffffffff
    xor a5, a5, a7
    and a2, a2, a5
fin: s32i a2, a10, 0 
    nop
    retw

.align 4
invertirCadena:
/*a2 = direccion cadena*/
    entry sp, 32
    mov  a3, a2 /* apuntador al inicio de la cadena */
	movi a4, 0

contar_caracteres: l32i a5, a2, 0
	beqz a5, fin_cuenta
    addi a2, a2, 1
    addi a4, a4, 1
	j contar_caracteres
fin_cuenta
mov a2, a4
retw
	
