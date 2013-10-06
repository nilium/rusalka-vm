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

// main
.main
    push 0x10
    load 24 -1.23456
    call .fabs 0x1000000
    pop 0x10
    fadd 3 4 3
    push 0x8
    call ^print 0x8

    load 4 0

@loop
    cmpl 6 4 0xFFFFFFF
    jgtel 6 @loop_done

    // itof 5 4
    // faddl 5 5 0.5
    // call ^print 0x20
    addl 4 4 1
    jumpl @loop

@loop_done
    pop 0x8
    return 3

// fabs(f)
.fabs
    uandl 4 4 0x7FFFFFFF
    return 4
