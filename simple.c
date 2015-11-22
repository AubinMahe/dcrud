#define UTIL_CONCAT(x,y) x ## y
#define UTIL_EVAL(x,y)   UTIL_CONCAT(x,y)

#define COLL_MAP_TYPE       collMapVoidPtr
#define COLL_MAP_NAME(NAME) UTIL_EVAL(COLL_MAP_TYPE,NAME)

typedef void * COLL_MAP_NAME(Value);
typedef void * COLL_MAP_NAME(Key);

int main() {
   collMapVoidPtrValue p1 = 0;
   collMapVoidPtrKey   p2 = 0;
   return 0;
}

