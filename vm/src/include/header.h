#ifndef __HEADER_H__
#define __HEADER_H__

#include <stdint.h>

/*

- Total Content Length
- Padding
- String...
    - Content length
    - Data
0         8         16         24         32
+-------------------+---------------------+
| Total Length      | Reserved            |
+---------+---------+---------------------+
| Length  | Data...                       |
+---------+-------------------------------+
*/
struct stringheader {
    uint16_t length;
    uint16_t rsvd;
    struct {
        uint8_t length;
        char *data;
    } *strings;
};

/*

- Version
- Header Length
- String Header
- Constants Header

0         8         16         24         32
+---------+--------------------+----------+
| Version | Header Length      | Reserved |
+---------+--------------------+----------+
| String Header...                        |
+------------------------------+----------+

*/
struct header {
    uint8_t version;
    uint16_t length;
    uint8_t rsvd;
    struct stringheader strings;
};

#endif