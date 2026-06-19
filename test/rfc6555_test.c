// SPDX-License-Identifier: GPL-3.0-or-later
#include <assert.h>
#include <malloc.h>

#include "happy-eyeballs/rfc6555.h"

void test_context();

int main() {
	test_context();
}

#define MIN_CTX_LEN 4 /* Copied from the implementation */

#if defined(_WIN32)
#define msize _msize
#elif defined(__APPLE__)
#define msize malloc_size
#else
#define msize malloc_usable_size
#endif

void test_context() {
	rfc6555_ctx *ctx;

	ctx = rfc6555_context_create();
	assert(ctx != NULL);
	assert(msize(ctx) >= sizeof(rfc6555_ctx));
	assert(ctx->fds != NULL);
	assert(msize(ctx->fds) >= MIN_CTX_LEN * sizeof(int));
	assert(ctx->original_flags != NULL);
	assert(msize(ctx->original_flags) >= MIN_CTX_LEN * sizeof(int));
	assert(ctx->rps != NULL);
	assert(msize(ctx->rps) >= MIN_CTX_LEN * sizeof(struct addrinfo*));

	rfc6555_context_destroy(ctx);
}
