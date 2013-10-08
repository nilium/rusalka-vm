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
.main
    push [argv[0]]
    load $1 -1.23456
    call .fabs [$1]
    fadd argv[0] argv[0] rp
    call ^print [argv[0]]
    load $20 ~woop
    call ^prints [$20]

    push [$0]
    load $0 0

@loop
    cmpl $2 $0 0xFFFFFFF
    jgtel $2 @loop_done

    // itof 5 4
    // faddl 5 5 0.5
    // call ^print 0x20
    addl $0 $0 1
    jumpl @loop

@loop_done
    pop [$0]
    return $0

// fabs(f)
.fabs
    uandl argv[0] argv[0] 0x7FFFFFFF
    return argv[0]
