#ifndef __AIO_H__
#define __AIO_H__

#include "esp_http_client.h"
#include "esp_log.h"

typedef struct aio
{
    const char *username;
    const char *key;
} aio_t;

void aio_init(const char *, const char *);
void aio_create_group(const char *);
void aio_create_feed(const char *, const char *);
void aio_create_data(const char *, const char *);

#endif 
