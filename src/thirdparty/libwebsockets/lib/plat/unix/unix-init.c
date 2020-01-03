/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010-2018 Andy Green <andy@warmcat.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation:
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "core/private.h"

#include <pwd.h>
#include <grp.h>

#ifdef LWS_WITH_PLUGINS
#include <dlfcn.h>
#endif
#include <dirent.h>

#if defined(LWS_HAVE_MALLOC_TRIM)
#include <malloc.h>
#endif

#if defined(LWS_WITH_NETWORK)
static void
lws_sul_plat_unix(lws_sorted_usec_list_t *sul)
{
	struct lws_context_per_thread *pt =
		lws_container_of(sul, struct lws_context_per_thread, sul_plat);
	struct lws_context *context = pt->context;
#if defined(LWS_ROLE_CGI) || defined(LWS_ROLE_DBUS)
	time_t now = time(NULL);
#endif

#if !defined(LWS_NO_DAEMONIZE)
	/* if our parent went down, don't linger around */
	if (pt->context->started_with_parent &&
	    kill(pt->context->started_with_parent, 0) < 0)
		kill(getpid(), SIGTERM);
#endif
#if defined(LWS_HAVE_MALLOC_TRIM)
	malloc_trim(4 * 1024);
#endif

	if (pt->context->deprecated && !pt->context->count_wsi_allocated) {
		lwsl_notice("%s: ending deprecated context\n", __func__);
		kill(getpid(), SIGINT);
		return;
	}

	lws_check_deferred_free(context, 0, 0);

	lws_context_lock(context, "periodic checks");
	lws_start_foreach_llp(struct lws_vhost **, pv,
			      context->no_listener_vhost_list) {
		struct lws_vhost *v = *pv;
		lwsl_debug("deferred iface: checking if on vh %s\n", (*pv)->name);
		if (_lws_vhost_init_server(NULL, *pv) == 0) {
			/* became happy */
			lwsl_notice("vh %s: became connected\n", v->name);
			*pv = v->no_listener_vhost_list;
			v->no_listener_vhost_list = NULL;
			break;
		}
	} lws_end_foreach_llp(pv, no_listener_vhost_list);
	lws_context_unlock(context);

#if defined(LWS_ROLE_CGI)
	role_ops_cgi.periodic_checks(context, 0, now);
#endif
#if defined(LWS_ROLE_DBUS)
	role_ops_dbus.periodic_checks(context, 0, now);
#endif

	__lws_sul_insert(&pt->pt_sul_owner, &pt->sul_plat, 30 * LWS_US_PER_SEC);
}
#endif

int
lws_plat_init(struct lws_context *context,
	      const struct lws_context_creation_info *info)
{
	int fd;
#if defined(LWS_WITH_NETWORK)
	/*
	 * master context has the process-global fd lookup array.  This can be
	 * done two different ways now; one or the other is done depending on if
	 * info->fd_limit_per_thread was snonzero
	 *
	 *  - default: allocate a worst-case lookup array sized for ulimit -n
	 *             and use the fd directly as an index into it
	 *
	 *  - slow:    allocate context->max_fds entries only (which can be
	 *             forced at context creation time to be
	 *             info->fd_limit_per_thread * the number of threads)
	 *             and search the array to lookup fds
	 *
	 * the default way is optimized for server, if you only use one or two
	 * client wsi the slow way may save a lot of memory.
	 *
	 * Both ways allocate an array of struct lws *... one allocates it for
	 * all possible fd indexes the process could produce and uses it as a
	 * map, the other allocates for an amount of wsi the lws context is
	 * expected to use and searches through it to manipulate it.
	 */

	context->lws_lookup = lws_zalloc(sizeof(struct lws *) *
					 context->max_fds, "lws_lookup");

	if (!context->lws_lookup) {
		lwsl_err("%s: OOM on alloc lws_lookup array for %d conn\n",
			 __func__, context->max_fds);
		return 1;
	}

	lwsl_info(" mem: platform fd map: %5lu B\n",
		    (unsigned long)(sizeof(struct lws *) * context->max_fds));
#endif
	fd = lws_open(SYSTEM_RANDOM_FILEPATH, O_RDONLY);

	context->fd_random = fd;
	if (context->fd_random < 0) {
		lwsl_err("Unable to open random device %s %d\n",
			 SYSTEM_RANDOM_FILEPATH, context->fd_random);
		return 1;
	}

#if defined(LWS_WITH_PLUGINS)
	if (info->plugin_dirs)
		lws_plat_plugins_init(context, info->plugin_dirs);
#endif


#if defined(LWS_WITH_NETWORK)
	/* we only need to do this on pt[0] */

	context->pt[0].sul_plat.cb = lws_sul_plat_unix;
	__lws_sul_insert(&context->pt[0].pt_sul_owner, &context->pt[0].sul_plat,
			 30 * LWS_US_PER_SEC);
#endif

	return 0;
}

int
lws_plat_context_early_init(void)
{
#if !defined(LWS_AVOID_SIGPIPE_IGN)
	signal(SIGPIPE, SIG_IGN);
#endif

	return 0;
}

void
lws_plat_context_early_destroy(struct lws_context *context)
{
}

void
lws_plat_context_late_destroy(struct lws_context *context)
{
#ifdef LWS_WITH_PLUGINS
	if (context->plugin_list)
		lws_plat_plugins_destroy(context);
#endif
#if defined(LWS_WITH_NETWORK)
	if (context->lws_lookup)
		lws_free_set_NULL(context->lws_lookup);
#endif
	if (!context->fd_random)
		lwsl_err("ZERO RANDOM FD\n");
	if (context->fd_random != LWS_INVALID_FILE)
		close(context->fd_random);
}
