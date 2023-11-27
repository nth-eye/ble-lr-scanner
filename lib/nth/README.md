# nth

# Extended COBS - xCOBS

<!--  ┬├└┌│┐┘┤┴ ─ ┼ -->

```
┌─────────┬──────────────┬──────────┬─────────────┐
| Section | payload size | payload  | CRC32-HDLC  |
├─────────┼--------------┼----------┼-------------┤
| Bytes   | 2            | N        | 4           |
├─────────┼--------------┼----------┼-------------┤
| Sample  | 03 00        | 0b 00 b5 | f0 71 1f 48 |
└─────────┴──────────────┴──────────┴─────────────┘
            \                                   /
             \          COBS encoding          /
              \_______________________________/
                     |     |     |     |
                     v     v     v     v
┌─────────┬─────┬───────────────────────────────┬─────┐
| Section | SOF | encoded content (max size)    | EOF |
├─────────┼-----┼-------------------------------┼-----┤
| Bytes   |  1  |  6 + N + ceil((6 + N) / 254)  |  1  |
├─────────┼-----┼-------------------------------┼-----┤
| Sample  | 00  | 02 03 02 0b 06 b5 f0 71 1f 48 | 00  |
└─────────┴─────┴───────────────────────────────┴─────┘
```

> ===========================================================

- [ ] source
    - [ ] cbor
    - [ ] coap
        - [x] address
        - [x] base
        - [ ] config
            - [ ] get rid of macros
        - [x] option
        - [ ] packet
            - [ ] get rid of const_cast<> (100% safe, but bad smell)
        - [ ] session
    - [ ] container
        - [ ] list
        - [x] pool
        - [x] ring
        - [x] stack
        - [x] vector
    - [ ] crypto
    - [ ] math
        - [ ] bigint
        - [ ] galois
        - [x] general
        - [x] interpolation
        - [x] trigonometry
    - [ ] misc
        - [x] crc
        - [x] git
        - [ ] qr
    - [ ] print
        - [ ] ? dragonbox
        - [ ] format
        - [ ] print
        - [ ] log
    - [ ] util
        - [x] bit 
        - [x] bitset
        - [x] bitslide
        - [x] expected
        - [x] half
        - [x] meta
        - [x] scope
        - [x] storage
        - [ ] string
            - [ ] comments
            - [ ] dragonbox (requires bigint)
            - [ ] to_chars for float (requires dragonbox)
            - [ ] to_string for float (requires to_chars)
        - [ ] time
        - [x] typeid

> ===========================================================

- [ ] tests
    - [ ] cbor
    - [ ] coap
        - [x] address
        - [x] option
        - [x] packet
        - [ ] session
    - [ ] container
        - [ ] list
        - [x] pool
        - [x] ring
        - [x] stack
        - [x] vector
    - [ ] math
        - [ ] bigint
        - [ ] galois
        - [x] general
        - [x] interpolation
        - [ ] trigonometry
            - [ ] tilt
            - [ ] rill
            - [ ] pitch
    - [ ] misc
        - [x] crc
        - [x] git
        - [ ] qr
    - [ ] util
        - [ ] bit
            - [ ] putle
            - [ ] putbe
            - [ ] getle
            - [ ] getbe
        - [x] bitset
        - [x] bitslide
        - [x] expected
        - [x] half
        - [x] meta
        - [ ] storage
        - [ ] string
            - [x] to_chars for int
            - [ ] to_chars for float
            - [x] to_string for int
            - [ ] to_string for float
        - [ ] time
        - [x] typeid

> ===========================================================