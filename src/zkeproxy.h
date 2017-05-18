/* 
 *  zksh - ZooKeeper Shell utils.
 *
 *  Copyright (C) 2017 Marius Rieder
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef _ZKEPROXY_H
#define _ZKEPROXY_H

/**
 * \file zkeproxy.h
 * \brief ZKSH Ephemeral proxy definitions
 *
 * To make ephemeral mode working the zksh command forks to background and
 * continues to run till the calling process dies.
 * After the fork the remaining foreground process justs proxies stdin, stdout
 * and stderr until the backgrounded worker signals by SIGUSR1 that it is time
 * to sucessfully end.
 */

/**
 * \brief Initialise the ephemeral proxy configuration
 *
 * This methode takes care of the forking and proxy setup part. Only the child
 * taking care of the ZooKeeper part will return. The master process will
 * continue to run and proxy stdin, stdout and stderr and exit as soon as
 * eigther the child exists or it received a SIGUSR1.
 */
int zksh_eproxy_init();

/**
 * \brief Detaches from the proxy process
 *
 * This methode ensures the proxy process terminates sucessfully.
 */
void zksh_eproxy_detach();

/**
 * \brief Wait for the calling process to die.
 *
 * Send periodicaly a signal 0 to the calling process to verify its existance.
 */
void zksh_eproxy_wait();

#endif /* _ZKEPROXY_H */
