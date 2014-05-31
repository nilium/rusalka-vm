/*
  Copyright (c) 2013 Noel Raymond Cower.

  This file is part of Rusalka VM.

  Rusalka VM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Rusalka VM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Rusalka VM.  If not, see <http://www.gnu.org/licenses/>.
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
.fabs:
    pop       rp
    if rp < 0.0
      neg     rp rp
    return

// function rot13(mem_in, mem_out) -> bytes written
.rot13: let* mem_in, mem_out, length {
    pop mem_in
    pop mem_out

    let* length_out {
        memlen length     mem_in
        memlen length_out mem_out

        if length_out < length
            load length length_out
    }

    let* index, char, base {
        load index 0
        for index < length {
            peek char mem_in index 0

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
            poke mem_out char index 0
            add index index 1
        }
    }
    load rp length
    return
}
