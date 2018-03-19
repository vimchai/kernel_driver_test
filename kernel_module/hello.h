#include <linux/cdev.h>  
#include <linux/mutex.h> 

#define  HELLO_DEV_NAME "hello"
#define  HELLO_DEV_CLASS "hello"
#define  HELLO_DEV_STRING_LEN 10

struct hello_dev {
    struct cdev dev;
    struct mutex mutex;
    int  val;
    char *str[HELLO_DEV_STRING_LEN];
};