/*--------------
	Error Macro
----------------*/
#if !defined __ErrorMacro__
#define __ErrorMacro__

#define er(A) if(A) return; else
#define e0r(A) if(!(A)) return; else
#define emr(A) if((A)<0) return; else

#define er0(A) if(A) return 0; else
#define e0r0(A) if(!(A)) return 0; else
#define emr0(A) if((A)<0) return 0; else

#define erm(A) if(A) return -1; else
#define e0rm(A) if(!(A)) return -1; else
#define emrm(A) if((A)<0) return -1; else

#define erN(A) if(A) return NULL; else
#define e0rN(A) if(!(A)) return NULL; else
#define emrN(A) if((A)<0) return NULL; else

#define eb(A) if(A) break; else
#define e0b(A) if(!(A)) break; else
#define emb(A) if((A)<0) break; else

#define egs(A) if(A) goto s; else
#define e0gs(A) if(!(A)) goto s; else
#define emgs(A) if((A)<0) goto s; else

#define egf(A) if(A) goto f; else
#define e0gf(A) if(!(A)) goto f; else
#define emgf(A) if((A)<0) goto f; else

#define egfn(A,n) if(A) goto f##n; else
#define e0gfn(A,n) if(!(A)) goto f##n; else
#define emgfn(A,n) if((A)<0) goto f##n; else

#endif
