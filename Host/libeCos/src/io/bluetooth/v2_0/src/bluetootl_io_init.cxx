#ifdef CYGPKG_NET_BLUEZ_STACK

#include "pkgconf/system.h"
#include "pkgconf/hal.h"
#include "cyg/infra/cyg_type.h"

externC int hci_uart_init(void);
//#define NET_INIT CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_LIBC)
#define NET_INIT CYGBLD_ATTRIB_INIT_AFTER(CYG_INIT_LIBC)

class hci_init_class{
public:
    hci_init_class(void){ 
        hci_uart_init();
    }
};

// And here's an instance of the class just to make the code run
//static 
hci_init_class _hci_init NET_INIT;
#endif

