CC             = gcc
CCC            = g++
CFLAGS_RELEASE = -Iinc -I../C/inc -Wno-long-long -pthread -O3 -fPIC
CFLAGS_DEBUG   = -Iinc -I../C/inc -Wno-long-long -pthread -O0 -fPIC\
 -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion

