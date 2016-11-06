#ifdef WIN32

int inet_pton( int af, const char * src, void * dst ) {
   (void)af;
   (void)src;
   (void)dst;
   return 1;
}

#endif
