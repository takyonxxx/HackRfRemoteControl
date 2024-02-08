/*
 * Copyright 2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef GNURADIO_CONFIG_H
#define GNURADIO_CONFIG_H
#ifndef TRY_SHM_VMCIRCBUF
/* #undef TRY_SHM_VMCIRCBUF */
#endif
#ifndef GR_PERFORMANCE_COUNTERS
#define GR_PERFORMANCE_COUNTERS
#endif
#ifndef GR_CTRLPORT
#define GR_CTRLPORT
#endif
#ifndef GR_RPCSERVER_ENABLED
/* #undef GR_RPCSERVER_ENABLED */
#endif
#ifndef GR_RPCSERVER_THRIFT
/* #undef GR_RPCSERVER_THRIFT */
#endif
#ifndef THRIFT_HAS_VERSION_H
/* #undef THRIFT_HAS_VERSION_H */
#endif
#ifndef THRIFT_HAS_THREADFACTORY_H
/* #undef THRIFT_HAS_THREADFACTORY_H */
#endif
#ifndef GR_MPLIB_GMP
/* #undef GR_MPLIB_GMP */
#endif
#ifndef GR_MPLIB_MPIR
#define GR_MPLIB_MPIR
#endif

#endif /* GNURADIO_CONFIG_H */
