
#if 1 
#define T(x) Serial.print(__LINE__); Serial.print(":"); Serial.print(x); Serial.print("\n")
#define T_V(x,y) Serial.print(__LINE__); Serial.print(": "); Serial.print(x);Serial.print(": "); Serial.print(y); Serial.print("\n")
#else
#define T(x) 
#define T_V(x,y)
#endif 

