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
    call      .fabs 1
    add       $0 rp 123.456
    push      $0
    call      ^print 1
    push      $0
    call      ^print 1

    load      $2 ~woop
    push      $2
    call      ^prints 1

    load      $2 "woop"
    push      $2
    call      ^prints 1

    for $0 < 256 {
      add     $0 $0 1.0
      push    $0
      call    ^print 1
    }

    load      rp $0
    return

// fabs(f). Just import a freakin' fabs function, this is stupid since I can't
// just bitwise-and a double's sign bit since integers are strictly 32-bit
// and therefore cannot touch the latter 32 bits of a double.
.fabs:
    pop       rp
    if rp < 0.0
      neg     rp rp
    return
