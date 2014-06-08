/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

defdata woop "wooperton"

// main
.main:
    let accum {
        pop accum
        push accum
        call      .fabs 1
        add       accum rp 123.456

        push      accum
        call      ^print 1

        push      accum
        call      ^print 1

        push accum
    }

    let string {
        load      string ~woop
        push      string
        call      ^prints 1

        load      string "woop"
        push      string
        call      ^prints 1
    }

/*
    for $0 < 256 {
      add     $0 $0 1.0
      push    $0
      call    ^print 1
    }
*/

    let buffer {
        memdup buffer "abz"

        push buffer
        call ^print 1

        push buffer
        call ^prints 1

        push buffer
        push buffer
        call .rot13 2

        push buffer
        call ^prints 1

        push buffer
        push buffer
        call .rot13 2

        push buffer
        call ^prints 1

        free buffer
    }

    pop rp
    round      rp rp
    return

// fabs(f). Just import a freakin' fabs function, this is stupid since I can't
// just bitwise-and a double's sign bit since integers are strictly 32-bit
// and therefore cannot touch the latter 32 bits of a double.
// Additionally, because this pops directly to rp, the function syntax used
// by rot13 below actually won't work because aliasing reserved registers is
// forbidden by asm2bc.
.fabs:
    pop       rp
    if rp < 0.0
      neg     rp rp
    return

// function rot13(mem_in, mem_out) -> bytes written
// function rot13(mem_in, mem_out) -> bytes written
// .rot13: let* mem_in, mem_out, length {
function rot13(mem_in, mem_out; index, char, base, length) {
    let* length_out {
        memlen length     mem_in
        memlen length_out mem_out
        if length_out < length
            load length length_out
    }

    load index 0
    for index < length {
        peek char mem_in index MEMOP_UINT8

        if 'a' <= char {
            if char <= 'z' {
                load base 'a'
                jump @__rot13__apply_rotate
            }
        }

        if 'A' <= char {
            if char <= 'Z' {
                load base 'A'
                jump @__rot13__apply_rotate
            }
        }

        jump @__rot13__continue

        @__rot13__apply_rotate:
        sub char char base
        add char char 13
        imod char char 26
        add char char base

        @__rot13__continue:
        poke mem_out char index MEMOP_UINT8
        add index index 1
    }

    load rp length
    return
}

