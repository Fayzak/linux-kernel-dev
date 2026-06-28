#ifndef SEARCH_H
#define SEARCH_H

#include "build.h"

struct bucket_search_ctx *searchgetctx(void);
int search(struct bucket_search_ctx *ctx, unsigned int x);
void searchresult(struct bucket_search_ctx *ctx, unsigned char *str);
int searchrebuild(struct bucket_search_ctx *ctx);
int searchsetbucketid(struct bucket_search_ctx *ctx, unsigned int id);
int searchgetbucketdump(struct bucket_search_ctx *ctx, unsigned char *str);

#endif