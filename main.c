/*
 * Copyright (c) 2012 Arvin Schnell <arvin.schnell@gmail.com>
 * Copyright (c) 2012 Rob Clark <rob@ti.com>
 * Copyright (c) 2020 Antonin Stefanutti <antonin.stefanutti@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>

#include "glsl.h"
#include "drm-common.h"

#include "lease.h"

static const struct egl *egl;
static const struct gbm *gbm;
static const struct drm *drm;

int init(const char *shadertoy, const struct options *options) {
	int ret;
	int fd;

#if XCB_LEASE
		xcb_connection_t *connection;
		int screen;

		connection = xcb_connect(NULL, &screen);
		int err = xcb_connection_has_error(connection);
		if (err > 0) {
			printf("Connection attempt to X server failed with error %d, falling back to DRM\n", err);
			xcb_disconnect(connection);

			fd = find_drm_device();
		} else {
			xcb_randr_query_version_cookie_t rqv_c = xcb_randr_query_version(connection,XCB_RANDR_MAJOR_VERSION,XCB_RANDR_MINOR_VERSION);
			xcb_randr_query_version_reply_t *rqv_r = xcb_randr_query_version_reply(connection, rqv_c, NULL);
			if (!rqv_r || rqv_r->minor_version < 6) {
				printf("No new-enough RandR version: %d\n", rqv_r->minor_version);
				return -1;
			}
			free(rqv_r);

			fd = xcb_lease(connection, &screen);
		}
#else
		fd = find_drm_device();
#endif
	if (fd < 0) {
		printf("could not open DRM device\n");
		return -1;
	}

	drm = init_drm_legacy(fd, options);
	if (!drm) {
		printf("failed to initialize DRM\n");
		return -1;
	}

	uint32_t format = DRM_FORMAT_XRGB8888;
	uint64_t modifier = DRM_FORMAT_MOD_INVALID;

	gbm = init_gbm_device(drm, format);
	if (!gbm) {
		printf("failed to initialize GBM\n");
		return -1;
	}

	egl = init_egl(gbm, modifier, 0);
	if (!egl) {
		printf("failed to initialize EGL\n");
		return -1;
	}

	ret = init_shadertoy(gbm, egl, shadertoy);
	if (ret < 0) {
		return -1;
	}

	glClearColor((GLfloat) 0.5, (GLfloat) 0.5, (GLfloat) 0.5, (GLfloat) 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	return 0;
}

int main(int argc, char *argv[]) {
	const char *shadertoy = NULL;

	struct options options = {
			.connector = -1,
			.frames = 0,
			.mode = "",
	};

	int ret;

	shadertoy = argv[1];

	ret = init(shadertoy, &options);
	if (ret < 0) {
		return -1;
	}

	return drm->run(gbm, egl);
}
