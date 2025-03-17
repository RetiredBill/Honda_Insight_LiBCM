/******************************************************************************
* Bitfield manipulation macro definitions
*******************************************************************************/

#ifndef BITFIELDMACROS_H
  #define BITFIELDMACROS_H

  // _BV() redefined for forcing uint32_t
  #define BITVALUE(bit)  ((uint32_t)1<<(bit))

  // For every bitfield in a register, define it whith 2 #defines:
  //#define <name>_SHIFT x    // x is register bit position of (LSB of) bit
  //#define <name>_SIZE  y    // number of bits/width of bit field

  // Below is inspired by https://www.coranac.com/man/tonclib/group__grpCoreBit.htm
  // Generate a mask for a given field size. bits [(size+shift-1):shift] set
  #define BFN_MASK(name)        ( ( ( 1 << (name##_SIZE) ) - 1 ) << (name##_SHIFT) )
  // Get the value of a named bitfield from register reg. Equivalent to (var=) reg.name.
  #define BFN_GET(name, reg)      ( ( (reg) & BFN_MASK(name) ) >> (name##_SHIFT) )
  // Prepare a named bit-field for for insterion or combination.
  #define BFN_PREP(name, x)     ( ( (x) << (name##_SHIFT) ) & BFN_MASK(name) )
  // Set a named bitfield in reg to x. Equivalent to reg.name= x.
  #define BFN_SET(name, reg, x)   ( reg = ( (reg) & ~BFN_MASK(name) ) | BFN_PREP(name,x) )
  // Compare a named bitfield (at shift position) to named literal x.
  #define BFN_CMP(name, reg, x)   ( ( (reg) & BFN_MASK(name) ) == (x) )
  // Get the value of bitfield name from register reg, but don't down-shift.
  #define BFN_EXTR(name, reg)     ( (reg) & BFN_MASK(name) )
  // Set named bitfield in reg to x with pre-shifted x.
  #define BFN_SET2(name, reg, x)  ( reg = ( (reg) & ~BFN_MASK(name) ) | BFN_EXTR(name,x) )
  // Merge a named bitfield x into y. Equivalent to reg.name= x.
  #define BFN_MERG(name, y, x)   (  ( (y) & ~BFN_MASK(name) ) | BFN_PREP(name,x) )

#endif
