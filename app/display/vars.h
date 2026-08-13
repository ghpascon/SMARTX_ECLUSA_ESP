#include <UnicViewAD.h>
LCM Lcm(Serial);

LcmString app_string(1000, 100);
LcmString identification_string(1500, 100);

extern LOCK lock1;
extern LOCK lock2;
extern int tags_qty;
extern String identification;
extern int authorized;
extern bool is_connected;
