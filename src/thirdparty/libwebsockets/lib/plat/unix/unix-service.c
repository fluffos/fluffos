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

int
lws_poll_listen_fd(struct lws_pollfd *fd)
{
	return poll(fd, 1, 0);
}

int
_lws_plat_service_forced_tsi(struct lws_context *context, int tsi)
{
	struct lws_context_per_thread *pt = &context->pt[tsi];
	int m, n;

	lws_service_flag_pending(context, tsi);

	/* any socket with events to service? */
	for (n = 0; n < (int)pt->fds_count; n++) {
		if (!pt->fds[n].revents)
			continue;

		m = lws_service_fd_tsi(context, &pt->fds[n], tsi);
		if (m < 0) {
			lwsl_err("%s: lws_service_fd_tsi returned %d\n",
				 __func__, m);
			return -1;
		}
		/* if something closed, retry this slot */
		if (m)
			n--;
	}

	lws_service_do_ripe_rxflow(pt);

	return 0;
}

#define LWS_POLL_WAIT_LIMIT 2000000000

int
_lws_plat_service_tsi(struct lws_context *context, int timeout_ms, int tsi)
{
	volatile struct lws_foreign_thread_pollfd *ftp, *next;
	volatile struct lws_context_per_thread *vpt;
	struct lws_context_per_thread *pt;
	lws_usec_t timeout_us, us;
	int n = -1;
#if (defined(LWS_ROLE_WS) && !defined(LWS_WITHOUT_EXTENSIONS)) || defined(LWS_WITH_TLS)
	int m;
#endif

	/* stay dead once we are dead */

	if (!context || !context->vhost_list)
		return 1;

	pt = &context->pt[tsi];
	vpt = (volatile struct lws_context_per_thread *)pt;

	lws_stats_bump(pt, LWSSTATS_C_SERVICE_ENTRY, 1);

	if (timeout_ms < 0)
		timeout_ms = 0;
	else
		/* force a default timeout of 23 days */
		timeout_ms = LWS_POLL_WAIT_LIMIT;
	timeout_us = ((lws_usec_t)timeout_ms) * LWS_US_PER_MS;

	if (context->event_loop_ops->run_pt)
		context->event_loop_ops->run_pt(context, tsi);

	if (!pt->service_tid_detected) {
		struct lws _lws;

		memset(&_lws, 0, sizeof(_lws));
		_lws.context = context;

		pt->service_tid = context->vhost_list->protocols[0].callback(
					&_lws, LWS_CALLBACK_GET_THREAD_ID,
					NULL, NULL, 0);
		pt->service_tid_detected = 1;
	}

	us = lws_now_usecs();
	lws_pt_lock(pt, __func__);
	/*
	 * service ripe scheduled events, and limit wait to next expected one
	 */
	us = __lws_sul_service_ripe(&pt->pt_sul_owner, us);
	if (us && us < timeout_us)
		timeout_us = us;

	lws_pt_unlock(pt);

	/*
	 * is there anybody with pending stuff that needs service forcing?
	 */
	if (!lws_service_adjust_timeout(context, 1, tsi))
		timeout_us = 0;

	/* ensure we don't wrap at 2^31 with poll()'s signed int ms */

	timeout_us /= LWS_US_PER_MS; /* ms now */
	if (timeout_us > LWS_POLL_WAIT_LIMIT)
		timeout_us = LWS_POLL_WAIT_LIMIT;

	vpt->inside_poll = 1;
	lws_memory_barrier();
	n = poll(pt->fds, pt->fds_count, timeout_us /* ms now */ );
	vpt->inside_poll = 0;
	lws_memory_barrier();

	#if defined(LWS_WITH_DETAILED_LATENCY)
	/*
	 * so we can track how long it took before we actually read a
	 * POLLIN that was signalled when we last exited poll()
	 */
	if (context->detailed_latency_cb)
		pt->ust_left_poll = lws_now_usecs();
#endif

	/* Collision will be rare and brief.  Spin until it completes */
	while (vpt->foreign_spinlock)
		;

	/*
	 * At this point we are not inside a foreign thread pollfd
	 * change, and we have marked ourselves as outside the poll()
	 * wait.  So we are the only guys that can modify the
	 * lws_foreign_thread_pollfd list on the pt.  Drain the list
	 * and apply the changes to the affected pollfds in the correct
	 * order.
	 */

	lws_pt_lock(pt, __func__);

	ftp = vpt->foreign_pfd_list;
	//lwsl_notice("cleared list %p\n", ftp);
	while (ftp) {
		struct lws *wsi;
		struct lws_pollfd *pfd;

		next = ftp->next;
		pfd = &vpt->fds[ftp->fd_index];
		if (lws_socket_is_valid(pfd->fd)) {
			wsi = wsi_from_fd(context, pfd->fd);
			if (wsi)
				__lws_change_pollfd(wsi, ftp->_and,
						    ftp->_or);
		}
		lws_free((void *)ftp);
		ftp = next;
	}
	vpt->foreign_pfd_list = NULL;
	lws_memory_barrier();

	lws_pt_unlock(pt);

#if (defined(LWS_ROLE_WS) && !defined(LWS_WITHOUT_EXTENSIONS)) || defined(LWS_WITH_TLS)
	m = 0;
#endif
#if defined(LWS_ROLE_WS) && !defined(LWS_WITHOUT_EXTENSIONS)
	m |= !!pt->ws.rx_draining_ext_list;
#endif

#if defined(LWS_WITH_TLS)
	if (pt->context->tls_ops &&
	    pt->context->tls_ops->fake_POLLIN_for_buffered)
		m |= pt->context->tls_ops->fake_POLLIN_for_buffered(pt);
#endif

	if (
#if (defined(LWS_ROLE_WS) && !defined(LWS_WITHOUT_EXTENSIONS)) || defined(LWS_WITH_TLS)
		!m &&
#endif
		!n) { /* nothing to do */
		lws_service_do_ripe_rxflow(pt);

		return 0;
	}

	if (_lws_plat_service_forced_tsi(context, tsi))
		return -1;

	return 0;
}

int
lws_plat_check_connection_error(struct lws *wsi)
{
	return 0;
}

int
lws_plat_service(struct lws_context *context, int timeout_ms)
{
	return _lws_plat_service_tsi(context, timeout_ms, 0);
}
